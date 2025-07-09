#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

// Recursive Fibonacci function (inefficient for demonstration)
long long fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// More efficient iterative Fibonacci
long long fibonacci_iterative(int n) {
    if (n <= 1) return n;
    
    long long a = 0, b = 1, result = 0;
    for (int i = 2; i <= n; i++) {
        result = a + b;
        a = b;
        b = result;
    }
    return result;
}

int main(int argc, char *argv[]) {
    int n = 50; // Default to 40 for reasonable time
    int use_iterative = 0;
    
    // Parse command line arguments
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    if (argc > 2 && strcmp(argv[2], "-i") == 0) {
        use_iterative = 1;
    }
    
    // Limit n to prevent extremely long execution
    if (n > 50) {
        printf("Warning: n > 50 may take very long time. Using n = 50.\n");
        n = 50;
    }
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    printf("Fibonacci Process (PID: %d) started for n=%d\n", getpid(), n);
    printf("Using %s algorithm\n", use_iterative ? "iterative" : "recursive");
    
    long long result;
    if (use_iterative) {
        result = fibonacci_iterative(n);
    } else {
        result = fibonacci(n);
    }
    
    gettimeofday(&end, NULL);
    
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("Fibonacci(%d) = %lld\n", n, result);
    printf("Process PID: %d completed in %.3f seconds\n", getpid(), elapsed);
    
    return 0;
}
