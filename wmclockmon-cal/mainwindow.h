/*
 * Create the main window.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>

GtkWidget *application;
GtkWidget *alarmlist;
GtkWidget *wid_backlight;
GtkWidget *wid_blink;
GtkWidget *wid_styledir;
GtkWidget *wid_stylename;
GtkWidget *wid_color;
GtkWidget *wid_command;
GtkWidget *wid_msgcmd;
GtkWidget *wid_h12;
GtkWidget *wid_itm;
GtkWidget *wid_locale;

int selected_row;

gint list_unsel_cb (GtkCList *clist,
        gint row,
        gint column,
        GdkEventButton *event,
        void *data);
void create_mainwindow();

#endif
