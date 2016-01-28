/*
 *    WMClockMon - A dockapp to display time and date
 *    Copyright (C) 2002  Thomas Nemeth <tnemeth@free.fr>
 *
 *    Based on work by Seiichi SATO <ssato@sh.rim.or.jp>
 *    Copyright (C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>
 *    And on work by Mark Staggs <me@markstaggs.net>
 *    Copyright (C) 2002  Mark Staggs <me@markstaggs.net>

 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.

 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.

 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <locale.h>
#include <ctype.h>
#include "dockapp.h"
#include "backlightB_on.xpm"
#include "backlightB_off.xpm"
#include "backlightI_on.xpm"
#include "backlightI_off.xpm"
#include "backlight0_on.xpm"
#include "backlight0_off.xpm"
#include "parts.xpm"
#include "letters.xpm"


#define FREE(data) {if (data) free(data); data = NULL;}
#define SET_STRING(str, val) {if (str) free(str); str = xstrdup(val);}

#if defined(netbsd) || defined(openbsd) || defined(freebsd) || defined(darwin)
#  define BSDTIMEZONE
#endif


#define DEFAULT_CFGFILE ".wmclockmonrc"
#define DEFAULT_CONFIGDIR ".wmclockmoncal"
#define SIZE        58
#define MAXSTRLEN   512
#define WINDOWED_BG ". c #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

#define MTIME 1
#define ATIME 2


typedef enum { LIGHTON, LIGHTOFF } Light;

typedef enum { CLOCK, INTERNET, BINARY } TMode;

typedef enum {T_INT, T_BOOL, T_STRING, T_FILE} KType;

typedef struct {
    char *parts_s, *letters_s, *itime_s, *btime_s; /* styles files */
    char *parts, *letters; /* parts and letters pixmap files */
    char *backl, *backu;   /* main pixmap files */
    char *ibackl, *ibacku; /* internet time pixmap files */
    char *bbackl, *bbacku; /* binary time pixmap files */
    int  ncolors;          /* number of shadow colors */
    int  bdigith, bdigitw; /* big digits size */
    int  sdigith, sdigitw; /* small digits size */
    int  letterh, letterw; /* letters size */
    int  bsep, ssep, lsep; /* big, small digits and letters separations */
    int  hposx, hposy; /* hours */
    Bool hbig;         /* big digits for hours */
    int  mposx, mposy; /* minutes */
    Bool mbig;         /* big digits for minutes */
    int  sposx, sposy; /* seconds */
    Bool sbig, csec;   /* big digits for seconds, colon seconds */
    int  aposx, aposy; /* AM display */
    int  pposx, pposy; /* PM display */
    int  lposx, lposy; /* ALRM display */
    int  wposx, wposy; /* week day */
    int  dposx, dposy; /* day */
    int  oposx, oposy; /* month */
    int  bposx, bposy; /* beats */
    Bool bbig;         /* big digits for beats */
    int  tposx, tposy; /* 10th of beat */
    Bool tbig, tdisp;  /* big digits for 10th of beats, display them */
    int  gposx, gposy; /* beats graph bar */
    Bool gdisp;        /* display it */
    int  binhx, binhy; /* Binary hour X / Y */
    int  binmx, binmy; /* Binary minutes X / Y */
    int  binsx, binsy; /* Binary seconds X / Y */
    int  binzx, binzy; /* Binary time size X / size Y */
    int  binwx, binwy; /* Binary weekday X /Y */
    int  bindx, bindy; /* Binary day X / Y */
    int  binox, binoy; /* Binary month X / Y */
    int  binix, biniy; /* Binary date size X / Y */
    int  bind1x, bind1y; /* Binary delta X / delta Y, time same number */
    int  bind2x, bind2y; /* Binary delta X / delta Y, time tens->units */
    int  bind3x, bind3y; /* Binary delta X / delta Y, date same number */
    int  bind4x, bind4y; /* Binary delta X / delta Y, date tens->units */
} Style;


typedef struct {
    char *key;
    KType type;
    void *var;
    void *defval;
} StyleDef;


typedef struct Alarm {
    char         *entry;
    char         *alarm_time;
    char         *alarm_date;
    char         *message;
    Bool          on;
    Bool          cal;
    struct Alarm *next;
} Alarm;


static Style default_style = {
    /* FILES */
    NULL, NULL, NULL, NULL, /* styles files */
    NULL, NULL,           /* parts and letters pixmap files */
    NULL, NULL,           /* main pixmap files */
    NULL, NULL,           /* internet time pixmap files */
    NULL, NULL,           /* binary time pixmap files */
    /* DEFAULT SIZES */
    2,                    /* number of shadow colors */
    20, 10,               /* big digits size */
    9, 5,                 /* small digits size */
    7, 5,                 /* letters size */
    /* DEFAULT THEME - MAIN PART */
    2, 1, 1,              /* big, small digits and letters separations */
    5, 6, True,           /* hours pos and size */
    32, 6, True,          /* minutes pos and size */
    43, 28, False, False, /* seconds pos, size and colon */
    5, 28,                /* AM */
    5, 36,                /* PM */
    18, 28,               /* alarm */
    5, 47,                /* day of week */
    24, 45,               /* day */
    37, 47,               /* month */
    20, 6, True,          /* beats */
    /* DEFAULT THEME - BEATS FOR INTERNET TIME */
    49, 28, False, True,  /* 10th of beat pos, size and display */
    6, 45, True,          /* graph pos and display */
    /* DEFAULT THEME - BINARY PART */
    5, 5,                 /* Binary hour X / Y */
    23, 5,                /* Binary minutes X / Y */
    41, 5,                /* Binary seconds X / Y */
    6, 6,                 /* Binary time size X / size Y */
    14, 38,                /* Binary weekday X /Y */
    25, 38,               /* Binary day X / Y */
    43, 38,               /* Binary month X / Y */
    4, 3,                 /* Binary date size X / Y */
    0, 2,                 /* Binary delta X / delta Y, time same number */
    2, 0,                 /* Binary delta X / delta Y, time tens->units */
    0, 1,                 /* Binary delta X / delta Y, date same number */
    2, 0                  /* Binary delta X / delta Y, date tens->units */
};


static char *upcases[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "\250",
    NULL
};


Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap backdropI_on;
Pixmap backdropI_off;
Pixmap backdropB_on;
Pixmap backdropB_off;
Pixmap parts;
Pixmap letters;
Pixmap mask;
Style  style;
static char      *display_name     = "";
static char      *light_color      = NULL; /* led or backlight color */
static unsigned  update_interval   = 1;
static char      *style_name       = NULL;
static char      *style_dir        = NULL;
static Light     backlight         = LIGHTOFF;
static Bool      switch_authorized = True;
static char      *command          = NULL;
static char      *msgcmd           = NULL;
static char      *message          = NULL;
static Alarm     *alarms           = NULL;
static char      *config_file      = NULL;
static time_t    config_mtime      = 0;
static Bool      h12               = False;
static TMode     time_mode         = CLOCK;
static Bool      showcal           = False;
static Bool      calalrms          = False;
static Bool      use_locale        = True;
static struct tm *timeinfos;
static double    swtime;
static char*     label             = NULL;


/* prototypes */
static void rotate_style();
static Bool stringdiff(const char *s1, const char *s2);
static char *set_filename(const char *file_name, const char *ext);
static Bool set_style_value(StyleDef opt, char *value);
static void set_style_default(StyleDef opt);
static void load_stylepart(const char *filename, StyleDef *opts);
static void load_style(const char *stylename);
static void init_pixmap(char **src_pix, Pixmap *dst_pix, const char *text,
        XpmColorSymbol *c, int n, int keep_mask);
