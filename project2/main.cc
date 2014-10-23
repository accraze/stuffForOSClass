#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>

using namespace std;

#define NUM_PFUNC 6
#define DEBUG true


std::vector<std::string> header;
std::vector<std::string> pthreadStruct;
std::vector<std::string> workFunc;
std::vector<std::string> collection;
std::vector<std::string> mainFunc;
std::vector<std::string> header;


implementation parallelFuncs[NUM_PFUNC];
vector<string> variableList;

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
void _checkNumThreads(string line, vector<string>& header);
bool _convertThreadIdentifiers(string line, vector<string>& list);
void _convertDirective(string resultName,vector<string>& header, vector<string>& list, implementation *dict);
void closeStruct();
void closeWorkFunc();


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
	// This method gets the number of threads from
	// the openMP program and puts it in the header
	// of the Pthreads implementation.

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

void closeStruct(){
	pthreadStruct.push_back("} thread_data_t;");
	pthreadStruct.push_back("");

}

void closeWorkFunc(){
	pthreadStruct.push_back("} thread_data_t;");
	pthreadStruct.push_back("}");
	pthreadStruct.push_back("");

}

void load(implementation *parallelFuncs) {
	/*
		Loads all pthread template code into 
		either the pthread struct, the pthread WorkFunction
		or the main function vectors.
	*/
	
	pthreadStruct.push_back("typedef struct _thread_data_t {");
	pthreadStruct.push_back("  int tid;");

	workFunc.push_back("void *do_work(void *arg) {");
	workFunc.push_back("void *do_work(void *arg) {");
	workFunc.push_back("  thread_data_t *data = (thread_data_t *)arg;");

	mainFunc.push_back("int main(int argc, char **argv) {");
	mainFunc.push_back("  pthread_t thr[NUM_THREADS];");
	mainFunc.push_back("  int z, rc;");
	mainFunc.push_back("  thread_data_t thr_data[NUM_THREADS];");
	mainFunc.push_back("");
	mainFunc.push_back(" for (z = 0; z < NUM_THREADS; ++z) {");
	mainFunc.push_back("    thr_data[i].tid = i;");
	mainFunc.push_back("    if ((rc = pthread_create(&thr[i], NULL, do_work, &thr_data[i]))) {");
	mainFunc.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\n\", rc);");
	mainFunc.push_back("      return EXIT_FAILURE;");
	mainFunc.push_back("    }");
	mainFunc.push_back("  }");
	mainFunc.push_back("");
	mainFunc.push_back("  for (i = 0; i < NUM_THREADS; ++i) {");
	mainFunc.push_back("    pthread_join(thr[i], NULL);");
	mainFunc.push_back("  }");
	mainFunc.push_back("");
	mainFunc.push_back("  return EXIT_SUCCESS;");
	mainFunc.push_back("}");

	DEBUG_PRINT("ALL LOADED@!!@");
	//DEBUG_ARRAY(parallelFuncs);
}

void parseHeader(ifstream& file, vector<string>& header) {
	/*
		Parses all #include statements and adds them to the
		the new program header. It will also replace the 
		<omp.h> header with a <pthread.h> header.
	*/

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

void _addPThreadCodeToNewCode(string name, vector<string>& list,vector<string>& header, implementation *dict) {
	

	for(int i = 0; i < NUM_PFUNC; i++) {
		if(dict[i].name == name) {
			
			loadVector(dict[i].header, header);
			loadVector(dict[i].collection, list);
		}
	}
}

void _convertDirective(string resultName,vector<string>& header, vector<string>& list, implementation *dict) {
 	/* 
 		matches the resultName name to the correct Pthread implementation stored in dict. 
 		Then it adds the new implementation to the program list. 
 	*/

 	int i;
 	for(i = 0; i < NUM_PFUNC; i++){
 	
 		if(dict[i].name == resultName) {
 			_addPThreadCodeToNewCode(resultName, list, header, dict);
 			break;
 		}
 	}
 }

void _convertClauses(string line, vector<string>& header){
	_checkNumThreads(line, header);
	_checkPrivateClause(line, header);
	//_checkShared(line, header);

	int check = line.find("omp_get_thread_num()");

	if(check != std::string::npos){
		replace(line, "omp_get_thread_num()", "data->id");
	}

	//addNumThreads(line, header);
}

void _checkPrivateClause(string line){
	int check = line.find("private(");
	
	if(check != std::string::npos){
		string list = line.substr(check, line.length());
		char privateList[list.size()+1];//as 1 char space for null is also required
		strcpy(privateList, myWord.c_str());

		vector<char> variables;
		parseVariableList(privateList, variables);

		addPrivateVariables(variables);
		
	}

}

void addPrivateVariables(vector<char>& variables){
	/*
		Adds all private variables to the pThreadStruct
	*/

	for( std::vector<string>::const_iterator i = variables.begin(); i != variables.end(); ++i)
		 	string line = "int ";
		 	line += *i;
		 	line += ";\n";
		 	pthreadStruct.push_back(line);
	 }
}

