// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <string>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <vector>
#include <stdio.h>
#include <sys/timeb.h>

class Argument {
public:
	bool isPID = false;
	uint16_t pid = 0;
	bool force = 0;
	std::string filepath = "";
	std::string cmdline = "";
	Argument(bool a, const char* b, const char* c);
	~Argument();
};

std::string defineHelp(const char*); // Sets help text
bool isValidPID(const char*); // Checks if PID is valid
int64_t calcDuration(const char*); // Calculates time duration to use in milliseconds (string to int64_t)
std::string calcDurStr(int64_t); // Reverse calculation of time duration in milliseconds (uint64_t to string)
int doPIDStuffs(uint16_t, bool); // Actual brains of the PID termination work
int newProcess(std::string, std::string); // Opens new process when called
std::string processFirst(const char*); // Parse escape characters from first command line arg

#endif //PCH_H