static void init_pixfile(char *src_name, Pixmap *dst_pix, const char *text,
        XpmColorSymbol *c, int n, int keep_mask);
static void graphics_init();
static void control(unsigned int btn, int x, int y, unsigned int state);
static void draw_dockapp();
static void update();
static void switch_light();
static void draw_background(Light back);
static void draw_bigdigit(int num, int x, int y);
static void draw_smalldigit(int num, int x, int y);
static char equiv(char letter);
static void draw_textdigit(const char *text, int x, int y);
static void draw_timedigit();
static void draw_datedigit();
static void draw_itimedigit();
static void draw_binarytime();
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
static void time_update();
static Bool raise_alarm();
static Bool fexist(const char *filename);
static Bool filestat(const char *filename, time_t *time, int mode);
static int  my_system(char *cmd, char *opt);
void *xmalloc(size_t size);
char *xstrdup(const char *string);
static void alrm_add(Alarm **list, const char *value);
static void free_alrm(Alarm **list);
static int nb_alrm(Alarm *list);
static Bool alarms_on(Alarm *list);
static void switch_alarms(Alarm *list);
static Bool getbool(char *value);
static Bool load_cfgfile();
static char *get_calend_file(int type);
static int cal_alrms_chg();
static void load_cal_file(int type);
static void load_calalrms();
static void reload_alarms();
static void show_cal_file(int type);
static void show_cal();
static char *robust_home();
static void signal_reload();



int main(int argc, char **argv) {
    XEvent event;
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
#ifdef SA_NOCLDWAIT
    sa.sa_flags = SA_NOCLDWAIT;
#else
    sa.sa_flags = 0;
#endif
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    /* Set default for style dir : */
    SET_STRING(style_dir, DATADIR);

    /* Init time */
    time_update();

    /* Load default configuration file */
    if (! config_file) {
        char *Home = robust_home();
        config_file = xmalloc(strlen(Home) + strlen(DEFAULT_CFGFILE) + 2);
        sprintf(config_file, "%s/%s", Home, DEFAULT_CFGFILE);
    }
    load_cfgfile();
    FREE(config_file);

    /* Parse CommandLine */
    parse_arguments(argc, argv);

    if (! config_file) {
        char *Home = robust_home();
        config_file = xmalloc(strlen(Home) + strlen(DEFAULT_CFGFILE) + 2);
        sprintf(config_file, "%s/%s", Home, DEFAULT_CFGFILE);
    } else {
        load_cfgfile();
    }
    style = default_style;
    load_style(style_name);

    if (use_locale) {
        setlocale(LC_TIME, "");
        setlocale(LC_CTYPE, "");
        setlocale(LC_COLLATE, "");
    }

    /* Initialize Application */
    dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask);

    graphics_init();

    if (showcal) show_cal();

    /* Main loop */
    while (1) {
        if (dockapp_nextevent_or_timeout(&event, update_interval * 1000 - 10)) {
            /* Next Event */
            switch (event.type) {
                case ButtonPress:
                    control(event.xbutton.button,
                            event.xbutton.x,
                            event.xbutton.y,
                            event.xbutton.state);
                    break;
                default: break;
            }
        } else {
            /* Time Out */
            update();
        }
    }

    return 0;
}


static void rotate_style() {
    DIR           *dir;
    struct dirent *dir_ent;
    char          *next = NULL;
    Bool           is_next = False;

    if (! style_dir) return;
    if ((dir = opendir(style_dir)) == NULL) return;
    while ((dir_ent = readdir(dir)) != NULL) {
        if (strstr(dir_ent->d_name, ".mwcs") != NULL) {
            if (is_next || (! style_name)) {
                next = xstrdup(dir_ent->d_name);
                is_next = False;
                break;
            }
            if (style_name && (strstr(style_name, dir_ent->d_name) != NULL))
                is_next = True;
        }
    }
    closedir(dir);
    if (next) {
        FREE(style_name);
        style_name = xmalloc(strlen(next)+strlen(style_dir)+2);
        sprintf(style_name, "%s/%s", style_dir, next);
    } else {
        FREE(style_name);
        FREE(style.parts_s);
        FREE(style.letters_s);
        FREE(style.itime_s);
        FREE(style.btime_s);
        FREE(style.parts);
        FREE(style.letters);
        FREE(style.backl);
        FREE(style.backu);
        FREE(style.ibackl);
        FREE(style.ibacku);
        FREE(style.bbackl);
        FREE(style.bbacku);
        style = default_style;
    }
    FREE(next);
}


static Bool stringdiff(const char *s1, const char *s2) {
    if (!s1 && !s2) return False;
    if (!s1 && s2)  return True;
    if (s1 && !s2)  return True;
    if (strcmp(s1, s2) == 0) return False;
    return True;
}


static char *set_filename(const char *file_name, const char *ext) {
    int  length = style_dir ? strlen(style_dir) + 1 : 0;
    char *filename = NULL;
    if (! file_name) return NULL;
    if ((file_name[0] == '.') || (file_name[0] == '/') ||
        (ext && (strstr(file_name, ext) != NULL))) { /* local file */
        char *e;
        filename = xstrdup(file_name);
        e = strrchr(filename, '/');
        if ((!style_dir) && (e != NULL)) {
            char *p = filename;
            int  i = 0, style_len = strlen(filename) - strlen(e);
            style_dir = xmalloc(style_len + 2);
            while (p != e) {
                style_dir[i++] = p[0];
                p++;
            }
            style_dir[i] = 0;
        }
    } else {
        length += strlen(file_name);
        length += ext ? strlen(ext) + 1 : 1;
        filename = xmalloc(length);
        sprintf(filename, "%s%s%s%s",
                style_dir ? style_dir : "",
                style_dir ? "/" : "",
                file_name,
                ext ? ext : "");
    }
    return filename;
}


static Bool set_style_value(StyleDef opt, char *value) {
    Bool res = True;
    char *string1 = NULL;
    char *string2 = NULL;

    switch (opt.type) {
        case T_INT:
            *(int *)(opt.var) = atoi(value);
            break;
        case T_BOOL:
            *(Bool *)(opt.var) = getbool(value);
            break;
        case T_STRING:
            string1 = *(char **)(opt.var);
            SET_STRING(string1, value);
            *(char **)(opt.var) = string1;
            break;
        case T_FILE:
            string1 = *(char **)(opt.var);
            string2 = set_filename(value, NULL);
            SET_STRING(string1, string2);
            *(char **)(opt.var) = string1;
            FREE(string2);
            break;
        default: res = False;
    }
    return res;
}


static void set_style_default(StyleDef opt) {
    char *string1 = NULL;
    char *string2 = NULL;

    switch (opt.type) {
        case T_INT:
            *(int *)(opt.var) = *(int *)(opt.defval);
            break;
        case T_BOOL:
            *(Bool *)(opt.var) = *(Bool *)(opt.defval);
            break;
        case T_STRING:
            string1 = *(char **)(opt.var);
            string2 = *(char **)(opt.defval);
            SET_STRING(string1, string2);
            *(char **)(opt.var) = string1;
            break;
        case T_FILE:
            string1 = *(char **)(opt.var);
            string2 = set_filename(*(char **)(opt.defval), NULL);
            SET_STRING(string1, string2);
            *(char **)(opt.var) = string1;
            FREE(string2);
            break;
    }
}


