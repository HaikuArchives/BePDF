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

#ifndef _DISPLAY_CID_FONTS_H
#define _DISPLAY_CID_FONTS_H

#include <List.h>
#include <Message.h>

class DisplayCIDFont;

class DisplayCIDFonts {
public:
	DisplayCIDFonts(const BMessage& fonts);
	virtual ~DisplayCIDFonts();
	
	status_t Archive(BMessage& archive);

	enum Type {
		kType1,
		kTrueType,
		kUnknownType
	};
	
	bool Contains(const char* name) const;
	int32 GetSize() const;
	void Get(int32 index, BString& name, BString& file, Type& type) const;
	void Get(const char* name, BString& file, Type& type) const;
	void Set(const char* name, const char* file = "", Type type = kUnknownType);
	
private:
	void Get(DisplayCIDFont* displayName, BString& name, BString& file, Type& type) const;
	DisplayCIDFont* Get(int32 index) const;
	DisplayCIDFont* Find(const char* name) const;

	BList mFonts;
};

#endif
