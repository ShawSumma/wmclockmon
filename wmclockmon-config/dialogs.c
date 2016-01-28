/*
 * dialogs.
 */


#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <gtk/gtk.h>
#include "defines.h"
#include "dialogs.h"


void ync_dialog(const char *title, const char *text, void *f_yes, void *f_no) {
    GtkWidget *dialog;
    GtkWidget *bouton;
    GtkWidget *label;

    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                       GTK_SIGNAL_FUNC(gtk_widget_destroy),
                       NULL);
    gtk_signal_connect(GTK_OBJECT(dialog), "destroy", 
                       GTK_SIGNAL_FUNC(gtk_widget_destroy), 
                       NULL);

    bouton = gtk_button_new_with_label("Yes");
    gtk_signal_connect(GTK_OBJECT(bouton), "clicked", 
                       GTK_SIGNAL_FUNC(f_yes), 
                       NULL);
    gtk_signal_connect_object(GTK_OBJECT(bouton), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), 
                       bouton, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(bouton), GTK_CAN_DEFAULT);
    gtk_widget_grab_default(GTK_WIDGET(bouton));
    gtk_widget_draw_default(GTK_WIDGET(bouton));
    gtk_widget_show(bouton);

    bouton = gtk_button_new_with_label("No");
    gtk_signal_connect(GTK_OBJECT(bouton), "clicked", 
                       GTK_SIGNAL_FUNC(f_no), 
                       NULL);
    gtk_signal_connect_object(GTK_OBJECT(bouton), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), 
                       bouton, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(bouton), GTK_CAN_DEFAULT);
    gtk_widget_show(bouton);

    bouton = gtk_button_new_with_label("Cancel");
    gtk_signal_connect_object(GTK_OBJECT(bouton), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), 
                       bouton, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(bouton), GTK_CAN_DEFAULT);
    gtk_widget_show(bouton);

    gtk_container_set_border_width(GTK_CONTAINER
                                   (GTK_DIALOG(dialog)->vbox), 6);
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE,
                       TRUE, 6);
    gtk_widget_show(label);

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);
}


void ok_dialog(const char *title, const char *text) {
    GtkWidget *dialog;
    GtkWidget *bouton;
    GtkWidget *label;

    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                       GTK_SIGNAL_FUNC(gtk_widget_destroy),
                       NULL);
    gtk_signal_connect(GTK_OBJECT(dialog), "destroy", 
                       GTK_SIGNAL_FUNC(gtk_widget_destroy), 
                       NULL);

    bouton = gtk_button_new_with_label("OK");
    gtk_signal_connect_object(GTK_OBJECT(bouton), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), 
                       bouton, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(bouton), GTK_CAN_DEFAULT);
    gtk_widget_grab_default(GTK_WIDGET(bouton));
    gtk_widget_draw_default(GTK_WIDGET(bouton));
    gtk_widget_show(bouton);

    gtk_container_set_border_width(GTK_CONTAINER
                                   (GTK_DIALOG(dialog)->vbox), 6);
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE,
                       TRUE, 6);
    gtk_widget_show(label);

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);
}


void file_dialog(const char *title,
                  const char *dir, const char *filter,
                  void *ok_cb, void *cancel_cb) {
    GtkWidget *dialog;

    if (! title) title = "File...";

    dialog = gtk_file_selection_new(title);
    gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                       GTK_SIGNAL_FUNC(gtk_widget_destroy),
                       NULL);
    gtk_signal_connect(GTK_OBJECT(dialog), "destroy", 
                       GTK_SIGNAL_FUNC(gtk_widget_destroy), 
	               NULL);

    if (ok_cb != NULL)
        gtk_signal_connect(
            GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
            "clicked",
            GTK_SIGNAL_FUNC(ok_cb), dialog);
    gtk_signal_connect_object(
        GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
        "clicked",
        GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));

    if (cancel_cb != NULL)
        gtk_signal_connect(
            GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
            "clicked",
            GTK_SIGNAL_FUNC(cancel_cb), dialog);
    gtk_signal_connect_object(
        GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
        "clicked",
        GTK_SIGNAL_FUNC(gtk_widget_destroy),
        GTK_OBJECT(dialog));

    if (dir) {
        gtk_file_selection_set_filename(
            GTK_FILE_SELECTION(dialog),
            dir);
    }

    if (filter) {
        gtk_file_selection_complete(GTK_FILE_SELECTION(dialog), filter);
    }

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);
}

