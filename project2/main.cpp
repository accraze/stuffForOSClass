#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>

#define NUM_PFUNC 6

#define DEBUG false

using namespace std;

typedef struct implementation {
  string name;
  std::vector<std::string> collection;
  std::vector<std::string> header;
} implementation;

string checkForDirective(string line);
void parseHeader(ifstream& file, vector<string>& header);
void load(std::string (&parallelFuncs)[NUM_PFUNC]);

void DEBUG_PRINT (string line) {
	if(DEBUG == true){
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		cout << line << endl;		
	}

}


string checkForDirective (string line) {
	if(line == "#pragma omp parallel num_threads(16)") {
		return "par";
	}
	return "null";
}

void load(implementation *parallelFuncs) {
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

	DEBUG_PRINT("ALL LOADED@!!@");
}

void parseHeader(ifstream& file, vector<string>& header) {
	string line;

	std::getline (file,line);

	header.push_back(line);

	while(line.find("#include") != string::npos) {
		std::getline (file,line);
		
		if(line == "#include	<omp.h>") {
			line = "#include 	<pthread.h>";
		}

		header.push_back(line);
	}
}

void skipInputLines(ifstream& file) {
	string line;

	std::getline(file, line);

	if(line == "{") {
		
		while(line != "}") {
			std::getline(file, line);
		}
	}
}

void _addPThreadCoteToNewCode(vector<string>& list,vector<string>& header, implementation *dict) {
}

void loadVector(vector<string>& source, vector<string>& target) {
 	DEBUG_PRINT("VECTOR LOAD");
 	
 	for( std::vector<string>::const_iterator i = source.begin(); i != source.end(); ++i)
 		target.push_back(*i);
}

void parseResult(string resultName,vector<string>& header,
					 vector<string>& list,implementation *dict) {
 	
 	for(int i = 0; i < NUM_PFUNC; i++){
 	
 		if(dict[i].name == resultName) {
 			//addPThreadCodeToNewCode(list, header, dict);
 			break;
 		}
 	}
 }

void DEBUG_VECTOR(vector<string> dict) {
 	if(DEBUG){
		 	DEBUG_PRINT("VECTOR DEBUG");	

		 	for( std::vector<string>::const_iterator i = dict.begin(); i != dict.end(); ++i)
		    	std::cout << *i << ' ' << endl;
	 	}
 }

int main (int argc, char *argv[]) {
	//read in program from command line arguments
	implementation parallelFuncs[NUM_PFUNC];
	vector<string> header;
	vector<string> list;

	load(parallelFuncs);
	
	ifstream in_stream;
	
	string line;
	string result;
	
	//in_stream.open(argv[1]);
	in_stream.open("INPUT/par.cc");
	
	parseHeader(in_stream, header);

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);
		//cout<< line;
		
		//check line for directives
		result = checkForDirective(line);
		
		if(result != "null"){
			//cout << result << endl;
			//parseResult(result, , header, list, parallelFuncs);
			skipInputLines(in_stream);
		} else {
			list.push_back(line);
		}	
	}

	DEBUG_VECTOR(list);
	
	in_stream.close();
	
return 0;	
}