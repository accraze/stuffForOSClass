//g++ -Wall parpost.cc -o a1.out -pthread -lpthread

#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

#define NUM_THREADS 16
 
typedef struct _thread_data_t {
  int tid;
} thread_data_t;
 
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  
  printf( "The parallel region is executed by thread %d\n", data->tid );
  
  if( data->tid == 2 )
		printf( "   Thread %d does things differently\n", data->tid );
  
  pthread_exit(NULL);
}

int main()
{

  pthread_t thr[NUM_THREADS];
  int i, rc;
  thread_data_t thr_data[NUM_THREADS];
  
  for (i = 0; i < NUM_THREADS; ++i) {
    thr_data[i].tid = i;
    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }
 
	return(0);
}

