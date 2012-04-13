#include "Process.hpp"
#include <stdlib.h>
#include <sys/wait.h>
using namespace std;

Process::Process(const std::vector<char*>& args, bool verbose=false) {
	if (verbose) {
		cerr << "Process: Process constructor in verbose mode." << endl;
		verbose = true;
	}
	else
		verbose = false;
    m_pid = fork();
    if (m_pid == 0) { // PID of 0 indicates child process
        // Build an array of c-strings to pass to execvp:
        const char **argsC = new const char* [args.size()];
        int i=0;
        for (i=0; i<args.size(); i++) {
            if (verbose) {
                cerr << "Process: Copying argument " << i << ": ";
                if (args[i]==NULL) cerr << "NULL" << endl;
                else cerr << args[i] << endl;
            }
            argsC[i] = args[i];
        }
        execvp(args[0], (char **) argsC); // Replace child process with target process
        delete argsC;
    }
}

Process::~Process() {
    if (verbose) {
	    cerr << "Process: Destructor in verbose mode" << endl;
	    cerr << "Process: Killing process " << m_pid << endl;
	}
	kill(m_pid, SIGTERM);
}
