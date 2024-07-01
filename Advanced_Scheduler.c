#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h> // Include semaphore library
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define MAX_PROCESSES 100
#define MAX_PROGRAM_NAME 256

// Define some global variables
int NCPU;
int TSLICE;
int shmid; // Shared memory ID

sem_t scheduler_sem; // Semaphore for scheduling
sem_t print_sem;
sem_t scheduler_queue_sem; // Semaphore for printing

struct ProcessQueue* scheduler_queue;
struct ProcessQueue shell_queue;
struct TerminatedQueue* terminated_queue;

// Struct to represent a process
struct Process {
    pid_t pid;
    char command[MAX_PROGRAM_NAME];
    int state; // 0: Running, 1: Waiting, -1: Finished
    struct timeval start_time; // Track start time
    struct timeval end_time;   // Track end time
    long long total_execution_time;
    long long waiting_time;
    int priority;
};

// Struct to represent a queue of processes
struct ProcessQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};


// Struct for terminated processes
struct TerminatedQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};

// Function to enqueue a process in the queue
void enqueue(struct ProcessQueue* queue, struct Process process) {
    if (queue->rear == MAX_PROCESSES - 1) {
        printf("Queue is full.\n");
        return;
    }
    queue->processes[queue->rear] = process;
}

// Signal handler for SIGUSR1 - Used to wake up the scheduler
void handleSIGUSR1(int signo) { // Signal the scheduler to wake up
    sem_post(&scheduler_sem);
}

// Function to print the terminated process queue
void printTerminatedQueue(struct TerminatedQueue* queue) {

    for (int i = 0; i <= queue->rear; i++) {

        int waiting_time = ((queue->processes[i].end_time.tv_sec - queue->processes[i].start_time.tv_sec) * 1000) + ((queue->processes[i].end_time.tv_usec - queue->processes[i].start_time.tv_usec) / 1000);
        printf("Terminated Process with PID %d. Execution Time: %lld ms and %lld ms waiting time\n", queue->processes[i].pid, queue->processes[i].total_execution_time, queue->processes[i].waiting_time);
    }
}

