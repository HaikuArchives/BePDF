#ifndef CONNECT_APP_H
#define CONNECT_APP_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include "MsgConsts.h"
#include "ToolTip.h"

#define APPLICATION "Test"
#define VERSION "1.0"

class AppWindow : public BWindow {
	ToolTip *mToolTip;
	ToolTipItem *mTipItem;
public:
	BMenuBar *menubar;
	AppWindow(BRect);
	bool QuitRequested();
	void AboutRequested();	
	void MessageReceived(BMessage *message);
};

class App : public BApplication {
public:
	AppWindow *window;
	App();
};

#define my_app ((App*)be_app)
#endif