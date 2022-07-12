/* tinytest -- A very simple test harness for embedded systems.
*/

#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "tinytest.h"

// Printf defers to TT_VPRINTF defined in tinytest_local.h
#ifndef TT_VPRINTF
#define TT_VPRINTF tt_vprintf

#ifndef TT_VPRINTF_BUFLEN
#error "TT_VPRINTF_BUFLEN must be explicitly defined."
#endif

static void tt_vprintf(tt_pgm_str_t fmt, va_list args) {
    union {
        long int i;
        unsigned long int u;
        const char *str;			// Normal string in RAM.
        tt_pgm_str_t pstr;			/* Fixed string, AVR stores in Flash, non-AVR targets define this type as 'const char*', but is OK
										to have two union members of same type. */
    } a;    						// Holds value currently being converted.
    char c;    						// Holds a char from the format string.
    char buf[TT_VPRINTF_BUFLEN]; 	// Reverse formatted number.
    char* pb;    					// Used as pointer to an element in buf.
    char informat = 0; 				// Start off not reading a format.
    char base;						// Number base.
	char fill_zero;					// True for zero fill.

    // Start scanning.
    while ('\0' != (c = (*fmt++))) {
        if (informat) {
            switch (c) {
            case 'x':					// Hex int...
                base = 16;
                a.u = va_arg(args, unsigned);
				fill_zero = 1;
                goto print_unsigned;

			case 'd':            		// Signed decimal integer...
                a.i = va_arg(args, int);
				fill_zero = 0;
                if (a.i < 0) {			// Handle negative integer...
                    tt_putchar('-');
                    a.u = -a.i;
                }
print_unsigned:
                pb = buf + sizeof(buf) - 1;		// Last element.
				*pb-- = '\0';					// Terminate.
                do {							// Fill with digits from least significant.
                    *pb = (char)(a.u % base) + '0';
                    if (*pb > '9')
                        *pb += 'a' - ('9' + 1);
                    --pb;
                    a.u /= base;
                } while (a.u > 0);
				if (fill_zero) {				// Zero fill...
					while (pb > buf)
						*pb-- = '0';
				}
				a.str = pb + 1;
                goto print_string;

            case 's':		           				// Char string in RAM...
                a.str = va_arg(args, const char*);
print_string:
                while ('\0' != (c = (*a.str++)))
                    tt_putchar(c);
                goto done_format;

#ifdef TT_VPRINTF_PSTR
            case 'P':		           				// Pstring if not same as RAM string...
                a.pstr = va_arg(args, tt_pgm_str_t);
                while ('\0' != (c = (tt_pgm_str_read(a.pstr++))))
                    tt_putchar(c);
                goto done_format;
#endif

			case '%':						// Literal `%'...
                tt_putchar(c);
done_format:    informat = 0;
                break;

			default:						// Ignore nything else.
				break;
            }
        }
        else { // Normal character or looking for a '%'.
            if ('%' == c) {		// We have a format spec coming, default to base 10.
                informat = 1;
                base = 10;
            }
            else
                tt_putchar(c);	// Not part of a format, just print it.
        }
    }
}
#endif

// Local printf() defers to vprintf().
static void tt_printf(tt_pgm_str_t fmt, ...) {
    va_list args;
    va_start(args, fmt);
    TT_VPRINTF(fmt, args);
    va_end(args); // Should always call this, even though it is usually a no-op.
}

// This struct holds the context of where we are when we are running a test.
static struct {
    jmp_buf here;                       // Used to implement immediate exit from test function on failures.
    tt_pgm_str_t tf_filename; 			// Filename of currently running test function.
    int tf_lineno;                      // Line number of currently running test function.
    tt_pgm_str_t test_desc;  			// Description of test, e.g. "test_foo(1245)".
    int pass_count, fail_count, ignore_count; // Count of the number of tests that have been passed/failed/ignored.
    const char* groupstr;   			// If non-NULL then only test descriptions containing this string are run.
    tt_fixture_func_t setup, teardown;  // User functions called before & after a test. May be NULL.
    tt_fixture_func_t dump;   			// User function to emit diagnostics on a fail. May be NULL.
    int output_mode;                    // Controls verbosity of output.
} f_ctx;

void ttRegisterFixture(tt_fixture_func_t setup, tt_fixture_func_t dump, tt_fixture_func_t teardown) {
    f_ctx.setup = setup;
    f_ctx.teardown = teardown;
    f_ctx.dump = dump;
}

