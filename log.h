#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>
#include <stdbool.h>

#define LOG_EN		1
#define ASSERT_EN	0

enum log_level {
	LOG_LEVEL_OFF   = 0,
	LOG_LEVEL_FATAL,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_VERBOSE,
	LOG_LEVEL_ITEMS,
};

#if LOG_EN
	#define LOG(fmt, ...)	LOG_INF(fmt, ##__VA_ARGS__)
//	#include <stdio.h>
//	#define LOG	 printf
#else
	#define LOG(fmt, ...)	((void)0)
#endif

#define PRINTF	  LOG_INF

#define LOG_FATAL(fmt, ...)		log_printf(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)		log_printf(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)		log_printf(LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_INF(fmt, ...)		log_printf(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)		log_printf(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(fmt, ...)	log_printf(LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)

#if ASSERT_EN
	#define STRIPPATH(s)\
		(sizeof(s) > 2 && (s)[sizeof(s)-2] == '/' ? (s) + sizeof(s) - 1 : \
		sizeof(s) > 3 && (s)[sizeof(s)-3] == '/' ? (s) + sizeof(s) - 2 : \
		sizeof(s) > 4 && (s)[sizeof(s)-4] == '/' ? (s) + sizeof(s) - 3 : \
		sizeof(s) > 5 && (s)[sizeof(s)-5] == '/' ? (s) + sizeof(s) - 4 : \
		sizeof(s) > 6 && (s)[sizeof(s)-6] == '/' ? (s) + sizeof(s) - 5 : \
		sizeof(s) > 7 && (s)[sizeof(s)-7] == '/' ? (s) + sizeof(s) - 6 : \
		sizeof(s) > 8 && (s)[sizeof(s)-8] == '/' ? (s) + sizeof(s) - 7 : \
		sizeof(s) > 9 && (s)[sizeof(s)-9] == '/' ? (s) + sizeof(s) - 8 : \
		sizeof(s) > 10 && (s)[sizeof(s)-10] == '/' ? (s) + sizeof(s) - 9 : \
		sizeof(s) > 11 && (s)[sizeof(s)-11] == '/' ? (s) + sizeof(s) - 10 : (s))

	#define __JUSTFILE__ STRIPPATH(__FILE__)

	#define ASSERT(e)	   ((e) ? (void)0 : log_assert(__JUSTFILE__, __LINE__))
#else
	#define ASSERT(e)	   ((void)0)
#endif


#ifdef __cplusplus
extern "C" {
#endif

void log_level_set(int lv);
int log_printf(int lv, const char *fmt, ...);
void log_hexdump(int lv, const void *data, uint32_t size);
void log_assert(const char *src_file_path, int line);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */
