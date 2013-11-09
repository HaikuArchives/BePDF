//========================================================================
//
// BeLoadProgressMonitor.h (Loading Progress Monitor)
//
// Copyright 2005 Michael W. Pfeiffer
//
//========================================================================

#ifndef LPNOTIFIER_H
#define LPNOTIFIER_H

#include <Messenger.h>

#include "LoadProgressMonitor.h"

class BLooper;
class ShowStatusWindow;

class BeLoadProgressMonitor : public LoadProgressMonitor
{
public:
	static BeLoadProgressMonitor* getInstance();
	void setMessenger(BMessenger* messenger);

	void setNumPages(int numPages);
	void loadingPage(int page);
	void setStage(LoadProgressMonitor::Stage stage);

private:
	BMessenger* mMessenger;
	
	BeLoadProgressMonitor();
};

#endif
