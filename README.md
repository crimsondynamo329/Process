# Process Library
This library allows for easy spawning and control of processes from a C++ program.

## HW6 Discussion Questions
* Is this a good use of Object Oriented design? Why or why not?

	**Yes, this seems like a very good use of object-oriented design.  It greatly reduces the use of duplicated code, increases modularity, improves code readability, and supports other areas of UNIX and object-oriented design philosophies.**
* There are different possible desired behaviors for what happens when the Process object goes out of scope:
	* The Process object waits until the child process has finished on its own (this has the effect of stalling the main program at the point at which the process object goes out of scope).
	* The Process object waits for a specified time period for the child to finish on its own and then sends a TERM or KILL signal.
	* The Process object sends a TERM or KILL signal imediately if the child process has not already finished at the time the Process destructor is called (when the object goes out of scope)
	* ...
	
	How might you modify your Process class to allow the programmer an easy way to use different destruction policies?
	
	**It would be trivial to overload the destructor, but this is not possible (that I know of) or recommended in C++.  Destructors are almost never called explicitly so even if it were possible, it would be unorthodox.  To get around this, an option could be passed to the contructor to indicate the desired destruction policy.  The destructor would then check this option and continue accordingly.  In addition, a member function could change this option in case the desired destruction policy changes after the object is contructed.**
* How might you handle the case when a child process terminates before the parent process destroys it? I.e. how might the parent be signaled when a child terminates, and then read in any data it has written to its output.

	**The SIGCHLD signal seems to be perfect for this application.  From what I understand, SIGCHLD is sent to the parent when any child process terminates.  The parent's signal handler could then call any appropriate read function, cleanup function, etc that is needed.  Some flag in the Process object would probably have to be set to indicate that the process is no longer usable.**
* Feel free to implement any of these features for extra credit, but if they prevent the code from compiling or working as expected, then be sure to put them in a separate branch. Also, explicitly mention in your README file what you have implemented.
