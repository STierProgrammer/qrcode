#pragma once

#include <stdio.h>
#include <stdlib.h>
#define FPRINTF(fd, fmt, ...) fprintf(fd, fmt, ##__VA_ARGS__)

#define eprintf(fmt, ...)       FPRINTF(stderr, fmt, ##__VA_ARGS__)
#define printfln(fmt, ...)      FPRINTF(stdout, fmt "\n", ##__VA_ARGS__)
#define eprintfln(fmt, ...)     printf(fmt "\n", ##__VA_ARGS__)
