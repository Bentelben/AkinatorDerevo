#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

#define LOGGER_DIR "logs"

struct logger_t {
    size_t id;
    char dirPath[512];
    char logPath[1024];
};

FILE *LoggerOpenLogFile(logger_t logger, char const *name, char const *mode);
void LoggerInitialize(logger_t *logger);
FILE *LoggerStartEvent(logger_t logger, char const *title);
void LoggerEndEvent(logger_t logger, FILE *file);

#endif
