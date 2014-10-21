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


implementation parallelFuncs[NUM_PFUNC];

//************************ //
//      Declarations      //
//***********************//

void DEBUG_PRINT (string line);
void DEBUG_VECTOR(vector<string>& dict);
void DEBUG_ARRAY(implementation *dict);
void _addPThreadCodeToNewCode(string name, vector<string>& list,vector<string>& header, implementation *dict);
void loadVector(vector<string>& source, vector<string>& target);
void saveProcessedProgram(vector<string>& header, vector<string>& list, string programName);
void skipInputLines(ifstream& file, vector<string>& list);
void parseHeader(ifstream& file, vector<string>& header);
void load(implementation *parallelFuncs);
string checkForDirective (string line, vector<string>& header);
string getNumThreads(string directive);
void addNumThreads(string inputLine, vector<string>& header);
void removeOmpRefs (vector<string>& functionsList);
void parseProgram (ifstream& in_stream, vector<string>& header, vector<string>& list);
bool replace(std::string& str, const std::string& from, const std::string& to);


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

string checkForDirective (string line, vector<string>& header) {
	
	int check = line.find("#pragma omp parallel for ");
	if(check != std::string::npos) {
		addNumThreads(line, header);

		return "parfor";
	}

	check = line.find("#pragma omp parallel ");
	if(check != std::string::npos) {
		addNumThreads(line, header);
		return "par";
	}


	return "null";
}

void addNumThreads(string inputLine, vector<string>& header) {
	string num = getNumThreads(inputLine);

	string line = "#define NUM_THREADS ";
	line += num;

	header.push_back(line);
}

string getNumThreads(string directive) {
	/*
		Parses the openMP directive and returns the 
		number of threads.
	*/
	string numThreads;
	int start = directive.find("(");
	int end = directive.find(")");
	
	numThreads = directive.substr(start+1, end);

	end = numThreads.find(")");
	if(end != std::string::npos){
		numThreads = numThreads.substr(0, end);
	}

	DEBUG_PRINT(numThreads);

	return numThreads;
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
	//parallelFuncs[0].header.push_back("#define NUM_THREADS ");
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

	parallelFuncs[1].name = "parfor";
	parallelFuncs[1].collection.push_back("  pthread_t thr[NUM_THREADS];");
	parallelFuncs[1].collection.push_back("  int z, rc;");
	parallelFuncs[1].collection.push_back("  thread_data_t thr_data[NUM_THREADS];");
	parallelFuncs[1].collection.push_back("  ");
	parallelFuncs[1].collection.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	parallelFuncs[1].collection.push_back("    thr_data[z].tid = z;");	
	parallelFuncs[1].collection.push_back("    if ((rc = pthread_create(&thr[z], NULL, do_work, &thr_data[z]))) {");
	parallelFuncs[1].collection.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\\n\", rc);");
	parallelFuncs[1].collection.push_back("      return EXIT_FAILURE;");
	parallelFuncs[1].collection.push_back("    }");
	parallelFuncs[1].collection.push_back("  }");
	parallelFuncs[1].collection.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	parallelFuncs[1].collection.push_back("    pthread_join(thr[z], NULL);");
	parallelFuncs[1].collection.push_back("  }");
	parallelFuncs[1].collection.push_back(" ");
	//parallelFuncs[0].header.push_back("#define NUM_THREADS ");
	parallelFuncs[1].header.push_back("#define SIZE 16");
	parallelFuncs[1].header.push_back(" ");
	parallelFuncs[1].header.push_back("typedef struct _thread_data_t {");
	parallelFuncs[1].header.push_back("  int tid;");
	parallelFuncs[1].header.push_back("} thread_data_t;");
	parallelFuncs[1].header.push_back(" ");
	parallelFuncs[1].header.push_back("void *do_work(void *arg) {");
	parallelFuncs[1].header.push_back("  thread_data_t *data = (thread_data_t *)arg;");
	parallelFuncs[1].header.push_back("  ");
	parallelFuncs[1].header.push_back(" int tid = data->tid;");
	parallelFuncs[1].header.push_back(" int chunk_size = (SIZE / NUM_THREADS); ");
	parallelFuncs[1].header.push_back(" int start = tid * chunk_size; ");
	parallelFuncs[1].header.push_back(" int end = start + chunk_size;");
	parallelFuncs[1].header.push_back("  ");
	parallelFuncs[1].header.push_back(" for(int i = start; i < end; i++) printf( \"Thread %d executes loop iteration %d\\n\", tid, i );;");
	parallelFuncs[1].header.push_back("  ");
	parallelFuncs[1].header.push_back("  pthread_exit(NULL);");
	parallelFuncs[1].header.push_back("}");
	parallelFuncs[1].header.push_back("");

	parallelFuncs[2].name = "critical";
	parallelFuncs[2].collection.push_back("  pthread_t thr[NUM_THREADS];");
	parallelFuncs[2].collection.push_back("  int z, rc;");
	parallelFuncs[2].collection.push_back("  thread_data_t thr_data[NUM_THREADS];");
	parallelFuncs[2].collection.push_back("  ");
	parallelFuncs[2].collection.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	parallelFuncs[2].collection.push_back("    thr_data[z].tid = z;");	
	parallelFuncs[2].collection.push_back("    if ((rc = pthread_create(&thr[z], NULL, do_work, &thr_data[z]))) {");
	parallelFuncs[2].collection.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\\n\", rc);");
	parallelFuncs[2].collection.push_back("      return EXIT_FAILURE;");
	parallelFuncs[2].collection.push_back("    }");
	parallelFuncs[2].collection.push_back("  }");
	parallelFuncs[2].collection.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	parallelFuncs[2].collection.push_back("    pthread_join(thr[z], NULL);");
	parallelFuncs[2].collection.push_back("  }");
	parallelFuncs[2].collection.push_back(" ");
	//parallelFuncs[0].header.push_back("#define NUM_THREADS ");
	parallelFuncs[2].header.push_back("pthread_mutex_t var=PTHREAD_MUTEX_INITIALIZER;");
	parallelFuncs[2].header.push_back("int sum, a[5];");
	parallelFuncs[2].header.push_back(" ");
	parallelFuncs[2].header.push_back(" ");
	parallelFuncs[2].header.push_back("typedef struct _thread_data_t {");
	parallelFuncs[2].header.push_back("  int tid;");
	parallelFuncs[2].header.push_back("} thread_data_t;");
	parallelFuncs[2].header.push_back(" ");
	parallelFuncs[2].header.push_back("void *do_work(void *arg) {");
	parallelFuncs[2].header.push_back("  thread_data_t *data = (thread_data_t *)arg;");
	parallelFuncs[2].header.push_back("  ");
	parallelFuncs[2].header.push_back(" int tid = data->tid;");
	parallelFuncs[2].header.push_back(" int chunk_size = (SIZE / NUM_THREADS); ");
	parallelFuncs[2].header.push_back(" int start = tid * chunk_size; ");
	parallelFuncs[2].header.push_back(" int end = start + chunk_size;");
	parallelFuncs[2].header.push_back("  ");
	parallelFuncs[2].header.push_back(" for(int i = start; i < end; i++) printf( \"Thread %d executes loop iteration %d\\n\", tid, i );;");
	parallelFuncs[2].header.push_back("  ");
	parallelFuncs[2].header.push_back("  pthread_exit(NULL);");
	parallelFuncs[2].header.push_back("}");
	parallelFuncs[2].header.push_back("");


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

