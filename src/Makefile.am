bin_PROGRAMS = wmclockmon

wmclockmon_SOURCES =   \
	main.c \
	dockapp.c \
	dockapp.h \
	backlight0_off.xpm \
	backlight0_on.xpm \
	backlightI_off.xpm \
	backlightI_on.xpm \
	backlightB_off.xpm \
	backlightB_on.xpm \
	parts.xpm \
	letters.xpm

CPPFLAGS = @CPPFLAGS@ @DFLAGS@

INCLUDES = @HEADER_SEARCH_PATH@

wmclockmon_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETGID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
