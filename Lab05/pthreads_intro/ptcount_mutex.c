#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS  3

// Represents thread arguments
//
// tid represents the unique thread id assigned by pthread
// inc represents the amount the thread will increment by when calling the
// inc_count method
// loop represents the number of times to call inc_count
struct thread_args {
	int tid;
	int inc;
	int loop;
};

// Global count variable shared by all threads
int count = 0;

// Mutex to protect critical sections
pthread_mutex_t count_mutex;

/*
 * This routine will be executed by each thread we choose to create.
 * The routine a new thread will execute is given as an arguent to the
 * pthread_create() call.
 */
void *inc_count(void *arg)
{
// loc tracks the amount incremented locally.  Since it is stored on this
// thread's stack, it will behave properly regardless of how protection is
// implemented.  In contrast, the count variable is global, and we could
// easily corrupt this data if it is not accessed properly
	int i,loc;
	struct thread_args *my_args = (struct thread_args*) arg;

	loc = 0;
	for (i = 0; i < my_args->loop; i++) {
		/*
		 * How many machine instructions are required to increment count
		 * and loc. Where are these variables stored? What implications
		 * does their repsective locations have for critical section
		 * existence and the need for Critical section protection?
		 */
	
		// Each increment will use three instructions: a load, an add, and a
		// store.  Count needs to be protected, because it is possible that the
		// scheduler could interleave instructions from different threads and
		// corrupt the value.  For example, if thread 1 executes load, then is
		// pre-empted, and thread 2 executes load, then both are operating on the
		// same initial value.  After both threads have completed, the two counts
		// appear as one, and the final result is less than it should be.
		// 
		// Protecting by a mutex is a relatively slow means of
		// protecting the critical section.

		// Begin critical section
		
		pthread_mutex_lock(&count_mutex);
		count = count + my_args->inc;
		pthread_mutex_unlock(&count_mutex);

		// End critical section
		loc = loc + my_args->inc;
	}
	printf("Thread: %d finished. Counted: %d\n", my_args->tid, loc);
	free(my_args);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){

	int i, loop, inc;
	struct thread_args *targs;
	pthread_t threads[NUM_THREADS];

	// Special attributes.  Stored in a struct with ints, pointers, and a
	// parameters struct that are used internally to manage the thread
	// behavior.
	pthread_attr_t attr;

	if (argc != 3) {
		printf("Usage: PTCOUNT LOOP_BOUND INCREMENT\n");
		exit(0);
	}

	/*
	 * First argument is how many times to loop. The second is how much
	 * to increment each time.
	 */
	loop = atoi(argv[1]);
	inc = atoi(argv[2]);

	/* Initialize mutex */
	pthread_mutex_init(&count_mutex, NULL);

	/* For portability, explicitly create threads in a joinable state 
	 *
	 * per the documentation, this is a simple setter which designates
	 * that the thread will allow itself to be joined (and in turn
	 * terminated)
	 *
	 * */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* Create each thread using pthread_create.  The start routine for
	 * each thread should be inc_count. The attribute object should be
	 * attr. You should pass as the thread's sole argument the populated
	 * targs struct. Note we create a different copy of it for each
	 * thread.
	 */
	for (i = 0; i < NUM_THREADS; i++) {
		targs = malloc(sizeof(targs));
		targs->tid = i;
		targs->loop = loop;
		targs->inc = inc;

		// Generate a thread.  Store the thread in threads[i], use the
		// attributes we specified above, pass the function pointer
		// for the incrementing function (functions pass by reference
		// by default like arrays in c), and pass our arguments struct
		pthread_create(&threads[i], &attr, inc_count, (void*)targs);
	}
	
	/* Wait for all threads to complete using pthread_join.  The threads
	 * do not return anything on exit, so the second argument is NULL
	 */
	for (i = 0; i < NUM_THREADS; i++) {
		// Behaves similarly to waitpid.  Since we stored the count
		// globally, there is no need to catch the return, so the
		// second argument is NULL.  If we were say, making a signal
		// processing routine with parallel threads, we would catch
		// the return with a matching pointer type.
		pthread_join(threads[i], NULL);
	}
	
	printf("Main(): Waited on %d threads. Final value of count = %d. Done.\n",
	      NUM_THREADS, count);
	
	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&count_mutex);
	pthread_exit (NULL);
}

