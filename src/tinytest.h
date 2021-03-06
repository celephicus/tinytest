/*  Tinytest -- A very simple test harness for embedded systems.
     See LICENSE.TXT for license.
*/

#ifndef TINYTEST_H__
#define TINYTEST_H__

#include <stdarg.h> // We use variable argument lists, so clients need this header. 
#include <stdlib.h> // NULL is very useful. 

// Local settings - customise to suit your local platform. 
#include "tinytest_local.h"

// Set settings defaults. 

/* Tinytest's program strings, aka a "pstring". The AVR processor can store constant strings in Flash memory, which saves RAM, but
	otherwise is a pain, as they require special macros to declare, and special library functions. 
	The defaults below set a pstring to be equivalent to `const char[]'. For details of the macros below refer to tinytest_local.h.
*/
#ifndef TT_FMT_PSTR
#define TT_FMT_PSTR "%s"
#endif
#ifndef tt_pgm_str_t
#define tt_pgm_str_t const char*
#endif
#ifndef TT_ATTR_PGM
#define TT_ATTR_PGM /* empty */
#endif
#ifndef TT_PSTR
#define TT_PSTR(_s) (_s)
#endif
#ifndef tt_pgm_str_read
#define tt_pgm_str_read(_s) (*(_s))
#endif
#ifndef tt_strcmp_pstr
#include <string.h>
#define tt_strcmp_pstr(_ps, _s) strcmp(_ps, _s)
#endif

// Get the filename for a file in one place only. This save a lot of space compared with using __FILE__, which is the full path.
#define TT_DECLARE_MODULE(name_) static tt_pgm_str_t TT_FILENAME = TT_PSTR(name_)

// Various modes that the tests can be run it, controlling the amount of output. 
enum { 
    TT_OUTPUT_MODE_QUIET,       // No output at all. 
    TT_OUTPUT_MODE_CONCISE,     // Only print a single 'F' for failures, 'I' for ignored, or a '.' for success. With summary report at end. 
    TT_OUTPUT_MODE_DEFAULT,     // Print full information for failures only. With summary report at end. 
    TT_OUTPUT_MODE_VERBOSE,     // Prints information for all tests, and diagnostic messages. With summary report at end. 
};

/* Tinytest can be used in 3 ways, depending on your needs, The simplest is basic mode, which is suitable for running on a desktop system. 

Basic:
	In which case, you only really need to write the function runTests(), which will contain calls to the followiny Tinytest functions only:
	
		ttSetDump()		  	-- register a dump function to emit diagnostics on a failure.
		ttRegisterFixture() -- register setup/teardown functions for all subsequent tests.
		ttRunTest()		  	-- run a test function, with filename, line number & descripton, usually the function name.
		TT_TEST_SIMPLE()	-- run a test function, with other values set.
		
	Then call ttMain with argc & argv from the command line.

Advanced:	
	For advanced use, instead of calling ttMain, call these functions in order;
	    ttStart()
		ttRunTests();
		ttFinish();

Magic:
	The magic mode uses a Python script to discover all test functions in a directory and builds a source file with the necessary 
	functions to run those tests. This requires the use of special macros that are read by the script, and used to build the source file.
	In general function declarations are autogenerated, so test files do not need header files.
	
	All files matching the pattern 'test*.c' are scanned. 
	Any function definitions matching `void testXXX()' are considered tests and are run directly.
	Any function definitions matching `void testXXX(args)` are considered parameterised tests and use the TT_TEST_CASE() macro to
	generate a stub function with signature `void f(void)' that can be run as a test. Note that the arguments should be constants 
	if possible.
	
	The macros TT_BEGIN_FIXTURE(setup, teardown) & TT_END_FIXTURE() use fixture functions for all tests. 
	The macro TT_DUMP_FUNC(dumper) sets a dump function, which must be externally linked. 
	The macro TT_IGNORE_FILE aborts scanning of the rest of the file. 
	The macro TT_INCLUDE_EXTRA may be used to include header files into the autogenerated file.
*/	
	
/** Initialise the library before we actually run any tests. 
	Set the output_mode value to one of the TT_OUTPUT_MODE_xxx values to control the verbosity of the output. 
	If the groupstr is non-null then tests are only run if their description contains groupstr. */
void ttStart(int output_mode, const char* groupstr);

// Function that runs the tests. Either write it manually or use the code generator. 
void ttRunTests(void);

// Finish performing tests, and print a summary message. 
int ttFinish(void);

// Main function that calls the runTests() function after processing the command line arguments. 
int ttMain(int argc, char* argv[]);

/*
	These functions/macros should only be used within the body of runTests().
*/
	
// Type of a setup, teardown or dump function. 
typedef void (*tt_fixture_func_t)(void);

