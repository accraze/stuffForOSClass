#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;

//************************ //
// GLobal Vars Section
//************************ //

bool FIRST_DIRECTIVE = false;

vector<string> header;
vector<string> pthreadStruct;
vector<string> workFunc;
vector<string> mainFunc;

string privateVariableList[20];
int privateCount = 0;


//************************ //
//      Declarations      //
//***********************//

void DEBUG_PRINT (string line);
void DEBUG_VECTOR(vector<string>& dict);
void runScanner(string fileName);
void loadVector(vector<string>& source, vector<string>& target);
void saveProcessedProgram(string programName);
void parseHeader(ifstream& file);
void parseVariableList(string list);
void loadPthreadTemplates();
string checkForDirective (string line);
string getNumThreads(string directive);
void getForLoopSize(ifstream& file);
string getProgramName(string fileName);
void addNumThreads(string line);
void addNumSize(string num);
void addPrivateVariables(string variable);
void addMainFuncTemplate();
void parseProgram (ifstream& in_stream);
bool replace(std::string& str, const std::string& from, const std::string& to);
void _checkNumThreads(string line);
bool _convertThreadIdentifiers(string line);
void _convertDirective(string resultName, ifstream& file);
void closeStruct();
void closeWorkFunc();
void closeCriticalSection();
void closeMainFunc();
void _closeSingleSection();
void _closeForSection ();
void _closeTemplates();
void _buildParCode (ifstream& file);
bool checkForFunction(string line);
bool _checkIfPrivate(string line);
void _readDirectiveProgram(ifstream& file);
bool checkForVariables(string line);
void _convertClauses(string line);
void _loadSingleTemplate();

//************//
// FUNCTIONS //
//**********//

void addNumThreads(string inputLine) {
	/* 
	This method gets the number of threads from
	the openMP program and puts it in the header
	 of the Pthreads implementation.
	*/
	string num = getNumThreads(inputLine);

	string line = "#define NUM_THREADS ";
	line += num;

	header.push_back(line);
	header.push_back("");
}

void addNumSize(string num) {
	/* 
	This method gets the size of for loop from
 	the openMP program and puts it in the 
 	workFunc of the Pthreads implementation.
	*/

	string line = "#define SIZE ";
	line += num;

	header.push_back(line);
	header.push_back("");
}

void addPrivateVariables(string variable){
	/*
		Adds a private variables to the pThreadStruct
	*/
	string line;

 	line = "  int ";
 	line += variable;
 	line += ";";

 	pthreadStruct.push_back(line);
}

void addMainFuncTemplate(){
	/*
		Adds all the pthread boilerplate 
		code to the main function of new program.
	*/

	mainFunc.push_back("  pthread_t thr[NUM_THREADS];");
	mainFunc.push_back("  int z, rc;");
	mainFunc.push_back("  thread_data_t thr_data[NUM_THREADS];");
	mainFunc.push_back("");
	mainFunc.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	mainFunc.push_back("    thr_data[z].tid = z;");
	mainFunc.push_back("    if ((rc = pthread_create(&thr[z], NULL, do_work, &thr_data[z]))) {");
	mainFunc.push_back("      fprintf(stderr, \"error: pthread_create, rc: %d\\n\", rc);");
	mainFunc.push_back("      return EXIT_FAILURE;");
	mainFunc.push_back("    }");
	mainFunc.push_back("  }");
	mainFunc.push_back("");
	mainFunc.push_back("  for (z = 0; z < NUM_THREADS; ++z) {");
	mainFunc.push_back("    pthread_join(thr[z], NULL);");
	mainFunc.push_back("  }");
	mainFunc.push_back("");
}

void loadVector(vector<string>& source, vector<string>& target) {
 	DEBUG_PRINT("VECTOR LOAD");
 	
 	for( std::vector<string>::const_iterator i = source.begin(); i != source.end(); ++i)
 		target.push_back(*i);
 	DEBUG_PRINT("TARGET LOADED!!");
 	DEBUG_VECTOR(target);
}

void loadPthreadTemplates() {
	/*
		Loads all pthread template code into 
		either the pthread struct, the pthread WorkFunction
		or the main function vectors.
	*/
	
	pthreadStruct.push_back("typedef struct _thread_data_t {");
	pthreadStruct.push_back("  int tid;");

	workFunc.push_back("void *do_work(void *arg) {");
	workFunc.push_back("  thread_data_t *data = (thread_data_t *)arg;");

	mainFunc.push_back("int main(int argc, char **argv) {");

	DEBUG_PRINT("ALL LOADED@!!@");
}

