//========================================================================
//
// LoadProgressMonitor.h (Loading Progress Monitor)
//
// Copyright 2002-2005 Michael W. Pfeiffer
//
//========================================================================

#ifndef LOAD_PROGRESS_MONITOR_H
#define LOAD_PROGRESS_MONITOR_H

#ifdef __GNUC__
#pragma interface
#endif

class LoadProgressMonitor
{	
public:
	LoadProgressMonitor();
	virtual ~LoadProgressMonitor();
	
	enum Stage {
		kBeginLoadPages,
		kDoneLoadPages,
	};

	virtual void setNumPages(int numPages);
	virtual void loadingPage(int page);
	virtual void setStage(Stage stage);
};

#endif
