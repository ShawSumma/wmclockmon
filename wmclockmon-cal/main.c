/* WMMaiLoad-Config
 * Configuration window for wmmaiload dockapp, Thomas Nemeth 2002.
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include "../config.h"
#include "defines.h"
#include "tools.h"
#include "mainwindow.h"
#include "main.h"


void Usage () {
    printf("Usage: "PACKAGE"-cal [-h] [-v]\n"
           " -h          : short usage help\n"
           " -v          : show version\n\n");
    exit(1);
}


void quit_app() {
    gtk_main_quit();
}


void app_init(int argc, char *argv[]) {
    int    ind = 1;
    time_t tnow;

    while (ind < argc) {
        if (argv[ind][0] == '-') {
            switch (argv[ind][1]) {
                case 'h' :
                    printf(PACKAGE"-cal by Thomas Nemeth - v "VERSION"\n");
                    Usage();
                    break;
                case 'v' :
                    printf(PACKAGE"-cal by Thomas Nemeth - v "VERSION"\n");
                    exit(1);
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
    time(&tnow);
    timeinfos = localtime(&tnow);
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    gtk_init(&argc, &argv);
    app_init(argc, argv);
    create_mainwindow();

    gtk_main();

    return 0;
}
