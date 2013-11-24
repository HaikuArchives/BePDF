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

#include "FileSpec.h"

#include "Annotation.h"

static char *gFileAttachmentFileNameKeys[] = {
	"Unix",
	"F",
	"DOS",
	"Mac",
	NULL
};

FileSpec::FileSpec() 
	: mRef(empty_ref)
{
}

FileSpec::FileSpec(FileSpec* copy) 
	: mDescription(copy->mDescription)
	, mFileName(copy->mFileName)
	, mRef(copy->mRef)
{

}

FileSpec::FileSpec(Dict* fileSpec) {
	SetTo(fileSpec);
}

FileSpec::~FileSpec() {
}

bool FileSpec::SetTo(Dict* fileSpec) {
	mDescription.clear();
	mFileName.clear();
	mRef = empty_ref;

	if (!fileSpec->is("Filespec")) {
		return false;
	}
	
	// optional PDF 1.6 description for files in EmbeddedFiles name tree
	Object obj;
	if (fileSpec->lookup("Desc", &obj) != NULL && obj.isString()) {
		mDescription.append(obj.getString());
	}
	obj.free();
	
	// mandatory file name
	if (!ReadFileName(fileSpec)) {
		mDescription.clear();
		return false;
	}

	
	// mandatory ref to embedded file
	if (!ReadEmbeddedFileRef(fileSpec)) {
		mDescription.clear();
		mFileName.clear();
		return false;
	}
	
	return true;
}

bool FileSpec::ReadFileName(Dict *fileSpec) {
	for (int i = 0; gFileAttachmentFileNameKeys[i] != NULL; i++) {
		Object obj;
		char *key = gFileAttachmentFileNameKeys[i];
		if (fileSpec->lookup(key, &obj) != NULL && obj.isString()) {
			mFileName.append(obj.getString()->getCString());
			return true;
		}
		obj.free();
	}
	return false;
}

bool FileSpec::ReadEmbeddedFileRef(Dict* fileSpec) {
	bool found = false;
	Object obj;
	if (fileSpec->lookup("EF", &obj) != NULL && obj.isDict()) {
		for (int i = 0; (!found) && gFileAttachmentFileNameKeys[i] != NULL; i++) {
			Object stream;
			char *key = gFileAttachmentFileNameKeys[i];
			// Is there a reference to a stream?
			// We do not test here if the stream really exists!
			if (obj.dictLookupNF(key, &stream) != NULL && stream.isRef()) {
				mRef = stream.getRef();
				found = true; // leave for loop
			}
			stream.free();
		}
	}
	obj.free();
	return found;
}
	
bool FileSpec::IsValid() {
	return mFileName.getLength() > 0 && !is_empty_ref(mRef);
}

GString* FileSpec::GetDescription() {
	return &mDescription;
}

GString* FileSpec::GetFileName() {
	return &mFileName;
}

FileSpec::SaveReturnCode 
FileSpec::Save(XRef* xref, const char* file) {
	if (is_empty_ref(mRef)) {
		return kMissingEmbeddedStreamError;
	}

	Object ref;
	ref.initRef(mRef.num, mRef.gen);
	Object obj;
	if (ref.fetch(xref, &obj) != NULL && obj.isStream()) {
			Stream* stream = obj.getStream();
			
			FILE* f = fopen(file, "wb");
			if (f == NULL) {
				return kFileOpenError;
			}
			
			stream->reset();
			SaveReturnCode rc = SaveStream(stream, f);
			stream->close();
			
			fclose(f);
			obj.free();
			return rc;
	}
	obj.free();
	return kMissingEmbeddedStreamError;
}

FileSpec::SaveReturnCode
FileSpec::SaveStream(Stream* stream, FILE* file) {
	const int kBufferSize = 4096;
	unsigned char buffer[kBufferSize];
	while (true) {
		// fill buffer
		int length;
		int ch;
		for (length = 0; length < kBufferSize && ((ch = stream->getChar()) != EOF); length ++) {
			buffer[length] = (unsigned char)ch;
		}
		
		// end of stream reached
		if (length == 0) {
			return kOk;
		}
		
		// write buffer
		if (fwrite(buffer, length, 1, file) != 1) {
			return kFileWriteError;
		}
	}
}