void parseVariableList(char *list, vector<char>& variables){
		int i = 0;
		
		while(privateList[i] != ")"){
			if(privateList[i] != ","){
				variables.push_back(privateList[i]);
			}
		}

}

void _checkNumThreads(string line, vector<string>& header) {
	/* 
		Checks to see if the num_threads clauses is contained
		in the file line. If it is, then the number is parsed
	 	and added as a macro in the pthreads implementation. 
	*/

	int check = line.find("num_threads");

	if(check != std::string::npos){
		addNumThreads(line, header);
	}
}

bool _convertThreadIdentifiers(string line, vector<string>& list){
	/* 
		This function looks for all calls to the
	 	openmp_get_thread_num runtime library and
	 	replaces them with pthread implementation
	 */
	
	int check = line.find("omp_get_thread_num()");

	if(check != std::string::npos){
		replace(line, "omp_get_thread_num()", "data->id");
		
		list.push_back(line);
		
		return true;
	}

	return false;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    // this function replaces part of a string with another string

    size_t start_pos = str.find(from); // find start
    if(start_pos == std::string::npos) 
        return false;
    str.replace(start_pos, from.length(), to); //replace
    return true;
}

void checkForVariables(string line) {
	int check  = line.find("int ");
	if (check != std::string::npos){
		string list = line.substr(check, line.length()); //get rid of int type

		variableList.push_back(list);

		DEBUG_PRINT(list);	

		// check  = line.find(",");
		
		// if(check != std::string::npos){
			
		// 	while(check != std::string::npos) {
		// 		string var = line.substr(0, check); //get rid of int type
				
		// 		variableList.push_back(var);
				
		// 		string list = line.substr(check, line.length()); //get rid of int type
			
		// 		check  = line.find(",");
		// 	}
		// }
		// else {
		// 	string noInt = line.substr(check, line.length());
			
		// 	string var = noInt.substr(0, noInt.length() - 1);

		// 	DEBUG_PRINT(var);
			
		// 	variableList.push_back(var);	
		// }
		
		
	}
}

void parseProgram (ifstream& in_stream, vector<string>& header, vector<string>& list){
	/*  
		Runs through each line of the openMP program and
	    checks for a directive and clauses. 
		If the line contains a directive, it will parse the directive and
		each of the following clauses. 
	*/

	string line;
	string directive;

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);
		//check for variable instant
		checkForVariables(line);

		//check line for directives
		directive = checkForDirective(line);
		
		if (directive != "null") {
			
			_convertClauses(line);
			//_convertDirective(directive, header, list, parallelFuncs);
			
			//skipInputLines(in_stream, list);
		} 
		else {
			//bool hasConverted = _convertThreadIdentifiers(line, list);
			int check = line.find("omp");
			
			if(check != std::string::npos){
				list.push_back(line);
			}
		}	
	}
}

int main (int argc, char *argv[]) {	
	//vector<string> header;
	//vector<string> list;
	string programName;

	load(parallelFuncs);

	ifstream in_stream;
	
	string line;
	string result;
	
	//in_stream.open(argv[1]);
	programName = getProgramName("INPUT/parfor.cc");
	in_stream.open("INPUT/parfor.cc");

	printf("Processing OpenMP program....\n");
	
	parseHeader(in_stream, header);
	parseProgram(in_stream, header, list);

	//DEBUG_VECTOR(header);
	//DEBUG_VECTOR(list);

	printf("Writing processed pthread program to file....\n");
	
	saveProcessedProgram(header, list, programName);

	printf("All done!\n");
	
	in_stream.close();
	
	return 0;	
}