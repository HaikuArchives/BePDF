/*  
	Copyright (C) 2005 Michael Pfeiffer
	
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

#ifndef ATTACHMENTS_H
#define ATTACHMENTS_H

// BeOS
#include <be/support/String.h>
#include <be/support/List.h>
#include <be/interface/ListView.h>

#include "TreeParser.h"

class Attachments : public NameTreeParser {
	BList mAttachments;
	
public:
	~Attachments();
	
	bool DoName(const char* name, Object* value);
	void Replace(BListView *view);
};

#endif
