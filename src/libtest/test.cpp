// ----------------------------------------------------------------------------
// Copyright (c) Solidra LLC. All rights reserved.
//
// Author: Paul Glendenning

#include <iostream>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
//#include <boost/log/trivial.hpp>
#include "test.h"


namespace Test {

// ----------------------------------------------------------------------------

struct TestManager::TestCaseHolder
{
    TestManager::TestCase* theTest;
    const char* name;

    void Run() {
        if (theTest) theTest();
    }

    TestCaseHolder(TestManager::TestCase* m=0, const char* n=0): theTest(m), name(n) {}
};


TestManager::TestManager() {
}


TestManager::~TestManager() {
}


// Linux can use good old fork. We need a few extras to add timeouts to waitpid.
TEST_API int TestManager::_wakeupPipe[2] = {0};


bool TestManager::IsRunning()
{
	return _wakeupPipe[1] != 0;
}


std::vector<std::unique_ptr<TestManager::TestCaseHolder>>& TestManager::GetTests() {
	// C++ does not guarantee construction order of class statics.
	// This is the standard trick to solve that issue since it is guaranteed
    // that testHolders will be constructed correctly before the first call to
    // this function.
	static std::vector<std::unique_ptr<TestCaseHolder>> testHolders;
	return testHolders;
}


const TestManager::TestCaseHolder* TestManager::AddTestCase(TestManager::TestCase* test, const char* class_name)
{
    // Thread safe since this function is called before main so no threads
    if (test)
        GetTests().push_back(std::move(std::unique_ptr<TestCaseHolder>(new TestCaseHolder(test, class_name))));   
    return GetTests().back().get();
}


size_t TestManager::GetTestCount()
{
	return GetTests().size();
}


int TestManager::RunInCurrentProcess()
{
	int failed = 0;

	if (IsRunning()) {
		std::cerr << "FATAL ERROR - recursive call to TestManager::Run()\n";
		exit(1);
	}
	_wakeupPipe[1] = 1; // So IsRunning() returns true

	std::vector<std::unique_ptr<TestCaseHolder>>& allTests = GetTests();

	for (unsigned i = 0; i < allTests.size(); ++i) {
		std::cout << "TEST #" << i << " - " << allTests[i]->name;
		try {
			allTests[i]->Run();
			std::cerr << "PASSED" << std::endl << std::flush;
		} catch(TestException& e) {
			++failed;
			std::cerr << "FAILED in file \"" << e._module << "\", line " << e._line <<  std::endl << std::flush;
		} catch(std::exception& e) {
			++failed;
			std::cerr << "FAILED with std::exception- " << e.what() <<  std::endl << std::flush;
		} catch(...) {
			++failed;
			std::cerr << "FAILED with unknown exception" <<  std::endl << std::flush;
		}
	}
	_wakeupPipe[1] = 0;
	std::cerr << std::endl;
	return failed;
}


bool TestManager::RunTest(const std::string& testName)
{
	int failed = 0;

	if (IsRunning()) {
		std::cerr << "FATAL ERROR - recursive call to TestManager::Run()\n";
		exit(1);
	}
	_wakeupPipe[1] = 1; // So IsRunning() returns true

	std::vector<std::unique_ptr<TestCaseHolder>>& allTests = GetTests();

	for (unsigned i = 0; i < allTests.size(); ++i) {
		if (testName != allTests[i]->name)
			continue;
		std::cout << "TEST #" << i << " - " << allTests[i]->name;
		try {
			allTests[i]->Run();
			std::cerr << "PASSED" << std::endl << std::flush;
		} catch(TestException& e) {
			++failed;
			std::cerr << "FAILED in file \"" << e._module << "\", line " << e._line <<  std::endl << std::flush;
		} catch(std::exception& e) {
			++failed;
			std::cerr << "FAILED with std::exception- " << e.what() <<  std::endl << std::flush;
		} catch(...) {
			++failed;
			std::cerr << "FAILED with unknown exception" <<  std::endl << std::flush;
		}
	}
	_wakeupPipe[1] = 0;
	std::cerr << std::endl;
	return 0 == failed;
}


void TestManager::SignalWakeup(int)
{
	static char ch = 0;
	write(_wakeupPipe[1], &ch, 1);
}


int TestManager::RunInForkedProcess(unsigned timeoutInSecs)
{
	if (IsRunning()) {
		std::cerr << "FATAL ERROR - recursive call to TestManager::Run()\n";
		exit(1);
	}

	int failed = 0;
	struct sigaction action = { 0 };
	struct sigaction actionPrev = { 0 };

	// Use select on our pipe to add a timeout to waitpid().
	if (pipe(_wakeupPipe) == -1) {
		std::cerr << "FATAL ERROR - could not create wake up pipe in TestManager::RunInForkedProcess()\n";
		exit(1);
	}
	fcntl(_wakeupPipe[0], F_SETFL, fcntl(_wakeupPipe[0],F_GETFL)|O_NONBLOCK);
	fcntl(_wakeupPipe[1], F_SETFL, fcntl(_wakeupPipe[1],F_GETFL)|O_NONBLOCK);

	// Catch SIGCHLD
	action.sa_handler = SignalWakeup;
	action.sa_flags |= 0;
	sigaction(SIGCHLD, &action, &actionPrev);

	std::vector<std::unique_ptr<TestCaseHolder>>& allTests = GetTests();

	for (unsigned i = 0; i < allTests.size(); ++i) {
		std::cout << "TEST #" << i << " - " << allTests[i]->name;
		pid_t childId = fork();
		if (childId == 0) {
			// Child process - close pipe and remove signal handler
			close(_wakeupPipe[0]);
			close(_wakeupPipe[1]);
			sigaction(SIGCHLD, &actionPrev, NULL);

			try {
				allTests[i]->Run();
				std::cerr << "PASSED" << std::endl << std::flush;
				exit(0);
            } catch(TestException& e) {
                ++failed;
                std::cerr << "FAILED in file \"" << e._module << "\", line " << e._line <<  std::endl << std::flush;
            } catch(std::exception& e) {
                ++failed;
                std::cerr << "FAILED with std::exception- " << e.what() <<  std::endl << std::flush;
            } catch(...) {
                ++failed;
                std::cerr << "FAILED with unknown exception" <<  std::endl << std::flush;
            }
			exit(1);

		} else {
			// Parent process
			int err = 0;
			int status = 0;
			auto timestamp = std::chrono::system_clock::now();
			do {
				// Ensure we don't exceed our timeout
				std::chrono::duration<double> diff = std::chrono::system_clock::now() - timestamp;

				fd_set  rfds;
				timeval tv = { 0 };

				if (diff.count() > timeoutInSecs) {
					std::cerr << "FAILED on timeout\n" << std::flush;
					kill(childId, SIGKILL);
					++failed;
					break;
				}

				tv.tv_sec = timeoutInSecs - diff.count();
				tv.tv_usec = 0;
				FD_ZERO(&rfds);
				FD_SET(_wakeupPipe[0], &rfds);

				err = select(_wakeupPipe[0]+1, &rfds, NULL, NULL, &tv);
				if (err > 0) {
					// Clear wakeup pipe
					static char dummy[256];
					while (read(_wakeupPipe[0], dummy, sizeof(dummy)) > 0)
					    /* do nothing */;
				}
				else if (0 == err) {
					std::cerr << "FAILED on timeout\n" << std::flush;
					kill(childId, SIGKILL);
					++failed;
					break;
				}

				// Check waitpid result
				err = waitpid(childId, &status, WNOHANG|WUNTRACED|WCONTINUED);
				if (err == 0) {
					// loop
				} else if (err < 0) {
					std::cerr << "FAILED on waitpid\n" << std::flush;
					kill(childId, SIGKILL);
					++failed;
				} else if (WIFEXITED(status)) {
					// This may not the best way to check if the failed test outputed a message
					// since abort() and possibly other functions return a fixed status code.
					if (1 == WEXITSTATUS(status))
						++failed;
					else if (WEXITSTATUS(status))
					    std::cerr << "EXITED with code " << WEXITSTATUS(status) << std::endl << std::flush;
				} else if (WIFSIGNALED(status)) {
					std::cerr << "KILLED by signal "  << WTERMSIG(status) << std::endl << std::flush;
					++failed;
				} else if (WIFSTOPPED(status)) {
					std::cerr << "STOPPED by signal "  << WSTOPSIG(status) << std::endl << std::flush;
				}

			} while (0 == err || (err > 0 && !WIFEXITED(status) && !WIFSIGNALED(status)));
		}
	}

	sigaction(SIGCHLD, &actionPrev, NULL);
	close(_wakeupPipe[0]);
	close(_wakeupPipe[1]);
	_wakeupPipe[0] = _wakeupPipe[1] = 0;
	return failed;
}

// ----------------------------------------------------------------------------
// END IMPLEMENTATION
//
}   // namespace Test
