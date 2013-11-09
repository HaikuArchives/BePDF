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
#ifndef RESOURCE_LOADER_H
#define RESOURCE_LAODER_H

#include <Bitmap.h>
#include <Cursor.h>
#include <TranslatorFormats.h>

BBitmap *LoadLargeIcon(const char *name);
BBitmap *LoadLargeIcon(int32 id);

BBitmap *LoadMiniIcon(const char *name);
BBitmap *LoadMiniIcon(int32 id);

BBitmap *LoadBitmap(const char *name, uint32 type_code = B_TRANSLATOR_BITMAP);
BBitmap *LoadBitmap(int32 id, uint32 type_code = B_TRANSLATOR_BITMAP);

BCursor *LoadCursor(const char *name);
BCursor *LoadCursor(int32 id);

#endif
