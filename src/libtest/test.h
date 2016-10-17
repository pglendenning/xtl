#ifndef TEST_H_0A4DAA98_5A88_468F_B0C0_FC4D166CFD48
#define TEST_H_0A4DAA98_5A88_468F_B0C0_FC4D166CFD48
//-----------------------------------------------------------------------------
// Copyright (c) Solidra LLC
//
// Recommendation:
// Place all tests in the name space Test::TestImpl.
// Author: Paul Glendenning

#include <vector>
#include <string>
#include <memory>

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with the
// TEST_EXPORTS symbol defined on the command line. This symbol should not
// be defined on any project that uses this DLL.

#ifdef _USRDLL
	#if _MSC_VER
		#ifdef __TEST_API_EXPORTS__
		#define TEST_API __declspec(dllexport)
		#else
		#define TEST_API __declspec(dllimport)
		#endif
	#else
        // Assume GNU C
		#ifdef __TEST_API_EXPORTS__
        #define TEST_API __attribute__((visibility("default")))
        #else
        #define TEST_API
        #endif
	#endif
#else
	#define TEST_API
#endif

namespace Test {

//-----------------------------------------------------------------------------

class TEST_API TestManager final {
public:
	struct TestCaseHolder;

    typedef void (TestCase) (void);

	TestManager();
	~TestManager();

    static size_t GetTestCount();
	static int RunInForkedProcess(unsigned timeoutInSecs);
    static int RunInCurrentProcess();
    static bool RunTest(const std::string& testName);
    static bool IsRunning();
    static const TestCaseHolder* AddTestCase(TestCase* test, const char* class_name);

private:
	static int _wakeupPipe[2];
	static void SignalWakeup(int);

    /// @cond
    static std::vector<std::unique_ptr<TestCaseHolder>>& GetTests();

	// Disabled features
	TestManager(const TestManager&);
	TestManager& operator = (const TestManager&);
    /// @endcond
};


/// This exception is never explicilty thrown. It is used by the TEST_ASSERT()
/// macro.
class TEST_API TestException final
{
public:
    TestException(const char* filename, int line);
    std::string _module;
    int         _line;

	// Need these so we link to the DLL version
	TestException(const TestException&);
	TestException& operator = (const TestException&);
	~TestException();
};

#define	NOAPI_DECL

#define REGISTER_TEST_EXTERN(test_class, api_decl) \
    struct api_decl __REGISTER_##test_class final { \
		__REGISTER_##test_class(); \
        static void Run(void); \
    }; \
    __REGISTER_##test_class::__REGISTER_##test_class() {  \
        static const char* test_class##_name = #test_class; \
        ::Test::TestManager::AddTestCase(Run, test_class##_name); \
    } \
    api_decl void __REGISTER_##test_class::Run()


#define REGISTER_TEST(test_class) \
    REGISTER_TEST_EXTERN(test_class, NOAPI_DECL)

/// @{
/// Call to verify a test passed or failed
#if defined(DEBUG)
#define TEST_ASSERT(expr) \
    (::Test::TestManager::IsRunning() && !(expr))? throw ::Test::TestException(__FILE__, __LINE__): void(0)
#else
#define TEST_ASSERT(expr)	void(0)
#endif
/// @}

//-----------------------------------------------------------------------------
}       // namespace Test
#endif  // defined(TEST_H_0A4DAA98_5A88_468F_B0C0_FC4D166CFD48)
