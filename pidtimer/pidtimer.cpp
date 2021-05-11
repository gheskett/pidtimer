/**
 * Written by Gregory Heskett (gheskett)
 *
 * This is meant to be a command line application with the focus of terminating any number of processes after a certain length of time.
 * An example of a useful application for this would include a music player that plays music or sounds overnight but needs to be terminated by morning.
 *
 * This application can also be used to open/execute files with arguments after the set amount of time.
 * An example of this could be to play an obnoxious audio file as an alarm.
 *
 */


 /**
 * Usage: pidtimer.exe <[days]:[hours]:[minutes]:[seconds].[milliseconds]> [OPTIONAL ARGS] ...
 * 
 * OPTIONAL ARGUMENTS:
 *		-k, --kill	 <PID>                   (Gracefully terminate a process)
 *		-f, --force	 <PID>                   (Force close a process)
 *		-o, --open	 <file path + cmd args>  (Use escape characters for quotes!)
 *		-r, --remind                         (Open pidtimer.jpg or pidtimer.png)
 * 
 * USAGE EXAMPLES:
 *  pidtimer.exe 3:07:42:13.962 --kill 3479
 *  pidtimer.exe 420:69 -k 42069 -o "\"funky music.mp3\"" -f 19573
 *  pidtimer.exe 1:30:00 -o "send_message.exe \"This is a message string!\""
 * 
 */

#include "pch.h"

using namespace std;

// Arguments constructor
Argument::Argument(bool a, const char* b, const char *c) {
	isPID = a;
	if (isPID) {
		pid = (uint16_t) atoi(b);
		force = (atoi(c) != 0);
	}
	else {
		filepath = b;
		cmdline = c;
	}
}

// Arguments deconstructer
Argument::~Argument() {

}

int main(int argc, char **argv)
{
	system(" ");  // Enable VT100 support

	int64_t duration; // Time to wait before execution of arguments
	vector<Argument> arguments; // list of arguments to be executed once timer expires

	if (argc < 3) {
		// Invalid argument syntax
		printf(defineHelp(argv[0]).c_str());
		return 1;
	}

	try {
		duration = calcDuration(argv[1]);
		if (duration < 0) {
			throw exception();
		}
	}
	catch (exception) {
		printf("ERROR: Invalid duration value!\n\n");
		printf(defineHelp(argv[0]).c_str());
		return 1;
	}

	for (int i = 2; i < argc; i += 2) {
		// kill PID argument
		if (!strcmp(argv[i], "-k") || !strcmp(argv[i], "--kill") || !strcmp(argv[i], "-f") || !strcmp(argv[i], "--force")) {
			if (argc == i + 1) {
				// Invalid argument syntax
				printf(defineHelp(argv[0]).c_str());
				return 1;
			}

			try {
				if (!isValidPID(argv[i + 1])) {
					printf("WARNING: Invalid PID value: %s\n", argv[i + 1]);
					continue;
				}

				HANDLE process = OpenProcess(PROCESS_TERMINATE, true, (uint16_t) atoi(argv[i + 1]));
				if (!process) {
					printf("WARNING: Process doesn't exist or cannot be accessed for termination: %d\n", (uint16_t) atoi(argv[i + 1]));
					CloseHandle(process);
					continue;
				}
				CloseHandle(process);

				if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--force")) {
					arguments.emplace_back(true, argv[i + 1], "1");
				}
				else {
					arguments.emplace_back(true, argv[i + 1], "0");
				}
			}
			catch(exception) {
				printf("WARNING: Unable to parse PID of argument %d!\n", i + 2);
				continue;
			}
		}
		// Open new process argument
		else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--open")) {
			if (argc == i + 1) {
				// Invalid argument syntax
				printf(defineHelp(argv[0]).c_str());
				return 1;
			}

			string filepath = processFirst(argv[i + 1]);
			FILE* file;
			fopen_s(&file, filepath.c_str(), "r");
			if (!file) {
				printf("WARNING: Cannot find/open file: %s\n", filepath.c_str());
				continue;
			}
			fclose(file);

			arguments.emplace_back(false, filepath.c_str(), argv[i + 1]);
		}
		// Open pidtimer.jpg or pidtimer.png; used to serve essentially as an image reminder shortcut
		else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--remind")) {

			char buf[MAX_PATH];
			GetModuleFileNameA(NULL, buf, MAX_PATH);

			string filepath = buf;
			size_t ext = filepath.find_last_of(".");
			if (ext != string::npos) {
				filepath = filepath.substr(0, ext);
			}

			string filepathImg = filepath + ".jpg";

			FILE* file;
			fopen_s(&file, filepathImg.c_str(), "r");
			if (!file) {
				filepathImg = filepath + ".png";
				fopen_s(&file, filepathImg.c_str(), "r");
				if (!file) {
					printf("WARNING: Cannot find/open file: %s\n", filepath.c_str());
					i--;
					continue;
				}
			}
			fclose(file);

			filepathImg = "\"" + filepathImg + "\"";

			arguments.emplace_back(false, filepathImg.c_str(), filepathImg.c_str());

			i--;
		}
		else {
			// Invalid argument syntax
			printf("ERROR: Argument %d is not valid!\n", i + 1);
			printf(defineHelp(argv[0]).c_str());
			return 1;
		}
	}

	if (arguments.size() == 0) {
		// FAIL
		printf("ERROR: No valid arguments were entered!\n\n");
		printf(defineHelp(argv[0]).c_str());
		return 1;
	}

	// Code to display timer over stdout (exact times are calculated quickly to compensate for processing delays)
	if (duration >= 1000) {
		struct timeb t_start;
		struct timeb t_current;
		ftime(&t_start);
		printf("\nTime remaining: %s", calcDurStr(duration).c_str());
		Sleep((DWORD) (duration % 1000));
		ftime(&t_start);
		duration -= duration % 1000;
		int64_t diff;
		int64_t total = 0;
		int64_t sleepTime;
		while (true) {
			if (total >= duration) {
				printf("\x1B[2K\rTime remaining: %s\n\n", calcDurStr(duration - total).c_str());
				break;
			}
			ftime(&t_current);
			diff = (int64_t)(1000.0 * (int64_t) (t_current.time - t_start.time) + ((short) t_current.millitm - (short) t_start.millitm));
			printf("\x1B[2K\rTime remaining: %s", calcDurStr(duration - total).c_str());
			sleepTime = total - diff + 1000;
			if (sleepTime > 0)
				Sleep((DWORD) sleepTime);
			total += 1000;
		}
	}
	else {
		Sleep((DWORD) duration);
	}

	// Do what?
	for (Argument& i : arguments) {
		if (i.isPID) {
			doPIDStuffs(i.pid, i.force);
		}
		else {
			newProcess(i.filepath, i.cmdline);
		}
	}
	
	return 0;
}