string getNumThreads(string directive) {
	/*
		Parses the openMP directive and returns the 
		number of threads.
	*/

	string numThreads;
	int start = directive.find("num_threads(");
	int end = directive.find(")");
	
	numThreads = directive.substr(start+12, directive.length());

	end = numThreads.find(")");
	if(end != std::string::npos){
		numThreads = numThreads.substr(0, end);
	}

	DEBUG_PRINT(numThreads);

	return numThreads;
}

void getForLoopSize(ifstream& file){
	string line;
	
	std::getline (file,line);
	int check = line.find("<");
	if(check != std::string::npos){
		string num = line.substr(check + 1, line.find(")"));
		string numFix = num.substr(0, num.find(";"));
		addNumSize(numFix);
	}
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
	
	// look for extension dot
	end = fileName.find(".");
	
	programName = fileName.substr(start, end);

	// check for dir path
	int loc = programName.find("/");
	
	while(loc != std::string::npos){
		//remove path
		programName = programName.substr(loc + 1, end);

		// check again for path
		loc = programName.find("/");
	}

	DEBUG_PRINT(programName);

	return programName;
}

void closeStruct(){
	/*
		Closes the pthreadStruct vector
	*/

	pthreadStruct.push_back("} thread_data_t;");
	pthreadStruct.push_back("");
}

void closeWorkFunc(){
	/*
		Closes the wordThreadFunc vector
	*/

	workFunc.push_back("  pthread_exit(NULL);");
	workFunc.push_back("}");
	workFunc.push_back("");
}

void closeCriticalSection(){
	/*
		Closes the critical template code
	*/

	workFunc.push_back("");
	workFunc.push_back("  pthread_mutex_unlock(&var); // unlock once you are done");
}

void closeMainFunc(){
	/*
		Closes the mainFunc pthread boilerplate code
		in the main function of new program.
	*/

	mainFunc.push_back("");
	mainFunc.push_back("  return EXIT_SUCCESS;");
	mainFunc.push_back("}");
}

void _closeSingleSection (){
	/*
		Closes the Single Section template code
		in the do_work function of new program.
	*/

	workFunc.push_back("  }");
}

void _closeForSection (){
	/*
		Closes the For Section template code
		in the do_work function of new program.
	*/	
	workFunc.push_back("  }");
}

void parseHeader(ifstream& file) {
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

void parseProgram (ifstream& in_stream){
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
		//check for function names and brackets
		bool isFunction = checkForFunction(line);


		if(!isFunction){

			//check for variable instant
			bool isVar = checkForVariables(line);
			if(!isVar){

				//check line for directives
				directive = checkForDirective(line);
				
				if (directive != "null") {	
					if(!FIRST_DIRECTIVE){
						FIRST_DIRECTIVE = true;
						addMainFuncTemplate();
					}

					_convertClauses(line);
					
					_convertDirective(directive, in_stream);
				} 
				else {
					bool hasConverted = _convertThreadIdentifiers(line);
					
					if(!hasConverted){
						mainFunc.push_back(line);
					}
				}
			}
		}	
	}
	_closeTemplates();
}

void parseVariableList(string list){
	/*
		This function takes a comma separated string
		and adds each variable into the privatre variable List;

	*/

	string variable;

	variable = list.substr(0, list.find(")"));

	privateVariableList[privateCount] = variable;
	
	DEBUG_PRINT(variable);
	
	privateCount++;

	addPrivateVariables(variable);
}

void saveProcessedProgram(string programName) {
	/* 
		Takes the processed header, pthreadStruct, workFunc 
		and mainFunc, then writes to file 
	*/

	string fileName = "OUTPUT/" + programName + "post1.cc";

	ofstream output_file(fileName.c_str());
    
    ostream_iterator<std::string> output_iterator(output_file, "\n");
    
    //write the program header to file
    copy(header.begin(), header.end(), output_iterator);
    
    //write the program functions to file
    copy(pthreadStruct.begin(), pthreadStruct.end(), output_iterator);
    
    copy(workFunc.begin(), workFunc.end(), output_iterator);
    
    copy(mainFunc.begin(), mainFunc.end(), output_iterator);
}

void _buildParCode (ifstream& file){
	/*
		Skips the open curly brace
		and then enters the directive
		reading loop.
	*/

	string line;

	//skip open curly brace
	std::getline (file,line);

	_readDirectiveProgram(file);
}

