#include "logger.h"

#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

static void SPrintTime(char *const buffer, size_t const n, char const *const timespec) {
    time_t const t = time(NULL);
    struct tm *const time = localtime(&t);
    strftime(buffer, n, timespec, time);
}

void LoggerInitialize(logger_t *const logger) {
    size_t id = (size_t)logger;

    assert(logger);

    logger->id = id;
    {
        char buffer[256] = "";
        SPrintTime(buffer, 256, "%Y_%m_%d__%H_%M_%S");
        snprintf(logger->dirPath, 512, LOGGER_DIR "/%s__id%zu", buffer, id);
        snprintf(logger->logPath, 1024, "%s/log.html", logger->dirPath);
    }

    {
        char buffer[512+6] = "";
        snprintf(buffer, 512+6, "mkdir %s", logger->dirPath);
        system(buffer);
    }
}

FILE *LoggerStartEvent(logger_t logger, char const *title) {
    FILE *const file = fopen(logger.logPath, "a");
    fprintf(file, "<h2>Event: `%s`</h4>\n", title);
    {
        char buffer[256] = "";
        SPrintTime(buffer, 256, "%d %B %Y, %H:%M:%S");
        fprintf(file, "<h4>%s</h7>\n", buffer);
    }
    fprintf(file, "<pre>\n");
    return file;
}

void LoggerEndEvent(logger_t logger, FILE *file) {
    fprintf(file, "\n</pre>\n");
    fclose(file);

    {
        char buffer[1024+19];
        snprintf(buffer, 1024+19, "cp %s latest_log.html", logger.logPath);
        system(buffer);
    }
}
