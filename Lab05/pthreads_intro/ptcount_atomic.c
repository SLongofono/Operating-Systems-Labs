#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS  3

struct thread_args {
	int tid;
	int inc;
	int loop;
};

int count = 0;
pthread_mutex_t count_mutex;

/*
 * This routine will be executed by each thread we choose to create.
 * The routine a new thread will execute is given as an arguent to the
 * pthread_create() call.
 */
void *inc_count(void *arg)
{
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

		// Normally, the below code would require a load, and add, and
		// a store inctruction, giving a potential opportunity for the
		// scheduler to interrupt a thread and corrupting the count
		// memory.  Instead, we use the atomic increment built into
		// the POSIX standard, which allows us to forego use of the
		// mutex.  Provided we have the GNU compiler, we can use it,
		// otherwise, we need to fall back on the mutex solution
		//

		// Begin critical section

		#ifndef __GNUC__
		// __atomic_add_fetch(ptr, val, memorder)
		// ptr - a pointer to the type to be incremented.  The
		// compiler will figure out what size
		// val - the amount to increment by.  Again, give it a size_t
		// or integer type and the compiler will handle it
		// memorder - there are a number of flags to enforce which
		// threads take precedence to facilitate order of execution.
		__atomic_add_fetch(&count, my_args->inc,__ATOMIC_RELAXED);
		#else
		pthread_mutex_lock(&count_mutex);
		#endif
		count = count + my_args->inc;

		#ifndef __GNUC__
		printf("USING ATOMIC\n");// do nothing, we are already done
		#else
		pthread_mutex_unlock(&count_mutex);
		#endif
		// End critical section

		loc = loc + my_args->inc;
	}
	printf("Thread: %d finished. Counted: %d\n", my_args->tid, loc);
	free(my_args);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int i, loop, inc;
	struct thread_args *targs;
	pthread_t threads[NUM_THREADS];
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

	/* For portability, explicitly create threads in a joinable state */
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
		pthread_create(&threads[i], &attr, inc_count, (void*)targs);
		/* Make call to pthread_create here */
	}

	/* Wait for all threads to complete using pthread_join.  The threads
	 * do not return anything on exit, so the second argument is NULL
	 */
	for (i = 0; i < NUM_THREADS; i++) {
		/* Make call to pthread_join here */
		pthread_join(threads[i], NULL);
	}

	printf ("Main(): Waited on %d threads. Final value of count = %d. Done.\n",
		NUM_THREADS, count);

	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&count_mutex);
	pthread_exit (NULL);
}
