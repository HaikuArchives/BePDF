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
#include "StringLocalization.h"
#include "LocaleCatalog.h"
#include <String.h>
#include <Directory.h>
#include <Entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

StringLocalization *StringLocalization::mSingleton = NULL;
StringLocalization *my_locale = NULL;

StringLocalization::StringLocalization(BTextFileCatalog *locale) {
	mLocale = locale;
	my_locale = this;
};

StringLocalization::~StringLocalization() {
	delete mLocale; mLocale = NULL;
};

StringLocalization *StringLocalization::GetInstance(const char *filename) {
	if (mSingleton == NULL && filename != NULL) {
		BString file(filename);
		BTextFileCatalog *locale = new BTextFileCatalog(&file, 0);
		if (locale->InitCheck() == B_OK) {
			mSingleton = new StringLocalization(locale);
		} else {
			delete locale;
		}	
	}
	return mSingleton;
}

char *StringLocalization::GetString(const char *text) {
	const BString *translation;
	BString id(text);
	if (mLocale->GetString(&id, &translation) == B_OK) {
		return (char*)translation->String();
	} else {
		return (char*)text;
	}
}

bool StringLocalization::GetLocaleCode(BString& language, BString& country) {
	// retrieve locale code from environment variable(s)
	char* env = getenv("LC_ALL");
	if (env == NULL || strlen(env) == 0) {
		env = getenv("LC_MESSAGES");
		if (env == NULL || strlen(env) == 0) {
			return false;
		}
	}
	
	// remove optional character encoding specifier
	BString locale(env);
	int encodingIndex = locale.FindFirst('.');
	if (encodingIndex >= 0) {
		locale.Truncate(encodingIndex);
	}
	
	// separate locale code from country code
	int countryIndex = locale.FindFirst("_");
	if (countryIndex >= 0) {
		int n = locale.Length() - countryIndex - 1;
		locale.CopyInto(country, countryIndex+1, n);
		locale.Truncate(countryIndex);
	}
	
	language = locale;
	return true;
}

bool StringLocalization::FromEnvironmentVariables() {
	BString language;
	BString country;
	return GetLocaleCode(language, country);
}

// XXX If this is too slow. Create symbolic links to .catalog files, where the
// symoblic link name is the language code.
bool StringLocalization::FindLanguage(BPath localeFolder, const BString& language, const BString& country, BString& localeName) {
	BString fullLC(language);
	fullLC << "_" << country;
	const BString id("LanguageCode");
	
	BDirectory dir(localeFolder.Path());
	BEntry entry;
	BString foundSpecific;
	BString foundGeneral;
	
	while (dir.GetNextEntry(&entry) == B_OK) {
		char name[B_FILE_NAME_LENGTH];


		// skip files without .catalog extension
		if (entry.GetName(name) != B_OK ||
			strstr(name, ".catalog") == NULL) {
			continue;
		}
		
		// read catalog file
		BPath path(localeFolder);
		path.Append(name);
		BString pathToFile(path.Path());
		BTextFileCatalog* catalog = new BTextFileCatalog(&pathToFile, 0);
		if (catalog != NULL && catalog->InitCheck() == B_OK) {
			const BString* lc;
			if (catalog->GetString(&id, &lc) == B_OK) {
				if (lc->FindFirst(fullLC) == 0) {
					localeName = name;
					break;
				} else if (lc->FindFirst(language) == 0) {
					if (lc->FindFirst('_') >= 0) {
						foundSpecific = name;
					} else {
						foundGeneral = name;
					}
				}
			}
		}
		delete catalog;
	}
	
	if (localeName.Length() == 0) {
		localeName = foundGeneral;
		if (localeName.Length() == 0) {
			localeName = foundSpecific;
		}
	}
	
	return localeName.Length() > 0;
}

bool StringLocalization::Initialize(BPath localeFolder, const char* filename) {
	BString language;
	BString country;
	BString localeName;
	// try to retrieve file name from environment variables
	if (!GetLocaleCode(language, country) ||
		!FindLanguage(localeFolder, language, country, localeName)) {
		localeName = filename;
	}

	BPath locale(localeFolder); 
	locale.Append(localeName.String());
	// initialize global localization object (my_locale)
	if (!StringLocalization::GetInstance(locale.Path())) {
		// language not found, try same language with upper case letter
		BString s(localeName);
		char c = toupper(s[0]);
		char *b = s.LockBuffer(s.Length());
		b[0] = c;
		s.UnlockBuffer();
		locale = localeFolder; locale.Append(s.String());
		if (!StringLocalization::GetInstance(locale.Path())) {
			// try "default.catalog", "Default.catalog" and "English.catalog"
			locale = localeFolder; locale.Append("default.catalog");
			BPath locale2(localeFolder); locale2.Append("Default.catalog");
			BPath locale3(localeFolder); locale3.Append("English.catalog");
			return StringLocalization::GetInstance(locale.Path()) ||
				StringLocalization::GetInstance(locale2.Path()) ||
				StringLocalization::GetInstance(locale3.Path());
		}
	}
	return true;
}

