#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h> 
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_PROCESSES 100
#define MAX_PROGRAM_NAME 256

int NCPU;
int TSLICE;
int shmid; 

sem_t scheduler_sem; // Semaphores for scheduling
sem_t print_sem;
sem_t scheduler_queue_sem; 

struct ProcessQueue* scheduler_queue;
struct ProcessQueue shell_queue;
struct TerminatedQueue* terminated_queue;

struct Process {
    pid_t pid;
    char command[MAX_PROGRAM_NAME];
    int state; // 0: Running, 1: Waiting
    struct timeval start_time;
    struct timeval end_time;   
    long long total_execution_time;
    long long waiting_time;
};

struct ProcessQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};
struct TerminatedQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};

void enqueue(struct ProcessQueue* queue, struct Process process) {
    if (queue->rear == MAX_PROCESSES - 1) {
        printf("Queue is full.\n");
        return;
    }
    queue->processes[queue->rear] = process;
}

void handleSIGUSR1(int signo) { // Signal the scheduler to wake up
    sem_post(&scheduler_sem);
}

void printTerminatedQueue(struct TerminatedQueue* queue) {

    for (int i = 0; i <= queue->rear; i++) {

        int waiting_time = ((queue->processes[i].end_time.tv_sec - queue->processes[i].start_time.tv_sec) * 1000) + ((queue->processes[i].end_time.tv_usec - queue->processes[i].start_time.tv_usec) / 1000);
        printf("Terminated Process with PID %d. Execution Time: %lld ms and %lld ms waiting time\n", queue->processes[i].pid, queue->processes[i].total_execution_time, queue->processes[i].waiting_time);
    }
}

// Signal handler for child process completion
void handleSIGCHLD(int signo) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        sem_wait(&scheduler_queue_sem); // Lock the scheduler_queue

        // Find the process in the scheduling queue
        for (int i = 0; i <= scheduler_queue->rear; i++) {
            if (scheduler_queue->processes[i].pid == pid) {
                scheduler_queue->processes[i].state = -1; // Set state to finished
                gettimeofday(&scheduler_queue->processes[i].end_time, NULL);
                struct timeval elapsedTime;
                elapsedTime.tv_sec = scheduler_queue->processes[i].end_time.tv_sec - scheduler_queue->processes[i].start_time.tv_sec;
                elapsedTime.tv_usec = scheduler_queue->processes[i].end_time.tv_usec - scheduler_queue->processes[i].start_time.tv_usec;
                long long elapsed = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
                scheduler_queue->processes[i].total_execution_time += elapsed;
                scheduler_queue->processes[i].waiting_time += elapsed;
                //printf("Process with PID %d set to -1\n", scheduler_queue->processes[i].pid);
                // Move the process to the terminated queue
                terminated_queue->rear++;
                terminated_queue->processes[terminated_queue->rear] = scheduler_queue->processes[i];
                // Remove the process from the scheduling queue by shifting elements
                for (int j = i; j < scheduler_queue->rear; j++) {
                    scheduler_queue->processes[j] = scheduler_queue->processes[j + 1];
                }
                scheduler_queue->rear--;
                break;
            }
        }
        //printf("Printing terminated queue.....\n");
        //printTerminatedQueue(terminated_queue);
        sem_post(&scheduler_queue_sem); // Unlock the scheduler_queue
    }
}

