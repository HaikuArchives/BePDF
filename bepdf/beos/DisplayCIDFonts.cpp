/*
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2011 Michael Pfeiffer.
 * 	 Copyright (C) 2013 waddlesplash.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "DisplayCIDFonts.h"

#include <String.h>

#define FONT "font"

#define NAME "name"
#define FILE "file"
#define TYPE "type"

#define TRUE_TYPE "tt"
#define TYPE1     "t1"

class DisplayCIDFont {
public:
	DisplayCIDFont(const char* name, const char* file, DisplayCIDFonts::Type type);

	const char* GetName() const;
	const char* GetFile() const;
	DisplayCIDFonts::Type GetType() const;

	void Set(const char* name, const char* file, DisplayCIDFonts::Type type);

private:
	BString mName;
	BString mFile;
	DisplayCIDFonts::Type mType;
};

// implementation of DisplayCIDFont
DisplayCIDFont::DisplayCIDFont(const char* name, const char* file, DisplayCIDFonts::Type type)
	: mName(name)
	, mFile(file)
	, mType(type)
{
}

const char* DisplayCIDFont::GetName() const {
	return mName.String();
}

const char* DisplayCIDFont::GetFile() const {
	return mFile.String();
}

DisplayCIDFonts::Type DisplayCIDFont::GetType() const {
	return mType;
}

void DisplayCIDFont::Set(const char* name, const char* file, DisplayCIDFonts::Type type) {
	mName = name;
	mFile = file;
	mType = type;
}


// implementation of DisplayCIDFonts

DisplayCIDFonts::DisplayCIDFonts(const BMessage& DisplayCIDFonts) {
	BMessage msg;
	for (int32 index = 0; DisplayCIDFonts.FindMessage(FONT, index, &msg) == B_OK; index ++) {
		BString name;
		BString file;
		BString typeString;
		msg.FindString(NAME, &name);
		msg.FindString(FILE, &file);
		msg.FindString(TYPE, &typeString);
		Type type = kUnknownType;
		if (typeString == TYPE1) {
			type = kType1;
		} else if (typeString == TRUE_TYPE) {
			type = kTrueType;
		}
		mFonts.AddItem(new DisplayCIDFont(name.String(), file.String(), type));
	}


}

DisplayCIDFonts::~DisplayCIDFonts() {
	for (int32 index = 0; index < GetSize(); index ++) {
		delete Get(index);
	}
	mFonts.MakeEmpty();
}

bool DisplayCIDFonts::Contains(const char* name) const {
	return Find(name) != NULL;
}

int32 DisplayCIDFonts::GetSize() const {
	return mFonts.CountItems();
}

DisplayCIDFont* DisplayCIDFonts::Get(int32 index) const {
	return (DisplayCIDFont*)mFonts.ItemAt(index);
}

DisplayCIDFont* DisplayCIDFonts::Find(const char* name) const {
	for (int32 index = 0; index < GetSize(); index ++) {
		if (strcmp(name, Get(index)->GetName()) == 0) {
			return Get(index);
		}
	}

	return NULL;
}

status_t DisplayCIDFonts::Archive(BMessage& archive) {
	for (int32 index = 0; index < GetSize(); index ++) {
		DisplayCIDFont* name = Get(index);
		BMessage msg;
		msg.AddString(NAME, name->GetName());
		msg.AddString(FILE, name->GetFile());
		const char* typeString = "";
		if (name->GetType() == kType1) {
			typeString = TYPE1;
		} else if (name->GetType() == kTrueType) {
			typeString = TRUE_TYPE;
		}
		msg.AddString(TYPE, typeString);
		archive.AddMessage(FONT, &msg);
	}
	return B_OK;
}

void DisplayCIDFonts::Get(DisplayCIDFont* font, BString& name, BString& file, Type& type) const {
	if (font == NULL) {
		name = "";
		file = "";
		type = kUnknownType;
		return;
	}
	name = font->GetName();
	file = font->GetFile();
	type = font->GetType();
}

void DisplayCIDFonts::Get(int32 index, BString& name, BString& file, Type& type) const {
	Get(Get(index), name, file, type);
}

void DisplayCIDFonts::Get(const char* name, BString& file, Type& type) const {
	BString dummyName;
	Get(Find(name), dummyName, file, type);
}

void DisplayCIDFonts::Set(const char* name, const char* file, Type type) {
	DisplayCIDFont* font = Find(name);
	if (font == NULL) {
		font = new DisplayCIDFont(name, file, type);
		mFonts.AddItem(font);
	} else {
		font->Set(name, file, type);
	}
}
