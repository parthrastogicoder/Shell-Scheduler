#include "../include/scheduler.h"
#include "../include/globals.h"

// Add missing includes
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

// Basic scheduler implementation
int run_basic_scheduler(void) {
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

    // Fork the scheduler process
    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0) {
        // Child process (Scheduler)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = TSLICE * 1000000;
        int i = 0;

        while (1) {
            if (i > scheduler_queue->rear || i >= MAX_PROCESSES) {
                i = 0;
            }

            // Check if processes are waiting
            int processesToStart = NCPU;
            for (; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1) {
                    // Start a waiting process
                    sem_wait(&print_sem);
                    sem_post(&print_sem);
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0;
                    processesToStart--;
                }
            }

            // Sleep for TSLICE
            usleep(TSLICE * 1000);

            // Stop running processes
            for (int j = 0; j <= scheduler_queue->rear; j++) {
                if (scheduler_queue->processes[j].state == 0) {
                    sem_wait(&print_sem);
                    sem_post(&print_sem);
                    kill(scheduler_queue->processes[j].pid, SIGSTOP);
                    gettimeofday(&scheduler_queue->processes[j].end_time, NULL);
                    struct timeval elapsedTime;
                    elapsedTime.tv_sec = scheduler_queue->processes[j].end_time.tv_sec - scheduler_queue->processes[j].start_time.tv_sec;
                    elapsedTime.tv_usec = scheduler_queue->processes[j].end_time.tv_usec - scheduler_queue->processes[j].start_time.tv_usec;
                    long long elapsed = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
                    scheduler_queue->processes[j].total_execution_time += elapsed;
                    scheduler_queue->processes[j].waiting_time += (scheduler_queue->rear - 1) * TSLICE;
                    scheduler_queue->processes[j].state = 1;
                }
            }
        }
    } else {
        // Parent process (Shell)
        shell_queue.rear = -1;

        while (1) {
            char command[MAX_PROGRAM_NAME];
            printf("\nSimpleShell$ ");
            scanf("%s", command);

            if (strcmp(command, "exit") == 0) {
                break;
            } else if (strcmp(command, "submit") == 0) {
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
                    new_process.state = 1;
                    new_process.total_execution_time = 0;
                    new_process.waiting_time = TSLICE;
                    scheduler_queue->rear++;
                    enqueue(scheduler_queue, new_process);

                    // Send SIGUSR1 to the scheduler
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
                break;
            }
            for (int i = 0; i <= shell_queue.rear; i++) {
                if (shell_queue.processes[i].pid == pid) {
                    sem_wait(&print_sem);
                    printf("Process with PID %d finished execution. Execution Time: %lld ms\n", 
                           pid, shell_queue.processes[i].total_execution_time);
                    sem_post(&print_sem);
                    shell_queue.processes[i].state = -1;
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
    
    return 0;
}

// Basic scheduler standalone main
#ifndef INTEGRATED_BUILD
int main(void) {
    return run_basic_scheduler();
}
#endif
