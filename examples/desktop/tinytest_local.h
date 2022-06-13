/*  Tinytest -- A very simple test harness for embedded systems.
     See LICENSE.TXT for license.
*/

/* This is a SAMPLE of a local configuration, adjust to suit your target. */

#ifndef TINYTEST_LOCAL_H__
#define TINYTEST_LOCAL_H__

/*
	TINYTEST CONFIGURATION  
*/	

/* When Tinytest checks integers it needs a type defined to do the check, Typically this is either `int' or `long int'. If not defined 
	it defaults to `int'. Don't forget to verify that the printf format is correct. */

/*
	TARGET CONFIGURATION  
*/	

/* 
	Printf:
		Tinytest may be used on targets with no printf! So it defines it's own minimal vprintf, which you don't have to use if you have one available. 
		The only format specifiers used are `%s' and whatever you have defined for TT_FMT_INT, TT_FMT_HEX, TT_FMT_PSTR. 
		If the macro TT_VPRINTF(fmt, args) is defined then it is used, else tinytest uses its minimal vprintf. 
		Note that the format string is of type `tt_pgm_str_t', so this must be set correctly. 
		
	Pgm strings:
		Tinytest uses a lot of strings, some targets (notably the Atmel AVR) can put strings into Flash memory. We use some macros to minimise RAM usage for 
		such targets. 
		
		The `tt_pgm_str_t' macro is the type used for pgm strings, which are not only const but cannot be written as they are in a different
		memory space. If not defined, then it is typedefed to `const char*'. 
		The `TT_ATTR_PGM' macro is the attribute used to tag pgm string values. If not defined it defaults to empty.
		The `TT_PSTR()` macro is used to declare an inline pgm string. If not defined it defaults to empty.
		The `tt_pgm_str_read()' macro reads a char from a pgm string address. If not defined it defaults to simple pointer derefence. 
		The `TT_FMT_PSTR' macro is the printf format used for such strings. It defaults to `"%s"'.
		The `tt_strcmp_pstr macro must be set to a function with prototype int f(tt_pgm_str_t, const char*), if not set it defaults to strcmp(*). 
		
		void f(tt_pgm_str_t str) { ... }
		tt_pgm_str_t str TT_ATTR_PGM = TT_PSTR(" ... ");
		char c = tt_pgm_str_read(str)
		tt_printf(TT_PSTR(TT_FMT_PSTR), str);
		if (tt_strcmp_pstr(TT_PSTR("foo"), str1)) ...
		
	Main function:
		Tinytest has a main function that allows running tests with options for output verbosity and whether to pause before exiting. 
		If you want this function included then the macro `TT_WANT_TT_MAIN' must be defined _and_ the macro `tt_wait_enter()' must be defined to 
		`getchar()' or similar. 
*/

/* Tinytest can include appropriate configuration, or you can do it all yourself. */

#if defined(__AVR__)

/*
		AVR TARGET
*/	

#define tt_int_t int

/* Printf formats for decimal & hex int type.  */
#define TT_FMT_INT "%d"
#define TT_FMT_HEX "%04x"

/* Use tinytest's vprintf. */
#undef TT_VPRINTF
#define TT_FMT_PSTR "%P"
#define TT_VPRINTF_BUFLEN (10) /* Make sure that the maximum number can fit in here! */

#include <pgmspace.h>
#define tt_pgm_str_t PGM_P				// From pgmspace.h, I think it's actually `const char*'. 
#define TT_ATTR_PGM PROGMEM		// From pgmspace.h.
#define TT_PSTR(_s) PSTR(_s)		// From pgmspace.h.
#define tt_pgm_str_read(_s) ((char)pgm_read_byte((_s))) 		// From pgmspace.h.
#define tt_strcmp_pstr(_ps, _s) (strcmp_P(_ps, _s))	// From pgmspace.h, not string.h as you might think. 

/* No tt_main(). */
#undef TT_WANT_TT_MAIN
#undef tt_wait_enter

#else

/*
	ALL OTHER TARGETS
*/	

#define tt_int_t int

/* Printf formats for decimal & hex int type.  */
#define TT_FMT_INT "%d"
#define TT_FMT_HEX "%08x"		// 32 bit ints. 

#if 0
/* Use stdio's vprintf. */
#include <stdio.h>
#define TT_VPRINTF(_fmt, ...) vprintf(_fmt, __VA_ARGS__)
#else
/* Use tinytest's vprintf. */
#include <stdio.h>
#undef TT_VPRINTF
#define tt_putchar(_c) putchar(_c)
#undef TT_FMT_PSTR
#define TT_VPRINTF_BUFLEN (10) /* Make sure that the maximum number can fit in here! */
#endif

/* Use normal strings. */
#undef tt_pgm_str_t
#undef TT_ATTR_PGM
#undef TT_PSTR
#undef tt_pgm_str_read
#undef tt_strcmp_pstr

/* Use tt_main(). */
#define TT_WANT_TT_MAIN
#define tt_wait_enter() (getchar())

#endif

#endif /* TINYTEST_LOCAL_H__ */
