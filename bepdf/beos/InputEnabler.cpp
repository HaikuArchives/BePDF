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

#include "HWindow.h"
#include "InputEnabler.h"

void InputEnablerItem::Update() {
	if (cur_state != new_state) {
		cur_state = new_state;
		UpdateItem(cur_state);
	}
}

IEMenuItem::IEMenuItem(BMenuItem* menu) : InputEnablerItem(true), menu(menu) {
	BMessage* m = menu->Message();
	int32 cmd;
	if (B_OK == m->FindInt32(CMD_IDX_LABEL, &cmd)) {
		SetCmd(cmd);
	}
}

// Implementation of InputEnabler
InputEnabler::~InputEnabler() {
	for (int i = mItems.CountItems()-1; i >= 0; i--) {
		InputEnablerItem* item = (InputEnablerItem*)mItems.ItemAt(i);
		delete item;
	}
}

void InputEnabler::Register(InputEnablerItem* item) {
	mItems.AddItem(item);
}

void InputEnabler::Unregister(int type, int32 cmd) {
	InputEnablerItem* item = FindItem(type, cmd);
	if (item) {
		mItems.RemoveItem(item); delete item;
	}
}

InputEnablerItem* InputEnabler::FindItem(int type, int32 cmd) {
	for (int i = mItems.CountItems()-1; i >= 0; i--) {
		InputEnablerItem* item = (InputEnablerItem*)mItems.ItemAt(i);
		if (item->Type() == type && item->Cmd() == cmd) return item;
	}	
	return NULL;
}
		
void InputEnabler::SetEnabled(int type, int32 cmd, bool enable) {
	InputEnablerItem* item = FindItem(type, cmd);
	if (item) {
		item->SetEnabled(enable);
	}
}

void InputEnabler::SetEnabled(int32 cmd, bool enable) {
	for (int i = mItems.CountItems()-1; i >= 0; i--) {
		InputEnablerItem* item = (InputEnablerItem*)mItems.ItemAt(i);
		if (item->Cmd() == cmd) {
			item->SetEnabled(enable);
		}
	}
}

void InputEnabler::Update() {
	for (int i = mItems.CountItems()-1; i >= 0; i--) {
		InputEnablerItem* item = (InputEnablerItem*)mItems.ItemAt(i);
		item->Update();
	}
}
