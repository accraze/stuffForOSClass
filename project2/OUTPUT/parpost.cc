#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

int id;

#define NUM_THREADS 16

typedef struct _thread_data_t {
  int tid;
  int id;
} thread_data_t;

void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
		data->id = data->tid;

		printf( "The parallel region is executed by thread %d\n", data->id );

		if( data->id == 2 )
			printf( "   Thread %d does things differently\n", data->id );
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


	return(0);


  return EXIT_SUCCESS;
}