// Signal handler for child process completion SIGCHILD
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
    if (sem_init(&scheduler_sem, 0, 0) == -1) {
        perror("sem_init (scheduler_sem)");
        exit(1);
    }
    if (sem_init(&print_sem, 0, 1) == -1) {
        perror("sem_init (print_sem)");
        exit(1);
    }
    if (sem_init(&scheduler_queue_sem, 0, 1) == -1) {
        perror("sem_init (scheduler_queue_sem)");
        exit(1);
    }

    // Set up the SIGCHLD signal handler
    if (signal(SIGCHLD, handleSIGCHLD) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    printf("Enter the number of CPUs: ");
    scanf("%d", &NCPU);
    printf("Enter the time quantum (TSLICE) in milliseconds: ");
    scanf("%d", &TSLICE);

    // Register the SIGUSR1 signal handler
    if (signal(SIGUSR1, (void (*)(int)) handleSIGUSR1) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

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
    if (scheduler_pid == -1) {
        perror("fork");
        exit(1);
    }
    if (scheduler_pid == 0) {
        // Child process (SimpleScheduler)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = TSLICE * 1000000; // TSLICE in microseconds
        int k = 0;

        // printf("Scheduler process is starting.\n
        while (1) {
            // printf("Scheduler process is waiting for a signal.\n");
            // sem_wait(&scheduler_sem); // Wait for a process to schedule
            // printf("Scheduler process received a signal.\n");

            if (k > scheduler_queue->rear || k >= MAX_PROCESSES) {
                k = 0;
            }
            k = 0;
            // Check if processes are waiting
            int processesToStart = NCPU;
            int flag = 0;
            // printf("Queue rear: %d\n", scheduler_queue->rear);
            // printf("processesToStart: %d\n", processesToStart);
            // printf("Starting i: %d\n", i);
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                //printf("Current i: %d\n", i);
                if (scheduler_queue->processes[i].state == 1  && scheduler_queue->processes[i].priority == 1 && (flag == 0 || flag == 1)) {
                    // Start a waiting process
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    //printf("Process with PID %d started execution with priority 1.\n", scheduler_queue->processes[i].pid);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0; // Set state to running
                    processesToStart--;
                    flag = 1;
                    k = i;
                }
            }
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1  && scheduler_queue->processes[i].priority == 2 && (flag == 0 || flag == 2)) {
                    // Start a waiting process
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    //printf("Process with PID %d started execution with priority 2.\n", scheduler_queue->processes[i].pid);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0; // Set state to running
                    processesToStart--;
                    flag = 2;
                    k = i;
                }
            }
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1  && scheduler_queue->processes[i].priority == 3 && (flag == 0 || flag == 3)) {
                    // Start a waiting process
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    //printf("Process with PID %d started execution with priority 3.\n", scheduler_queue->processes[i].pid);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0; // Set state to running
                    processesToStart--;
                    flag = 3;
                    k = i;
                }
            }
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1  && scheduler_queue->processes[i].priority == 4 && (flag == 0 || flag == 4)) {
                    // Start a waiting process
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    //printf("Process with PID %d started execution with priority 4.\n", scheduler_queue->processes[i].pid);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0; // Set state to running
                    processesToStart--;
                    flag = 4;
                    k = i;
                }
            }
            //printf("Scheduler process going for sleeping.\n");
            // Sleep for TSLICE
            int t = TSLICE;
            if (flag != 0){
                t = TSLICE/flag;
            }
            usleep(t * 1000);
            //printf("Scheduler process woke up after sleeping, flag = %d, tslice = %d.\n", flag, t);

            // Stop running processes
            for (int j = 0; j <= scheduler_queue->rear; j++) {
                if (scheduler_queue->processes[j].state == 0) {
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    //printf("Process with PID %d stopped execution.\n", scheduler_queue->processes[j].pid);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    if (kill(scheduler_queue->processes[j].pid, SIGSTOP) == -1) {
                        perror("kill (SIGSTOP)");
                    }
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
                int prior = 1; // Default priority is 1
                // Try to read the program name
                scanf("%s", program);
            
                // Try to read the priority if available
                if (scanf("%d", &prior) != 1) {
                    // Priority was not entered, so it remains as 1
                    prior = 1;
                }
                if (prior < 1 || prior > 4) {
                    printf("Invalid priority. Priority must be between 1 and 4.\n");
                    prior = 1;
                    continue;
                }

                // Now you can use 'program' and 'prior' as needed.
                //printf("Program: %s, Priority: %d\n", program, prior);

                pid_t child_pid = fork();
                if (child_pid == -1) {
                    perror("fork");
                    continue;
                }
                if (child_pid == 0) {
                    // Child process
                    usleep(TSLICE * 1000);
                    execlp(program, program, NULL);
                    kill(program,SIGSTOP);
                    perror("Execution failed");
                    exit(1);
                } else {
                    // Parent process (shell)
                    struct Process new_process;
                    new_process.pid = child_pid;
                    strcpy(new_process.command, program);
                    new_process.priority=prior;
                    new_process.state = 1; // Set state to waiting
                    new_process.total_execution_time = 0; // Set initial execution time = 0
                    new_process.waiting_time = TSLICE; // Initialize waiting time to 0
                    if (scheduler_queue->rear < MAX_PROCESSES - 1) {
                        scheduler_queue->rear++;
                        enqueue(scheduler_queue, new_process);
                        kill(new_process.pid,SIGSTOP);

                        // Send SIGUSR1 to the scheduler to wake it up
                        if (kill(scheduler_pid, SIGUSR1) == -1) {
                            perror("kill (SIGUSR1)");
                        }
                    } else {
                        printf("Scheduler queue is full. Cannot submit more processes.\n");
                    }
                }
            } else {
                // Execute other commands or system commands
                if (system(command) == -1) {
                    perror("system");
                }
            }
        }

        // Wait for child processes to complete
        while (shell_queue.rear >= 0) {
            int status;
            pid_t pid = wait(&status);
            if (pid == -1) {
                // No more child processes to wait for
                break;
            }
            for (int i = 0; i <= shell_queue.rear; i++) {
                if (shell_queue.processes[i].pid == pid) {
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    printf("Process with PID %d finished execution. Execution Time: %lld ms\n", pid, shell_queue.processes[i].total_execution_time);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
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
    if (shmdt(scheduler_queue) == -1) {
        perror("shmdt (scheduler_queue)");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl (scheduler_queue)");
    }

    // Destroy semaphores
    if (sem_destroy(&scheduler_sem) == -1) {
        perror("sem_destroy (scheduler_sem)");
    }
    if (sem_destroy(&print_sem) == -1) {
        perror("sem_destroy (print_sem)");
    }
    if (sem_destroy(&scheduler_queue_sem) == -1) {
        perror("sem_destroy (scheduler_queue_sem)");
    }
    exit(0);
    return 0;
}
// Aryan Jain & Parth Rastogi - Group 8 Sec A