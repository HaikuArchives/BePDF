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

#include "Settings.h"
#include <Message.h>
#include <FindDirectory.h>
#include <Path.h>
#include <File.h>
#include <malloc.h>


GlobalSettings::GlobalSettings() : 
	mChanged(false)
{
	SETTINGS(DEFINE_VARIABLE)
	STRING_SETTINGS(DEFINE_STRING_VARIABLE)

	BPath path;
	if (B_OK == find_directory(B_DESKTOP_DIRECTORY, &path)) {
		mDefaultPanelDirectory = path.Path();
	} else {
		mDefaultPanelDirectory = "/boot/home/Desktop";
	}
	mPanelDirectory = mDefaultPanelDirectory;

}

bool GlobalSettings::HasChanged() const {
	return mChanged;
}

SETTINGS(DEFINE_SETTER)
SETTINGS(DEFINE_GETTER)
STRING_SETTINGS(DEFINE_STRING_SETTER)
STRING_SETTINGS(DEFINE_STRING_GETTER)

void GlobalSettings::SetPanelDirectory(const char *dir) {
	mChanged = mChanged || (mPanelDirectory != dir); mPanelDirectory = dir;
}


const char * GlobalSettings::GetPanelDirectory() const {
	return mPanelDirectory.String();
}

void GlobalSettings::SetDisplayCIDFonts(const BMessage& fonts) {
	mChanged = true;
	mDisplayCIDFonts = fonts;
}

void GlobalSettings::GetDisplayCIDFonts(BMessage& fonts) const {
	fonts = mDisplayCIDFonts;
}

BRect GlobalSettings::GetWindowRect() const {
	BRect rect;
	float w, h;
	rect.SetLeftTop(GetWindowPosition());
	GetWindowSize(w, h);
	rect.right = rect.left + w - 1;
	rect.bottom = rect.top + h - 1;
	return rect;
}

// BArchivable: 
GlobalSettings::GlobalSettings(BMessage *archive) {
	mChanged = false;

    SETTINGS(LOAD_SETTINGS)
    STRING_SETTINGS(LOAD_STRING_SETTINGS)

	if (B_OK != archive->FindString("panelDirectory", &mPanelDirectory))
		mPanelDirectory = mDefaultPanelDirectory;
	
	archive->FindMessage("displayCIDFonts", &mDisplayCIDFonts);
}

status_t GlobalSettings::Archive(BMessage *archive, bool deep) const {
	archive->AddString("class", "GlobalSettings");

	SETTINGS(STORE_SETTINGS)
	STRING_SETTINGS(STORE_STRING_SETTINGS)

	archive->AddString("panelDirectory", mPanelDirectory.String());
	archive->AddMessage("displayCIDFonts", &mDisplayCIDFonts);
	return B_OK;
}

BArchivable *GlobalSettings::Instantiate(BMessage *archive) {
	if (validate_instantiation(archive, "GlobalSettings")) {
		return new GlobalSettings(archive);
	} else
		return NULL;
}

void GlobalSettings::Load(const char* filename) {
	BPath path(filename);
	if (path.InitCheck() == B_OK) {
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() == B_OK) {
			BMessage archive;
			archive.Unflatten(&file);
			GlobalSettings *s = (GlobalSettings*)GlobalSettings::Instantiate(&archive);

			if (s != NULL) {
				*this = *s; delete s;
			}
		}
	}
}

void GlobalSettings::Save(const char* filename, bool force) {
BPath path(filename);
	if (HasChanged() || force) {
		BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		if (file.InitCheck() == B_OK) {
			BMessage archive;
			Archive(&archive);
			archive.Flatten(&file);
		}
	}
}


// FileAttributes
#pragma mark ------- FileAttributes --------
void FileAttributes::SetPage(int32 page) {
	this->page = page;
}

int32 FileAttributes::GetPage() const {
	return page;
}

void FileAttributes::SetLeftTop(float left, float top) {
	this->left = left;
	this->top = top;
}

void FileAttributes::GetLeftTop(float &left, float &top) {
	left = this->left; top = this->top;	
}

