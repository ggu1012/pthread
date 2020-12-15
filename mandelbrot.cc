#include "mandelbrot.h"

#include <assert.h>
#include <pthread.h>

#include <cstring>
#include <iostream>

using namespace std;

// Mandelbrot convergence map
unsigned mandelbrot[resolution * resolution];
// Lock and cond variables for thread_exit() and thread_join()
pthread_mutex_t lock;
pthread_cond_t cond;

// used for indicating running thread.
// rthread = 0 -> all threads ended their jobs.
uint rthread;

// global pointer for sharing num array.
uint* num_ptr;

// This must be called at the end of thread function
void thread_exit(void) {
    pthread_mutex_lock(&lock);
    // one thread has completed its job.
    rthread--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
}

// This is called by the main thread.
void thread_join(void) {
    pthread_mutex_lock(&lock);
    // While loop until every thread has done their own job.
    while (rthread != 0)
        pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);
}

// Thread function to calculate a part of Mandelbrot set.
void* thread_mandelbrot(void* arg) {
    // thread_num gets the value i from calc_mandelbrot,
    // which indicates the thread number.
    uint thread_num = *((uint*)arg);
    uint calc_amount = num_ptr[thread_num];

    int start_point = 0;
    // find the start point for this thread by SUM(num[thread_before]) + 1
    for (uint i = 0; i < thread_num; i++)
        start_point += num_ptr[i];
    start_point++;
    // find the end point for this thread
    int end_point = start_point + calc_amount - 1;

    // for indicating pixel position
    uint W, H;
    float r, x;

    pthread_mutex_lock(&lock);
    cout << "thread num : " << thread_num << endl;
    pthread_mutex_unlock(&lock);

    // iteration for calculating ma1ndelbrot set for [start_point, end_point]
    for (int now = start_point; now <= end_point; now++) {
        // Convert the 1D element to (W, H), and to (r, x)
        // H = now / 768 + 1, W = now % 768
        H = now / 768 + 1;
        W = now % 768;
        r = minW + 3.2 * W / 768;
        x = minH + 3.1 * H / 768;

        // used for counting iteration.
        // later, mandelbrot[now] = n;
        int n = 0;

        // Two complex numbers for Z0 and Zn.
        Complex Z0 = Complex(r, x);
        Complex Zn = Complex(r, x);

        // r + jx is the position for mandelbrot iteration.
        while (n <= max_iterations) {
            // Z(n+1) = Z(n)^2 + Z(0)
            Zn = Zn.operator*(Zn);
            Zn = Zn.operator+(Z0);

            // |Zn| > 2.0, out of bound
            if (Zn.magnitude2() > 2.0)
                break;
            n++;
        }

        // save the iteration number
        mandelbrot[now] = n;
    }

    thread_exit();  // Wake up a thread waiting on the condition variable.
    return 0;
}

// Calculate the Mandelbrot convergence map.
void calc_mandelbrot(void) {
    rthread = num_threads;
    int total_complex = resolution * resolution;
    int norm = total_complex / num_threads;

    /*
     *  Algorithm for distributing complex numbers to each thread                       
     * 
     *  1. Set threads[0] ~ threads[num_thread - 2]
     *      to handle norm = (total_complex % num_threads) complex numbers. 
     * 
     *  2. If threads[num_thread - 1] turned out to handle 
     *     more than (norm + 2) complex numbers, redistribute.
     *     Tail of the array contains (norm + N) complex numbers.
     *     Add 1 to the number of complex numbers for 
     *     threads[num_thread - N] ~ threads[num_thread - 2].
     * 
     *  3. So, in total,
     *      0      1           num_thread - N                    num_thread - 1
     *    -----------------------------------------------------------------------    
     *   | norm | norm | ... |    norm + 1    |  norm + 1 | ... |    norm + 1    |
     *    ----------------------------------------------------------------------- 
     *      
     *    sum = norm * (num_thread - N) + (norm + 1) * N 
     *        = norm * num_thread + N
     *        = resolution * resolution
     */

    // Number of complex number that would be contained in the array tail
    // without redistribution.
    int tail_num = total_complex - norm * (num_threads - 1);

    // num array contains how much complex numbers would be held for certain thread.
    // total sum of num[] = 768 * 768
    // num[num_threads+1] is used for indicating the thread number(T0, T1, ...).
    uint* num = new uint[num_threads];

    // Fill the num array with norm.
    for (uint i = 0; i < num_threads; i++) {
        num[i] = norm;
    }

    // Redistribute.
    if (tail_num >= norm + 2) {
        // Start of thread array that would be redistributed.
        int mod_threads = num_threads - (tail_num - norm);
        // num[] = norm + 1 for num_thread - N
        for (uint i = mod_threads; i < num_threads; i++) {
            num[i] = norm + 1;
        }
    }

    // pass num array as num_ptr
    num_ptr = &num[0];

    // create threads with number of num_threads.
    pthread_t* threads = new pthread_t[num_threads];
    int* tmp = new int[num_threads];

    // pass the value as pointer to avoid race.
    for (uint i = 0; i < num_threads; i++) {
        tmp[i] = i;
        assert(!pthread_create(&threads[i], NULL, &thread_mandelbrot, &tmp[i]));        
    }  
    thread_join();  // Check the condition variable.

    // free allocated regions
    delete tmp;
    delete threads;
    delete num;

    return;
}
