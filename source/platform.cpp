// platform specific stuff

#include "common.h"
#include "device.h"
#include "platform.h"
#include "ui.h"

#include <cstdarg>

namespace {
using namespace flashcart_core;

int loglevel = LOG_INFO;

char const *const priority_strings[] = {
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO] = "INFO",
    [LOG_NOTICE] = "NOTICE",
    [LOG_WARN] = "WARN",
    [LOG_ERR] = "ERROR"
};

char const * prioritytostr(log_priority priority) {
    return (priority >= LOG_PRIORITY_MAX) ? "?!#$" : priority_strings[priority];
}


}

char const * loglevel_str() {
    return prioritytostr((log_priority)loglevel);
}

void toggleLoglevel(void) {
    if (loglevel == 0) {
        loglevel = LOG_PRIORITY_MAX;
    }
    loglevel = loglevel - 1;
}

namespace flashcart_core {
    namespace platform {


void showProgress(uint32_t current, uint32_t total, const char* status_string) {
    ShowProgress(BOTTOM_SCREEN, current, total, status_string);
}

int logMessage(log_priority priority, const char *fmt, ...) {
    if (priority < loglevel) return 0;

    static bool first_open = true;
    // Overwrite if this is our first time opening the file.
    FILE *logfile = fopen("fat1:/ntrboot/ntrboot.log", first_open ? "w" : "a");
    if (!logfile) return -1;
    first_open = false;

    va_list args;
    va_start(args, fmt);

    const char *priority_str = prioritytostr(priority);

    char *log_fmt;
    if (asprintf(&log_fmt, "[%s]: %s\n", priority_str, fmt) < 0) {
        return -1; // would pass the actual return value back, but I don't think we care.
    }

    int result = vfprintf(logfile, log_fmt, args);
    fclose(logfile);
    va_end(args);

    free(log_fmt);
    return result;
}


    }
}
