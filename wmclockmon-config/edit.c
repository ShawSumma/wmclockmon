/*
 * edit.c
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "defines.h"
#include "tools.h"
#include "edit.h"

static GtkWidget *wid_status;
static GtkWidget *wid_time;
static GtkWidget *wid_date;
static GtkWidget *wid_mesg;

void set_data(GtkWidget *widget, GtkWidget *data) {
    char *wstat = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wid_status)->entry));
    char *wtime = gtk_entry_get_text(GTK_ENTRY(wid_time));
    char *wdate = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wid_date)->entry));
    char *wmesg = gtk_entry_get_text(GTK_ENTRY(wid_mesg));
    int   has_day = (strcmp(wdate, "ALL") != 0);
    int   has_msg = (strcmp(wdate, "") != 0);
    int   len = strlen(wstat) + 1 + strlen(wtime) + 1;
   
    len += has_day ? strlen(wdate) + 1 : 0;
    len += has_msg ? strlen(wmesg) + 1 : 0;
    newalarm = xmalloc(len);

    sprintf(newalarm, "%s@%s%s%s%s%s",
            wstat,
            wtime,
            has_day ? "-" : "",
            has_day ? wdate : "",
            has_msg ? "." : "",
            has_msg ? wmesg : "");
}


void edit_dialog(const char *title,
        int on,
        const char *atime,
        const char *adate,
        const char *amesg,
        void *f_ok) {
    GtkWidget *dialog;
    GtkWidget *bouton;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GList     *items = NULL;

    FREE(newalarm);
    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                       GTK_SIGNAL_FUNC(gtk_widget_destroy),
                       NULL);
    gtk_signal_connect(GTK_OBJECT(dialog), "destroy", 
                       GTK_SIGNAL_FUNC(gtk_widget_destroy), 
                       NULL);

    bouton = gtk_button_new_with_label("Ok");
    gtk_signal_connect(GTK_OBJECT(bouton), "clicked", 
                       GTK_SIGNAL_FUNC(set_data), 
                       NULL);
    gtk_signal_connect(GTK_OBJECT(bouton), "clicked", 
                       GTK_SIGNAL_FUNC(f_ok), 
                       NULL);
    gtk_signal_connect_object(GTK_OBJECT(bouton), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), 
                       bouton, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(bouton), GTK_CAN_DEFAULT);
    gtk_widget_grab_default(GTK_WIDGET(bouton));
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

    hbox = gtk_hbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
    gtk_widget_show(hbox);

    /* STATUS */
    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(hbox), vbox);
    gtk_widget_show(vbox);

    label = gtk_label_new("Status");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 6);
    gtk_widget_show(label);

    items = g_list_append(items, "On");
    items = g_list_append(items, "Off");
    wid_status = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(wid_status), items);
    gtk_combo_set_value_in_list(GTK_COMBO(wid_status), TRUE, FALSE);
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(wid_status)->entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), wid_status, FALSE, TRUE, 6);
    gtk_widget_show(wid_status);
    if (on)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wid_status)->entry), "On");
    else
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wid_status)->entry), "Off");


    /* HOUR */
    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(hbox), vbox);
    gtk_widget_show(vbox);

    label = gtk_label_new("Hour");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 6);
    gtk_widget_show(label);

    wid_time = gtk_entry_new();
    if (atime) gtk_entry_set_text(GTK_ENTRY(wid_time), atime);
    gtk_box_pack_start(GTK_BOX(vbox), wid_time, FALSE, TRUE, 6);
    gtk_widget_show(wid_time);


    /* DAY */
    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(hbox), vbox);
    gtk_widget_show(vbox);

    label = gtk_label_new("Day");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 6);
    gtk_widget_show(label);

    items = NULL;
    items = g_list_append(items, "ALL");
    items = g_list_append(items, "1");
    items = g_list_append(items, "2");
    items = g_list_append(items, "3");
    items = g_list_append(items, "4");
    items = g_list_append(items, "5");
    items = g_list_append(items, "6");
    items = g_list_append(items, "7");
    wid_date = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(wid_date), items);
    gtk_combo_set_value_in_list(GTK_COMBO(wid_date), TRUE, TRUE);
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(wid_date)->entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), wid_date, FALSE, TRUE, 6);
    gtk_widget_show(wid_date);
    if (adate)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wid_date)->entry), adate);

    
    /* MESSAGE */
    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
    gtk_widget_show(vbox);

    label = gtk_label_new("Message");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 6);
    gtk_widget_show(label);

    wid_mesg = gtk_entry_new();
    if (amesg) gtk_entry_set_text(GTK_ENTRY(wid_mesg), amesg);
    gtk_box_pack_start(GTK_BOX(vbox), wid_mesg, FALSE, TRUE, 6);
    gtk_widget_show(wid_mesg);



    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);
}

