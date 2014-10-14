#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>

#define NUM_PFUNC 6
#define DEBUG true

using namespace std;

typedef struct implementation {
  string name;
  std::vector<std::string> collection;
  std::vector<std::string> header;
} implementation;

//************************ //
//      Declarations      //
//***********************//

void DEBUG_PRINT (string line);
void DEBUG_VECTOR(vector<string>& dict);
void DEBUG_ARRAY(implementation *dict);
void _addPThreadCodeToNewCode(string name, vector<string>& list,vector<string>& header, implementation *dict);
void loadVector(vector<string>& source, vector<string>& target);
void saveProcessedProgram(vector<string>& header, vector<string>& list, string programName);
void skipInputLines(ifstream& file);
void parseHeader(ifstream& file, vector<string>& header);
void load(implementation *parallelFuncs);
string checkForDirective (string line);


 //**************************//
 //      DEBUG UTILITIES    //
//*************************//
void DEBUG_PRINT (string line) {
	if(DEBUG == true){
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		cout << line << endl;		
	}
}

void DEBUG_VECTOR(vector<string>& dict) {
 	if(DEBUG){
		 DEBUG_PRINT("VECTOR DEBUG");	

		 for( std::vector<string>::const_iterator i = dict.begin(); i != dict.end(); ++i)
		 	std::cout << *i << ' ' << endl;
	 	}
}

void DEBUG_ARRAY(implementation *dict){
	for(int i = 0; i < NUM_PFUNC; i++) {
		DEBUG_PRINT("HEADER");
		DEBUG_VECTOR(dict[i].header);
		DEBUG_PRINT("LIST");
		DEBUG_VECTOR(dict[i].collection);

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
	parallelFuncs[0].collection.push_back("    thr_data[i].tid = i;");	
	parallelFuncs[0].collection.push_back("    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {");
	parallelFuncs[0].collection.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\\n\", rc);");
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
	parallelFuncs[0].header.push_back("  printf( \"The parallel region is executed by thread %d\\n\", data->tid );");
	parallelFuncs[0].header.push_back("  ");
	parallelFuncs[0].header.push_back("  if( data->tid == 2 )");
	parallelFuncs[0].header.push_back("		printf( \"   Thread %d does things differently\\n\", data->tid );");
	parallelFuncs[0].header.push_back("  ");
	parallelFuncs[0].header.push_back("  pthread_exit(NULL);");
	parallelFuncs[0].header.push_back("}");
	parallelFuncs[0].header.push_back("");

	DEBUG_PRINT("ALL LOADED@!!@");
	//DEBUG_ARRAY(parallelFuncs);
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

void loadVector(vector<string>& source, vector<string>& target) {
 	DEBUG_PRINT("VECTOR LOAD");
 	
 	for( std::vector<string>::const_iterator i = source.begin(); i != source.end(); ++i)
 		target.push_back(*i);
 	DEBUG_PRINT("TARGET LOADED!!");
 	DEBUG_VECTOR(target);
}

void _addPThreadCodeToNewCode(string name, vector<string>& list,vector<string>& header, implementation *dict) {
	for(int i = 0; i < NUM_PFUNC; i++) {
		if(dict[i].name == name) {
			
			loadVector(dict[i].header, header);
			loadVector(dict[i].collection, list);
		}
	}
}

void parseResult(string resultName,vector<string>& header, vector<string>& list, implementation *dict) {
 	
 	for(int i = 0; i < NUM_PFUNC; i++){
 	
 		if(dict[i].name == resultName) {
 			_addPThreadCodeToNewCode(resultName, list, header, dict);
 			break;
 		}
 	}
 }

void saveProcessedProgram(vector<string>& header, vector<string>& list, string programName) {
	string fileName = "./" + programName + "post.cc";


	ofstream output_file(fileName.c_str());
    
    ostream_iterator<std::string> output_iterator(output_file, "\n");
    
    copy(header.begin(), header.end(), output_iterator);

    copy(list.begin(), list.end(), output_iterator);
}

string getProgramName(string fileName) {
	/*
		Parses the program name from the path or filename
		given when executing the preprocessor. Find the '.'
		character in fileName extension, then parse all '/ 'directory
		characters.
	*/
	string programName;
	int start=0;
	int end;
	end = fileName.find(".");
	
	programName = fileName.substr(start, end);

	int loc = programName.find("/");
	while(loc != std::string::npos){
		programName = programName.substr(loc + 1, end);
		loc = programName.find("/");
	}

	DEBUG_PRINT(programName);

	return programName;
}

int main (int argc, char *argv[]) {
	implementation parallelFuncs[NUM_PFUNC];
	vector<string> header;
	vector<string> list;
	string programName;

	load(parallelFuncs);

	ifstream in_stream;
	
	string line;
	string result;
	
	//in_stream.open(argv[1]);
	programName = getProgramName("INPUT/par.cc");
	in_stream.open("INPUT/par.cc");
	
	parseHeader(in_stream, header);

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);

		//check line for directives
		result = checkForDirective(line);
		
		if(result != "null"){
			parseResult(result, header, list, parallelFuncs);
			
			skipInputLines(in_stream);
		} else {
			list.push_back(line);
		}	
	}

	//DEBUG_VECTOR(header);
	//DEBUG_VECTOR(list);

	saveProcessedProgram(header, list, programName);
	
	in_stream.close();
	
return 0;	
}