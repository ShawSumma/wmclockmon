/*
 * Create the main window.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include "../config.h"
#include "defines.h"
#include "mainwindow.h"
#include "main.h"
#include "tools.h"


#define UNIQUE 1
#define YEAR   2
#define MONTH  3

#define UNIQSTR "%04d-%02d-"
#define YEARSTR "XXXX-%02d-"
#define MONTSTR "XXXX-XX-"


/* Calendar part */
static GtkWidget *calendar;
static GtkWidget *closewindow;

/* Editor part */
static GtkWidget *text_buttons;
static GtkWidget *label_u;
static GtkWidget *label_y;
static GtkWidget *label_m;
static GtkWidget *button_u = NULL;
static GtkWidget *button_y = NULL;
static GtkWidget *button_m = NULL;
static GtkWidget *edit;
static GtkWidget *save;
static GtkWidget *delete;
static GtkWidget *cancel;
static int        shown = 1;
static int        dateb = 0;
static char       daystr[MAXSTRLEN + 1];


static void show_editor() {
    gtk_widget_hide(calendar);
    gtk_widget_hide(closewindow);
    gtk_widget_grab_default(GTK_WIDGET(cancel));
    gtk_widget_grab_focus(GTK_WIDGET(edit));
    gtk_widget_show(text_buttons);
    gtk_widget_show(edit);
    gtk_widget_show(save);
    gtk_widget_show(delete);
    gtk_widget_show(cancel);
}


static void hide_editor() {
    gtk_widget_hide(text_buttons);
    gtk_widget_hide(edit);
    gtk_widget_hide(save);
    gtk_widget_hide(delete);
    gtk_widget_hide(cancel);
    gtk_widget_show(calendar);
    gtk_widget_grab_default(GTK_WIDGET(closewindow));
    gtk_widget_grab_focus(GTK_WIDGET(closewindow));
    gtk_widget_show(closewindow);
}


static void toggle_displ(GtkWidget *widget, void *data) {
    switch (shown) {
        case 1:
            show_editor();
            shown = 2;
            break;
        case 2:
            hide_editor();
            shown = 1;
            break;
    }
}


static void load_file(const char *datestr) {
    FILE *file;
    char *filename = get_file(datestr);

    GtkTextIter iter;
    GtkTextBuffer *buf;

    buf = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit)));
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &iter);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &iter);

    if ((file = fopen(filename, "r")) != NULL) {
        while (! feof(file)) {
            char line[MAXSTRLEN + 1];
            bzero(line, MAXSTRLEN + 1);
            fgets(line, MAXSTRLEN, file);
            if (line[0] != 0)
	      gtk_text_buffer_insert(buf, &iter, line, -1);
        }
        fclose(file);
    }
    FREE(filename);
}


static void toggle_button(GtkWidget *button) {
    int is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

    if (is_active)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
}


static void toggle_buttons(int button) {
    switch (button) {
        case UNIQUE: toggle_button(button_u); break;
        case YEAR:   toggle_button(button_y); break;
        case MONTH:  toggle_button(button_m); break;
        default:     break;
    }
}


static void to_button(int button) {
    if (dateb == 0) dateb = button;
    if (button != dateb) {
        int b = dateb;
        dateb = button;
        toggle_buttons(b);
    }
}


static void set_buttons_text() {
    unsigned int  year, month, day;
    char datestr[MAXSTRLEN + 1];

    bzero(datestr, MAXSTRLEN + 1);
    gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
    month++;
    snprintf(datestr, MAXSTRLEN, UNIQSTR"%02d", year, month, day);
    gtk_label_set_text(GTK_LABEL(label_u), datestr);
    snprintf(datestr, MAXSTRLEN, YEARSTR"%02d", month, day);
    gtk_label_set_text(GTK_LABEL(label_y), datestr);
    snprintf(datestr, MAXSTRLEN, MONTSTR"%02d", day);
    gtk_label_set_text(GTK_LABEL(label_m), datestr);
}


static void editor_flush() {
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), "", 0);
}


static void editor_fill(int which) {
    char *dstr;
    GtkWidget *label = NULL;
    GtkTextIter iter;

    switch (which) {
        case UNIQUE: label = label_u; break;
        case YEAR:   label = label_y; break;
        case MONTH:  label = label_m; break;
        default:     break;
    }
    gtk_label_get(GTK_LABEL(label), &dstr);
    strcpy(daystr, dstr);
    to_button(which);
    editor_flush();
    load_file(daystr);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &iter);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &iter);
}


