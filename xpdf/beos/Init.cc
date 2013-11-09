//========================================================================
//
// Compatibility pthread functions.
//
// Copyright 2005 Michael W. Pfeiffer
//
//========================================================================
#include "aconf.h"
#include "gtypes.h"
#include "parseargs.h"
#include "GString.h"
#include "GlobalParams.h"
#include "Object.h"
#include "EncodingReader.h"

//------------------------------------------------------------------------
// command line options
//------------------------------------------------------------------------

static char enableT1libStr[16] = "";
static char enableFreeTypeStr[16] = "";
static char antialiasStr[16] = "";
static char hintingStr[16] = "";
static char paperSize[15] = "";
static int paperWidth = 0;
static int paperHeight = 0;
static GBool level1 = gFalse;
static char textEncName[128] = "";
static char textEOL[16] = "";
static GBool printCommands = gFalse;
static GBool quiet = gFalse;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
#if HAVE_T1LIB_H
  {"-t1lib",      argString,      enableT1libStr, sizeof(enableT1libStr),
   "enable t1lib font rasterizer: yes, no"},
#endif
#if HAVE_FREETYPE_FREETYPE_H | HAVE_FREETYPE_H
  {"-freetype",   argString,      enableFreeTypeStr, sizeof(enableFreeTypeStr),
   "enable FreeType font rasterizer: yes, no"},
#endif
  {"-aa",         argString,      antialiasStr,   sizeof(antialiasStr),
   "enable font anti-aliasing: yes, no"},
  {"-hinting",         argString,      hintingStr,   sizeof(hintingStr),
   "enable font FreeType hinting: yes, no"},
  {"-paper",      argString,      paperSize,      sizeof(paperSize),
   "paper size (letter, legal, A4, A3, match)"},
  {"-paperw",     argInt,         &paperWidth,    0,
   "paper width, in points"},
  {"-paperh",     argInt,         &paperHeight,   0,
   "paper height, in points"},
  {"-level1",     argFlag,        &level1,        0,
   "generate Level 1 PostScript"},
  {"-enc",    argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-eol",    argString,   textEOL,        sizeof(textEOL),
   "output end-of-line convention (unix, dos, or mac)"},
  {"-cmd",        argFlag,        &printCommands, 0,
   "print commands as they're executed"},
  {"-q",          argFlag,        &quiet,         0,
   "don't print any messages or errors"},
  {"-v",          argFlag,        &printVersion,  0,
   "print copyright and version info"},
  {"-h",          argFlag,        &printHelp,     0,
   "print usage information"},
  {"-help",       argFlag,        &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

void InitXpdf(const char *configFile, const char *fontDirectory, const char* encodingDirectory) {
	// read config file
	globalParams = new GlobalParams((char*)configFile);
	// set default values
	globalParams->setupBaseFonts((char*)fontDirectory);
	globalParams->setTextEncoding("UTF-8");
	globalParams->setTextEOL("unix");
	globalParams->setAntialias("yes");
	globalParams->setVectorAntialias("yes");
	
	// read encodings
	readEncodings(globalParams, encodingDirectory);
	
	// set parameters
	if (paperSize[0]) {
		if (!globalParams->setPSPaperSize(paperSize)) {
			fprintf(stderr, "Invalid paper size\n");
		}
	} else {
		if (paperWidth) {
			globalParams->setPSPaperWidth(paperWidth);
		}
		if (paperHeight) {
			globalParams->setPSPaperHeight(paperHeight);
		}
	}
	if (level1) {
		globalParams->setPSLevel(psLevel1);
	}
	if (textEncName[0]) {
		globalParams->setTextEncoding(textEncName);
	}
	if (textEOL[0]) {
		if (!globalParams->setTextEOL(textEOL)) {
			fprintf(stderr, "Bad '-eol' value on command line\n");
		}
	}
	if (enableT1libStr[0]) {
		if (!globalParams->setEnableT1lib(enableT1libStr)) {
			fprintf(stderr, "Bad '-t1lib' value on command line\n");
		}
	}
	if (enableFreeTypeStr[0]) {
		if (!globalParams->setEnableFreeType(enableFreeTypeStr)) {
			fprintf(stderr, "Bad '-freetype' value on command line\n");
		}
	}
	if (antialiasStr[0]) {
		if (!globalParams->setAntialias(antialiasStr)) {
			fprintf(stderr, "Bad '-aa' value on command line\n");
		}
	}
	// TBD should BePDF support it still?
#if 0
	if (hintingStr[0]) {
		if (!globalParams->setFreeTypeHinting(hintingStr)) {
			fprintf(stderr, "Bad '-hinting' value on command line\n");
		}
	}
#endif
	if (printCommands) {
		globalParams->setPrintCommands(printCommands);
	}
	if (quiet) {
		globalParams->setErrQuiet(quiet);
	}
}

ArgDesc *GetGlobalArgDesc() {
	return argDesc;
}

bool GetPrintHelp() {
	return printHelp;
}