void _loadParforTemplate (){
	workFunc.push_back("  int tid = data->tid; ");
	workFunc.push_back("  int chunk_size = (SIZE / NUM_THREADS); ");
	workFunc.push_back("  int start = tid * chunk_size; ");
	workFunc.push_back("  int end = start + chunk_size;");
	workFunc.push_back("");
	workFunc.push_back("  for(i = start; i < end; i++)");
}

void _loadForTemplate (){
	/*
		loads basic omp for construct templates
		into the worker function of new code.
	*/


	workFunc.push_back("  int tid = data->tid; ");
	workFunc.push_back("  int chunk_size = (SIZE / NUM_THREADS); ");
	workFunc.push_back("  int start = tid * chunk_size; ");
	workFunc.push_back("  int end = start + chunk_size;");
	workFunc.push_back("");
	workFunc.push_back("  for(i = start; i < end; i++){");
}

void _loadCriticalTemplate (){
	/*
		loads critical construct templates
		into the worker function of new code.
	*/

	header.push_back("pthread_mutex_t var=PTHREAD_MUTEX_INITIALIZER;");
	header.push_back("");

	workFunc.push_back("  pthread_mutex_lock(&var);  // lock the critical section");
	workFunc.push_back("");	
}

void _loadSingleTemplate(){
	/*
		loads basic omp for construct templates
		into the worker function of new code.
	*/
	header.push_back("bool signalFlag = false;");
	header.push_back("");

	
	workFunc.push_back("  if(!signalFlag){");
	workFunc.push_back("    signalFlag = true;");
}

void _buildParForCode(ifstream& file){

	_loadParforTemplate();

	getForLoopSize(file);

	_readDirectiveProgram(file);
}

void _buildCriticalCode(ifstream& file){
	// builds Critical section tempalate code

	string line;

	_loadCriticalTemplate();

	std::getline (file,line);

	_readDirectiveProgram(file);
	
	closeCriticalSection();
}

void _buildForCode(ifstream& file){
	// Builds For Construct tempalate code
	string line;
	
	getForLoopSize(file);

	_loadForTemplate();

	std::getline (file,line);
	bool isPrivate = _checkIfPrivate(line);
	bool isFunc = checkForFunction(line);
	
	if(!isPrivate && !isFunc){
		workFunc.push_back(line);
	}

	if(isFunc){
		_readDirectiveProgram(file);
	}

	_closeForSection();
}

void _buildSingleCode(ifstream& file){
	// builds Single construct tempalate code
	_loadSingleTemplate();

	_readDirectiveProgram(file);

	_closeSingleSection();
}

bool _checkIfPrivate(string line){
	/*
		Checks to see if line contains
		known private variables. If one 
		is found, then we replace it with the
		pthread call, then we check a 2nd time
		and add newly appended line to the workFunc

	*/

	DEBUG_PRINT("Checking if Private");
	int i;
	for( i = 0; i < privateCount; i++){

		// first check for single var with space and semicolon
		int check = line.find(privateVariableList[i] + " ");
		int check2 = line.find(privateVariableList[i] + ";");

		if(check != std::string::npos && check != std::string::npos ){

			replace(line, privateVariableList[i] + " ", "data->" + privateVariableList[i]);

			// 2nd check when handling list.
			check = line.find(privateVariableList[i]+",");
			
			if(check != std::string::npos){
				string oldLine = line.substr(0, check);
				string newLine = line.substr(check, line.length());

				replace(newLine, privateVariableList[i]+ ",", "data->" + privateVariableList[i]+",");

				string processedLine = oldLine + newLine;
				DEBUG_PRINT(processedLine);
				
				int checkTID = processedLine.find("omp_get_thread_num(),");
				if(checkTID != std::string::npos){
					replace(processedLine, "omp_get_thread_num(),", "data->tid,");
				}

				line = processedLine;
			}
			
			workFunc.push_back(line);
			return true;
		}
	}
	return false;
}

