#include	<stdio.h>
#include	<stdlib.h>
#include 	<pthread.h>

#define NUM_THREADS 4
typedef struct _thread_data_t {
  int tid;
} thread_data_t;

} thread_data_t;
}

void *do_work(void *arg) {
void *do_work(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
int main()
{
	int i, n;

	for( i = 0; i < 16; i++ )
		printf( "Thread %d executes loop iteration %d\n", data->id, i );

	return(0);
}

int main(int argc, char **argv) {
  pthread_t thr[NUM_THREADS];
  int z, rc;
  thread_data_t thr_data[NUM_THREADS];

 for (z = 0; z < NUM_THREADS; ++z) {
    thr_data[i].tid = i;
    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d
", rc);
      return EXIT_FAILURE;
    }
  }

  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }

  return EXIT_SUCCESS;
}
