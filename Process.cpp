#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Process.hpp"
#include <algorithm>
using namespace std;

Process::Process(const vector<string>& argss, bool verbose) :
	verbose(verbose),
	m_name(argss[0]),
	m_pid((pid_t)NULL),
	m_writepipe {-1,-1},
	m_readpipe {-1,-1},
	m_pwrite((FILE*)NULL),
	m_pread((FILE*)NULL)
{
	if (verbose)
		cerr << "Process(): constructor in verbose mode." << endl;
	
	// This will hold the description of any non-fatal errors that occur
	string nonfatalErrorDesc = "";
		
	if (pipe(m_writepipe) < 0) {
		perror("Process(): pipe(m_writepipe)");
		throw ProcessException("Process(): pipe(m_writepipe) fails", true);
	}
	if (pipe(m_readpipe) < 0) {
		perror("Process(): pipe(m_readpipe)");
		// Previous file descriptors must be closed
		if (close(m_writepipe[0]) < 0) perror("Process(): close(m_writepipe[0])");
		if (close(m_writepipe[1]) < 0) perror("Process(): close(m_writepipe[1])");
		// The initial error will be the only exception thrown
		throw ProcessException("Process(): pipe(m_readpipe) fails", true);
	}
	
	m_pid = fork();
	if (m_pid < 0) { // PID of -1 indicates error from fork()
		perror("Process(): fork");
		throw ProcessException("Process(): fork reports an error", true);
	}
	
	else if (m_pid == 0) { // PID of 0 indicates child process
		// Close the unused file descriptors
		if ( (close(PARENT_READ) < 0) || (close(PARENT_WRITE) < 0) ) {
			// If errors occurr in either close() they will be non-fatal
			perror("Process(): child close parent descriptors");
			nonfatalErrorDesc.append("Process(): error closing parent descriptors\n");
		}
		// Transfer the child file descriptors to stdin and stdout
		if ( (dup2(CHILD_WRITE,1) < 0) || (dup2(CHILD_READ,0) < 0) ) {
			// Errors in either duplication ARE fatal
			perror("Process(): child descriptor duplication");
			// Previous file descriptors must be closed
			if (close(m_writepipe[0]) < 0) perror("Process(): close(m_writepipe[0])");
			if (close(m_writepipe[1]) < 0) perror("Process(): close(m_writepipe[1])");
			if (close(m_readpipe[0]) < 0) perror("Process(): close(m_readpipe[0])");
			if (close(m_readpipe[1]) < 0) perror("Process(): close(m_readpipe[1])");
			// The initial error will be the only exception thrown
			throw ProcessException("Process(): child descriptor duplication fails", true);
		}
		if ( (close(CHILD_WRITE) < 0) || (close(CHILD_READ) < 0) ) {
			// If errors occurr in either close() they will be non-fatal
			perror("Process(): child close child descriptors");
			nonfatalErrorDesc.append("Process(): error closing child descriptors\n");
		}
		// Build an array of c-strings to pass to execvp:
		vector<const char*> args;
		transform(argss.begin(),argss.end(), back_inserter(args), [](string s)
			   {
			   return s.c_str();
			   } );
		args.push_back( NULL );
		// Replace child process with target process
		execvp(args[0], const_cast<char**>(&args[0]));
		// If execvp returns, there's been an error
		perror("Process(): execvp");
		throw ProcessException("Process(): execvp returns indicating failure", true);
	}
	
	else { // PID nonzero indicates parent process
		if (verbose)
			cerr << "Process(): forked child " << m_name << " to PID " << m_pid << endl;
		if ( (close(CHILD_WRITE) < 0) || (close(CHILD_READ) < 0) ) {
			// If errors occurr in either close() they will be non-fatal
			perror("Process(): parent close child descriptors");
			nonfatalErrorDesc.append("Process(): error closing child descriptors\n");
		}
		// Open the child read/write file descriptors as file streams
		// errors in either open ARE fatal
		m_pread = fdopen(PARENT_READ, "r");
		m_pwrite = fdopen(PARENT_WRITE, "w");
		if (m_pread == NULL) {
			perror("Process(): open parent read descriptor");
			// The other stream must be closed
			if (fclose(m_pwrite) != 0) perror("Process(): close parent write descriptor");
			throw ProcessException("Process(): opening parent read descriptor fails", true);
		} else if (m_pwrite == NULL) {
			perror("Process(): open parent write descriptor");
			// The other stream must be closed
			if (fclose(m_pread) != 0) perror("Process(): close parent read descriptor");
			throw ProcessException("Process(): opening parent write descriptor fails", true);
		}
	}
	
	// If a non-fatal error occurred, throw the exception now
	if (!nonfatalErrorDesc.empty())
		throw ProcessException(nonfatalErrorDesc, false);
}

Process::~Process() {
	if (verbose) {
		cerr << "~Process(): Destructor in verbose mode" << endl
			<< "~Process(): Killing process " << m_pid << endl;
	}
	
	// This will hold the description of any non-fatal errors that occur
	string nonfatalErrorDesc = "";
	
	// Close the parent file descriptors
	// Errors in either close are not fatal
	if (fclose(m_pwrite) != 0) {
		perror("~Process(): close parent write descriptor");
		nonfatalErrorDesc.append("~Process(): closing parent write descriptor fails\n");
	}
	if (fclose(m_pread) != 0) {
		perror("~Process(): close parent read descriptor");
		nonfatalErrorDesc.append("~Process(): closing parent read descriptor fails\n");
	}
	
	char pidString[10];
	sprintf(pidString, "%i", m_pid);
	// Terminate the child process
	// Errors here are not fatal (the user can handle it)
	if (kill(m_pid, SIGTERM) < 0) {
		perror("~Process(): kill child with SIGTERM");
		nonfatalErrorDesc.append("~Process(): kill sending SIGTERM to PID ");
		nonfatalErrorDesc.append(pidString);
		nonfatalErrorDesc.append(" fails\n");
	}
	// Wait for the child to terminate
	if (waitpid(m_pid, NULL, 0) < 0) {
		perror("~Process(): waitpid after SIGTERM sent");
		nonfatalErrorDesc.append("~Process(): waitpid for PID ");
		nonfatalErrorDesc.append(pidString);
		nonfatalErrorDesc.append(" fails\n");
	}
	
	// If a non-fatal error occurred, throw the exception now
	if (!nonfatalErrorDesc.empty())
		throw ProcessException(nonfatalErrorDesc, false);
}

void Process::write(const std::string& line) {
	if (fputs(line.c_str(), m_pwrite) < 0)
		throw ProcessException("Process::write(): writing to parent write descriptor fails", false);
	if (fflush(m_pwrite) < 0) {
		perror("Process::Write() fflush parent write descriptor");
		throw ProcessException("Process::write(): flushing parent write descriptor fails", false);
	}
}

std::string Process::read() {
	string line;
	char *mystring = NULL;
	size_t n;
	if (getline(&mystring, &n, m_pread) < 0) {
		perror("Process::read(): getline");
		throw ProcessException("Process::read(): getline fails", false);
	}
	line = mystring;
	// The memory allocated by getline must be freed
	free(mystring);
	return line;
}