static void load_stylepart(const char *filename, StyleDef *opts) {
    FILE *file;
    int   i = 0;

    if ((!filename) || (filename[0] == '-')) {
        for (i = 0 ; opts[i].key ; i++)
            set_style_default(opts[i]);
        return;
    }
    if ((file = fopen(filename, "r")) == NULL) return;
    while (!feof(file)) {
        char line[MAXSTRLEN + 1], *value;
        int  j, set = False;
        bzero(line, MAXSTRLEN + 1);
        fgets(line, MAXSTRLEN, file);
 
        i++;
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
        if ((line[0] == '#') || (line[0] == 0)) continue;
        value = strchr(line, '=') + 1;
        while ((value[0] == ' ') && (value[0] != 0)) value++;
        if (value[0] == 0) continue;

        for (j = 0 ; opts[j].key ; j++) {
            int length = strlen(opts[j].key);
            if (strlen(line) <= length) continue;
            if (strncmp(line, opts[j].key, length) == 0) {
                set_style_value(opts[j], value);
                set = True;
                break;
            }
        }
        if (!set)
            fprintf(stderr, "Error in %s at line %d :\n[%s].\n",
                    filename, i, line);
    }
}


static void load_style(const char *stylename) {
    StyleDef  parts_style_opts[] = {
        {"Parts",          T_FILE, &style.parts,   &default_style.parts},
        {"BDigitHeight",   T_INT,  &style.bdigith, &default_style.bdigith},
        {"BDigitWidth",    T_INT,  &style.bdigitw, &default_style.bdigitw},
        {"SDigitHeight",   T_INT,  &style.sdigith, &default_style.sdigith},
        {"SDigitWidth",    T_INT,  &style.sdigitw, &default_style.sdigitw},
        {"BDigitSep",      T_INT,  &style.bsep,    &default_style.bsep},
        {"SDigitSep",      T_INT,  &style.ssep,    &default_style.ssep},
        {NULL,             T_STRING, NULL}
    };
    StyleDef  letters_style_opts[] = {
        {"Letters",        T_FILE, &style.letters, &default_style.letters},
        {"LetterHeight",   T_INT,  &style.letterh, &default_style.letterh},
        {"LetterWidth",    T_INT,  &style.letterw, &default_style.letterw},
        {"LetterSep",      T_INT,  &style.lsep,    &default_style.lsep},
        {NULL,             T_STRING, NULL}
    };
    StyleDef  itime_style_opts[] = {
        {"IBacklightOn",   T_FILE, &style.ibackl, &default_style.ibackl},
        {"IBacklightOff",  T_FILE, &style.ibacku, &default_style.ibacku},
        {"Beats_PosX",     T_INT,  &style.bposx,  &default_style.bposx},
        {"Beats_PosY",     T_INT,  &style.bposy,  &default_style.bposy},
        {"Beats_Big",      T_BOOL, &style.bbig,   &default_style.bbig},
        {"10thOB_PosX",    T_INT,  &style.tposx,  &default_style.tposx},
        {"10thOB_PosY",    T_INT,  &style.tposy,  &default_style.tposy},
        {"10thOB_Big",     T_BOOL, &style.tbig,   &default_style.tbig},
        {"10thOB_Display", T_BOOL, &style.tdisp,  &default_style.tdisp},
        {"Graph_PosX",     T_INT,  &style.gposx,  &default_style.gposx},
        {"Graph_PosY",     T_INT,  &style.gposy,  &default_style.gposy},
        {"Graph_Display",  T_BOOL, &style.gdisp,  &default_style.gdisp},
        {NULL,             T_STRING, NULL}
    };
    StyleDef  btime_style_opts[] = {
        {"BBacklightOn",  T_FILE, &style.bbackl, &default_style.bbackl},
        {"BBacklightOff", T_FILE, &style.bbacku, &default_style.bbacku},
        {"Bin_HX",        T_INT,  &style.binhx,  &default_style.binhx},
        {"Bin_HY",        T_INT,  &style.binhy,  &default_style.binhy},
        {"Bin_MX",        T_INT,  &style.binmx,  &default_style.binmx},
        {"Bin_MY",        T_INT,  &style.binmy,  &default_style.binmy},
        {"Bin_SX",        T_INT,  &style.binsx,  &default_style.binsx},
        {"Bin_SY",        T_INT,  &style.binsy,  &default_style.binsy},
        {"Bin_ZX",        T_INT,  &style.binzx,  &default_style.binzx},
        {"Bin_ZY",        T_INT,  &style.binzy,  &default_style.binzy},
        {"Bin_WX",        T_INT,  &style.binwx,  &default_style.binwx},
        {"Bin_WY",        T_INT,  &style.binwy,  &default_style.binwy},
        {"Bin_DX",        T_INT,  &style.bindx,  &default_style.bindx},
        {"Bin_DY",        T_INT,  &style.bindy,  &default_style.bindy},
        {"Bin_OX",        T_INT,  &style.binox,  &default_style.binox},
        {"Bin_OY",        T_INT,  &style.binoy,  &default_style.binoy},
        {"Bin_IX",        T_INT,  &style.binix,  &default_style.binix},
        {"Bin_IY",        T_INT,  &style.biniy,  &default_style.biniy},
        {"Bin_d1X",       T_INT,  &style.bind1x, &default_style.bind1x},
        {"Bin_d1Y",       T_INT,  &style.bind1y, &default_style.bind1y},
        {"Bin_d2X",       T_INT,  &style.bind2x, &default_style.bind2x},
        {"Bin_d2Y",       T_INT,  &style.bind2y, &default_style.bind2y},
        {"Bin_d3X",       T_INT,  &style.bind3x, &default_style.bind3x},
        {"Bin_d3Y",       T_INT,  &style.bind3y, &default_style.bind3y},
        {"Bin_d4X",       T_INT,  &style.bind4x, &default_style.bind4x},
        {"Bin_d4Y",       T_INT,  &style.bind4y, &default_style.bind4y},
        {NULL,            T_STRING, NULL}
    };
    StyleDef  main_style_opts[] = {
        {"PartsStyle",     T_FILE, &style.parts_s, &default_style.parts_s},
        {"LettersStyle",   T_FILE, &style.letters_s, &default_style.letters_s},
        {"ITimeStyle",     T_FILE, &style.itime_s, &default_style.itime_s},
        {"BTimeStyle",     T_FILE, &style.btime_s, &default_style.btime_s},
        {"BacklightOn",    T_FILE, &style.backl, &default_style.backl},
        {"BacklightOff",   T_FILE, &style.backu, &default_style.backu},
        {"NbColors",       T_INT,  &style.ncolors, &default_style.ncolors},
        {"Hours_PosX",     T_INT,  &style.hposx, &default_style.hposx},
        {"Hours_PosY",     T_INT,  &style.hposy, &default_style.hposy},
        {"Hours_Big",      T_BOOL, &style.hbig,  &default_style.hbig},
        {"Minutes_PosX",   T_INT,  &style.mposx, &default_style.mposx},
        {"Minutes_PosY",   T_INT,  &style.mposy, &default_style.mposy},
        {"Minutes_Big",    T_BOOL, &style.mbig,  &default_style.mbig},
        {"Seconds_PosX",   T_INT,  &style.sposx, &default_style.sposx},
        {"Seconds_PosY",   T_INT,  &style.sposy, &default_style.sposy},
        {"Seconds_Big",    T_BOOL, &style.sbig,  &default_style.sbig},
        {"Seconds_Colon",  T_BOOL, &style.csec,  &default_style.csec},
        {"AM_PosX",        T_INT,  &style.aposx, &default_style.aposx},
        {"AM_PosY",        T_INT,  &style.aposy, &default_style.aposy},
        {"PM_PosX",        T_INT,  &style.pposx, &default_style.pposx},
        {"PM_PosY",        T_INT,  &style.pposy, &default_style.pposy},
        {"ALRM_PosX",      T_INT,  &style.lposx, &default_style.lposx},
        {"ALRM_PosY",      T_INT,  &style.lposy, &default_style.lposy},
        {"Weekday_PosX",   T_INT,  &style.wposx, &default_style.wposx},
        {"Weekday_PosY",   T_INT,  &style.wposy, &default_style.wposy},
        {"Day_PosX",       T_INT,  &style.dposx, &default_style.dposx},
        {"Day_PosY",       T_INT,  &style.dposy, &default_style.dposy},
        {"Month_PosX",     T_INT,  &style.oposx, &default_style.oposx},
        {"Month_PosY",     T_INT,  &style.oposy, &default_style.oposy},
        {NULL,             T_STRING, NULL}
    };
    char *partsfile   = xstrdup(style.parts_s);
    char *lettersfile = xstrdup(style.letters_s);
    char *itimefile   = xstrdup(style.itime_s);
    char *btimefile   = xstrdup(style.btime_s);
    char *filename;

    if (! stylename) return;
    filename = set_filename(stylename, ".mwcs");

    /* main style */
    load_stylepart(filename, main_style_opts);

    /* parts */
    if (stringdiff(partsfile, style.parts_s) == True)
        load_stylepart(style.parts_s, parts_style_opts);

    /* letters */
    if (stringdiff(lettersfile, style.letters_s) == True)
        load_stylepart(style.letters_s, letters_style_opts);

    /* internet time */
    if (stringdiff(itimefile, style.itime_s) == True)
        load_stylepart(style.itime_s, itime_style_opts);

    /* binary clock */
    if (stringdiff(btimefile, style.btime_s) == True)
        load_stylepart(style.btime_s, btime_style_opts);

    FREE(partsfile);
    FREE(lettersfile);
    FREE(itimefile);
    FREE(btimefile);
    FREE(filename);
}


