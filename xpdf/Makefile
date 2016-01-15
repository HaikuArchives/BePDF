## Haiku Generic Makefile v2.6 ##

## Fill in this file to specify the project being created, and the referenced
## Makefile-Engine will do all of the hard work for you. This handles any
## architecture of Haiku.

# The name of the binary.
NAME = libxpdf
TARGET_DIR = .

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = STATIC

# 	If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG =

#	The following lines tell Pe and Eddie where the SRCS, RDEFS, and RSRCS are
#	so that Pe and Eddie can fill them in for you.
#%{
# @src->@

#	Specify the source files to use. Full paths or paths relative to the
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
#	Also note that spaces in folder names do not work well with this Makefile.
SRCS = beos/BeSplashOutputDev.cc \
	beos/EncodingReader.cc \
	beos/Init.cc \
	fofi/FoFiBase.cc \
	fofi/FoFiEncodings.cc \
	fofi/FoFiIdentifier.cc \
	fofi/FoFiTrueType.cc \
	fofi/FoFiType1.cc \
	fofi/FoFiType1C.cc \
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
	splash/SplashXPath.cc \
	splash/SplashXPathScanner.cc \
	xpdf/AcroForm.cc \
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
	xpdf/Form.cc \
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
	xpdf/NameToCharCode.cc \
	xpdf/Object.cc \
	xpdf/OptionalContent.cc \
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
	xpdf/TextString.cc \
	xpdf/UnicodeMap.cc \
	xpdf/UnicodeTypeTable.cc \
	xpdf/XFAForm.cc \
	xpdf/XRef.cc \
	xpdf/Zoox.cc
	#beos/OptionalContent.cc
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

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS =

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS =

# End Pe/Eddie support.
# @<-src@
#%}

#	Specify libraries to link against.
#	There are two acceptable forms of library specifications:
#	-	if your library follows the naming pattern of libXXX.so or libXXX.a,
#		you can simply specify XXX for the library. (e.g. the entry for
#		"libtracker.so" would be "tracker")
#
#	-	for GCC-independent linking of standard C++ libraries, you can use
#		$(STDCPPLIBS) instead of the raw "stdc++[.r4] [supc++]" library names.
#
#	- 	if your library does not follow the standard library naming scheme,
#		you need to specify the path to the library and it's name.
#		(e.g. for mylib.a, specify "mylib.a" or "path/mylib.a")
LIBS = be

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS =

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.
SYSTEM_INCLUDE_PATHS = .

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS =

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O2), or leave blank (for the default optimization level).
OPTIMIZE := SOME

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES =

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES =

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS =

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS :=

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER :=

#	Specify any additional compiler flags to be used.
COMPILER_FLAGS := $(shell freetype-config --cflags)

#	Specify any additional linker flags to be used.
LINKER_FLAGS = $(shell freetype-config --libs)

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION :=

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH =

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
