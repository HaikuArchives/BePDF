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

#include <be/support/UTF8.h>

#include "TextConversion.h"

#include <stdio.h>
#include <ctype.h>

#include "BeFontEncoding.h"

BString *ToUtf8(uint32 encoding, const char *string, int32 len) {
	int32 srcLen = len, destLen = 255;
	int32 state = 0;
	char buffer[256];
	int32 srcStart = 0;
	BString *str = new BString(); 
	int i = 0;
	if (len == 0) return str;

	do {
		convert_to_utf8(encoding, &string[srcStart], &srcLen, buffer, &destLen, &state); 
		srcStart += srcLen;
		len -= srcLen;
		srcLen = len;
		
		// Note: don't replace code below with str->Append(buffer, destLen);
		// as this does not work if buffer is Unicode (UCS2) encoded
		char *b = str->LockBuffer(i + destLen);
		memcpy(&b[i], buffer, destLen);
		str->UnlockBuffer(i + destLen);		
		i += destLen;
		destLen = 255;
	} while (len > 0);

	return str;
};

// PDF Text to Utf8-String
BString *TextToUtf8(const char *string, int32 len) {
	unsigned char *s = (unsigned char*)string;
	if ((s[0] == 0xfe) && (s[1] == 0xff)) {
		// string in UTF-16 (skip unicode order marker = 2 bytes)
		return ToUtf8(B_UNICODE_CONVERSION, (char*)&s[2], len-2); 
	} else {
		// string in PDFDocEncoding
		BString *str = new BString();
		while (*s) {
			str->Append(gStandardEncoding.getUtf8(*s));
			s++;
		}
		return str;
	}
}

bool TextToUtf8(GString* string, BString* result) {
	if (string->getCString() == NULL) {
		return false;
	}
	
	BString* utf8 = TextToUtf8(string->getCString(), string->getLength());
	if (utf8 == NULL) {
		return false;
	}
	
	result->SetTo(*utf8);
	delete utf8;
	return true;
}

GString *Utf8ToUcs2(const char *string, bool addOrderMarker) {
	int32 len = strlen(string);
	int32 srcLen = len, destLen = 255;
	int32 state = 0;
	char buffer[256];
	int32 srcStart = 0;
	GString *str = new GString(); 
	if (len == 0) return str;
	if (addOrderMarker) {
		str->append(0xfe); str->append(0xff);
	}
	do {
		convert_from_utf8(B_UNICODE_CONVERSION, &string[srcStart], &srcLen, buffer, &destLen, &state); 
		srcStart += srcLen;
		len -= srcLen;
		srcLen = len;
		
		str->append(buffer, destLen);
		destLen = 255;
	} while (len > 0);

	return str;
};

Unicode *Utf8ToUnicode(const char *string, int32 *length) {
	GString *ucs2 = Utf8ToUcs2(string, false);
	if (ucs2 == NULL) {
		return NULL;
	}
	
	*length = ucs2->getLength() / 2;
	Unicode *unicode = new Unicode[*length];
	if (unicode == NULL) {
		return NULL;
	}
	
	struct Char {
		uchar low;
		uchar high;
	};
	Char* ucs2Ptr = reinterpret_cast<Char*>(ucs2->getCString());
	for (int i = 0; i < *length; i ++) {
		unicode[i] = 256 * ucs2Ptr[i].low + ucs2Ptr[i].high;
	}

	delete ucs2;
	return unicode;
}

BString *ToAscii(const char *string) {
	int32 len = strlen(string), srcLen = len, destLen = 255;
	int32 state = 0;
	char buffer[256];
	int32 srcStart = 0;
	BString *str = new BString(); 
	do {
		convert_from_utf8(B_ISO1_CONVERSION, &string[srcStart], &srcLen, buffer, &destLen, &state); 
		srcStart += srcLen;
		len -= srcLen;
		srcLen = len;
		
		str->Append(buffer, destLen);
		destLen = 255;
	} while (len > 0);
	return str;	
}