/* Sets 3 functions that are called as part of each test. 
	The setup function is called before each test is run. It may use TEST_ASSERT_xxx macros which cause the test
	  to fail. 
    The dump function is called when a test fails. This can be used to dump stuff via the tt_printf() function.
      The output should have a trailing newline.
    The teardown function is called after a test fails or runs to completion. It may use TEST_ASSERT_xxx macros,
      which cause the test to fail. 
	Any function may be set to NULL for an empty function. */
void ttRegisterFixture(tt_fixture_func_t setup, tt_fixture_func_t dump, tt_fixture_func_t teardown);

#define ttUnregisterFixture() ttRegisterFixture(NULL, NULL, NULL)

/** Emit a diagnostic message (if non-NULL). The string should not contain a trailing newline, as the function 
	will print one. The message is processed by printf, so arguments can be inserted. */
void ttDiagnostic(tt_pgm_str_t msg, ...);

// Call a test function with an explicit description. The lineno argument is the first line of the function. 
void ttRunTest(void (*test_func)(void), tt_pgm_str_t filename, int lineno, tt_pgm_str_t desc);

// Basic command to run a test function and fill in the filename, line number & description. 
#define TT_TEST_SIMPLE(x_) 	ttRunTest(x_, TT_FILENAME, __LINE__, TT_PSTR(#x_ "()"))

/*
	These functions/macros should only be used within the body of a test function.
*/

// Cause an explicit test failure. 
#define TT_FAIL(msg_) do { \
    tt_print_fail_message(TT_FILENAME, __LINE__,  TT_PSTR("Failure: " TT_FMT_PSTR), msg_);  \
    tt_abort(TINY_TEST_FAIL);         \
} while (0)

// Check that a condition is true. 
#define TT_ASSERT(cond_) if (cond_) {} else do { \
    tt_print_fail_message(TT_FILENAME, __LINE__,  TT_PSTR("Expected `" TT_FMT_PSTR "' to be true"), TT_PSTR(#cond_)); \
    tt_abort(TINY_TEST_FAIL);         \
 } while (0)

#define TT_ASSERT_GENERIC(type_, fmt_, value_, expected_) do { \
  const type_ _tt__value = (type_)(value_); \
  const type_ _tt__expected = (type_)(expected_); \
  if (_tt__value == _tt__expected) {} else {  \
    tt_print_fail_message(TT_FILENAME, __LINE__,  TT_PSTR("Expected `" TT_FMT_PSTR "' == " fmt_ ", got " fmt_), TT_PSTR(#value_), _tt__expected, _tt__value); \
    tt_abort(TINY_TEST_FAIL);         \
  } \
} while (0)
	
// Check the value of an integer. 
#define TT_ASSERT_INT(value_, expected_) TT_ASSERT_GENERIC(tt_int_t, TT_FMT_INT, value_, expected_)
#define TT_ASSERT_INT_HEX(value_, expected_) TT_ASSERT_GENERIC(tt_int_t, "0x" TT_FMT_HEX, value_, expected_)

// Check the value of a string. 
#define TT_ASSERT_STR(value_, expected_) do { \
  const char* _tt__value = (value_); \
  tt_pgm_str_t _tt__expected = TT_PSTR(expected_); \
  if (0 == tt_strcmp_pstr(_tt__expected, _tt__value)) {} else { \
    tt_print_fail_message(TT_FILENAME, __LINE__,  TT_PSTR("Expected `" TT_FMT_PSTR "' == \"%s\", got \"%s\""), #value_, _tt__expected, _tt__value); \
    tt_abort(TINY_TEST_FAIL);         \
  } \
} while (0)

/* If the test has not failed so far, cause the test function to be flagged as ignored. Called by the TT_IGNORE()
    macro. */
#define TT_IGNORE() tt_abort(TINY_TEST_IGNORED) 

// A little helper function for checking that a memory buffer has not been corrupted by filling it with random values. Set seed to any value you like. 
void ttFillMemory(void* buf, size_t len, int seed);

// Macro to verify that the buffer has not been written to. 
#define TT_VERIFY_MEMORY(buf_, len_, seed_) tt_verify_memory((buf_), (len_), (seed_), TT_FILENAME, __LINE__)

// Factor for the TT_VERIFY_MEMORY() macro. 
void ttVerifyMemory(const void* buf, size_t len, int seed, tt_pgm_str_t filename, int lineno);

/*
    These should not be called directly. 
*/
enum { TINY_TEST_SUCCESS, TINY_TEST_FAIL, TINY_TEST_IGNORED };
void tt_print_fail_message(tt_pgm_str_t filename, int lineno, tt_pgm_str_t msg, ...);
void tt_abort(int reason);

// The script that builds a runtests() function uses these pseudo-macros in the test code. The definitions turn them into no-ops. 
#define TT_TEST_CASE(...) // empty 
#define TT_BEGIN_FIXTURE(a, b) // empty 
#define TT_END_FIXTURE() // empty 
#define TT_DUMP_FUNC(a) // empty 
#define TT_INCLUDE_EXTRA(a) // empty 

#endif // TINYTEST_H__ 
