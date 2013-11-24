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

#ifndef FILE_SPECIFICATION_H
#define FILE_SPECIFICATION_H

// xpdf
#include <GString.h>
#include <Object.h>
#include <XRef.h>

#include <stdio.h> // for FILE

// PDF Filespec
class FileSpec {

public:
	enum SaveReturnCode {
		kOk,
		kMissingEmbeddedStreamError,
		kFileOpenError,
		kFileWriteError,
	};

private:	
	GString mDescription;
	GString mFileName;
	Ref     mRef;

	bool ReadFileName(Dict* fileSpec);
	bool ReadEmbeddedFileRef(Dict* fileSpec);
	bool Save(Dict* fileSpec, const char* file);
	SaveReturnCode SaveStream(Stream* stream, FILE* file);

public:
	FileSpec();
	FileSpec(FileSpec* copy);
	FileSpec(Dict* fileSpec);
	~FileSpec();
	
	// Returns true if the FileSpec is valid
	bool IsValid();
	
	bool SetTo(Dict* fileSpec);
	// Returns description. Can be empty.
	GString* GetDescription();
	// Returns file name. 
	GString* GetFileName();
	// Save embedded file to file
	SaveReturnCode Save(XRef* xref, const char* file);
};

#endif

