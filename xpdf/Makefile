## BeOS Generic Makefile v2.0 ##

## Fill in this file to specify the project being created, and the referenced
## makefile-engine will do all of the hard work for you.  This handles both
## Intel and PowerPC builds of the BeOS.

## Application Specific Settings ---------------------------------------------

# specify the name of the binary
NAME= libxpdf
TARGET_DIR=.

# specify the type of binary
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel Driver
TYPE= STATIC

#	specify the source files to use
#	full paths or paths relative to the makefile can be included
# 	all files, regardless of directory, will have their object
#	files created in the common object directory.
#	Note that this means this makefile will not work correctly
#	if two source files with the same name (source.c or source.cpp)
#	are included from different directories.  Also note that spaces
#	in folder names do not work well with this makefile.
SRCS= beos/BeSplashOutputDev.cc \
beos/EncodingReader.cc \
beos/Init.cc \
fofi/FoFiBase.cc \
fofi/FoFiEncodings.cc \
fofi/FoFiTrueType.cc \
fofi/FoFiType1.cc \
fofi/FoFiType1C.cc \
goo/gmem.c \
goo/parseargs.c  \
goo/FixedPoint.cc \
goo/GHash.cc \
goo/GList.cc \
goo/GString.cc \
goo/gfile.cc \
goo/gmem.cc \
goo/gmempp.cc \
splash/Splash.cc \
splash/SplashBitmap.cc \
splash/SplashClip.cc \
splash/SplashFTFont.cc \
splash/SplashFTFontEngine.cc \
splash/SplashFTFontFile.cc \
splash/SplashFont.cc \
splash/SplashFontEngine.cc \
splash/SplashFontFile.cc \
splash/SplashFontFileID.cc \
splash/SplashPath.cc \
splash/SplashPattern.cc \
splash/SplashScreen.cc \
splash/SplashState.cc \
splash/SplashT1Font.cc \
splash/SplashT1FontEngine.cc \
splash/SplashT1FontFile.cc \
splash/SplashXPath.cc \
splash/SplashXPathScanner.cc \
xpdf/Annot.cc \
xpdf/Array.cc \
xpdf/BuiltinFont.cc \
xpdf/BuiltinFontTables.cc \
xpdf/CMap.cc \
xpdf/Catalog.cc \
xpdf/CharCodeToUnicode.cc \
xpdf/CoreOutputDev.cc \
xpdf/Decrypt.cc \
xpdf/Dict.cc \
xpdf/Error.cc \
xpdf/FontEncodingTables.cc \
xpdf/Function.cc \
xpdf/Gfx.cc \
xpdf/GfxFont.cc \
xpdf/GfxState.cc \
xpdf/GlobalParams.cc \
xpdf/ImageOutputDev.cc \
xpdf/JArithmeticDecoder.cc \
xpdf/JBIG2Stream.cc \
xpdf/JPXStream.cc \
xpdf/Lexer.cc \
xpdf/Link.cc \
xpdf/LoadProgressMonitor.cc \
xpdf/NameToCharCode.cc \
xpdf/Object.cc \
xpdf/Outline.cc \
xpdf/OutputDev.cc \
xpdf/PDFCore.cc \
xpdf/PDFDoc.cc \
xpdf/PDFDocEncoding.cc \
xpdf/PSOutputDev.cc \
xpdf/PSTokenizer.cc \
xpdf/Page.cc \
xpdf/Parser.cc \
xpdf/PreScanOutputDev.cc \
xpdf/SecurityHandler.cc \
xpdf/SplashOutputDev.cc \
xpdf/Stream.cc \
xpdf/TextOutputDev.cc \
xpdf/UnicodeMap.cc \
xpdf/UnicodeTypeTable.cc \
xpdf/XRef.cc
# beos/OptionalContent.cc
# xpdf/XPDFApp.cc
# xpdf/XPDFCore.cc
# xpdf/XPDFTree.cc
# xpdf/XPDFViewer.cc
# xpdf/XpdfPluginAPI.cc
# xpdf/pdffonts.cc
# xpdf/pdfimages.cc
# xpdf/pdfinfo.cc
# xpdf/pdftoppm.cc
# xpdf/pdftops.cc
# xpdf/pdftotext.cc
# xpdf/xpdf.cc

#	specify the resource files to use
#	full path or a relative path to the resource file can be used.
RSRCS=

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
LIBS= be

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
SYSTEM_INCLUDE_PATHS = .

#	additional paths to look for local headers
#	thes use the form: #include "header"
#	source file directories are automatically included
LOCAL_INCLUDE_PATHS = beos \
fofi \
goo \
splash \
xpdf

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
COMPILER_FLAGS := -Wno-write-strings $(shell freetype-config --cflags)

#	specify additional linker flags
LINKER_FLAGS := $(shell freetype-config --libs)

## include the makefile-engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
