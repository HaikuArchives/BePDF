//========================================================================
//
// Initialization of xpdf.
//
// Copyright 2005 Michael W. Pfeiffer
//
//========================================================================
#ifndef _INIT_H
#define _INIT_H

#include "parseargs.h"

void InitXpdf(const char *configFile, 
	const char *fontDirectory, 
	const char* encodingDirectory);

ArgDesc* GetGlobalArgDesc();

bool GetPrintHelp();

#endif

