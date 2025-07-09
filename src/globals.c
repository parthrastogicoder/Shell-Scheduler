#include "../include/globals.h"

// Global variables definition
int NCPU;
int TSLICE;
int shmid;

sem_t scheduler_sem;
sem_t print_sem;
sem_t scheduler_queue_sem;

struct ProcessQueue* scheduler_queue;
struct ProcessQueue shell_queue;
struct TerminatedQueue* terminated_queue;
