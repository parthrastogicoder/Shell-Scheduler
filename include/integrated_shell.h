#ifndef INTEGRATED_SHELL_H
#define INTEGRATED_SHELL_H

#include "shell.h"
#include "scheduler.h"

// Integration modes
typedef enum {
    SHELL_ONLY,
    BASIC_SCHEDULER,
    ADVANCED_SCHEDULER
} OperationMode;

// Function declarations
void display_menu(void);
OperationMode get_operation_mode(void);
int run_integrated_shell(OperationMode mode);
void show_help(void);

#endif // INTEGRATED_SHELL_H
