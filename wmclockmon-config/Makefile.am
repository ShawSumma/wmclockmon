bin_PROGRAMS = wmclockmon

wmclockmon_SOURCES =   \
	actions.c \
	actions.h \
	configfile.h \
	defines.h \
	dialogs.c \
	dialogs.h \
	edit.c \
	edit.h \
	main.c \
	mainwindow.c \
	mainwindow.h \
	tools.c \
	tools.h \
	variables.h

CPPFLAGS = @CPPFLAGS@ @DFLAGS@

INCLUDES = @HEADER_SEARCH_PATH@

wmclockmon_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETGID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
