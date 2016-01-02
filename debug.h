#ifndef _DSD_DEBUG_H
#define _DSD_DEBUG_H

#undef __format
#undef __print_arg

#undef __nargs_0
#undef __nargs_1

#undef __eval_0
#undef __eval_1
#undef __eval_2

#undef __print_0
#undef __print_1
#undef __print_2
#undef __print_3
#undef __arg_1
#undef __arg_2
#undef __loop
#undef __loop_helper
#undef __empty_helper

#undef __print_info
#undef __custom_debug
#undef __custom_idebug

#undef __debug_P_1
#undef __debug_P_2
#undef __idebug_P_1
#undef __idebug_P_2
#undef __debug_raw_P_1
#undef __debug_raw_P_2

#undef __idebug_0
#undef __idebug_1
#undef __idebug_arg_1
#undef __idebug_arg_2

#undef debug
#undef idebug
#undef debug_raw
#undef idebug_raw

/* If NDEBUG is defined, the debugging functions shouldn't print anything. The
 * standard states that assert.h should support being re-included, to allow
 * having different NDEBUG states in different places (e.g. a library header
 * defining NDEBUG shouldn't stop you from debugging). To support the same
 * behavior, we don't use include guards and instead undefine and redefine
 * everything each time the header is included.
 */
#ifdef NDEBUG

#define debug(...) ((void)0)
#define idebug(...) ((void)0)
#define debug_raw(...) ((void)0)
#define idebug_raw(...) ((void)0)

#else

/* If stdio wasn't included yet (best effort), instead of including it, we just
 * declare printf. This allows the inclusion of this header in the middle of a
 * function, for quick debugging.
 */
#if !defined(_STDIO_H_) && !defined(_STDIO_H)
int printf(const char *restrict format, ...);
#endif

/* C11 added the _Generic construct, which is similar to a switch on an
 * expression's type. We use it to select the correct format string for printf.
 *
 * There's a few tricks in here:
 * - Since we want to support literal strings (that have type char[n]), we match
 *   with char[sizeof(arg)]. Both _Generic and sizeof are resolved at
 *   compilation time, and in the right order.
 * - We use a really hacky trick to print a complex double, that relies on the
 *   fact that the memory layout of a complex double on most compilers (not
 *   portable though!) is just the real-part double next to the imaginary-part
 *   double. So we just use "%g + %gi", which might produce several compiler
 *   warnings, but will (probably) work.
 *
 * Structs, unions, pointers/arrays (except for null terminated strings and
 * void* pointers), complex floats and complex long doubles are NOT supported.
 * With a little bit of work (or a lot) both complex types and arrays with a
 * known size (i.e. sizeof(array) works) could be supported.
 */
#define __format(arg) _Generic((arg), \
                      char: "%c", \
               signed char: "%hhd", \
             unsigned char: "%hhu", \
              signed short: "%hd", \
            unsigned short: "%hu", \
                signed int: "%d", \
              unsigned int: "%u", \
                  long int: "%ld", \
         unsigned long int: "%lu", \
             long long int: "%lld", \
    unsigned long long int: "%llu", \
                     _Bool: "%d", \
                     float: "%g", \
                    double: "%g", \
               long double: "%Lg", \
           _Complex double: "%g + %gi", \
                    char *: "%s", \
                    void *: "%p", \
         char[sizeof(arg)]: "%s", \
                   default: "<unknown>" \
)

/* Print an argument regardless of its type (see __format). */
#define __print_arg(arg) printf(__format(arg), arg)

/* __nargs_0 calls __nargs_1 with all the arguments, plus 15 twos and 1 one at
 * the end (the zero is there just to avoid a warning when __nargs_1 has an
 * empty __VA_ARGS__). __nargs_1 selects the 17th argument, and discards the
 * rest. In combination, they resolve to 1 if there's just one argument, or 2
 * if there's more than that.
 */
#define __nargs_0(...) __nargs_1(__VA_ARGS__, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0)
#define __nargs_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, ...) q

/* Force 4 * 4 = 16 preprocessor passes on the arguments. */
#define __eval_0(...) __eval_1(__eval_1(__eval_1(__eval_1(__VA_ARGS__))))
#define __eval_1(...) __eval_2(__eval_2(__eval_2(__eval_2(__VA_ARGS__))))
#define __eval_2(...) __VA_ARGS__

/* Eval what follows as many times as necessary. */
#define __print_0(...) __eval_0(__print_1(__VA_ARGS__))
/* Get the cardinality of the arguments. */
#define __print_1(P, ...) __print_2(__nargs_0(__VA_ARGS__), P, __VA_ARGS__)
/* Force a preprocessor pass, to eval the __nargs_0 macro. */
#define __print_2(num, ...) __print_3(num, __VA_ARGS__)
/* Switch based on the number of arguments. */
#define __print_3(num, ...) __arg_ ## num (__VA_ARGS__)
/* If there's only one argument: */
/* Print the argument. */
#define __arg_1(P, arg) P ## 1(arg)
/* If there's two or more arguments: */
/* Print the first argument. */
#define __arg_2(P, arg, ...) P ## 2(arg), __loop(P, __VA_ARGS__)
/* Call __print_1 with the rest of the arguments. */
#define __loop(...) __loop_helper __empty_helper() () (__VA_ARGS__)
/* Loop helpers, they force re-evaluation after the first pass. */
#define __loop_helper() __print_1
#define __empty_helper()