void skipInputLines(ifstream& file, vector<string>& list) {
	string line;

	std::getline(file, line);

	if(line == "{") {
		
		while(line != "}") {
			std::getline(file, line);
		}
	} else {

		while(line == "	return(0);"){
			std::getline(file, line);
			DEBUG_PRINT(line);
			if (line == "	return(0);")
			{
				list.push_back(line);
				break;
			}
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

void saveProcessedProgram(vector<string>& header, vector<string>& functionsList, string programName) {
	/* Takes the processed header and functionList and writes to file */

	string fileName = "./" + programName + "post.cc";

	ofstream output_file(fileName.c_str());
    
    ostream_iterator<std::string> output_iterator(output_file, "\n");
    
    //write the program header to file
    copy(header.begin(), header.end(), output_iterator);
    //write the program functions to file
    copy(functionsList.begin(), functionsList.end(), output_iterator);
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

// void removeOmpRefs (vector<string>& functionsList) {
// 	// string *scr;
// 	// scr = new string[functionsList.size()];
// 	// for(int i=0; i<functionsList.size(); i++){
//  //   		scr[i] = functionsList[i];//Copy the vector to the string
// 	// }

// 	// for(int j = 0; j < functionsList.size(); j++){
// 	// 	int check = scr[j].find(' omp_');

// 	// 	if(check != std::string::npos){
// 	// 		DEBUG_PRINT("FOUND OMP REF");
// 	// 		DEBUG_PRINT(scr[j]);
// 	// 		functionsList.erase(functionsList.begin() + j);
// 	// 	}
// 	// }

// 	for( std::vector<string>::const_iterator i = functionsList.begin(); i != functionsList.end(); ++i) {
// 		string ele = i + "";

// 		int check = ele.find('omp');
// 		if(check != std::string::npos){
// 			i = "";
// 		}
// 	}
// }

void _convertDirectives(){
	printf("YO!\n");
}

void _convertClauses(){
	printf("YO!\n");
}

void _convertThreadIdentifiers(ifstream& in_stream){
	/* This function looks for all calls to the
	 openmp_get_thread_num runtime library and
	 replaces them with pthread implementation
	 */
	string line;

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);
		int check = line.find("omp_get_thread_num()");

		if(check != std::string::npos){
			replace(line, "omp_get_thread_num()", "data->id");
		}


	}
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    // this function replaces part of a string with another string

    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void parseProgram (ifstream& in_stream, vector<string>& header, vector<string>& list){
	string line;
	string result;

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);

		//check line for directives
		result = checkForDirective(line, header);
		
		if(result != "null"){
			parseResult(result, header, list, parallelFuncs);
			
			skipInputLines(in_stream, list);
		} else {
			int check = line.find("omp");
			if(!(check != std::string::npos))
				list.push_back(line);
		}	
	}
}


int main (int argc, char *argv[]) {
	
	vector<string> header;
	vector<string> list;
	string programName;

	load(parallelFuncs);

	ifstream in_stream;
	
	string line;
	string result;
	
	//in_stream.open(argv[1]);
	programName = getProgramName("INPUT/parfor.cc");
	in_stream.open("INPUT/parfor.cc");
	
	parseHeader(in_stream, header);

	parseProgram(in_stream, header, list);

	//DEBUG_VECTOR(header);
	//DEBUG_VECTOR(list);
	//removeOmpRefs(list);
	saveProcessedProgram(header, list, programName);
	
	in_stream.close();
	
return 0;	
}