static void init_pixmap(char **src_pix, Pixmap *dst_pix, const char *text,
                        XpmColorSymbol *c, int n, int keep_mask) {
    if (*dst_pix) XFreePixmap(display, *dst_pix);
    if (!dockapp_xpm2pixmap(src_pix, dst_pix, &mask, c, n)) {
        fprintf(stderr, "Error initializing %s image.\n", text);
        exit(1);
    }
    if (!keep_mask && mask) XFreePixmap(display, mask);
}


static void init_pixfile(char *src_name, Pixmap *dst_pix, const char *text,
                        XpmColorSymbol *c, int n, int keep_mask) {
    if (*dst_pix) XFreePixmap(display, *dst_pix);
    if (!dockapp_file2pixmap(src_name, dst_pix, &mask, c, n)) {
        fprintf(stderr, "Error initializing %s image.\n", text);
        exit(1);
    }
    if (!keep_mask && mask) XFreePixmap(display, mask);
}


static void graphics_init() {
    XpmColorSymbol colors[3] = { {"Back0", NULL, 0}, {"Back1", NULL, 0}, {"Back2", NULL, 0} };
    int ncolor = 0;

    if (light_color) {
        ncolor = style.ncolors;
        if (ncolor == 2) {
            colors[0].pixel = dockapp_getcolor(light_color);
            colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
        } else {
            colors[0].pixel = dockapp_getcolor(light_color);
            colors[1].pixel = dockapp_dividecolor(light_color, 3);
            colors[2].pixel = dockapp_blendedcolor(light_color, -50, -50, -50, 1.0);
        }
    }

    /* change raw xpm data to pixmap */
    /*
    if (dockapp_iswindowed)
        backlit_pix[1] = backgrd_pix[1] = WINDOWED_BG;
    */

    if (style.parts)
        init_pixfile(style.parts, &parts, "parts",
                colors, ncolor, False);
    else
        init_pixmap(parts_xpm,    &parts, "parts",
                colors, ncolor, False);

    if (style.letters)
        init_pixfile(style.letters, &letters, "letters",
                colors, ncolor, False);
    else
        init_pixmap(letters_xpm,    &letters, "letters",
                colors, ncolor, False);

    if (style.ibackl)
        init_pixfile(style.ibackl,     &backdropI_on, "backlit background 2",
                colors, ncolor, False);
    else
        init_pixmap(backlightI_on_xpm, &backdropI_on, "backlit background 2",
                colors, ncolor, False);

    if (style.ibacku)
        init_pixfile(style.ibacku,      &backdropI_off, "background 2",
                colors, ncolor, False);
    else
        init_pixmap(backlightI_off_xpm, &backdropI_off, "background 2",
                colors, ncolor, False);

    if (style.bbackl)
        init_pixfile(style.bbackl,      &backdropB_on, "backlit background 3",
                colors, ncolor, False);
    else
        init_pixmap(backlightB_on_xpm, &backdropB_on, "backlit background 3",
                colors, ncolor, False);

    if (style.bbacku)
        init_pixfile(style.bbacku,      &backdropB_off, "background 3",
                colors, ncolor, False);
    else
        init_pixmap(backlightB_off_xpm, &backdropB_off, "background 3",
                colors, ncolor, False);

    if (style.backl)
        init_pixfile(style.backl,      &backdrop_on, "backlit background",
                colors, ncolor, False);
    else
        init_pixmap(backlight0_on_xpm, &backdrop_on, "backlit background",
                colors, ncolor, False);

    if (style.backu)
        init_pixfile(style.backu,       &backdrop_off, "background",
                colors, ncolor, True);
    else
        init_pixmap(backlight0_off_xpm, &backdrop_off, "background",
                colors, ncolor, True);

    /* shape window */
    if (!dockapp_iswindowed) dockapp_setshape(mask, 0, 0);
    if (mask) XFreePixmap(display, mask);

    /* pixmap : draw area */
    if (pixmap) XFreePixmap(display, pixmap);
    pixmap = dockapp_XCreatePixmap(SIZE, SIZE);

    /* Initialize pixmap */
    draw_background(backlight);

    dockapp_set_background(pixmap);
    update();
    dockapp_show();
}


static void control(unsigned int btn, int x, int y, unsigned int state) {
    switch (btn) {
        case 1:
            if (time_mode == CLOCK) {
                if ( (x >= style.aposx) && (x <= style.aposx + 12) &&
                     (y >= style.aposy) && (y <= style.aposy + 7) ) {
                    h12 = !h12;
                    break;
                } else if ( (x >= style.pposx) && (x <= style.pposx + 12) &&
                            (y >= style.pposy) && (y <= style.pposy + 7) ) {
                    h12 = !h12;
                    break;
                } else if ( (x >= style.lposx) && (x <= style.lposx + 22) &&
                            (y >= style.lposy) && (y <= style.lposy + 7) ) {
                    if (alarms) switch_alarms(alarms);
                    break;
                }
            }
            if (state & ControlMask) {
                time_mode += 1;
                if (time_mode == 3) time_mode = CLOCK;
                time_update();
                draw_dockapp();
            } else {
                switch_light();
            }
            break;
        case 2:
            if (state & ControlMask) {
                my_system("wmclockmon-config -f", config_file);
            } else {
                rotate_style();
                load_style(style_name);
                graphics_init();
                draw_dockapp();
            }
            break;
        case 3:
            if (state & ControlMask) {
                my_system("wmclockmon-cal", NULL);
            } else {
                switch_authorized = !switch_authorized;
            }
            break;
        default: break;
    }
}


