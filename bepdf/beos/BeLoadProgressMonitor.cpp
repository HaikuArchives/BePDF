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

#ifdef __GNUC__
#pragma implementation
#endif

#include "BeLoadProgressMonitor.h"

#include <stdlib.h>

#include <locale/Catalog.h>
#include <be/app/Looper.h>
#include <be/app/Message.h>

#include "StatusWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BeLoadProgressMonitor"

static BeLoadProgressMonitor *gMonitor = NULL;

BeLoadProgressMonitor::BeLoadProgressMonitor()
  : mMessenger(NULL)
{
}

BeLoadProgressMonitor* BeLoadProgressMonitor::getInstance() {
	if (gMonitor == NULL) {
		gMonitor = new BeLoadProgressMonitor();
	}
	return gMonitor;
}

void BeLoadProgressMonitor::setMessenger(BMessenger* messenger) {
	if (mMessenger != NULL) {
		delete mMessenger;
		mMessenger = NULL;
	}

	if (messenger != NULL) {
		mMessenger = new BMessenger(*messenger);
	}
}

void BeLoadProgressMonitor::setNumPages(int numPages) {
	if (mMessenger == NULL) return;
	StatusWindow::SetTotal(mMessenger, numPages);
}

void BeLoadProgressMonitor::loadingPage(int page) {
	if (mMessenger == NULL) return;

	StatusWindow::SetCurrent(mMessenger, page+1);
}

void BeLoadProgressMonitor::setStage(LoadProgressMonitor::Stage stage) {
	if (mMessenger == NULL) return;

	const char* text = NULL;
	switch (stage) {
		case LoadProgressMonitor::kBeginLoadPages:
			text = "Loading pages";
			break;
		case LoadProgressMonitor::kDoneLoadPages:
			text = "Preparing for rendering";
			break;
	}
	if (text != NULL) {
		StatusWindow::SetText(mMessenger, B_TRANSLATE(text));
	}
}
