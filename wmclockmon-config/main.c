/* WMMaiLoad-Config
 * Configuration window for wmmaiload dockapp, Thomas Nemeth 2002.
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include "../config.h"
#include "defines.h"
#include "variables.h"
#include "tools.h"
#include "mainwindow.h"
#include "actions.h"


void Usage () {
    printf("Usage: "PACKAGE"-config [-h] [-v] [-f filename]\n"
           " -h          : short usage help\n"
           " -v          : show version\n"
           " -f filename : edit filename\n\n");
    exit(1);
}


void app_init(int argc, char *argv[]) {
    int   ind = 1;

    backlight         = 0;
    style_name        = NULL;
    style_dir         = NULL;
    switch_authorized = 1;
    h12               = 0;
    use_locale        = 1;
    config_file       = NULL;
    light_color       = NULL;
    command           = NULL;
    alarms            = NULL;
    showcal           = 0;
    calalrms          = 0;

    while (ind < argc) {
        if (argv[ind][0] == '-') {
            switch (argv[ind][1]) {
                case 'h' :
                    printf(PACKAGE"-config by Thomas Nemeth - v "VERSION"\n");
                    Usage();
                    break;
                case 'v' :
                    printf(PACKAGE"-config by Thomas Nemeth - v "VERSION"\n");
                    exit(1);
                    break;
                case 'f' :
                    ind++;
                    config_file = xstrdup(argv[ind]);
                    break;
                default:
                    printf("Unknown option: %s\n", argv[ind]);
                    Usage();
                    break;
            }
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[ind]);
            Usage();
        }
        ind++;
    }
    if (! config_file) {
        char *Home = robust_home();
        config_file = xmalloc(strlen(Home) + strlen(DEFAULT_CFGFILE) + 3);
        sprintf(config_file, "%s/%s", Home, DEFAULT_CFGFILE);
    }
}


int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);
    app_init(argc, argv);
    load_cfgfile();
    create_mainwindow();
    set_values();

    gtk_main();

    return 0;
}
