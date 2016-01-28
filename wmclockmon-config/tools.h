/*
 * Tools : memory management, file loading and saving
 */

#ifndef TOOLS_H
#define TOOLS_H

#include "defines.h"
#include <stdio.h>

#define FREE(data) {if (data) free (data); data = NULL;}

int   fexist(const char *filename);
void  alrm_add(Alarm **list, const char *value);
void  free_alrm(Alarm **list);
int   nb_alrm(Alarm *list);
void *xmalloc(size_t size);
char *xstrdup(const char *string);
int   getbool(char *value);
void  load_cfgfile();
char *robust_home();
void  save_cfgfile();

#endif
