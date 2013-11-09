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

#include "BitmapPool.h"
#include <be/interface/Bitmap.h>

BitmapPool* BitmapPool::mInstance = NULL;

BitmapPool::BitmapPool() {
	mSize = 0;
	mFree = NULL;
}

int64 BitmapPool::size(int width, int height, color_space cs) {
	int64 bytes = 0;

	switch (cs) {
		case B_RGB32_BIG:
		case B_RGBA32_BIG:
		case B_RGB32_LITTLE:
		case B_RGBA32_LITTLE:
			bytes = 4; break;
		case B_RGB24_BIG:
		case B_RGB24_LITTLE:
			bytes = 3; break;
		case B_RGB16_BIG:
		case B_RGB16_LITTLE:
			bytes = 2; break;
		case B_RGB15_BIG:
		case B_RGBA15_BIG:
		case B_RGB15_LITTLE:
		case B_RGBA15_LITTLE:
			bytes = 2; break;
		case B_CMAP8:
			bytes = 1; break;
		default: 
			bytes = 4;
	}

	return bytes * width * height;
}

int64 BitmapPool::freeMemorySize() {
	system_info info;
	get_system_info(&info);
	return B_PAGE_SIZE * (info.max_pages - info.used_pages);
}

BitmapPool* BitmapPool::getInstance() {
	if (mInstance == NULL) {
		mInstance = new BitmapPool();
	}
	return mInstance;
}

int BitmapPool::newBitmap() {
	Item* item = mFree;
	if (item != NULL) {
		mFree = item->next;
		return item->index;
	}
	item = new Item;
	item->size = 0;
	item->width = item->height = 0;
	item->bitmap = NULL;
	mPool.AddItem(item);
	return mPool.CountItems()-1;
}

void BitmapPool::deleteBitmap(int id) {
	Item* item = itemAt(id);
	mSize += item->size;

	delete item->bitmap; item->bitmap = NULL;
	item->size = 0; item->width = item->height = 0;
	item->next = mFree; mFree = item;
}

BBitmap* BitmapPool::getBitmap(int id, int width, int height, color_space cs) {
	Item* item = itemAt(id);
	BBitmap* bitmap = item->bitmap;	
	
	if (!bitmap || cs != bitmap->ColorSpace() || width > item->width || height > item->height) {
		delete bitmap; 
		mSize += item->size;
		int64 s = size(width, height, cs);
		int64 required = s - mSize;
		if (required < 0) required = 0;
		// TODO: reserve some free space
		if (required > freeMemorySize()) {
			// not enough memory available 
			item->bitmap = NULL; item->width = item->height = 0; item->size = 0;
			return NULL;
		}
		
		mSize -= s; if (mSize < 0) mSize = 0;
		item->size = s;
		
		bitmap = new BBitmap(BRect (0, 0, width, height), cs);
		item->bitmap = bitmap; item->width = width; item->height = height;
	}
	return bitmap;
}
