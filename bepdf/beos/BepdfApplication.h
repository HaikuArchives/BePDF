/*  
	BeOS Front-end du PDF file reader xpdf.
    Copyright (C) 1998 Hubert Figuiere
	Copyright (C) 2000-2002 Michael Pfeiffer
	
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
//
// BepdfApplication.h
// 	Be port of Xpdf
//
// (c)1998 Hubert Figuiere
//
// 



#ifndef _BE_PDFAPPLICATION_
#define _BE_PDFAPPLICATION_

#include <be/app/Application.h>
#include <be/app/Cursor.h>
#include <be/storage/FilePanel.h>
#include <be/storage/Path.h>
#include <be/storage/Node.h>
#include "Settings.h"
#include "Annotation.h"

#define BEPDF_APP_SIG "application/x-vnd.mp-xpdf"

class PDFWindow;
class OutputTracer;
class PDFDoc;

// Returns a filter for PDF files
BRefFilter* GetPdfFilter();

///////////////////////////////////////////////////////////
class BepdfApplication
	: BApplication
{
public:
	BepdfApplication();
	~BepdfApplication();

	virtual void ReadyToRun();
	virtual void RefsReceived(BMessage* msg );
	virtual void ArgvReceived(int32 argc, char** argv );
	virtual void MessageReceived (BMessage* msg);
	virtual void AboutRequested();
	virtual bool QuitRequested();
	
	// open file panel to open a PDF file
	void OpenFilePanel();
	// open file panel to save a file
	void OpenSaveFilePanel(BHandler* handler, BRefFilter* filter, BMessage* msg = NULL, const char* name = NULL);
	void OpenSaveToDirectoryFilePanel(BHandler* handler, BRefFilter* filter, BMessage* msg = NULL, const char* name = NULL);
		
	void LoadSettings();
	void SaveSettings();
		
	GlobalSettings* GetSettings() { return mSettings; };
		
	BCursor* pointerCursor; 
	BCursor* linkCursor;
	BCursor* handCursor;
	BCursor* grabCursor;
	BCursor* textSelectionCursor;
	BCursor* zoomCursor;
	BCursor* splitVCursor;
	BCursor* resizeCursor;
			
	BPath  *GetAppPath() { return &mAppPath; }
	BPath  *DefaultPDF() { return &mDefaultPDF; }
	team_id GetTeamID() { return mTeamID; }
	
	enum {
		NOTIFY_OPEN_MSG   = 'BPop', // BePDF document opened
		NOTIFY_CLOSE_MSG  = 'BPcl', // BePDF closed
		NOTIFY_QUIT_MSG   = 'BPqt', // Close all BePDF applications
		REQUEST_TITLE_MSG = 'BPrt', // Request the window titles
	};

	void Notify(uint32 cmd);
	void WindowClosed()       { mWindow = NULL; }
	
	BBitmap* GetAttachmentImage(int i) { return mAttachmentImages[i]; }
	BBitmap* GetTextAnnotImage(int i)  { return mTextAnnotImages[i]; }

	static void UpdateAttr(BNode &node, const char* name, type_code type, off_t offset, void* buffer, size_t length);
	static void UpdateFileAttributes(PDFDoc* doc, entry_ref* ref);
	
private:
	const char* GetVersion(BString &version);
	void Initialize();
	void InstallMimeType(const char* mime);	
	void InstallMimeTypes();
	void LoadImages(BBitmap* images[], const char* names[], int num);
	void FreeImages(BBitmap* images[], int num);
	void OpenSaveFilePanel(BHandler* handler, bool fileMode, BRefFilter* filter, BMessage* msg, const char* name);

	bool           mInitialized;
	bool           mGotSomething;
	bool           mReadyToQuit;
	BFilePanel*    mOpenFilePanel;
	BFilePanel*    mSaveFilePanel;
	BFilePanel*    mSaveToDirectoryFilePanel;
	BPath          mAppPath;
	BPath          mDefaultPDF;
	team_id        mTeamID;
	PDFWindow*     mWindow;

	GlobalSettings* mSettings;
	OutputTracer*   mStdoutTracer;
	OutputTracer*   mStderrTracer;

	BBitmap* mAttachmentImages[FileAttachmentAnnot::no_of_types];
	BBitmap* mTextAnnotImages[TextAnnot::no_of_types];
};

#define gApp ((BepdfApplication*)(be_app))

#endif
