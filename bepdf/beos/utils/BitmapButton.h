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
#ifndef BITMAP_BUTTON_H
#define BITMAP_BUTTON_H

#include <Button.h>
#include <Bitmap.h>

#include "ToolTipItem.h"

// Pre-condition:
//   Bitmap must exist in resource otherwise the BPictureButton constructor
//   will crash!
class ResourceBitmapButton : public BButton {
public:
	ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		BMessage *message,
		uint32 behavior = 0,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);

	ResourceBitmapButton(BRect frame, const char *name,
		const char *off, const char *on,
		const char *disabledOff, const char *disabledOn, // NULL for default
		BMessage *message,
		uint32 behavior = 0,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);

	ResourceBitmapButton(BRect frame, const char *name,
		int32 offId, int32 onId,
		BMessage *message,
		uint32 behavior = 0,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);

	ResourceBitmapButton(BRect frame, const char *name,
		int32 offId, int32 onId,
		int32 disabledOffId, int32 disabledOnId, // -1 for default
		BMessage *message,
		uint32 behavior = 0,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
};

#endif