/* These wrappers of __print_0 receive custom argument printing functions, and
 * take care of the formatting and printing of the arguments, and the optional
 * file and line information.
 */
#define __print_info(sep) printf("[%s:%d][%s]%s", __FILE__, __LINE__, __func__, sep)
#define __custom_debug(P, ...) (__print_0(P, __VA_ARGS__), printf("\n"))
#define __custom_idebug(P, sep, ...) (__print_info(sep), __custom_debug(P, __VA_ARGS__))

/* Here be debug */

#define __debug_P_1(arg) printf("%s = ", #arg), __print_arg(arg)
#define __debug_P_2(arg) __debug_P_1(arg), printf("\n")

#define debug(...) __custom_debug(__debug_P_, __VA_ARGS__)

/* Here be idebug */

#define __idebug_P_1(arg) printf("\t%s = ", #arg), __print_arg(arg)
#define __idebug_P_2(arg) __idebug_P_1(arg), printf("\n")

/* If there's only one argument, print everything in the same line. If there's
 * more, print the file information in the first line, and then one argument per
 * line.
 */
#define idebug(...) __idebug_0(__nargs_0(__VA_ARGS__), __VA_ARGS__)
#define __idebug_0(num, ...) __idebug_1(num, __VA_ARGS__)
#define __idebug_1(num, ...) __idebug_arg_ ## num (__VA_ARGS__)
#define __idebug_arg_1(...) __custom_idebug(__debug_P_, " ", __VA_ARGS__)
#define __idebug_arg_2(...) __custom_idebug(__idebug_P_, "\n", __VA_ARGS__)

/* Here be debug_raw and idebug_raw */

#define __debug_raw_P_1(arg) __print_arg(arg)
#define __debug_raw_P_2(arg) __debug_raw_P_1(arg), printf(" ")

#define debug_raw(...) __custom_debug(__debug_raw_P_, __VA_ARGS__)
#define idebug_raw(...) __custom_idebug(__debug_raw_P_, " ", __VA_ARGS__)

#endif /* NDEBUG */

enum dsd_log_levels {
  DSD_ERR = 1 << 0,
  DSD_WARN = 1 << 1,
  DSD_NOTICE = 1 << 2,
  DSD_INFO = 1 << 3,
  DSD_DEBUG = 1 << 4,

  DSD_COUNT =  6/* set to count of valid flags */
};

void dsd_set_log_level(int level);
int dsd_log_filter(int filter);

#ifdef _DEBUG

#define DSD_PRINT_END fprintf(stderr, "\033[0m")// end

#define dsd_debug(...) \
    idebug(__VA_ARGS__); \
    DSD_PRINT_END;
#define dsd_debug_raw(...) \
    idebug_raw(__VA_ARGS__); \
    DSD_PRINT_END;

/* ----------------------------- */

#define dsdl_err(...) do { \
  if (dsd_log_filter(DSD_ERR))\
      dsd_debug(__VA_ARGS__); \
  } while(0)

#define dsdl_warn(...) do { \
if (dsd_log_filter(DSD_WARN))\
    dsd_debug(__VA_ARGS__); \
} while(0)

#define dsdl_notice(...) do { \
  if (dsd_log_filter(DSD_NOTICE))\
      dsd_debug(__VA_ARGS__); \
  } while(0)

#define dsdl_info(...) do { \
  if (dsd_log_filter(DSD_INFO))\
      dsd_debug(__VA_ARGS__); \
  } while(0)

#define dsdl_debug(...) do { \
  if (dsd_log_filter(DSD_DEBUG))\
      dsd_debug(__VA_ARGS__); \
  } while(0)

/* ----------------------------- */

#define dsdl_err_raw(...) do { \
  if (dsd_log_filter(DSD_ERR))\
      dsd_debug_raw(__VA_ARGS__); \
  } while(0)

#define dsdl_warn_raw(...) do { \
  if (dsd_log_filter(DSD_WARN))\
      dsd_debug_raw(__VA_ARGS__); \
  } while(0)

#define dsdl_notice_raw(...) do { \
  if (dsd_log_filter(DSD_NOTICE))\
      dsd_debug_raw(__VA_ARGS__); \
  } while(0)

#define dsdl_info_raw(...) do { \
  if (dsd_log_filter(DSD_INFO))\
      dsd_debug_raw(__VA_ARGS__); \
  } while(0)

#define dsdl_debug_raw(...) do { \
  if (dsd_log_filter(DSD_DEBUG))\
      dsd_debug_raw(__VA_ARGS__); \
  } while(0)

#else /* no debug */

#define dsdl_notice(...) {}
#define dsdl_warn(...) {}
#define dsdl_err(...) {}
#define dsdl_info(...) {}
#define dsdl_debug(...) {}

#define dsdl_notice_raw(...) {}
#define dsdl_warn_raw(...) {}
#define dsdl_err_raw(...) {}
#define dsdl_info_raw(...) {}
#define dsdl_debug_raw(...) {}

#endif

#endif