static void draw_dockapp() {
    /* all clear */
    draw_background(backlight);

    /* draw digit */
    if (time_mode == INTERNET) {
        draw_itimedigit();
    } else if (time_mode == BINARY) {
        draw_binarytime();
    } else {
        draw_timedigit();
        draw_datedigit();
    }

    /* show */
    dockapp_copy2window(pixmap);
}


/* called by timer */
static void update() {
    static Light pre_backlight;
    static Bool  in_alarm_mode = False;

    /* check config_file modifications */
    if (load_cfgfile()) graphics_init();

    /* get current time */
    time_update();

    /* alarm mode */
    if (raise_alarm()) {
        if (!in_alarm_mode) {
            in_alarm_mode = True;
            my_system(command, NULL);
            if (message) my_system(msgcmd, message);
            pre_backlight = backlight;
        }
        if ( (switch_authorized) ||
             ( (switch_authorized) && (backlight != pre_backlight) ) ) {
            switch_light();
            return;
        }
    } else {
        if (in_alarm_mode) {
            in_alarm_mode = False;
            if (backlight != pre_backlight) {
                switch_light();
                return;
            }
        }
    }
    draw_dockapp();
}


/* called when mouse button pressed */
static void switch_light() {
    switch (backlight) {
        case LIGHTOFF: backlight = LIGHTON;  break;
        case LIGHTON:  backlight = LIGHTOFF; break;
    }
    /* redraw digit */
    time_update();
    draw_dockapp();
}


static void draw_background(Light back) {
    if (back == LIGHTON) {
        if (time_mode == INTERNET)
            dockapp_copyarea(backdropI_on, pixmap, 0, 0, 58, 58, 0, 0);
        else if (time_mode == BINARY)
            dockapp_copyarea(backdropB_on, pixmap, 0, 0, 58, 58, 0, 0);
        else
            dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
    } else {
        if (time_mode == INTERNET)
            dockapp_copyarea(backdropI_off, pixmap, 0, 0, 58, 58, 0, 0);
        else if (time_mode == BINARY)
            dockapp_copyarea(backdropB_off, pixmap, 0, 0, 58, 58, 0, 0);
        else
            dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
    }
}


static void draw_bigdigit(int num, int x, int y) {
    int dy = 0, w = style.bdigitw, h = style.bdigith;
    int incr = style.bdigitw + style.bsep;

    if (num < 0) num = 0;
    if (backlight == LIGHTON) dy = style.bdigith;

    /* draw digit */
    dockapp_copyarea(parts, pixmap, (num % 10) * w, dy, w, h, x + incr, y);
    dockapp_copyarea(parts, pixmap, (num / 10) * w, dy, w, h, x, y);
}


static void draw_smalldigit(int num, int x, int y) {
    int dx = 0, w = style.sdigitw, h = style.sdigith;
    int incr = style.sdigitw + style.bsep;
    int dy = 2 * style.bdigith;

    if (num < 0) num = 0;
    if (backlight == LIGHTON) dx = style.sdigitw * 10;

    /* draw digit */
    dockapp_copyarea(parts, pixmap, (num % 10) * w + dx, dy, w, h, x + incr, y);
    dockapp_copyarea(parts, pixmap, (num / 10) * w + dx, dy, w, h, x, y);
}


static char equiv(char letter) {
    int  i, sign, oldsign = 0;
    char letr[2], upcase, ret = 0;

    upcase = toupper(letter);
    letr[0] = upcase; letr[1] = 0;
    if (! isupper(upcase)) return letter;
    for (i = 0 ; upcases[i] ; i++) {
        sign = strcoll(letr, upcases[i]);
        if ((oldsign > 0) && (sign < 0)) ret = upcases[i-1][0];
        oldsign = sign;
    }
    if (ret == 0) ret = upcase;
    return ret;
}


static void draw_textdigit(const char *text, int x, int y) {
    int i, dy = style.letterh, incr = style.letterw + style.lsep;
    int w = style.letterw, h = style.letterh;

    if (backlight == LIGHTON) dy = style.letterh * 2;
    for (i = 0 ; text[i] ; i ++) {
        int pos = equiv(text[i]) - 'A';
        dockapp_copyarea(letters, pixmap, pos * w, dy, w, h, x + i * incr, y);
    }
}


static void draw_timedigit() {
    int hour = timeinfos->tm_hour, dx = 0, dy = 0;

    if (backlight == LIGHTON) {
        dx = style.sdigitw * 10; /* AM/PM/ALRM parts positions */
        dy = style.bdigith; /* colon seconds */
    }

    if (h12) {
        if (hour == 0)
            hour = 12;
        else
            hour = (hour > 12) ? hour - 12 : hour;
    }

    if (style.hbig)
        draw_bigdigit(hour, style.hposx, style.hposy);
    else
        draw_smalldigit(hour, style.hposx, style.hposy);

    if (style.mbig)
        draw_bigdigit(timeinfos->tm_min, style.mposx, style.mposy);
    else
        draw_smalldigit(timeinfos->tm_min, style.mposx, style.mposy);

    if (style.csec) {
        if (timeinfos->tm_sec % 2 == 1) {
            if (style.sbig) {
                dockapp_copyarea(parts, pixmap, 4, dy, 2, 2, style.sposx, style.sposy);
                dockapp_copyarea(parts, pixmap, 4, dy, 2, 2, style.sposx, style.sposy + 9);
            } else {
                dockapp_copyarea(parts, pixmap, 4, dy, 1, 1, style.sposx, style.sposy);
                dockapp_copyarea(parts, pixmap, 4, dy, 1, 1, style.sposx, style.sposy + 4);
            }
        }
    } else {
        if (style.sbig)
            draw_bigdigit(timeinfos->tm_sec, style.sposx, style.sposy);
        else
            draw_smalldigit(timeinfos->tm_sec, style.sposx, style.sposy);
    }

    if (h12) {
        /* Some say pm is when h>12 or h==0 but others (and my watch) told me
         * that the good way to handle am/pm is what is below
         */
        if (timeinfos->tm_hour >= 12) /* PM */
            dockapp_copyarea(parts, pixmap, 36 + dx, 49, 12, 7, style.pposx, style.pposy);
        else
            dockapp_copyarea(parts, pixmap, 23 + dx, 49, 12, 7, style.aposx, style.aposy);
    }
    if (alarms_on(alarms))
        dockapp_copyarea(parts, pixmap, dx, 49, 22, 7, style.lposx, style.lposy);
}


static void draw_datedigit() {
    char text[5];

    if (label) {
        draw_textdigit(label, style.wposx, style.wposy);
    } else {
        strftime(text, 4, "%a", timeinfos);
        draw_textdigit(text, style.wposx, style.wposy);
        draw_smalldigit(timeinfos->tm_mday, style.dposx, style.dposy);
        strftime(text, 4, "%b", timeinfos);
        draw_textdigit(text, style.oposx, style.oposy);
    }
}