// Called at the start of the test.
void ttStart(int output_mode, const char* groupstr) {
	memset(&f_ctx, 0, sizeof(f_ctx));		// Most things are zeroed.
    f_ctx.output_mode = output_mode;
    f_ctx.groupstr = groupstr;
}

void ttDiagnostic(tt_pgm_str_t msg, ...) {
    if (NULL != msg) {
		if (TT_OUTPUT_MODE_VERBOSE == f_ctx.output_mode) { // Only in verbose emit diagnostic messages.
			va_list args;
			va_start(args, msg);
			tt_printf(TT_PSTR("# "));
			TT_VPRINTF(msg, args);
			tt_printf(TT_PSTR(TT_NEWLINE));
			va_end(args);
		}
    }
}

void tt_abort(int reason) {
    longjmp(f_ctx.here, reason); // Make magic happen...
}

/** Print a failure diagnostic, a factor of the TT_ASSERT_xxx() macros. If msg is non-NULL, it is passed through
    vprintf, together with any trailing arguments. */
void tt_print_fail_message(tt_pgm_str_t filename, int lineno, tt_pgm_str_t msg, ...) {
    va_list args;

    switch (f_ctx.output_mode) {
	default:	 					// No output!
		break;
 	case TT_OUTPUT_MODE_CONCISE:
        tt_putchar('F');
		break;
	case TT_OUTPUT_MODE_DEFAULT:	 // Default & verbose both emit diagnostics for failures.
		// Fall through...
	case TT_OUTPUT_MODE_VERBOSE:
        tt_printf(TT_PSTR("%s:%d: "), filename, lineno);
        tt_printf(TT_PSTR("[%s:%d %s] FAIL: "), f_ctx.tf_filename, f_ctx.tf_lineno, f_ctx.test_desc);
        va_start(args, msg);
        TT_VPRINTF(msg, args);
        va_end(args);
		tt_printf(TT_PSTR("." TT_NEWLINE));		// A sentence must always end with a full stop.
		break;
    }
}

static void report(tt_pgm_str_t msg, char concise) {
    switch (f_ctx.output_mode) {
	default:	 					// No output!
		break;
 	case TT_OUTPUT_MODE_CONCISE:	// Just print a single char.
        tt_putchar(concise);
		break;
	case TT_OUTPUT_MODE_DEFAULT:	 // Default no output for success, ignored, only failures, which are handled by another output routine.
		break;
	case TT_OUTPUT_MODE_VERBOSE:
        tt_printf(TT_PSTR("[%s]: %s" TT_NEWLINE), f_ctx.test_desc, msg);
		break;
   }
}

void ttRunTest(void (*test_func)(void), tt_pgm_str_t filename, int lineno, tt_pgm_str_t desc) {
    if ((NULL == f_ctx.groupstr) || (NULL != strstr(desc, f_ctx.groupstr))) { // Decide whether to run this test...
        int exc;

        // Setup the test context.
        f_ctx.tf_filename = filename;
        f_ctx.tf_lineno = lineno;
        f_ctx.test_desc = desc;

        // Print leader for verbose mode.
        if (TT_OUTPUT_MODE_VERBOSE == f_ctx.output_mode)
            tt_printf(TT_PSTR("%s:%d: "), filename, lineno);

        // Call the test, set flag on failure.
        exc = setjmp(f_ctx.here);
        if (TINY_TEST_SUCCESS == exc) { 	    // When setjmp is called normally it just returns 0.
            if (NULL != f_ctx.setup)			// Call fixture setup func.
                f_ctx.setup();
            test_func();

            // If we get here then the test has passed.
            report("OK", '.');
            f_ctx.pass_count += 1;
        }
        else if (TINY_TEST_IGNORED == exc) { 	// This test has been flagged as IGNORED.
			report("IGNORED", 'I');				// Handle output reporting.
			f_ctx.ignore_count += 1;
        }
        else { // We have a failure. Call dump function if non-NULL.
            if (NULL != f_ctx.dump)
                f_ctx.dump();
            f_ctx.fail_count += 1;
        }

        if (NULL != f_ctx.teardown)
            f_ctx.teardown();
    }
}

int ttFinish(void) {
    switch (f_ctx.output_mode) {
	default:	 					// No output!
		break;
 	case TT_OUTPUT_MODE_CONCISE:
		// Fall through...
	case TT_OUTPUT_MODE_DEFAULT:
		// Fall through...
	case TT_OUTPUT_MODE_VERBOSE:
        tt_printf(TT_PSTR("------------------------------------------------\n"));
        tt_printf(TT_PSTR("Passed %d, failed %d, ignored %d.\n"),
          f_ctx.pass_count,
          f_ctx.fail_count,
          f_ctx.ignore_count);
        tt_printf((f_ctx.fail_count > 0) ? TT_PSTR("FAIL") : TT_PSTR("OK"));
        tt_printf(TT_PSTR(TT_NEWLINE));
		break;
    }
    return f_ctx.fail_count > 0;
}

