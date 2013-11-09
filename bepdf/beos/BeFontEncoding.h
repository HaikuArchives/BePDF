/*  
	Copyright (C) 2000-2002 Michael Pfeiffer
	
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
#ifndef BE_FONT_ENCODING
#define BE_FONT_ENCODING

typedef char utf8char[5];

typedef struct {
	char *name; 
	int16 code; // PDF char code (-1 = not used)
	int32 unicode; // BeOS char code
	utf8char utf8;
} BeCharacterEncoding;

class BeFontEncoding {
public:
	BeFontEncoding(BeCharacterEncoding *encoding);
	int32 getIndex(const char *name);
	const char *getUtf8(const char *name);
	const char *getUtf8At(int32 index);

	static uint16 toUnicode(const char *string);
	static int32 toUtf8(uint16 unicode, char *string);

	inline const char *getUtf8(unsigned char code) { return map[code]; }
private:
	BeCharacterEncoding *encoding;
	int32 size;
	utf8char map[256];
};

extern BeFontEncoding gSymbolEncoding;
extern BeFontEncoding gStandardEncoding;
extern BeFontEncoding gZapfdingbatsEncoding;


#endif