static void draw_itimedigit() {
    int dx = 0, dy = 0, v1, v10, v100, v0, v00, nb, pc;
    int bw = style.bdigitw, bh = style.bdigith;
    int binc = style.bdigitw + 2;
    int tw = style.sdigitw, th = style.bdigith;
    int xd = 100;

    if (backlight == LIGHTON) {
        dx = 50;
        dy = 20;
        xd = 102;
    }

    /* use floor(3) */    
    v100 = swtime / 100.0;
    v10  = (swtime - v100 * 100) / 10.0;
    v1   = (swtime - v100 * 100.0 - v10 * 10.0);
    v0   = (swtime - v100 * 100.0 - v10 * 10.0 - v1) * 10;
    v00  = (swtime - v100 * 100.0 - v10 * 10.0 - v1) * 1000;
    pc   = v00 - v0 * 100;

    /* draw main beats digit */
    if (!style.bbig) {
        bw = tw;
        bh = th;
    }
    dockapp_copyarea(parts, pixmap, v1 * 10, dy, bw, bh, style.bposx + 2 * binc, style.bposy);
    dockapp_copyarea(parts, pixmap, v10 * 10, dy, bw, bh, style.bposx + binc, style.bposy);
    dockapp_copyarea(parts, pixmap, v100 * 10, dy, bw, bh, style.bposx, style.bposy);

    /* draw 10th of beats */
    dockapp_copyarea(parts, pixmap, v0 * 5 + dx, 40, 5, 9, 49, 28);

    /* graph */
    for (nb = 0 ; nb < pc / 6.25 ; nb++)
        dockapp_copyarea(parts, pixmap, xd, 0, 2, 9, 6 + nb * 3, 45);
}


static void draw_bits(int num, int x, int y, int u, int d, int sx, int sy, int d1x, int d1y, int d2x, int d2y) {
    int v = num;
    int dx = u * (sx + d2x) + d1x;
    int dy = sy + d1y;
    int b[4], i = 0;
    int bx = 0;
    int by = 2 * style.bdigith + style.sdigith + 7 + d;

    if (num == 0) return;
    if (backlight == LIGHTON) bx = 50;

    b[3] = 0; b[2] = 0; b[1] = 0; b[0] = 0;
    while (v != 0) {
        if (i == 4) {
            fprintf(stderr, "Error : num is too big (%d)\n", num);
            exit(1);
        }
        b[i] = v % 2;
        v = v / 2;
        i++;
    }
    for (i = 0 ; i < 4 ; i++) {
        if (b[i]) {
            int px = x + dx;
            int py = y + (3 - i) * dy + d2y;
            dockapp_copyarea(parts, pixmap, bx, by, sx, sy, px, py);
        }
    }
}


static void draw_binarytime() {
    int sx = style.binzx;
    int sy = style.binzy;
    int d1x = style.bind1x;
    int d1y = style.bind1y;
    int d2x = style.bind2x;
    int d2y = style.bind2y;

    draw_bits(timeinfos->tm_hour / 10, style.binhx, style.binhy, 0, 0,
            sx, sy, d1x, d1y, d2x, d2y);
    draw_bits(timeinfos->tm_hour % 10, style.binhx, style.binhy, 1, 0,
            sx, sy, d1x, d1y, d2x, d2y);

    draw_bits(timeinfos->tm_min / 10, style.binmx, style.binmy, 0, 0,
            sx, sy, d1x, d1y, d2x, d2y);
    draw_bits(timeinfos->tm_min % 10, style.binmx, style.binmy, 1, 0,
            sx, sy, d1x, d1y, d2x, d2y);

    draw_bits(timeinfos->tm_sec / 10, style.binsx, style.binsy, 0, 0,
            sx, sy, d1x, d1y, d2x, d2y);
    draw_bits(timeinfos->tm_sec % 10, style.binsx, style.binsy, 1, 0,
            sx, sy, d1x, d1y, d2x, d2y);

    if (style.binix != -1) {
        int wd = timeinfos->tm_wday == 0 ? 7 : timeinfos->tm_wday;

        sx = style.binix;
        sy = style.biniy;
        d1x = style.bind3x;
        d1y = style.bind3y;
        d2x = style.bind4x;
        d2y = style.bind4y;

        draw_bits(wd, style.binwx, style.binwy, 0, 1,
            sx, sy, d1x, d1y, d2x, d2y);

        draw_bits(timeinfos->tm_mday / 10, style.bindx, style.bindy, 0, 1,
            sx, sy, d1x, d1y, d2x, d2y);
        draw_bits(timeinfos->tm_mday % 10, style.bindx, style.bindy, 1, 1,
            sx, sy, d1x, d1y, d2x, d2y);

        draw_bits((timeinfos->tm_mon + 1) / 10, style.binox, style.binoy, 0, 1,
            sx, sy, d1x, d1y, d2x, d2y);
        draw_bits((timeinfos->tm_mon + 1) % 10, style.binox, style.binoy, 1, 1,
            sx, sy, d1x, d1y, d2x, d2y);
    }
}


