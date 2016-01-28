/*
 * Constants and types definitions
 */

#ifndef DEFINES_H
#define DEFINES_H

#define DEFAULT_CFGFILE ".wmclockmonrc"

#define MAXSTRLEN 1024

#define WIN_WIDTH  420
#define WIN_HEIGHT 420

typedef struct Alarm {
    char         *entry;
    char         *time;
    char         *date;
    char         *msg;
    int           on;
    struct Alarm *next;
} Alarm;

#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif


#endif
