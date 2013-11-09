/*  
	BeOS Front-end to PDF file reader xpdf.
 	Copyright (C) 2003 Michael Pfeiffer

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

#ifndef _EntryMenuItem_h
#define _EntryMenuItem_h

#include <Bitmap.h>
#include <Entry.h>
#include <MenuItem.h>
#include <Mime.h>

class EntryMenuItem : public BMenuItem
{
public:
	EntryMenuItem(entry_ref* entry, const char* label, BMessage* message, char shortcut = 0, uint32 modifiers = 0);
	~EntryMenuItem();
	
	void GetContentSize(float* width, float* height);
	void DrawContent();
	
private:
	enum {
		kIconSize = 16,
		kTextIndent = kIconSize + 4,
	};
	
	BBitmap* GetIcon(BMimeType* mimeType);
	BBitmap* LoadIcon();

	entry_ref fEntry;
	BBitmap* fSmallIcon;
};

#endif
