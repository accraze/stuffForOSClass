#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdio.h>

using namespace std;
bool replace(std::string& str, const std::string& from, const std::string& to);


bool replace(std::string& str, const std::string& from, const std::string& to) {
    // this function replaces part of a string with another string

    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int main ()
{
	string line = "printf( \"Thread %d: local_sum = %d, sum = %d\\n\", omp_get_thread_num(), local_sum, sum );";
	bool answer = replace(line, "omp_get_thread_num()", "data->id");
	cout << line << endl;
	return(0);
}