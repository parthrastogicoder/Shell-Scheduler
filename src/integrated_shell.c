#include "../include/integrated_shell.h"

// Display menu for operation mode selection
void display_menu(void) {
    printf("\n=== Shell-Scheduler Integration Menu ===\n");
    printf("1. Simple Shell Only\n");
    printf("2. Basic Scheduler (Round Robin)\n");
    printf("3. Advanced Scheduler (Priority-based)\n");
    printf("4. Help\n");
    printf("5. Exit\n");
    printf("==========================================\n");
    printf("Enter your choice (1-5): ");
}

// Get operation mode from user
OperationMode get_operation_mode(void) {
    int choice;
    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch (choice) {
            case 1:
                return SHELL_ONLY;
            case 2:
                return BASIC_SCHEDULER;
            case 3:
                return ADVANCED_SCHEDULER;
            case 4:
                show_help();
                break;
            case 5:
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please select 1-5.\n");
        }
    }
}

// Show help information
void show_help(void) {
    printf("\n=== Shell-Scheduler Help ===\n");
    printf("This integrated system provides three operation modes:\n\n");
    
    printf("1. SIMPLE SHELL ONLY:\n");
    printf("   - Basic command-line interface\n");
    printf("   - Supports command execution, pipes, and background processes\n");
    printf("   - Commands: history, exit, any system command\n");
    printf("   - Use '&' for background execution\n");
    printf("   - Use '|' for piped commands\n\n");
    
    printf("2. BASIC SCHEDULER (Round Robin):\n");
    printf("   - Shell with process scheduling capability\n");
    printf("   - Round-robin scheduling algorithm\n");
    printf("   - Multiple CPU support\n");
    printf("   - Commands: submit <program>, exit, any system command\n");
    printf("   - Example: submit ./my_program\n\n");
    
    printf("3. ADVANCED SCHEDULER (Priority-based):\n");
    printf("   - Shell with priority-based scheduling\n");
    printf("   - Priority levels: 1 (highest) to 4 (lowest)\n");
    printf("   - Commands: submit <program> [priority], exit, any system command\n");
    printf("   - Example: submit ./my_program 2\n");
    printf("   - Default priority is 1 if not specified\n\n");
    
    printf("General Commands:\n");
    printf("   - exit: Exit the current mode\n");
    printf("   - history: Show command history (Shell mode only)\n");
    printf("   - Ctrl+C: Exit with history display\n");
    printf("=============================\n");
}

// Run the integrated shell based on selected mode
int run_integrated_shell(OperationMode mode) {
    printf("\n=== Starting ");
    switch (mode) {
        case SHELL_ONLY:
            printf("Simple Shell");
            break;
        case BASIC_SCHEDULER:
            printf("Basic Scheduler (Round Robin)");
            break;
        case ADVANCED_SCHEDULER:
            printf("Advanced Scheduler (Priority-based)");
            break;
    }
    printf(" ===\n\n");
    
    switch (mode) {
        case SHELL_ONLY:
            return run_shell();
        case BASIC_SCHEDULER:
            return run_basic_scheduler();
        case ADVANCED_SCHEDULER:
            return run_advanced_scheduler();
    }
    
    return 0;
}

// Main function for integrated shell
int main(void) {
    printf("Welcome to the Integrated Shell-Scheduler System!\n");
    printf("Developed by Aryan Jain & Parth Rastogi - Group 8 Sec A\n");
    
    while (1) {
        OperationMode mode = get_operation_mode();
        int result = run_integrated_shell(mode);
        
        if (result != 0) {
            printf("Error occurred in selected mode. Returning to menu...\n");
        }
        
        printf("\nReturning to main menu...\n");
    }
    
    return 0;
}
