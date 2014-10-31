#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;

#define DEBUG true

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

string checkForDirective (string line) {

	int check = line.find("#pragma omp critical");
	if(check != std::string::npos) {
		return "critical";
	}

	check = line.find("#pragma omp for");
	if(check != std::string::npos) {
		return "for";
	}

	check = line.find("#pragma omp parallel for ");
	if(check != std::string::npos) {
		return "parfor";
	}

	check = line.find("#pragma omp parallel ");
	if(check != std::string::npos) {
		return "par";
	}


	return "null";
}