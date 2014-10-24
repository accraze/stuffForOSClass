#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

#define NUM_THREADS 4

#define SIZE  16

typedef struct _thread_data_t {
  int tid;
} thread_data_t;

void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
	int i, n;

  int tid = data->tid; 
  int chunk_size = (SIZE / NUM_THREADS); 
  int start = tid * chunk_size; 
  int end = start + chunk_size;

  for(int i = start; i < end; i++)
		printf( "Thread %d executes loop iteration %d\n", data->tid, i );


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