static void check_button(int bnum, GtkWidget *button) {
    int is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

    if (!(button_u && button_y && button_m)) return;
    if (!is_active && (dateb == bnum))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    if (is_active && (dateb != bnum)) editor_fill(bnum);
}


static void cal_click() {
    set_buttons_text();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_u), TRUE);
    editor_fill(UNIQUE);
}


static void set_text_u() {
    check_button(UNIQUE, button_u);
}


static void set_text_y() {
    check_button(YEAR, button_y);
}


static void set_text_m() {
    check_button(MONTH, button_m);
}


static void save_datas() {
    char *filename = get_file(daystr);
    int   len      = strlen(robust_home()) + strlen(DEFAULT_CONFIGDIR);
    char *dirname  = xmalloc(len + 2);
    struct stat stat_buf;

    GtkTextIter ts, te;
    gchar *tbuf;
    int tlen;

    tlen = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))));
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &ts, &te);

    sprintf(dirname, "%s/%s", robust_home(), DEFAULT_CONFIGDIR);
    if (tlen > 0) {
        if (! ((stat(dirname, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode)))
            mkdir(dirname, 0755);

        if ((stat(dirname, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode)) {
            FILE *file = fopen(filename, "w");
            unsigned int year, month, day;

            if (file) {
	      tbuf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit))), &ts, &te, TRUE);
	      fprintf(file, "%s", tbuf);
	      g_free(tbuf);
	      fflush(file);
	      fclose(file);
            }
            gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
            gtk_calendar_mark_day(GTK_CALENDAR(calendar), day);
        }
    }
    FREE(filename);
}


static void delete_file() {
    char *filename = get_file(daystr);
    unsigned int year, month, day;

    unlink(filename);
    gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
    gtk_calendar_unmark_day(GTK_CALENDAR(calendar), day);
    FREE(filename);
    editor_flush();
}


static int check_day(const char *filename, const char *startstr) {
    int day = -1;

    if (strncmp(filename, startstr, strlen(startstr)) == 0) {
        char format[12];
        sprintf(format, "%s%%d", startstr);
        sscanf(filename, format, &day);
    }

    return day;
}


static void mark_days() {
    char          *Home = robust_home();
    DIR           *dir;
    struct dirent *dir_ent;
    char          *dirname = xmalloc(
            strlen(Home) +
            strlen(DEFAULT_CONFIGDIR) +
            3);

    gtk_calendar_clear_marks(GTK_CALENDAR(calendar));
    sprintf(dirname, "%s/%s", Home, DEFAULT_CONFIGDIR);
    if ((dir = opendir(dirname)) != NULL) {
        char startstr_u[9]; /* unique (full date) */
        char startstr_y[9]; /* yearly date */
        char startstr_m[9]; /* monthly date */
        unsigned int year, month, day;

        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        month++;
        sprintf(startstr_u, UNIQSTR, year, month);
        sprintf(startstr_y, YEARSTR, month);
        sprintf(startstr_m, MONTSTR);
        while ((dir_ent = readdir(dir)) != NULL) {
            int day_u = check_day(dir_ent->d_name, startstr_u);
            int day_y = check_day(dir_ent->d_name, startstr_y);
            int day_m = check_day(dir_ent->d_name, startstr_m);
            if (day_u != -1)
                gtk_calendar_mark_day(GTK_CALENDAR(calendar), day_u);
            if (day_y != -1)
                gtk_calendar_mark_day(GTK_CALENDAR(calendar), day_y);
            if (day_m != -1)
                gtk_calendar_mark_day(GTK_CALENDAR(calendar), day_m);
        }
        closedir(dir);
    }
    FREE(dirname);
}


