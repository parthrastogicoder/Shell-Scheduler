#include "../include/scheduler.h"
#include "../include/globals.h"

// Add missing includes
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

// Advanced scheduler implementation with priority support
int run_advanced_scheduler(void) {
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

    // Fork the scheduler process
    pid_t scheduler_pid = fork();
    if (scheduler_pid == -1) {
        perror("fork");
        exit(1);
    }
    if (scheduler_pid == 0) {
        // Child process (Advanced Scheduler with Priority)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = TSLICE * 1000000;
        int k = 0;

        while (1) {
            if (k > scheduler_queue->rear || k >= MAX_PROCESSES) {
                k = 0;
            }
            k = 0;

            // Check if processes are waiting
            int processesToStart = NCPU;
            int flag = 0;

            // Priority 1 (Highest)
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1 && 
                    scheduler_queue->processes[i].priority == 1 && 
                    (flag == 0 || flag == 1)) {
                    
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0;
                    processesToStart--;
                    flag = 1;
                    k = i;
                }
            }

            // Priority 2
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1 && 
                    scheduler_queue->processes[i].priority == 2 && 
                    (flag == 0 || flag == 2)) {
                    
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0;
                    processesToStart--;
                    flag = 2;
                    k = i;
                }
            }

            // Priority 3
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1 && 
                    scheduler_queue->processes[i].priority == 3 && 
                    (flag == 0 || flag == 3)) {
                    
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0;
                    processesToStart--;
                    flag = 3;
                    k = i;
                }
            }

            // Priority 4 (Lowest)
            for (int i = k; i <= scheduler_queue->rear && processesToStart > 0; i++) {
                if (scheduler_queue->processes[i].state == 1 && 
                    scheduler_queue->processes[i].priority == 4 && 
                    (flag == 0 || flag == 4)) {
                    
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
                    gettimeofday(&scheduler_queue->processes[i].start_time, NULL);
                    kill(scheduler_queue->processes[i].pid, SIGCONT);
                    scheduler_queue->processes[i].state = 0;
                    processesToStart--;
                    flag = 4;
                    k = i;
                }
            }

            // Sleep for TSLICE (adjusted by priority)
            int t = TSLICE;
            if (flag != 0) {
                t = TSLICE / flag;
            }
            usleep(t * 1000);

            // Stop running processes
            for (int j = 0; j <= scheduler_queue->rear; j++) {
                if (scheduler_queue->processes[j].state == 0) {
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
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
                int prior = 1; // Default priority is 1
                
                // Try to read the program name
                scanf("%s", program);
                
                // Try to read the priority if available
                if (scanf("%d", &prior) != 1) {
                    prior = 1;
                }
                if (prior < 1 || prior > 4) {
                    printf("Invalid priority. Priority must be between 1 and 4.\n");
                    prior = 1;
                    continue;
                }

                pid_t child_pid = fork();
                if (child_pid == -1) {
                    perror("fork");
                    continue;
                }
                if (child_pid == 0) {
                    // Child process
                    usleep(TSLICE * 1000);
                    execlp(program, program, NULL);
                    kill(getpid(), SIGSTOP);
                    perror("Execution failed");
                    exit(1);
                } else {
                    // Parent process (shell)
                    struct Process new_process;
                    new_process.pid = child_pid;
                    strcpy(new_process.command, program);
                    new_process.priority = prior;
                    new_process.state = 1;
                    new_process.total_execution_time = 0;
                    new_process.waiting_time = TSLICE;
                    
                    if (scheduler_queue->rear < MAX_PROCESSES - 1) {
                        scheduler_queue->rear++;
                        enqueue(scheduler_queue, new_process);
                        kill(new_process.pid, SIGSTOP);

                        // Send SIGUSR1 to the scheduler
                        if (kill(scheduler_pid, SIGUSR1) == -1) {
                            perror("kill (SIGUSR1)");
                        }
                    } else {
                        printf("Scheduler queue is full. Cannot submit more processes.\n");
                    }
                }
            } else {
                // Execute other commands
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
                break;
            }
            for (int i = 0; i <= shell_queue.rear; i++) {
                if (shell_queue.processes[i].pid == pid) {
                    if (sem_wait(&print_sem) == -1) {
                        perror("sem_wait (print_sem)");
                    }
                    printf("Process with PID %d finished execution. Execution Time: %lld ms\n", 
                           pid, shell_queue.processes[i].total_execution_time);
                    if (sem_post(&print_sem) == -1) {
                        perror("sem_post (print_sem)");
                    }
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
    
    return 0;
}

// Advanced scheduler standalone main
#ifndef INTEGRATED_BUILD
int main(void) {
    return run_advanced_scheduler();
}
#endif
