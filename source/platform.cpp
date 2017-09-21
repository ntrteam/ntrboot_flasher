// platform specific stuff

#include "common.h"
#include "device.h"
#include "protocol.h"
#include "protocol_ntr.h"
#include "ui.h"

#include <cstdarg>

void Flashcart::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, uint32_t flags) {
    NTR_SendCommand(cmdbuf, response_len, flags, resp);
}

void Flashcart::showProgress(uint32_t current, uint32_t total, const char* status_string) {
    ShowProgress(BOTTOM_SCREEN, current, total, status_string);
}

static FILE *logfile = nullptr;

static void open_logfile(void) {
    static bool first_open = true;
    // We want to overwrite if this is our first time opening the file this run.
    logfile = fopen("fat1:/ntrboot/ntrboot.log", first_open ? "w" : "a");
    first_open = false;
}

void close_logfile(void) {
    fclose(logfile);
    logfile = nullptr;
}

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

int Flashcart::logMessage(log_priority priority, const char *fmt, ...) {
    if (priority < LOG_LEVEL) return 0;
    if (!logfile) {
        open_logfile(); // automagicly open.
        if (!logfile) return -1;
    }
    va_list args;
    va_start(args, fmt);

    static char const *const priority_strings[] = {
        [LOG_DEBUG] = "DEBUG",
        [LOG_INFO] = "INFO",
        [LOG_NOTICE] = "NOTICE",
        [LOG_WARN] = "WARN",
        [LOG_ERR] = "ERROR"
    };
    const char *priority_str = (priority >= LOG_PRIORITY_MAX) ? "?!#$" : priority_strings[priority];

    char *log_fmt;
    if (asprintf(&log_fmt, "[%s]: %s\n", priority_str, fmt) < 0) {
        return -1; // would pass the actual return value back, but I don't think we care.
    }

    int result = vfprintf(logfile, log_fmt, args);
    va_end(args);

    free(log_fmt);
    return result;
}
