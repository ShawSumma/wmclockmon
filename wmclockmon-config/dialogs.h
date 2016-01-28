/*
 * dialogs.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

void ync_dialog(const char *title, const char *text, void *f_yes, void *f_no);
void ok_dialog(const char *title, const char *text);
void file_dialog(const char *title,
                 const char *dir, const char *filter,
                 void *ok_cb, void *cancel_cb);

#endif
