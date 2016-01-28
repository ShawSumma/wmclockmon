/*
 * Tools : memory management, file loading and saving
 */

#ifndef TOOLS_H
#define TOOLS_H

#include "defines.h"
#include <stdio.h>

#define FREE(data) {if (data) free (data); data = NULL;}

int   fexist(const char *filename);
void *xmalloc(size_t size);
char *xstrdup(const char *string);
int   getbool(char *value);
char *robust_home();
char *get_file(const char *datestr);

#endif