void _readDirectiveProgram (ifstream& file) {
	/* 
		This is the 2nd file reading loop
		which is only entered once the first 
		#pragma directive has been found.

		This function checks for additional #pragma
		statements and also handles private and shared vars.
	*/

	string line;
	std::getline (file,line);

	// check for exit or end of function
	int check = line.find("return(0)");
	int check2 = line.find("}");

	while(check == std::string::npos && check2 == std::string::npos){
		// does the line contain a new function or
		// curly brackets?
		bool isFunction = checkForFunction(line);

		//does the line contain any private variables?
		bool isPrivate = _checkIfPrivate(line);

		if(isFunction == false && isPrivate == false){
			_checkIfPrivate(line);

			//check each line for #pragma directives
			string directive = checkForDirective(line);
			
			if (directive != "null") {
				//handle private, shared and num_thread clauses
				_convertClauses(line);

				//add pthread template code etc..
				_convertDirective(directive, file);

			}         
			else if(line != "}"){
				bool hasConverted = _convertThreadIdentifiers(line);
				
				if(!hasConverted){
					workFunc.push_back(line);	
				}
			}
			else {
				break;
			}
		}
		
		std::getline (file,line);
			
		check = line.find("return(0)");
		check2 = line.find("}");
	}
}

void _convertDirective(string resultName, ifstream& file) {
 	/* 
 		matches the resultName name to the proper directive,
 		Then it builds the new implementation to the program list. 
 	*/

	if(resultName == "par") {
		DEBUG_PRINT("par!!");
		_buildParCode(file);
	}
	else if(resultName == "parfor"){
		DEBUG_PRINT("Parfor!!");
		_buildParForCode(file);
	}
	else if(resultName == "critical") {
		DEBUG_PRINT("Critical!!");
		_buildCriticalCode(file);

	}
	else if(resultName == "for") {
		DEBUG_PRINT("For!!");
		_buildForCode(file);
	}
	else if(resultName == "single") {
		DEBUG_PRINT("Single!!");
		_buildSingleCode(file);
	}
}

void _checkPrivateClause(string line){
	/*
		Checks to see if there is a 
		omp private clause in given line.
		If found, it adds to global privateList,
		incrememnts the privateCount and then
		adds code to pthread struct.
	*/ 
	int check = line.find("private(");
	
	if(check != std::string::npos){
		string list = line.substr(check + 8, line.length());

		parseVariableList(list);		
	}
}

void _checkNumThreads(string line) {
	/* 
		Checks to see if the num_threads clauses is contained
		in the file line. If it is, then the number is parsed
	 	and added as a macro in the pthreads implementation. 
	*/

	int check = line.find("num_threads");

	if(check != std::string::npos){
		addNumThreads(line);
	}
}

bool _convertThreadIdentifiers(string line){
	/* 
		This function looks for all calls to the
	 	openmp_get_thread_num runtime library and
	 	replaces them with pthread implementation
	 */
	
	int check = line.find("omp_get_thread_num()");

	if(check != std::string::npos){
		replace(line, "omp_get_thread_num()", "data->tid");
		
		workFunc.push_back(line);
		
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

bool checkForVariables(string line) {
	/*
		This checks to see if variables
		are being instantiated.
	*/
	int check  = line.find("int ");
	int check2  = line.find("=");
	
	if (check != std::string::npos && check2 == -1){
		DEBUG_PRINT("FOUND VAR");
		replace(line, "\t", "");
		header.push_back(line);
		header.push_back("");
		return true;
	} 
	else {
		return false;
	}
}

void _convertClauses(string line){
	/*
		checks for the presences of num_threads, private(list)
		and shared(list) openmp clauses. If they are found, they
		are converted to pthread implementation and added to our 
		new program.
	*/

	_checkNumThreads(line);
	_checkPrivateClause(line);
	//_checkShared(line, header);

}

bool checkForFunction(string line){
	int check = line.find("int main(");
	int check2 = line.find("{");
	int check3 = line.find("}");

	if(check != std::string::npos){
		return true;
	}

	if(check2 != std::string::npos){
		return true;
	}

	if(check3 != std::string::npos){
		return true;
	}
	return false;
}

void _closeTemplates() {
	closeStruct();
	
	closeWorkFunc();

	closeMainFunc();
}

void runOMPScanner(string programName){
	DEBUG_PRINT("RUNNING SCANNER");
	runScanner(programName);
}

int main (int argc, char *argv[]) {	
	string programName, line, result;
	string fileName = argv[1];
	ifstream in_stream;

	in_stream.open(argv[1]);

	programName = getProgramName(fileName);

	loadPthreadTemplates();

	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");

	printf("1) Processing OpenMP program.... ");
	cout << programName << endl;
	
	parseHeader(in_stream);
	
	parseProgram(in_stream);

	printf("2) Writing intermediate pthread program to file....\n");
	
	saveProcessedProgram(programName);

	printf("3) Running OMP Scanner\n");

	runOMPScanner(programName);

	printf("All done!! See OUTPUT directory for new file\n");
 	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");
	
	in_stream.close();
	
	return 0;	
}