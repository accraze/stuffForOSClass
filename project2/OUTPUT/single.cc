//g++ parpost.cpp -o a1.out -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
 
#define NUM_THREADS 4
#define SIZE 5

int id, i, a, b[5];
bool signalFlag = false;

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
  int i;
  int id;
} thread_data_t;
 
/* thread function */
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  if(!signalFlag){
    signalFlag = true;
    a = 10;
    data->id = data->tid;
    printf( "Single construct executed by thread %d\n", id );

  }

  int tid = data->tid; 
  int chunk_size = (SIZE / NUM_THREADS); 
  int start = tid * chunk_size; 
  int end = start + chunk_size;

  for(i = start; i < end; i++){
    data->id = data->tid;
    printf( "Thread %d is setting b[%d]=%d\n", id, i, a );
    b[i] = a;
  }

  pthread_exit(NULL);
}
 
int main(int argc, char **argv) {
  printf( "Testing the single construct\n" );

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


  printf( "After the parallel region:\n" );
  printf( "a = %d\n", a );
  for( i = 0; i < 5; i++ )
    printf( "b[%d] = %d\n", i, b[i] );
 
  return EXIT_SUCCESS;
}