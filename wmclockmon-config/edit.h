/*
 * edit.h
 */

#ifndef EDIT_H
#define EDIT_H

char *newalarm;

void edit_dialog(const char *title,
        int on,
        const char *atime,
        const char *adate,
        const char *amesg,
        void *f_ok);

#endif

