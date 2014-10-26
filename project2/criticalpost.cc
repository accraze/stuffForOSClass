#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

#define NUM_THREADS local_sum

typedef struct _thread_data_t {
  int tid;
  int local_sum;
} thread_data_t;

void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
	int i, a[5], local_sum, sum;
	printf( "Example of the critical construct\n" );

	for( i = 0; i < 5; i++ )
		a[i] = i;

	sum = 0;


	#pragma omp for 
	for( i = 0; i < 5; i++ )
		local_sum += a[i];

	#pragma omp critical
	{
		sum += local_sum;
		printf( "Thread %d: local_sum = %d, sum = %d\n", data->tid, local_sum, sum );
	}

	printf( "Sum should be 5(4)/2 = %d\n", 5*(5-1)/2 );
	printf( "Value of sum after parallel region: %d\n", sum );

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  pthread_t thr[NUM_THREADS];
  int z, rc;
  thread_data_t thr_data[NUM_THREADS];

  for (z = 0; z < NUM_THREADS; ++z) {
    thr_data[z].tid = z;
    if ((rc = pthread_create(&thr[z], NULL, do_work, &thr_data[z]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }

  for (z = 0; z < NUM_THREADS; ++z) {
    pthread_join(thr[z], NULL);
  }

  return EXIT_SUCCESS;
}
