#ifndef CONFIG_H
#define CONFIG_H

#define CPU_F (16000000UL) // makefile directive

// In general returns
typedef enum {
    SUCCESS,
    TIMEOUT,
    ERROR,
} Result;

#endif