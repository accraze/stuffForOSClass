#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

int id, i, a, b[5];

#define NUM_THREADS 4

bool signalFlag = false;

#define SIZE  5

typedef struct _thread_data_t {
  int tid;
  int i,id;
} thread_data_t;

void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  if(!signalFlag){
    signalFlag = true;
			a = 10;
			id = data->tid;
			printf( "Single construct executed by thread %d\n", id );
  }

  int tid = data->tid; 
  int chunk_size = (SIZE / NUM_THREADS); 
  int start = tid * chunk_size; 
  int end = start + chunk_size;

  for(i = start; i < end; i++){
			id = data->tid;
			printf( "Thread %d is setting b[%d]=%d\n", id, i, a );
			b[i] = a;
  }
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
	printf( "Testing the single construct\n" );

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


	printf( "After the parallel region:\n" );
	printf( "a = %d\n", a );
	for( i = 0; i < 5; i++ )
		printf( "b[%d] = %d\n", i, b[i] );
		
	return(0);


  return EXIT_SUCCESS;
}

