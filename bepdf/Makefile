## BeOS Generic Makefile v2.0 ##

## Fill in this file to specify the project being created, and the referenced
## makefile-engine will do all of the hard work for you.  This handles both
## Intel and PowerPC builds of the BeOS.

## Application Specific Settings ---------------------------------------------

# specify the name of the binary
NAME= BePDF

# specify the type of binary
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel Driver
TYPE= APP

#	specify the source files to use
#	full paths or paths relative to the makefile can be included
# 	all files, regardless of directory, will have their object
#	files created in the common object directory.
#	Note that this means this makefile will not work correctly
#	if two source files with the same name (source.c or source.cpp)
#	are included from different directories.  Also note that spaces
#	in folder names do not work well with this makefile.
SRCS= beos/AnnotAppearance.cpp \
beos/AnnotWriter.cpp \
beos/Annotation.cpp \
beos/AnnotationRenderer.cpp \
beos/AnnotationWindow.cpp \
beos/AttachmentView.cpp \
beos/Attachments.cpp \
beos/BeFontEncoding.cpp \
beos/BeLoadProgressMonitor.cpp \
beos/BePDF.cpp \
beos/BepdfApplication.cpp \
beos/BitmapPool.cpp \
beos/CachedPage.cpp \
beos/DisplayCIDFonts.cpp \
beos/EntryChangedMonitor.cpp \
beos/FileInfoWindow.cpp \
beos/FileSpec.cpp \
beos/FindTextWindow.cpp \
beos/History.cpp \
beos/InputEnabler.cpp \
beos/LayoutUtils.cpp \
beos/OutlinesWindow.cpp \
beos/PDFPrint.cpp \
beos/PDFSearch.cpp \
beos/PDFView.cpp \
beos/PDFWindow.cpp \
beos/PageCache.cpp \
beos/PageLabels.cpp \
beos/PageRenderer.cpp \
beos/PasswordWindow.cpp \
beos/PreferencesWindow.cpp \
beos/PrintSettingsWindow.cpp \
beos/PrintingProgressWindow.cpp \
beos/SaveThread.cpp \
beos/Settings.cpp \
beos/SplitView.cpp \
beos/StatusWindow.cpp \
beos/TextConversion.cpp \
beos/TraceWindow.cpp \
beos/TreeParser.cpp \
beos/htk/HWindow.cpp \
beos/locale/Catalog.cpp \
beos/utils/BitmapButton.cpp \
beos/utils/EntryMenuItem.cpp \
beos/utils/LayerView.cpp \
beos/utils/MultiButton.cpp \
beos/utils/ResourceLoader.cpp \
beos/utils/StatusBar.cpp \
beos/utils/StringLocalization.cpp \
beos/utils/Thread.cpp \
beos/utils/ToolBar.cpp \
beos/utils/ToolTip.cpp \
beos/utils/ToolTipItem.cpp
# beos/utils/Test.cpp

#	specify the resource files to use
#	full path or a relative path to the resource file can be used.
RDEFS= beos/BePDF.rdef

#	specify additional libraries to link against
#	there are two acceptable forms of library specifications
#	-	if your library follows the naming pattern of:
#		libXXX.so or libXXX.a you can simply specify XXX
#		library: libbe.so entry: be
#
#	- 	if your library does not follow the standard library
#		naming scheme you need to specify the path to the library
#		and it's name
#		library: my_lib.a entry: my_lib.a or path/my_lib.a
LIBS= be \
$(STDCPPLIBS) \
textencoding \
tracker \
translation \
layout \
santa \
../generated/libxpdf.a

#	specify additional paths to directories following the standard
#	libXXX.so or libXXX.a naming scheme.  You can specify full paths
#	or paths relative to the makefile.  The paths included may not
#	be recursive, so include all of the paths where libraries can
#	be found.  Directories where source files are found are
#	automatically included.
LIBPATHS=

#	additional paths to look for system headers
#	thes use the form: #include <header>
#	source file directories are NOT auto-included here
SYSTEM_INCLUDE_PATHS = /boot/system/develop/headers/liblayout \
/boot/system/develop/headers/santa \
../xpdf \
../xpdf/beos \
../xpdf/fofi \
../xpdf/goo \
../xpdf/splash \
../xpdf/xpdf

#	additional paths to look for local headers
#	thes use the form: #include "header"
#	source file directories are automatically included
LOCAL_INCLUDE_PATHS = beos/htk \
beos/locale \
beos/utils

#	specify the level of optimization that you desire
#	NONE, SOME, FULL
OPTIMIZE=FULL

#	specify any preprocessor symbols to be defined.  The symbols
#	will be set to a value of 1.  For example specify DEBUG if you want
#	DEBUG=1 to be set when compiling.
DEFINES=

#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS = ALL

#	specify whether image symbols will be created
#	so that stack crawls in the debugger are meaningful
#	if TRUE symbols will be created
SYMBOLS =

#	specify debug settings
#	if TRUE will allow application to be run from
#	a source-level debugger
DEBUGGER =

#	specify additional compiler flags for all files
COMPILER_FLAGS = -Wno-write-strings -fpermissive

#	specify additional linker flags
LINKER_FLAGS := $(shell freetype-config --libs)

## include the makefile-engine
include $(BUILDHOME)/etc/makefile-engine
