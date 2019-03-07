# aslog
Create a logger running in a separate thread to offload the work of writing a log to disc (or do anything else that can safely be done asynchronously).

This project uses global variables for state, thus only one instance can be used at a time. I might change that at some point if I feel like it.

## Usage
Call `aslog_init(size_t buffer_size, bool (*logger)(void *))` to set up aslog. `buffer_size` is the number of log items that can be buffered. If the buffer is full, `aslog_log_enqueue` will block until that is no longer the case. `logger` is a function that will be called by aslog to do the actual logging or whatever it is you want to do. `logger` will be called with a pointer to some data that has been previously enqueued by you and should return `true` for a successful log and `false` if something goes wrong.

Call `aslog_log_enqueue(void *data, bool final)` to add a piece of data (pointed to by `data`) to the log buffer. Pass `final = true` if the item is the last one you want to log, so aslog does not keep waiting for data that is never going to come. aslog will not do anything with `*data` except pass the pointer back to your configured callback function. You are responsible for memory management. aslog is not your dad.

Call `aslog_shutdown()` to clean up all memory used by aslog and `join()` the logger thread. Do this after you have enqueued your last log item (as indicated by the `final` flag).