void create_mainwindow() {
    GtkWidget *main_vbox;
    GtkWidget *buttons_hbox;

    /*** FENÊTRE PRINCIPALE ***/
    application = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(application), PACKAGE" Calendar");
        /*-- Connexion aux signaux --*/
    gtk_signal_connect(GTK_OBJECT(application), "delete_event",
                       GTK_SIGNAL_FUNC(quit_app),  NULL);
    gtk_signal_connect(GTK_OBJECT(application), "destroy",
                       GTK_SIGNAL_FUNC(quit_app), "WM destroy");
        /*-- Taille de la fenêtre --*/
    gtk_widget_set_usize(GTK_WIDGET(application), WIN_WIDTH, WIN_HEIGHT);
    gtk_widget_realize(application);

    /*** Zone principale de placement des widgets***/
    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(application), main_vbox);
    gtk_widget_show(main_vbox);

    calendar = gtk_calendar_new();
    gtk_calendar_display_options(GTK_CALENDAR(calendar),
            GTK_CALENDAR_SHOW_HEADING   |
            GTK_CALENDAR_SHOW_DAY_NAMES |
            GTK_CALENDAR_WEEK_START_MONDAY);
    gtk_calendar_select_month(GTK_CALENDAR(calendar),
            timeinfos->tm_mon, timeinfos->tm_year + 1900);
    gtk_calendar_select_day(GTK_CALENDAR(calendar), timeinfos->tm_mday);
    mark_days();
    gtk_box_pack_start(GTK_BOX(main_vbox), calendar, TRUE, TRUE, 1);
    gtk_signal_connect(GTK_OBJECT(calendar), "day-selected-double-click",
            GTK_SIGNAL_FUNC(cal_click), NULL);
    gtk_signal_connect(GTK_OBJECT(calendar), "day-selected-double-click",
            GTK_SIGNAL_FUNC(toggle_displ), NULL);
    gtk_signal_connect(GTK_OBJECT(calendar), "month-changed",
            GTK_SIGNAL_FUNC(mark_days), NULL);
    gtk_widget_show(calendar);

    edit = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(edit), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(edit), GTK_WRAP_WORD_CHAR);

    gtk_box_pack_start(GTK_BOX(main_vbox), edit, TRUE, TRUE, 1);


    /*** BOUTONS DE CHANGEMENT DE TEXTE ***/
    text_buttons = gtk_hbox_new(FALSE, 1);
    gtk_box_pack_start(GTK_BOX(main_vbox), text_buttons, FALSE, TRUE, 1);


    button_u = gtk_toggle_button_new();
    gtk_signal_connect(GTK_OBJECT(button_u), "clicked",
            GTK_SIGNAL_FUNC(set_text_u), NULL);
    gtk_box_pack_start(GTK_BOX(text_buttons), button_u, TRUE, TRUE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_u), TRUE);
    gtk_widget_show(button_u);
    label_u = gtk_label_new(" Unique ");
    gtk_widget_show(label_u);
    gtk_container_add(GTK_CONTAINER(button_u), label_u);


    button_y = gtk_toggle_button_new();
    gtk_signal_connect(GTK_OBJECT(button_y), "clicked",
            GTK_SIGNAL_FUNC(set_text_y), NULL);
    gtk_box_pack_start(GTK_BOX(text_buttons), button_y, TRUE, TRUE, 0);
    gtk_widget_show(button_y);
    label_y = gtk_label_new(" Yearly ");
    gtk_widget_show(label_y);
    gtk_container_add(GTK_CONTAINER(button_y), label_y);


    button_m = gtk_toggle_button_new();
    gtk_signal_connect(GTK_OBJECT(button_m), "clicked",
            GTK_SIGNAL_FUNC(set_text_m), NULL);
    gtk_box_pack_start(GTK_BOX(text_buttons), button_m, TRUE, TRUE, 0);
    gtk_widget_show(button_m);
    label_m = gtk_label_new(" Monthly ");
    gtk_widget_show(label_m);
    gtk_container_add(GTK_CONTAINER(button_m), label_m);



    /*** BOUTONS DE SAUVEGARDE ET ANNULATION ***/
    buttons_hbox = gtk_hbox_new(FALSE, 1);
    gtk_box_pack_start(GTK_BOX(main_vbox), buttons_hbox, FALSE, TRUE, 1);
    gtk_widget_show(buttons_hbox);


    closewindow = gtk_button_new_with_label(" Close ");
    gtk_signal_connect(GTK_OBJECT(closewindow), "clicked",
            GTK_SIGNAL_FUNC(quit_app), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_hbox), closewindow, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(closewindow), GTK_CAN_DEFAULT);
    gtk_widget_grab_default(GTK_WIDGET(closewindow));
    gtk_widget_show(closewindow);


    save = gtk_button_new_with_label(" Save ");
    gtk_signal_connect(GTK_OBJECT(save), "clicked",
            GTK_SIGNAL_FUNC(save_datas), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_hbox), save, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(save), GTK_CAN_DEFAULT);


    delete = gtk_button_new_with_label(" Delete ");
    gtk_signal_connect(GTK_OBJECT(delete), "clicked",
            GTK_SIGNAL_FUNC(delete_file), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_hbox), delete, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(delete), GTK_CAN_DEFAULT);


    cancel = gtk_button_new_with_label(" Close ");
    gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
            GTK_SIGNAL_FUNC(toggle_displ), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_hbox), cancel, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(cancel), GTK_CAN_DEFAULT);


    /*** AFFICHAGE DE LA FENÊTRE ***/
    gtk_widget_show(application);
}
