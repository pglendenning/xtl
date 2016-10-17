#include <iostream>
#include <cstdio>
#include <test.h>


using Test::TestManager;


void Usage()
{
    std::cout << "testunner [--nofork|--help|-h] [testname ...]\n";
    exit(0);
}


int main(int argc, char**argv)
{
    int fails = 0;
    if (argc == 1) {
	    fails = Test::TestManager::RunInForkedProcess(600);
    } else {
        bool nofork = false;
        int i;
        for (i = 1; i < argc; ++i) {
            if (std::string("--nofork") == argv[i])
                nofork = true;
            else if (std::string("-h") == argv[i] || std::string("--help") == argv[i])
                Usage();
            break;
        }
        if (i < argc) {
            for (; i < argc; ++i) {
                if (!TestManager::RunTest(argv[i]))
                    ++fails;
            }
        } else if (nofork)
            fails = Test::TestManager::RunInCurrentProcess();
	    else
            fails = Test::TestManager::RunInForkedProcess(600);
    }
    if (fails) std::cout << "There were " << fails << " failures" << std::endl;
    return fails? 1: 0;
}

