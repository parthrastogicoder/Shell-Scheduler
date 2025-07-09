#include "../include/scheduler.h"
#include "../include/globals.h"

// Function to enqueue a process in the queue
void enqueue(struct ProcessQueue* queue, struct Process process) {
    if (queue->rear == MAX_PROCESSES - 1) {
        printf("Queue is full.\n");
        return;
    }
    queue->processes[queue->rear] = process;
}

// Signal handler for SIGUSR1 - Used to wake up the scheduler
void handleSIGUSR1(int signo) {
    (void)signo; // Suppress unused parameter warning
    sem_post(&scheduler_sem);
}

// Function to print the terminated process queue
void printTerminatedQueue(struct TerminatedQueue* queue) {
    for (int i = 0; i <= queue->rear; i++) {
        printf("Terminated Process with PID %d. Execution Time: %lld ms and %lld ms waiting time\n", 
               queue->processes[i].pid, queue->processes[i].total_execution_time, queue->processes[i].waiting_time);
    }
}

// Signal handler for child process completion
void handleSIGCHLD(int signo) {
    (void)signo; // Suppress unused parameter warning
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        sem_wait(&scheduler_queue_sem);

        // Find the process in the scheduling queue
        for (int i = 0; i <= scheduler_queue->rear; i++) {
            if (scheduler_queue->processes[i].pid == pid) {
                scheduler_queue->processes[i].state = -1;
                gettimeofday(&scheduler_queue->processes[i].end_time, NULL);
                struct timeval elapsedTime;
                elapsedTime.tv_sec = scheduler_queue->processes[i].end_time.tv_sec - scheduler_queue->processes[i].start_time.tv_sec;
                elapsedTime.tv_usec = scheduler_queue->processes[i].end_time.tv_usec - scheduler_queue->processes[i].start_time.tv_usec;
                long long elapsed = elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
                scheduler_queue->processes[i].total_execution_time += elapsed;
                scheduler_queue->processes[i].waiting_time += elapsed;
                
                // Move the process to the terminated queue
                terminated_queue->rear++;
                terminated_queue->processes[terminated_queue->rear] = scheduler_queue->processes[i];
                
                // Remove the process from the scheduling queue
                for (int j = i; j < scheduler_queue->rear; j++) {
                    scheduler_queue->processes[j] = scheduler_queue->processes[j + 1];
                }
                scheduler_queue->rear--;
                break;
            }
        }
        sem_post(&scheduler_queue_sem);
    }
}