// Sets help text
string defineHelp(const char *arg) {
	string str = arg;
	str = str.substr(str.find_last_of("\\/") + 1);

	string s1 = "\nUsage: ";
	string s2 = " <[days]:[hours]:[minutes]:[seconds].[milliseconds]> [OPTIONAL ARGS] ...\n\n"
		"OPTIONAL ARGUMENTS:\n"
		"  -k, --kill   <PID>                   (Gracefully terminate a process)\n"
		"  -f, --force  <PID>                   (Force close a process)\n"
		"  -o, --open   <file path + cmd args>  (Use escape characters for quotes!)\n"
		"  -r, --remind                         (Open pidtimer.jpg or pidtimer.png)\n\nUSAGE EXAMPLES:\n  ";
	string s3 = " 3:07:42:13.962 --kill 3479\n  ";
	string s4 = " 420:69 -k 42069 -o \"\\\"funky music.mp3\\\"\" -f 19573\n  ";
	string s5 = " 1:30:00 -o \"send_message.exe \\\"This is a message string!\\\"\"\n";

	return s1 + str + s2 + str + s3 + str + s4 + str + s5;
}

// Checks if PID is valid
bool isValidPID(const char *arg) {
	int64_t pid = atol(arg);
	if (pid < 1 || pid > 65535)
		return false;
	return true;
}

