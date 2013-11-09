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
#ifndef LAYER_VIEW_H
#define LAYER_VIEW_H

#include <be/interface/View.h>
#include <be/support/List.h>

class LayerView : public BView {
	BList mLayers;
	int   mActive;
		
public:
	LayerView(BRect frame, const char *name, uint32 resizeMask, uint32 flags);

	void AddLayer(BView *view);
	int  Active() const        { return mActive; }
	void SetActive(int layer);
};

#endif