/*  Optional main function.

	Note that the data structures will be in RAM for the silly AVR but this is unimportant as you are unlikely to want to use this function on AVR.
*/

typedef struct {
    char optchar;
    void (*opt_handler)(int* argidx, char* argv[], void* val);
    void* val;
} option_def_t;

static int output_mode = TT_OUTPUT_MODE_DEFAULT;
static int pause = 0;
static int help = 0;
static char* tests;

static void opt_handler_bool_set(int* argidx, char* argv[], void* val) { *(int*)val = 1; }
static void opt_handler_verbose(int* argidx, char* argv[], void* val) { *(int*)val = TT_OUTPUT_MODE_VERBOSE; }
static void opt_handler_quiet(int* argidx, char* argv[], void* val) { *(int*)val = TT_OUTPUT_MODE_QUIET; }
static void opt_handler_concise(int* argidx, char* argv[], void* val) { *(int*)val = TT_OUTPUT_MODE_CONCISE; }

static void opt_handler_str(int* argidx, char* argv[], void* val) {
    *argidx += 1;
    *(const char**)val = argv[*argidx];
}
option_def_t OPTIONS[] = {
    { '?', opt_handler_bool_set, &help },
    { 'v', opt_handler_verbose, &output_mode },
    { 'q', opt_handler_quiet, &output_mode },
    { 'c', opt_handler_concise, &output_mode },
    { 'p', opt_handler_bool_set, &pause },
    { 'g', opt_handler_str, &tests },
};
#define NUM_OPTIONS ((int)(sizeof(OPTIONS) / sizeof(OPTIONS[0])))

static int handle_options(int argc, char* argv[]) {
    int argidx;

    for (argidx = 1; argidx < argc; ++argidx) {
        int i;
        char optchar = argv[argidx][1];
        for (i = 0; i < NUM_OPTIONS; ++i) {
            if (optchar == OPTIONS[i].optchar) {
                OPTIONS[i].opt_handler(&argidx, argv, OPTIONS[i].val);
                break;
            }
        }
        if (NUM_OPTIONS == i) {
            tt_printf(TT_PSTR("Illegal option: `%s'.\n"), argv[argidx] + 1);
			return 2;
		}
    }
	return 0;
}

int ttMain(int argc, char* argv[]) {
    int rc;

    rc = handle_options(argc, argv);
	if (rc)
		return rc;
	if (help) {
        tt_printf(TT_PSTR(
		  "Tinytest test harness %s [options]. Return value is number of failures.\n"
		  " Default is to print full information for failures only, with totals.\n"
		  "  -?  print help\n"
		  "  -q  quiet, no output at all\n"
		  "  -c  concise output `FI.' for fail/ignored/pass, with totals\n"
		  "  -v  verbose output, information for all tests\n"
		  "  -p  pause after running tests, print message and wait for return\n"
		  "  -g <str> only run tests containing str (case sensitive)\n"
		), argv[0]);
		return 1;
	}

    ttStart(output_mode, tests);
    ttRunTests();
    rc = ttFinish();
    if (pause) {
#ifdef tt_wait_enter
        tt_printf(TT_PSTR("Press the <enter> key to continue..."));
        tt_wait_enter();
#endif
    }
	return rc;
}

static void start_random(int seed) {
    srand(seed);
}
static char get_random() {
	char r;
	do { r = (char)rand(); } while ((0 == r) || (0xff == r)); // Exclude 0 & ff, both common values in munged memory.
	return r;
}
void ttFillMemory(void* buf, size_t len, int seed) {
	char* cbuf = (char*)buf;
    start_random(seed);
    while (len--)
        *cbuf++ = get_random();
}
void ttVerifyMemory(const void* buf, size_t len, int seed, tt_pgm_str_t filename, int lineno) {
	const char* cbuf = (char*)buf;
    start_random(seed);
    while (len--) {
        char expected = get_random();
        if (*cbuf != expected) {
            tt_print_fail_message(filename, lineno, TT_PSTR("Verify memory fail address: 0x%lx"), cbuf);
            tt_abort(TINY_TEST_FAIL);
        }
        ++cbuf;
    }
}

// eof

