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

#ifndef TEXT_CONVERSION_H
#define TEXT_CONVERSION_H

#include <String.h>
#include <be/support/UTF8.h>
#include <GString.h>

#include "CharTypes.h" // from xpdf

BString *ToUtf8(uint32 encoding, const char *string, int32 len);

// PDF Text to Utf8-String
BString *TextToUtf8(const char *string, int32 len);
bool TextToUtf8(GString* string, BString* result);
// UTF -> UCS2
GString *Utf8ToUcs2(const char *string, bool addOrderMarker = true);
// UTF -> Unicode (length returned in len)
Unicode *Utf8ToUnicode(const char *string, int32 *len);
// Utf8 To Ascii
BString *ToAscii(const char *string);

#endif