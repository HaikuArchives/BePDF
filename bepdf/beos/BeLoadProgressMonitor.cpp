//========================================================================
//
// BeLoadProgressMonitor.cc
//
// Copyright 2005 Michael W. Pfeiffer
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include "BeLoadProgressMonitor.h"

#include <stdlib.h>

#include <be/app/Looper.h>
#include <be/app/Message.h>

#include "StatusWindow.h"
#include "StringLocalization.h"

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
		StatusWindow::SetText(mMessenger, TRANSLATE(text));
	}
}