bool FileAttributes::Read(entry_ref *ref, GlobalSettings *s) {
	BNode node(ref);
	if (node.InitCheck() == B_OK) {
		int16 zoom;
		if (sizeof(zoom) != node.ReadAttr("bepdf:zoom", B_INT16_TYPE, 0, &zoom, sizeof(zoom))) {
			zoom = s->GetZoom();
		}
		int32 rotation, pos_x, pos_y, width, height;
		if (sizeof(rotation) != node.ReadAttr("bepdf:rotation", B_INT32_TYPE, 0, &rotation, sizeof(rotation))) {
			rotation = (int32)s->GetRotation();
		}
		if ((sizeof(pos_x) != node.ReadAttr("bepdf:pos_x", B_INT32_TYPE, 0, &pos_x, sizeof(pos_x))) ||
		   (sizeof(pos_y) != node.ReadAttr("bepdf:pos_y", B_INT32_TYPE, 0, &pos_y, sizeof(pos_y)))) {
			BPoint pos = s->GetWindowPosition();
			pos_x = (int32)pos.x; 
			pos_y = (int32)pos.y;
		}
		if ((sizeof(width) != node.ReadAttr("bepdf:width", B_INT32_TYPE, 0, &width, sizeof(width))) ||
			(sizeof(height) != node.ReadAttr("bepdf:height", B_INT32_TYPE, 0, &height, sizeof(height)))) {
			float w, h;
			s->GetWindowSize(w, h);
			width = (int32)w; height = (int32)h;
		}
		if (sizeof(page) != node.ReadAttr("bepdf:page", B_INT32_TYPE, 0, &page, sizeof(page))) {
			page = 1;
		}
		if (sizeof(left) != node.ReadAttr("bepdf:left", B_FLOAT_TYPE, 0, &left, sizeof(left))) {
			left = 0;
		}
		if (sizeof(top) != node.ReadAttr("bepdf:top", B_FLOAT_TYPE, 0, &top, sizeof(top))) {
			top = 0;
		}
		
		if (s->GetRestoreWindowFrame()) {
			s->SetWindowPosition(BPoint(pos_x, pos_y));
			s->SetWindowSize(width, height);
		}
		
		if (s->GetRestorePageNumber()) {
			s->SetZoom(zoom);
			s->SetRotation(rotation);
		}
		
		// read bookmarks
		ssize_t buf_size = 65536;
		ssize_t attr_size = 0;
		char *buffer = (char*)malloc(buf_size);
		while (buffer) {
			attr_size = node.ReadAttr("bepdf:bookmarks", B_MESSAGE_TYPE, 0, buffer, buf_size);
			if (attr_size == buf_size) {
				// resize buffer
				buf_size += 65536;
				buffer = (char*)realloc(buffer, buf_size);
			} else {
				// entire attribute read
				break;
			}
		}
		if (attr_size <= 0) {
			bookmarks.MakeEmpty();
		} else {
			bookmarks.Unflatten(buffer);
		}
		if (buffer) free(buffer);
		return true;
	}
	return false;
}

bool FileAttributes::Write(entry_ref *ref, GlobalSettings *s) {
	BNode node(ref);
	if (node.InitCheck() == B_OK) {
		int32 i;
		BPoint pos = s->GetWindowPosition();
		i = (int32)pos.x;
		if (sizeof(int32) != node.WriteAttr("bepdf:pos_x", B_INT32_TYPE, 0, &i, sizeof(i))) return false; 
		i = (int32)pos.y;
		if (sizeof(int32) != node.WriteAttr("bepdf:pos_y", B_INT32_TYPE, 0, &i, sizeof(i))) return false; 
		float width, height;
		s->GetWindowSize(width, height);
		i = (int32)width;
		if (sizeof(int32) != node.WriteAttr("bepdf:width", B_INT32_TYPE, 0, &i, sizeof(i))) return false; 
		i = (int32)height;
		if (sizeof(int32) != node.WriteAttr("bepdf:height", B_INT32_TYPE, 0, &i, sizeof(i))) return false; 
		// current page
		int16 zoom = s->GetZoom();
		if (sizeof(zoom) != node.WriteAttr("bepdf:zoom", B_INT16_TYPE, 0, &zoom, sizeof(zoom))) return false;
		i = (int32)s->GetRotation();
		if (sizeof(int32) != node.WriteAttr("bepdf:rotation", B_INT32_TYPE, 0, &i, sizeof(i))) return false;
		if (sizeof(page) != node.WriteAttr("bepdf:page", B_INT32_TYPE, 0, &page, sizeof(page))) return false;
		if (sizeof(left) != node.WriteAttr("bepdf:left", B_FLOAT_TYPE, 0, &left, sizeof(left))) return false;
		if (sizeof(top) != node.WriteAttr("bepdf:top", B_FLOAT_TYPE, 0, &top, sizeof(top))) return false;
		if (bookmarks.IsEmpty()) {
			node.RemoveAttr("bepdf:bookmarks");
		} else {
			ssize_t size = bookmarks.FlattenedSize();
			char *buffer = new char[size];
			if (buffer && B_OK == bookmarks.Flatten(buffer, size)) {
				node.WriteAttr("bepdf:bookmarks", B_MESSAGE_TYPE, 0, buffer, size);
			}
			delete []buffer;
		}
		return true;
	}
	return false;
}
