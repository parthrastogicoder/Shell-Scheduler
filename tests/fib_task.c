#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

// CPU intensive task - calculating Fibonacci using recursive method
long long fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int n = 50; // Fibonacci number to calculate
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    printf("CPU-intensive Fibonacci task (PID: %d) started for n=%d\n", getpid(), n);
    
    // Simulate some work before main computation
    for (int i = 0; i < 3; i++) {
        printf("PID %d: Preparation step %d/3\n", getpid(), i + 1);
        usleep(200000); // 0.2 seconds
    }
    
    printf("PID %d: Starting Fibonacci calculation...\n", getpid());
    long long result = fibonacci(n);
    
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("PID %d: Fibonacci(%d) = %lld\n", getpid(), n, result);
    printf("PID %d: Completed in %.3f seconds\n", getpid(), elapsed);
    
    return 0;
}