int main() {
    // Initialize semaphores
    sem_init(&scheduler_sem, 0, 0);
    sem_init(&print_sem, 0, 1);
    sem_init(&scheduler_queue_sem, 0, 1);

    // Set up the SIGCHLD signal handler
    signal(SIGCHLD, handleSIGCHLD);

    printf("Enter the number of CPUs: ");
    scanf("%d", &NCPU);
    printf("Enter the time quantum (TSLICE) in milliseconds: ");
    scanf("%d", &TSLICE);

    // Register the SIGUSR1 signal handler
    signal(SIGUSR1, (void (*)(int)) handleSIGUSR1);

    // Create shared memory for the process queue
    shmid = shmget(IPC_PRIVATE, sizeof(struct ProcessQueue), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    scheduler_queue = shmat(shmid, NULL, 0);

    if (scheduler_queue == (void*) -1) {
        perror("shmat");
        exit(1);
    }
    scheduler_queue->rear = -1;
    // Create shared memory for the terminated queue
    int terminated_shmid = shmget(IPC_PRIVATE, sizeof(struct TerminatedQueue), 0666 | IPC_CREAT);
    if (terminated_shmid < 0) {
        perror("shmget for terminated queue");
        exit(1);
    }

    terminated_queue = shmat(terminated_shmid, NULL, 0);

    if (terminated_queue == (void*) -1) {
        perror("shmat for terminated queue");
        exit(1);
    }
    terminated_queue->rear = -1;

    // Fork the SimpleScheduler process
    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0) {
        // Child process (SimpleScheduler)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = TSLICE * 1000000; // TSLICE in microseconds
        int i = 0;

        // printf("Scheduler process is starting.\n");
        while (1) {
            // printf("Scheduler process is waiting for a signal.\n");
            // sem_wait(&scheduler_sem); // Wait for a process to schedule
            // printf("Scheduler process received a signal.\n");

            if (i > scheduler_queue->rear || i >= MAX_PROCESSES) {
                i = 0;
            }

            // Check if processes are waiting
            int processesToStart = NCPU;
            // printf("Queue rear: %d\n", scheduler_queue->rear);
            // printf("processesToStart: %d\n", processesToStart);
            // printf("Starting i: %d\n", i);
            for (; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                //printf("Current i: %d\n", i);
                if (scheduler_queue->processes[i].state == 1) {
                    // Start a waiting process
                    sem_wait(&print_sem); // Wait for permission to print
                    //printf("Process with PID %d started execution.\n", scheduler_queue->processes[i].pid);
                    sem_post(&print_sem); // Allow others to print
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0; // Set state to running
                    processesToStart--;
                }
            }
            //printf("Scheduler process going for sleeping.\n");
            // Sleep for TSLICE
            usleep(TSLICE * 1000);
            //printf("Scheduler process woke up after sleeping.\n");

            // Stop running processes
            for (int j = 0; j <= scheduler_queue->rear; j++) {
                if (scheduler_queue->processes[j].state == 0) {
                    sem_wait(&print_sem); // Wait for permission to print
                    //printf("Process with PID %d stopped execution.\n", scheduler_queue->processes[j].pid);
                    sem_post(&print_sem); // Allow others to print
                    kill(scheduler_queue->processes[j].pid, SIGSTOP);
                    gettimeofday(&scheduler_queue->processes[j].end_time, NULL);
                    struct timeval elapsedTime;
                    elapsedTime.tv_sec = scheduler_queue->processes[j].end_time.tv_sec - scheduler_queue->processes[j].start_time.tv_sec;
                    elapsedTime.tv_usec = scheduler_queue->processes[j].end_time.tv_usec - scheduler_queue->processes[j].start_time.tv_usec;
                    long long elapsed = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
                    scheduler_queue->processes[j].total_execution_time += elapsed;
                    scheduler_queue->processes[j].waiting_time += (scheduler_queue->rear -1)* TSLICE;
                    scheduler_queue->processes[j].state = 1; // Set state to waiting
                }
            }
        }

    } else {
        // Parent process (SimpleShell)
       
        shell_queue.rear = -1;

        while (1) {
            char command[MAX_PROGRAM_NAME];
            printf("\nSimpleShell$ ");
            scanf("%s", command);

            if (strcmp(command, "exit") == 0) {
                // Exit the shell
                break;
            } else if (strcmp(command, "submit") == 0) {
                // User submits a program
                char program[MAX_PROGRAM_NAME];
                scanf("%s", program);

                pid_t child_pid = fork();
                if (child_pid == 0) {
                    // Child process
                    usleep(TSLICE * 1000);
                    execlp(program, program, NULL);
                    perror("Execution failed");
                    exit(1);
                } else {
                    // Parent process (shell)
                    struct Process new_process;
                    new_process.pid = child_pid;
                    strcpy(new_process.command, program);
                    new_process.state = 1; // Set state to waiting
                    new_process.total_execution_time = 0; // Set initial execution time = 0
                    new_process.waiting_time = TSLICE; 
                    scheduler_queue->rear++;
                    enqueue(scheduler_queue, new_process);

                    // Send SIGUSR1 to the scheduler to wake it up
                    kill(scheduler_pid, SIGUSR1);
                }
            } else {

                system(command);
            }
        }

        // Wait for child processes to complete
        while (shell_queue.rear >= 0) {
            int status;
            pid_t pid = wait(&status);
            if (pid == -1) {
                /// No more child processes to wait for
                break;
            }
            for (int i = 0; i <= shell_queue.rear; i++) {
                if (shell_queue.processes[i].pid == pid) {
                    sem_wait(&print_sem); // Wait for permission to print
                    printf("Process with PID %d finished execution. Execution Time: %lld ms\n", pid, shell_queue.processes[i].total_execution_time);
                    sem_post(&print_sem); // Allow others to print
                    shell_queue.processes[i].state = -1; // Set state to finished
                    gettimeofday(&shell_queue.processes[i].end_time, NULL);
                    struct timeval elapsedTime;
                    elapsedTime.tv_sec = shell_queue.processes[i].end_time.tv_sec - shell_queue.processes[i].start_time.tv_sec;
                    elapsedTime.tv_usec = shell_queue.processes[i].end_time.tv_usec - shell_queue.processes[i].start_time.tv_usec;
                    long long elapsed = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
                    shell_queue.processes[i].total_execution_time += elapsed;
                   
                }
            }
        }
        printTerminatedQueue(terminated_queue);
                    exit(0);
    }
    printTerminatedQueue(terminated_queue);
    // Clean up shared memory
    shmdt(scheduler_queue);
    shmctl(shmid, IPC_RMID, NULL);

    // Destroy semaphores
    sem_destroy(&scheduler_sem);
    sem_destroy(&print_sem);
    sem_destroy(&scheduler_queue_sem);
    exit(0);
    return 0;
}
