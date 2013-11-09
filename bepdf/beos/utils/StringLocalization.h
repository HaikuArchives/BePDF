/*  
	Copyright (C) 2000 Michael Pfeiffer
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef STRING_LOCALIZATION_H
#define STRING_LOCALIZATION_H

#include <SupportDefs.h>
#include <Path.h>

class BTextFileCatalog;

class StringLocalization {
private:
	StringLocalization(BTextFileCatalog *local);
	~StringLocalization();
	BTextFileCatalog *mLocale;
	
	static StringLocalization *mSingleton;
	
	static bool GetLocaleCode(BString& language, BString& country);
	static bool FindLanguage(BPath localeFolder, const BString& language, const BString& country, BString& localeName);
public:
	static bool FromEnvironmentVariables();
	static bool Initialize(BPath localeFolder, const char* filename);
	// needs to be called with a file name first to return a valid
	// StringLocalization object
	static StringLocalization *GetInstance(const char *filename = NULL);
	// returned string must not be deleted
	char *GetString(const char *text);
};

class Localization {
public:
	
};

// For easy access: Only one object application wide!
extern StringLocalization *my_locale;

#define TRANSLATE(text) my_locale->GetString(text)

#endif
