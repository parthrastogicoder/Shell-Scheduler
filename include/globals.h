#ifndef GLOBALS_H
#define GLOBALS_H

#include "scheduler.h"

// Global variables declaration
extern int NCPU;
extern int TSLICE;
extern int shmid;

extern sem_t scheduler_sem;
extern sem_t print_sem;
extern sem_t scheduler_queue_sem;

extern struct ProcessQueue* scheduler_queue;
extern struct ProcessQueue shell_queue;
extern struct TerminatedQueue* terminated_queue;

#endif // GLOBALS_H
