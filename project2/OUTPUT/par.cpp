//g++ parpost.cpp -o a1.out -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
 
#define NUM_THREADS 16
 
/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
} thread_data_t;
 
/* thread function */
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;

  printf( "The parallel region is executed by thread %d\n", data->tid );

  if( data->tid == 2 )
		printf( "   Thread %d does things differently\n", data->tid );
 
  pthread_exit(NULL);
}
 
int main(int argc, char **argv) {
  pthread_t thr[NUM_THREADS];
  int i, rc;
  /* create a thread_data_t argument array */
  thread_data_t thr_data[NUM_THREADS];

  /* create threads */
  for (i = 0; i < NUM_THREADS; ++i) {
    thr_data[i].tid = i;
    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  /* block until all threads complete */
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }
 
  return EXIT_SUCCESS;
}