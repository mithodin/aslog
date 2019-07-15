#ifndef bool
#include <stdbool.h>
#endif

typedef void * aslog_Logger_t;
aslog_Logger_t aslog_init(size_t buffer_size, bool (*logger)(void *));
void aslog_log_enqueue(aslog_Logger_t log, void *data, bool final);
bool aslog_shutdown(aslog_Logger_t);
