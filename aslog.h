bool aslog_init(size_t buffer_size, bool (*logger)(void *));
void aslog_log_enqueue(void *data, bool final);
bool aslog_shutdown(void);