// Calculates time duration to use in milliseconds (string to int64_t)
int64_t calcDuration(const char *arg) {
	string dur = arg;
	int64_t days = 0;
	int64_t hours = 0;
	int64_t minutes = 0;
	int64_t seconds = 0;
	int64_t milliseconds = 0;

	if (dur.find("::") != string::npos || dur.find(":.") != string::npos)
		return -1;

	size_t period = dur.find(".");
	if (period != string::npos) {
		for (size_t i = dur.length(); i < period + 4; ++i)
			dur += "0";

		milliseconds = atol(dur.substr(period + 1, 3).c_str());
		if (milliseconds < 0)
			return -1;
		dur = dur.substr(0, period);
	}

	int colonCount = 0;
	size_t index = 0;
	while (true) {
		index = dur.find(":", index);
		if (index == string::npos)
			break;

		colonCount++;
		index += 1;
	}
	if (colonCount > 3)
		return -1;

	index = 0;
	if (colonCount == 3) {
		size_t indLast = index;
		index = dur.find(":", index) + 1;
		days = atol(dur.substr(indLast, index - indLast - 1).c_str());
		days = days * 1000 * 60 * 60 * 24;
		if (days < 0)
			return -1;
		colonCount--;
	}
	if (colonCount == 2) {
		size_t indLast = index;
		index = dur.find(":", index) + 1;
		hours = atol(dur.substr(indLast, index - indLast - 1).c_str());
		hours = hours * 1000 * 60 * 60;
		if (hours < 0)
			return -1;
		colonCount--;
	}
	if (colonCount == 1) {
		size_t indLast = index;
		index = dur.find(":", index) + 1;
		minutes = atol(dur.substr(indLast, index - indLast - 1).c_str());
		minutes = minutes * 1000 * 60;
		if (minutes < 0)
			return -1;
		colonCount--;
	}
	seconds = atol(dur.substr(index).c_str());
	seconds *= 1000;
	if (seconds < 0)
		return -1;

	return days + hours + minutes + seconds + milliseconds;
}

// Reverse calculation of time duration in milliseconds (int64_t to string)
string calcDurStr(int64_t duration) {
	string ret = "";
	duration /= 1000;
	int64_t days = duration / 86400;
	int64_t hours = (duration / 3600) % 24;
	int64_t minutes = (duration / 60) % 60;
	int64_t seconds = duration % 60;

	ret += to_string(days);
	ret += ":";
	if (hours < 10)
		ret += "0";
	ret += to_string(hours);
	ret += ":";
	if (minutes < 10)
		ret += "0";
	ret += to_string(minutes);
	ret += ":";
	if (seconds < 10)
		ret += "0";
	ret += to_string(seconds);

	return ret;
}

// Actual brains of the PID termination work
int doPIDStuffs(uint16_t pid, bool forceClose) {
	HANDLE process = OpenProcess(PROCESS_TERMINATE, true, pid);
	if (!process) {
		printf("PID %d: Process already closed!\n", pid);
		CloseHandle(process);
		return 2;
	}

	int ret = 0;
	if (forceClose) {
		if (!TerminateProcess(process, 1)) {
			printf("PID %d: Process already closed!\n", pid);
			ret = 2;
		}
		else {
			printf("PID %d: Process terminated successfully!\n", pid);
		}
		CloseHandle(process);
		return ret;
	}

	CloseHandle(process);

	string sysCommand = "TASKKILL /PID " + to_string(pid);

	return system(sysCommand.c_str());
}

// Opens new process when called
int newProcess(string filepath, string cmdline) {
	STARTUPINFOA sinfo = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pinfo;
	string args = "cmd.exe /s /c \"" + cmdline + "\"";
	if (!CreateProcessA(NULL, (LPSTR) args.c_str(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &sinfo, &pinfo)) {
		printf("WARNING: New process failed to open: %s\n", filepath.c_str());
		return 1;
	}

	printf("%s: Attempted to open/execute successfully!\n", filepath.c_str());
	CloseHandle(pinfo.hProcess);
	CloseHandle(pinfo.hThread);
	return 0;
}

// Parse escape characters from first command line arg
string processFirst(const char* in) {
	size_t pos = 0;
	string str = in;
	size_t pos2 = str.find('"', 0);

	while (true) {
		if ((pos = str.find(' ', pos)) != std::string::npos && (pos < pos2 || pos2 == string::npos)) {
			if (pos == 0) {
				str = str.substr(1);
				continue;
			}

			// Escape characters don't work with spaces in Windows...
			/*
			if (str[pos - 1] == '\\') {
				str.replace(pos - 1, 2, " ");
				continue;
			}
			*/

			str = str.substr(0, pos);
			break;
		}
		else if (pos == string::npos) {
			break;
		}
		else { // Open quote parsing
			pos = pos2;
			pos2 = str.find('"', pos + 1);
			if (pos2 == string::npos) {
				pos++;
				continue;
			}
			str.replace(pos2, 1, "");
			str.replace(pos, 1, "");
			pos = pos2 - 2;
			pos2 = str.find('"', 0);
		}
	}

	pos2 = str.find('"', 0);
	while (pos2 != string::npos) {
		str.replace(pos2, 1, "");
		pos2 = str.find('"', 0);
	}

	return str;
}
