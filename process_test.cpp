#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <memory>
#include "Process.hpp"

int main(int argc, char *argv[])
{
	using namespace std;
	
	if (argc <= 1) {
		cerr << "Error: a process name must be given" << endl;
		return(EXIT_FAILURE);
	}
	
	vector<string> args;

	for(int nn=1; nn<argc; ++nn)
	{
	cerr << "Arg: " << argv[nn] << endl;
	args.push_back(argv[nn]);
	}

	string output;

	try {
		Process myproc(args,true);
	
		for(int n=0; n<5; ++n) {
			stringstream ss;
			ss << "1+" << n << endl;
			cerr << "calling write with line=" << ss.str() << "END" << endl;
			myproc.write(ss.str());
			output = myproc.read();
			cerr << "output from process: " << output << "END" << endl;
		}
		cerr << "About to destroy process" << endl;
		sleep(3);
	} catch(Process::ProcessException e) {
		if (e.fatal) cerr << "FATAL Error: ";
		else cerr << "Nonfatal Error: ";
		cerr << e.errDesc << endl;
	}
	cerr << "Process object destroyed" << endl;

	cerr << "Program exiting.  Confirm child process has been cleaned up." << endl;
	sleep(3);
	return(EXIT_SUCCESS);
}

