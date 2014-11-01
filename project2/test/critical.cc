//g++ parpost.cpp -o a1.out -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4
#define SIZE 5

pthread_mutex_t var=PTHREAD_MUTEX_INITIALIZER;
int sum, a[5];
/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
	int local_sum;
  int i;
} thread_data_t;


 
/* thread function */
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  data->local_sum = 0; 
  data->i = 0; 

  int tid = data->tid; 
  int chunk_size = (SIZE / NUM_THREADS); 
  int start = tid * chunk_size; 
  int end = start + chunk_size;

  for(data->i = start; data->i < end; data->i++) data->local_sum += a[data->i];;

  pthread_mutex_lock(&var);  // lock the critical section

  sum += data->local_sum;
  printf( "Thread %d: local_sum = %d, sum = %d\n", data->tid, data->local_sum, sum );
  
  pthread_mutex_unlock(&var); // unlock once you are done

  pthread_exit(NULL);
}
 
int main(int argc, char **argv) {
  int i;
  printf( "Example of the critical construct\n" );

  for( i = 0; i < 5; i++ )
  	a[i] = i;

  sum = 0;

  pthread_t thr[NUM_THREADS];
  int z, rc;
  /* create a thread_data_t argument array */
  thread_data_t thr_data[NUM_THREADS];

  /* create threads */
  for (z = 0; z < NUM_THREADS; ++z) {
    thr_data[z].tid = z;
    if ((rc = pthread_create(&thr[z], NULL, do_work, &thr_data[z]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }

  /* block until all threads complete */
  for (z = 0; z < NUM_THREADS; ++z) {
    pthread_join(thr[z], NULL);
  }

  printf( "Sum should be 5(4)/2 = %d\n", 5*(5-1)/2 );
  printf( "Value of sum after parallel region: %d\n", sum );

  return EXIT_SUCCESS;
}