/*  
	BeOS Front-end to PDF file reader xpdf.
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

#include "LayerView.h"
#include <be/interface/Window.h>
#include <be/interface/ScrollBar.h>

LayerView::LayerView(BRect frame, const char *name, uint32 resizeMask, uint32 flags) :
	BView(frame, name, resizeMask, flags) {
	mActive = -1;
}

void LayerView::AddLayer(BView *view) {
	AddChild(view);
	mLayers.AddItem(view);
	if (mActive != -1) {
		view->Hide();
	} else {
		mActive = 0;
	}
}

void LayerView::SetActive(int layer) {
	if (layer != mActive && layer >= 0 && layer < mLayers.CountItems()) {
		BView *view = (BView*)mLayers.ItemAt(mActive);
		view->Hide();
		view = (BView*)mLayers.ItemAt(layer);
		view->Show();
		mActive = layer;
	}
}
