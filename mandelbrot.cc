#include "mandelbrot.h"

#include <pthread.h>

// Mandelbrot convergence map
unsigned mandelbrot[resolution * resolution];
// Lock and cond variables for thread_exit() and thread_join()
pthread_mutex_t lock;
pthread_cond_t cond;

// This must be called at the end of thread function
void thread_exit(void) {


    
}

// This is called by the main thread.
void thread_join(void) {



}

// Thread function to calculate a part of Mandelbrot set.
void* thread_mandelbrot(void* arg) {




    thread_exit();  // Wake up a thread waiting on the condition variable.
    return 0;
}

// Calculate the Mandelbrot convergence map.
void calc_mandelbrot(void) {




    thread_join();  // Check the condition variable.
}