static void parse_arguments(int argc, char **argv) {
    int i, integer;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            print_help(argv[0]), exit(0);
        } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
            printf("%s version %s\n", PACKAGE, VERSION), exit(0);
        } else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) {
            display_name = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "--backlight") || !strcmp(argv[i], "-bl")) {
            backlight = LIGHTON;
        } else if (!strcmp(argv[i], "--light-color") || !strcmp(argv[i], "-lc")) {
            if (argc == i + 1)
                fprintf(stderr, "%s: argument \"%s\" needs an option.\n", argv[0], argv[i]), exit(1);
            SET_STRING(light_color, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w")) {
            dockapp_iswindowed = True;
        } else if (!strcmp(argv[i], "--broken-wm") || !strcmp(argv[i], "-bw")) {
            dockapp_isbrokenwm = True;
        } else if (!strcmp(argv[i], "--no-blink") || !strcmp(argv[i], "-nb")) {
            switch_authorized = False;
        } else if (!strcmp(argv[i], "--h12") || !strcmp(argv[i], "-12")) {
            h12 = True;
        } else if (!strcmp(argv[i], "--time-mode") || !strcmp(argv[i], "-tm")) {
            if (argc == i + 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if (sscanf(argv[i + 1], "%i", &integer) != 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if ((integer < 0) || (integer > 2))
                fprintf(stderr, "%s: argument %s must be in [0,2]\n",
                        argv[0], argv[i]), exit(1);
            time_mode = integer;
            i++;
        } else if (!strcmp(argv[i], "--alarm-cmd") || !strcmp(argv[i], "-c")) {
            SET_STRING(command, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--message-cmd") || !strcmp(argv[i], "-mc")) {
            SET_STRING(msgcmd, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--alarm") || !strcmp(argv[i], "-a")) {
            alrm_add(&alarms, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--cfg-file") || !strcmp(argv[i], "-f")) {
            SET_STRING(config_file, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--no-locale") || !strcmp(argv[i], "-nl")) {
            use_locale = False;
        } else if (!strcmp(argv[i], "--style") || !strcmp(argv[i], "-s")) {
            SET_STRING(style_name, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--style-dir") || !strcmp(argv[i], "-sd")) {
            SET_STRING(style_dir, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--label") || !strcmp(argv[i], "-l")) {
            SET_STRING(label, argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "--show-cal") || !strcmp(argv[i], "-sc")) {
            showcal = True;
        } else if (!strcmp(argv[i], "--cal-alrm") || !strcmp(argv[i], "-ca")) {
            calalrms = True;
            load_calalrms();
        } else {
            fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i]);
            print_help(argv[0]), exit(1);
        }
    }
}


static void print_help(char *prog)
{
    printf("Usage : %s [OPTIONS]\n"
           "%s - Window Maker digital clock dockapp\n"
           " -h,  --help                  show this help text and exit\n"
           " -v,  --version               show program version and exit\n"
           " -d,  --display <string>      display to use\n"
           " -bl, --backlight             turn on back-light\n"
           " -lc, --light-color <color>   backlight/led colour\n"
           " -tm, --time-mode <mode>      start with time mode (0: clock, 1: internet time, 2: binary clock)\n"
           " -w,  --windowed              run the application in windowed mode\n"
           " -bw, --broken-wm             activate broken window manager fix\n"
           " -a,  --alarm <HH:MM>         set alarm time to HH:MM (24h clock mode)\n"
           " -c,  --alarm-cmd <string>    command to launch when alarm raises\n"
           " -mc, --message-cmd <string>  command to launch when alarm raises and a message is associated\n"
           " -12, --h12                   12 hours clock mode (default is 24)\n"
           " -s,  --style <file>          style to use for display\n"
           " -sd, --style-dir <dir>       directory where styles are stored\n"
           " -nb, --no-blink              disable blinking when alarm is raised\n"
           " -f,  --cfgfile <filename>    use 'filename' as configuration file\n"
           " -nl, --no-locale             don't use current locale\n"
           " -l,  --label <string>        use a label instead of date\n"
           " -ca, --cal-alrm              load calendar alarms for today\n"
           " -sc, --show-cal              show calendar at startup/00:00\n",
           prog, prog);
}


static void time_update() {
    time_t     tnow;

    time(&tnow);
    timeinfos = localtime(&tnow);

    if (time_mode == INTERNET) {
        long localtmzone;
        swtime = timeinfos->tm_hour * 3600
               + timeinfos->tm_min * 60
               + timeinfos->tm_sec;
#ifdef BSDTIMEZONE
        localtmzone = timeinfos->tm_gmtoff;
#else
        localtmzone = timezone;
#endif
        swtime += localtmzone+3600;
        if (timeinfos->tm_isdst) swtime -= 3600;
        swtime *= 1000;
        swtime /= 86400;
        if (swtime >= 1000)
            swtime -= 1000;
        else
            if (swtime < 0) swtime += 1000;
    }
}


static Bool raise_alarm() {
    if ((timeinfos->tm_hour == 0) &&
        (timeinfos->tm_min  == 0) &&
        (timeinfos->tm_sec  == 0)) {
        if (showcal) show_cal();
        if (calalrms) reload_alarms();
    }
    if (alarms) {
        Alarm  *alrm = alarms;
        char    thistime[MAXSTRLEN + 1];
        char    thisdate[MAXSTRLEN + 1];

        strftime(thistime, MAXSTRLEN, "%H:%M", timeinfos);
        strftime(thisdate, MAXSTRLEN, "%u", timeinfos);
        while (alrm) {
            if (alrm->on && (strcmp(thistime, alrm->alarm_time) == 0)) {
                message = alrm->message;
                if (alrm->alarm_date) {
                    if (strcmp(thisdate, alrm->alarm_date) == 0)
                        return True;
                    else
                        message = NULL;
                } else {
                    return True;
                }
            }
            alrm = alrm->next;
        }
    }
    return False;
}


static Bool fexist(const char *filename) {
    FILE           *file;

    if ((file = fopen(filename, "r")) == NULL) return False;
    fclose(file);

    return True;
}


static Bool filestat(const char *filename, time_t *time, int mode) {
    struct stat s;
    time_t      t = *time;

    if (stat(filename, &s) == -1) {
        if (*time == 0) return False;
        return True;
    }
    switch (mode) {
        case MTIME: t = s.st_mtime; break;
        case ATIME: t = s.st_atime; break;
        default: break;
    }
    if (t == *time) {
        return False;
    } else {
        *time = t;
        return True;
    }
}


static int my_system(char *cmd, char *opt) {
    int           pid;
    extern char **environ;

    if (cmd == NULL) return 1;
    pid = fork();
    if (pid == -1) return -1;
    if (pid == 0) {
        pid = fork();
        if (pid == 0) {
            char *argv[3];
            char *thiscommand = xmalloc(strlen(cmd)
                    + (opt ? strlen(opt) + 4 : 1));
            sprintf(thiscommand, "%s %s%s%s", cmd,
                    opt ? "\"" : "",
                    opt ? opt  : "",
                    opt ? "\"" : "");
            argv[0] = "sh";
            argv[1] = "-c";
            argv[2] = thiscommand;
            argv[3] = 0;
            execve("/bin/sh", argv, environ);
            FREE(thiscommand);
            exit(0);
        }
        exit(0);
    }
    return 0;
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
    char *ret = string ? strdup(string) : NULL;
    if (string && (ret == NULL)) {
        perror("strdup() ");
        exit(-1);
    } else
        return ret;
}


static void put_alrm(Alarm **list, const char *entry,
        char *time, char *date, char *ison, char *mesg, Bool cal) {
    Alarm *lst = *list;
    Bool   ok  = True;

    if (! lst) {
        lst   = xmalloc(sizeof(Alarm));
        *list = lst;
    } else {
        if (strcmp(entry, lst->entry) == 0) ok = False;
        while ( (lst->next) && ok) {
            lst = lst->next;
            if (strcmp(entry, lst->entry) == 0) ok = False;
        }
        if (! ok) return;
        lst->next = xmalloc(sizeof(Alarm));
        lst = lst->next;
    }
    lst->entry      = xstrdup(entry);
    lst->alarm_time = time ? xstrdup(time) : NULL;
    lst->alarm_date = date ? xstrdup(date) : NULL;
    lst->on         = ison ? getbool(ison) : True;
    lst->message    = mesg ? xstrdup(mesg) : NULL;
    lst->cal        = cal;
    lst->next       = NULL;
}


static void alrm_add(Alarm **list, const char *value) {
    char  *time = NULL, *date = NULL, *ison = NULL, *mesg = NULL, *at;
    char  *tokstr = xstrdup(value);
    char  *toksav = tokstr;

    if (! value) return;
    at = strchr(value, '@');
    if (at) ison = strtok(tokstr, "@");
    time = strtok(at ? NULL : tokstr, "-.");
    if (strchr(value, '-')) date = strtok(NULL, ".");
    mesg = strtok(NULL, "\n\0");
    
    put_alrm(list, value, time, date, ison, mesg, False);

    FREE(toksav);
}


static void free_alrm(Alarm **list) {
    Alarm *lst = *list, *next;
    while (lst) {
        next = lst->next;
        FREE(lst->entry);
        FREE(lst->alarm_time);
        FREE(lst->alarm_date);
        FREE(lst->message);
        free(lst);
        lst = next;
    }
    *list = NULL;
}


static int nb_alrm(Alarm *list) {
    Alarm *alrm = list;
    int      n = 0;
    while (alrm) {
        n++;
        alrm = alrm->next;
    }
    return n;
}


static Bool alarms_on(Alarm *list) {
    Alarm *alrm = list;

    while (alrm) {
        if (alrm->on) return True;
        alrm = alrm->next;
    }
    return False;
}


static void switch_alarms(Alarm *list) {
    Alarm *alrm   = list;
    Bool   set_to = True;

    if (alarms_on(list)) set_to = False;
    while (alrm) {
        char *tokstr = xstrdup(alrm->entry);
        char *toksav = tokstr;
        Bool is_on   = alrm->cal || getbool(strtok(tokstr, "@"));

        if ((is_on && set_to) || (!set_to)) {
            alrm->on = set_to;
        }
        alrm = alrm->next;
        FREE(toksav);
    }
}


static Bool getbool(char *value) {
    int i;
    for (i = 0 ; value[i] ; i++) value[i] = tolower(value[i]);
    if (strcmp(value, "0") == 0) return False;
    if (strcmp(value, "1") == 0) return True;
    if (strcmp(value, "true") == 0) return True;
    if (strcmp(value, "false") == 0) return False;
    if (strcmp(value, "yes") == 0) return True;
    if (strcmp(value, "no") == 0) return False;
    if (strcmp(value, "on") == 0) return True;
    if (strcmp(value, "off") == 0) return False;
    printf("Error in converting \"%s\" to boolean value.\n", value);
    return False;
}


static Bool load_cfgfile() {
    FILE *file;
    int  i = 0, ok = True;
    char line[MAXSTRLEN + 1];
    char *value;

    if ((file = fopen(config_file, "r")) == NULL) {
        if (strstr(config_file, "/"DEFAULT_CFGFILE) == NULL)
            printf("Unable to open configuration file \"%s\".\n", config_file);
        ok = False;
    }
    if (ok && (! filestat(config_file, &config_mtime, MTIME))) {
        fclose(file);
        ok = False;
    }
    if (ok) {
        if (alarms) free_alrm(&alarms);
        while (! feof(file)) {
            bzero(line, MAXSTRLEN + 1);
            fgets(line, MAXSTRLEN, file);
            i++;
            if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
            if ((line[0] == '#') || (line[0] == 0)) continue;
            value = strchr (line, '=') + 1;
            while ((value[0] == ' ') && (value[0] != 0)) value++;
            if (value[0] == 0) continue;

            if (strncmp(line, "Backlight", 9) == 0) {
                backlight = getbool(value) ? LIGHTON : LIGHTOFF;
            } else if (strncmp(line, "Color", 5) == 0) {
                SET_STRING(light_color, value);
            } else if (strncmp(line, "Alarm", 5) == 0) {
                alrm_add(&alarms, value);
            } else if (strncmp(line, "Command", 7) == 0) {
                SET_STRING(command, value);
            } else if (strncmp(line, "MessageCmd", 10) == 0) {
                SET_STRING(msgcmd, value);
            } else if (strncmp(line, "Blink", 5) == 0) {
                switch_authorized = getbool(value);
            } else if (strncmp(line, "H12", 3) == 0) {
                h12 = getbool(value);
            } else if (strncmp(line, "Locale", 6) == 0) {
                use_locale = getbool(value);
            } else if (strncmp(line, "StyleDir", 8) == 0) {
                SET_STRING(style_dir, value);
            } else if (strncmp(line, "Style", 5) == 0) {
                SET_STRING(style_name, value);
            } else if (strncmp(line, "TimeMode", 5) == 0) {
                time_mode = atoi(value);
            } else if (strncmp(line, "ShowCal", 7) == 0) {
                showcal = getbool(value);
            } else if (strncmp(line, "CalAlrms", 8) == 0) {
                calalrms = getbool(value);
            } else {
                printf("Error in %s at line %d :\n[%s].\n", config_file, i, line);
            }
        }
        if (calalrms) load_calalrms();
        fclose(file);
    } else if (calalrms && cal_alrms_chg()) {
        reload_alarms();
    }
    return ok;
}


static char *get_calend_file(int type) {
    char *Home     = robust_home();
    char *filename = xmalloc(
            strlen(Home) +
            strlen(DEFAULT_CONFIGDIR) +
            18);

    switch (type) {
        case 1:
            sprintf(filename, "%s/%s/%04d-%02d-%02d",
                    Home,
                    DEFAULT_CONFIGDIR,
                    timeinfos->tm_year + 1900,
                    timeinfos->tm_mon + 1,
                    timeinfos->tm_mday);
            break;
        case 2:
            sprintf(filename, "%s/%s/XXXX-%02d-%02d",
                    Home,
                    DEFAULT_CONFIGDIR,
                    timeinfos->tm_mon + 1,
                    timeinfos->tm_mday);
            break;
        case 3:
            sprintf(filename, "%s/%s/XXXX-XX-%02d",
                    Home,
                    DEFAULT_CONFIGDIR,
                    timeinfos->tm_mday);
            break;
        default: exit(1);
    }
    return filename;
}


static int cal_alrms_chg() {
    static time_t  cal_u_mtime = 0;
    static time_t  cal_y_mtime = 0;
    static time_t  cal_m_mtime = 0;
    char          *cal_u_fname = NULL;
    char          *cal_y_fname = NULL;
    char          *cal_m_fname = NULL;
    int            chg_u, chg_y, chg_m;

    if (! calalrms) return False;
    cal_u_fname = get_calend_file(1);
    cal_y_fname = get_calend_file(2);
    cal_m_fname = get_calend_file(3);
    chg_u = filestat(cal_u_fname, &cal_u_mtime, MTIME);
    chg_y = filestat(cal_y_fname, &cal_y_mtime, MTIME);
    chg_m = filestat(cal_m_fname, &cal_m_mtime, MTIME);
    free(cal_u_fname);
    free(cal_y_fname);
    free(cal_m_fname);
    if (chg_u || chg_y || chg_m) return True;
    return False;
}


static void load_cal_file(int type) {
    FILE *file;
    char *calend_file = get_calend_file(type);


    if ((file = fopen(calend_file, "r")) != NULL) {
        while (! feof(file)) {
            char line[MAXSTRLEN + 1];
            bzero(line, MAXSTRLEN + 1);
            fgets(line, MAXSTRLEN, file);
            if ( (line[0] != 0) && (strncmp(line, "@ ", 2) == 0) ) {
                char time[MAXSTRLEN + 1];
                sscanf(line, "@ %s ", time);
                put_alrm(&alarms, line, time, NULL, NULL, line + 8, True);
            }
        }
        fclose(file);
    }
    free(calend_file);
}


static void load_calalrms() {
    int i;
    for (i = 1 ; i < 4 ; i++) load_cal_file(i);
}


static void reload_alarms() {
    FILE *file;
    int  i = 0;
    char line[MAXSTRLEN + 1];
    char *value;

    if (alarms) free_alrm(&alarms);
    if ((file = fopen(config_file, "r")) == NULL) {
        if (strstr(config_file, "/"DEFAULT_CFGFILE) == NULL)
            printf("Unable to open configuration file \"%s\".\n", config_file);
    } else {
        while (! feof(file)) {
            bzero(line, MAXSTRLEN + 1);
            fgets(line, MAXSTRLEN, file);
            i++;
            if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
            if ((line[0] == '#') || (line[0] == 0)) continue;
            value = strchr (line, '=') + 1;
            while ((value[0] == ' ') && (value[0] != 0)) value++;
            if (value[0] == 0) continue;

            if (strncmp(line, "Alarm", 5) == 0) alrm_add(&alarms, value);
        }
    }
    if (calalrms) load_calalrms();
}


static void show_cal_file(int type) {
    FILE *file;
    char *data = NULL;
    char *tmp  = NULL;
    char *calend_file = get_calend_file(type);

    if ((file = fopen(calend_file, "r")) != NULL) {
        while (! feof(file)) {
            char line[MAXSTRLEN + 1];
            bzero(line, MAXSTRLEN + 1);
            fgets(line, MAXSTRLEN, file);
            if (line[0] != 0) {
                int len = data ? strlen(data) : 0;
                tmp = xmalloc(len + strlen(line) + 1);
                sprintf(tmp, "%s%s", data ? data : "", line);
                FREE(data);
                data = tmp;
                tmp = NULL;
            }
        }
        fclose(file);
        my_system(msgcmd, data);
        FREE(data);
    }
    free(calend_file);
}


static void show_cal() {
    int i;
    for (i = 1 ; i < 4 ; i++) show_cal_file(i);
}


static char *robust_home() {
    if (getenv("HOME"))
        return getenv("HOME");
    else if (getenv("USER") && getpwnam(getenv("USER")))
        return getpwnam(getenv("USER"))->pw_dir;
    else if (getenv("LOGNAME") && getpwnam(getenv("LOGNAME")))
        return getpwnam (getenv ("LOGNAME") )->pw_dir;
    else if ((getuid() != -1) && (getpwuid(getuid())))
        return getpwuid(getuid())->pw_dir;
    else
        return "/";
}

