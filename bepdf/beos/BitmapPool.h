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

#ifndef _BITMAP_POOL_H
#define _BITMAP_POOL_H

#include <be/support/List.h>
#include <be/interface/GraphicsDefs.h>
#include <be/interface/Bitmap.h>

/*
   This is a workaround for a BeOS bug, to avoid crashes that are cause 
   by allocating too much memory in a BBitmap constructor.
*/

class BitmapPool {
	typedef struct item {
		int      index;
		int64    size; // free
		int      width;
		int      height;
		BBitmap* bitmap;
		struct item* next; // list of free items
	} Item;
	
	int64              mSize; // free
	BList              mPool;
	static BitmapPool* mInstance;
	Item*              mFree; // list of free items

	static int64 freeMemorySize();
	static int64 size(int width, int height, color_space cs);
	Item*        itemAt(int id) const { return (Item*)mPool.ItemAt(id); }
	BitmapPool();

public: 
	static BitmapPool* getInstance();	

	int      newBitmap(); 
	void     deleteBitmap(int id);
	// getBitmap returns NULL if bitmap could not be created
	BBitmap* getBitmap(int id, int width, int height, color_space cs);
};

class Bitmap {
	int        id;
	BitmapPool *pool;
	
public:
	Bitmap()  { pool = BitmapPool::getInstance(); id = pool->newBitmap(); }
	~Bitmap() { pool->deleteBitmap(id); }
	BBitmap* getBitmap(int width, int height, color_space cs) {
		return pool->getBitmap(id, width, height, cs);
	}
};

#endif
