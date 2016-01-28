/*
 * Tools : memory management, file loading and saving
 */

#include "../config.h"
#include "defines.h"
#include "variables.h"
#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include "configfile.h"


int fexist(const char *filename) {
    FILE *file;

    if ( (file = fopen(filename, "r") ) == NULL) return FALSE;
    fclose(file);

    return TRUE;
}


void alrm_add(Alarm **list, const char *value) {
    Alarm *lst = *list;
    int    ok  = TRUE;
    char  *time = NULL, *date = NULL, *ison = NULL, *mesg = NULL, *at;
    char  *tokstr = xstrdup(value);
    char  *toksav = tokstr;

    if (! value) return;
    if (! lst) {
        lst   = xmalloc(sizeof(Alarm));
        *list = lst;
    } else {
        if (strcmp(value, lst->entry) == 0) ok = FALSE;
        while ( (lst->next) && ok) {
            lst = lst->next;
            if (strcmp(value, lst->entry) == 0) ok = FALSE;
        }
        if (! ok) return;
        lst->next = xmalloc(sizeof(Alarm));
        lst = lst->next;
    }
    at = strchr(value, '@');
    if (at) ison = strtok(tokstr, "@");
    time = strtok(at ? NULL : tokstr, "-.");
    if (strchr(value, '-')) date = strtok(NULL, ".");
    mesg = strtok(NULL, "\n\0");
    lst->entry = xstrdup(value);
    lst->time  = time ? xstrdup(time) : NULL;
    lst->date  = date ? xstrdup(date) : NULL;
    lst->on    = ison ? getbool(ison) : TRUE;
    lst->msg   = mesg ? xstrdup(mesg) : NULL;
    lst->next  = NULL;
    FREE(toksav);
}


void free_alrm(Alarm **list) {
    Alarm *lst = *list, *next;
    while (lst) {
        next = lst->next;
        FREE(lst->entry);
        FREE(lst->time);
        FREE(lst->date);
        FREE(lst->msg);
        free(lst);
        lst = next;
    }
    *list = NULL;
}


int nb_alrm(Alarm *list) {
    Alarm *alrm = list;
    int      n = 0;
    while (alrm) {
        n++;
        alrm = alrm->next;
    }
    return n;
}




void *xmalloc(size_t size) {
    void *ret = malloc(size);
    if (ret == NULL) {
        perror("malloc() ");
        exit(-1);
    } else
        return ret;
}


char *xstrdup(const char *string) {
    char *ret = strdup(string);
    if (ret == NULL) {
        perror("strdup() ");
        exit(-1);
    } else
        return ret;
}


int getbool(char *value) {
    int i;
    for (i = 0 ; value[i] ; i++) value[i] = tolower(value[i]);
    if (strcmp(value, "0") == 0) return FALSE;
    if (strcmp(value, "1") == 0) return TRUE;
    if (strcmp(value, "true") == 0) return TRUE;
    if (strcmp(value, "false") == 0) return FALSE;
    if (strcmp(value, "yes") == 0) return TRUE;
    if (strcmp(value, "no") == 0) return FALSE;
    if (strcmp(value, "on") == 0) return TRUE;
    if (strcmp(value, "off") == 0) return FALSE;
    printf("Error in converting \"%s\" to boolean value.\n", value);
    return FALSE;
}


void load_cfgfile() {
    FILE *file;
    int  i = 0;
    char line[MAXSTRLEN + 1];
    char *value;

    if ((file = fopen(config_file, "r")) == NULL) {
        if (strstr(config_file, "/"DEFAULT_CFGFILE) == NULL)
            printf("Unable to open configuration file \"%s\".\n", config_file);
        return;
    }
    while (! feof(file)) {
        memset(line, 0, MAXSTRLEN + 1);
        fgets(line, MAXSTRLEN, file);
        i++;
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
        if ((line[0] == '#') || (line[0] == 0)) continue;
        value = strchr(line, '=') + 1;
        while ((value[0] == ' ') && (value[0] != 0)) value++;
        if (value[0] == 0) continue;

        if (strncmp(line, "Backlight", 9) == 0)
            backlight = getbool(value);
        else if (strncmp(line, "Color", 5) == 0)
            light_color = xstrdup(value);
        else if (strncmp(line, "Alarm", 5) == 0)
            alrm_add(&alarms, value);
        else if (strncmp(line, "Command", 7) == 0)
            command = xstrdup(value);
        else if (strncmp(line, "MessageCmd", 10) == 0)
            msgcmd = xstrdup(value);
        else if (strncmp(line, "Blink", 5) == 0)
            switch_authorized = getbool(value);
        else if (strncmp(line, "H12", 3) == 0)
            h12 = getbool(value);
        else if (strncmp(line, "Locale", 6) == 0)
            use_locale = getbool(value);
        else if (strncmp(line, "StyleDir", 8) == 0)
            style_dir = xstrdup(value);
        else if (strncmp(line, "Style", 5) == 0)
            style_name = xstrdup(value);
        else if (strncmp(line, "TimeMode", 5) == 0)
            time_mode = atoi(value);
        else if (strncmp(line, "ShowCal", 7) == 0)
            showcal = getbool(value);
        else if (strncmp(line, "CalAlrms", 8) == 0)
            calalrms = getbool(value);
        else
            printf("Error in %s at line %d :\n[%s].\n", config_file, i, line);
    }
}


char *robust_home() {
    if (getenv("HOME"))
        return getenv("HOME");
    else if (getenv("USER") && getpwnam(getenv("USER")))
        return getpwnam (getenv ("USER") )->pw_dir;
    else if (getenv ("LOGNAME") && getpwnam(getenv("LOGNAME")))
        return getpwnam(getenv("LOGNAME"))->pw_dir;
    else if ((getuid() != -1) && (getpwuid(getuid())))
        return getpwuid(getuid())->pw_dir;
    else
        return "/";
}


void save_cfgfile() {
    FILE  *file;
    Alarm *alrm = alarms;

    if ((file = fopen(config_file, "w")) == NULL) {
        if (strstr(config_file, "/"DEFAULT_CFGFILE) == NULL)
            printf("Unable to open configuration file \"%s\".\n", config_file);
        return;
    }

    fprintf(file, configfile,
            backlight ? "On" : "Off",
            light_color ? light_color : "", /*"#6ec63b",*/
            command ? command : "",
            msgcmd ? msgcmd : "",
            switch_authorized ? "Yes" : "No",
            h12 ? "True" : "False",
            time_mode,
            use_locale ? "Yes" : "No",
            style_dir ? style_dir : "",
            style_name ? style_name : "",
            showcal ? "Yes" : "No",
            calalrms ? "On" : "Off");
    while (alrm) {
        fprintf(file, alarmline,
                alrm->on ? "On" : "Off",
                alrm->time ? alrm->time : "12:00",
                alrm->date ? "-" : "",
                alrm->date ? alrm->date : "",
                alrm->msg ? "." : "",
                alrm->msg ? alrm->msg : "");
        alrm = alrm->next;
    }
    fclose(file);
}
