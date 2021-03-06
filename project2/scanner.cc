#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;

vector<string> program;


bool replaceRemove(std::string& str, const std::string& from, const std::string& to) {
    // this function replaces part of a string with another string

    size_t start_pos = str.find(from); // find start
    if(start_pos == std::string::npos) 
        return false;
    str.replace(start_pos, from.length(), to); //replace
    return true;
}

bool checkforOMPLibraryRoutine(string line){
	/* 
		This function looks for all calls to the
	 	openmp_get_thread_num runtime library and
	 	replaces them with pthread implementation
	 */
	
	int check = line.find("omp_get_thread_num();");


	if(check != std::string::npos){
		replaceRemove(line, "omp_get_thread_num()", "data->tid");
		
		program.push_back(line);
		
		return true;
	}

	return false;
}

void saveNewProgram(string programName){
	/* 
		Takes the processed header, pthreadStruct, workFunc 
		and mainFunc, then writes to file 
	*/

	string fileName = "OUTPUT/" + programName + "post.cc";

	ofstream output_file(fileName.c_str());
    
    ostream_iterator<std::string> output_iterator(output_file, "\n");
    
    //write the program header to file
    copy(program.begin(), program.end(), output_iterator);
}

void deleteOldProgram(string programName){
	/* 
		takes old intermediate file and deletes it
	*/

	if( remove( programName.c_str() ) != 0 )
    	perror( "Error deleting file" );
}


void runScanner(string programName){
	ifstream in_stream;

	string fileName = "OUTPUT/"+programName+"post1.cc";

	in_stream.open(fileName.c_str());

	string line;

	while(!in_stream.eof())
	{
		std::getline (in_stream,line);

		bool updated = checkforOMPLibraryRoutine(line);
		
		if(!updated){
			program.push_back(line);
		}

	}

	saveNewProgram(programName);

	deleteOldProgram(fileName);

}

