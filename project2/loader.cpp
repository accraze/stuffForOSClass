#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include "loader.h"


void load(string *parallelFuncs) {
	parallelFuncs[0].name = "par";
	parallelFuncs[0].collection.push_back("  pthread_t thr[NUM_THREADS];");
	parallelFuncs[0].collection.push_back("  int i, rc;");
	parallelFuncs[0].collection.push_back("  thread_data_t thr_data[NUM_THREADS];");
	parallelFuncs[0].collection.push_back("  ");
	parallelFuncs[0].collection.push_back("  for (i = 0; i < NUM_THREADS; ++i) {");
    parallelFuncs[0].collection.push_back("    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {");
    parallelFuncs[0].collection.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\n\", rc);");
    parallelFuncs[0].collection.push_back("      return EXIT_FAILURE;");
    parallelFuncs[0].collection.push_back("    }");
    parallelFuncs[0].collection.push_back("  }");
	parallelFuncs[0].collection.push_back("  for (i = 0; i < NUM_THREADS; ++i) {");
	parallelFuncs[0].collection.push_back("    pthread_join(thr[i], NULL);");
	parallelFuncs[0].collection.push_back("  }");
	parallelFuncs[0].collection.push_back(" ");
	parallelFuncs[0].header.push_back("#define NUM_THREADS 16");
	parallelFuncs[0].header.push_back(" ");
	parallelFuncs[0].header.push_back("typedef struct _thread_data_t {");
	parallelFuncs[0].header.push_back("  int tid;");
	parallelFuncs[0].header.push_back("} thread_data_t;");
	parallelFuncs[0].header.push_back(" ");
	parallelFuncs[0].header.push_back("void *do_work(void *arg) {");
	parallelFuncs[0].header.push_back("  thread_data_t *data = (thread_data_t *)arg;");
	parallelFuncs[0].header.push_back("  ");
	parallelFuncs[0].header.push_back("  printf( \"The parallel region is executed by thread %d\n\", data.tid );");
	parallelFuncs[0].header.push_back("  ");
	parallelFuncs[0].header.push_back("  if( data.tid == 2 )");
	parallelFuncs[0].header.push_back("		printf( \"   Thread %d does things differently\n\", data.tid );");
	parallelFuncs[0].header.push_back("  ");
	parallelFuncs[0].header.push_back("  pthread_exit(NULL);");
	parallelFuncs[0].header.push_back("}");

	cout << "ALL LOADED@!!@" << endl;

}