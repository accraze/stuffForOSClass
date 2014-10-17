#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

#define NUM_THREADS 4
#define SIZE 16
 
typedef struct _thread_data_t {
  int tid;
} thread_data_t;
 
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  
 int tid = data->tid;
 int chunk_size = (SIZE / NUM_THREADS); 
 int start = tid * chunk_size; 
 int end = start + chunk_size;
  
 for(int i = start; i < end; i++) printf( "Thread %d executes loop iteration %d\n", tid, i );
  
  pthread_exit(NULL);
}

int main()
{
	int i, n;

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
 
		printf( "Thread %d executes loop iteration %d\n", omp_get_thread_num(), i );

	return(0);
}

