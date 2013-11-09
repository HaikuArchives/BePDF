/*  
	BeOS Front-end du PDF file reader xpdf.
		
    Copyright (C) 1998 Hubert Figuiere
	Copyright (C) 2000-2005 Michael Pfeiffer

	$Id: BepdfApplication.cpp,v 1.52 2011/05/05 08:34:35 laplace Exp $
	
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

#include <stdlib.h>
#include <ctype.h>

#include <be/app/Application.h>
#include <be/storage/FilePanel.h>
#include <be/app/Roster.h>
#include <be/interface/Screen.h>
#include <be/StorageKit.h>
#include <be/be_apps/Deskbar/Deskbar.h>
#include <be/interface/Alert.h>

#include <gtypes.h>
#include <parseargs.h>
#include "config.h"
#include "Error.h"

#include "Init.h"
#include "PDFWindow.h"
#include "BepdfApplication.h"
#include "ResourceLoader.h"
#include "PasswordWindow.h"
#include "BePDF.h"
#include "TraceWindow.h"
#include "FileInfoWindow.h"

static const char * bePDFCopyright =
	"© 1997 Benoit Triquet\n"
	"© 1999-2000 Hubert Figuiere\n"
    "© 2000-2009 Michael Pfeiffer\n";

static const char * bePDFLocalization = 
    "\nLocalization to ";
static const char * bePDFLocalizationBy = " by ";

static const char * GPLCopyright = 
    "\n\n"
    "This program is free software; you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 2 of the License, or "
    "(at your option) any later version.\n"
	"\n"
    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n"
	"\n"
    "You should have received a copy of the GNU General Public License"
    "along with this program; if not, write to the Free Software"
    "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.";
    
static const char *PAGE_NUM_MSG = "bepdf:page_num";

static const char *settingsFilename = "BePDF";

static const char* attachmentNames[] = {
	"GRAPH_ANNOT",
	"PAPER_CLIP_ANNOT",
	"PUSH_PIN_ANNOT",
	"TAG_ANNOT",
	"UNKNOWN_ATTACHMENT_ANNOT"
};

static const char* textAnnotNames[] = {
	"COMMENT_ANNOT",
	"HELP_ANNOT",
	"INSERT_ANNOT",
	"KEY_ANNOT",
	"NEW_PARAGRAPH_ANNOT",
	"NOTE_ANNOT",
	"PARAGRAPH_ANNOT",
	"UNKNOWN_TEXT_ANNOT"
};

// the name of struct "stat" depends on OS
#ifdef __HAIKU__
	#define STAT stat_beos
#else // BEOS
	#define STAT stat
#endif

// Implementation of PDFFilter
class PDFFilter : public BRefFilter {
	static const char *valid_filetypes[];

public:
	bool Filter(const entry_ref *ref, BNode *node, struct STAT *st, const char *filetype);
};

static PDFFilter pdfFilter;

BRefFilter* GetPdfFilter() {
	return &pdfFilter;
}

const char * PDFFilter::valid_filetypes[] = {
	"application/x-vnd.Be-directory",
	"application/x-vnd.Be-symlink",
	"application/x-vnd.Be-volume",
	"application/pdf",
	"application/x-pdf",
	NULL
};

bool PDFFilter::Filter(const entry_ref *ref, BNode *node, struct STAT *st, const char *filetype) {
	for (int i = 0; valid_filetypes[i]; i++) {
		if (strcmp(filetype, valid_filetypes[i]) == 0) return true;
		// check file extension if filetype has not been set to application/pdf
		BString name(ref->name);
		name.ToUpper();
		int32 l = name.FindLast('.');
		if (l != B_ERROR) {
			if (name.FindFirst(".PDF", l) != B_ERROR) return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////
int main()
{
	new BepdfApplication ( );
	
	be_app->Run();

	delete be_app;
	return 0;
}



///////////////////////////////////////////////////////////
void BepdfApplication::LoadImages(BBitmap* images[], const char* names[], int num) {
	for (int i = 0; i < num; i++) {
		images[i] = LoadBitmap(names[i], 'BBMP');
		if (!images[i]) 
			fprintf(stderr, "Could not load bitmap %s\n", names[i]);
	}
}

///////////////////////////////////////////////////////////
void BepdfApplication::FreeImages(BBitmap* images[], int num) {
	for (int i = 0; i < num; i++) {
		delete images[i]; images[i] = NULL;
	}
}

///////////////////////////////////////////////////////////
BepdfApplication::BepdfApplication()
		: BApplication ( BEPDF_APP_SIG )
{
	mSettings = new GlobalSettings();
	mOpenFilePanel            = NULL;
	mSaveFilePanel            = NULL;
	mSaveToDirectoryFilePanel = NULL;
	mInitialized  = false;
	mGotSomething = false;
	mReadyToQuit  = false;
	mWindow = NULL;

	mStdoutTracer = NULL;
	mStderrTracer = NULL;
	pointerCursor = (BCursor*)B_CURSOR_SYSTEM_DEFAULT; 
	linkCursor = LoadCursor("LINK"); 
	handCursor = LoadCursor("HAND");
	grabCursor = LoadCursor("GRAB");
	textSelectionCursor = LoadCursor("TEXT_SELECTION");
	zoomCursor = LoadCursor("ZOOM");
	splitVCursor = LoadCursor("SPLIT_VERTICAL");
	resizeCursor = LoadCursor("RESIZE");
	
	LoadImages(mAttachmentImages, attachmentNames, FileAttachmentAnnot::no_of_types);
	LoadImages(mTextAnnotImages,  textAnnotNames,  TextAnnot::no_of_types);
		
	BEntry entry; app_info info;
	if (B_OK == be_app->GetAppInfo(&info)) {
		mTeamID = info.team;
		entry = BEntry(&info.ref);
		entry.GetPath(&mAppPath);
		mAppPath.GetParent(&mAppPath);
	} else {
		mAppPath.SetTo(".");
	}
	
	mDefaultPDF = mAppPath;
	mDefaultPDF.Append("docs/Start.pdf");
	
	BPath path(mAppPath);
	LoadSettings();
	path.Append("locale"); 
	if (!StringLocalization::Initialize(path, mSettings->GetLanguage())) {
		const char* s = "Localization catalog not found!";
		BAlert *info = new BAlert("Fatal Error", s, "Exit", 
			NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		info->Go();
		exit (-1);
	}
		
	InitBePDF();
}

#include <GlobalParams.h>
#include <GList.h>
#include <GString.h>
#include "DisplayCIDFonts.h"

static void setGlobalParameter(const char* type, const char* arg1, const char* arg2 = NULL) {
	GString line;
	line.append(type);
	line.append(" ");
	line.append(arg1);
	if (arg2 != NULL) {
		line.append(" ");
		line.append(arg2);
	}
	GString name("BepdfApplication");
	globalParams->parseLine(line.getCString(), &name, 0);
}

void 
BepdfApplication::Initialize()
{
	if (!mInitialized) {
		mInitialized = true;
		
		// built in fonts
		BPath fontDirectory(mAppPath);
		fontDirectory.Append("fonts");
		
		// built in encodings
		BPath encodingDirectory(mAppPath);
		encodingDirectory.Append("encodings");
		
		InitXpdf(NULL, fontDirectory.Path(), encodingDirectory.Path());
		
		// system fonts
		BPath systemFontsPath;
		if (find_directory(B_BEOS_FONTS_DIRECTORY, &systemFontsPath) == B_OK) {
			BDirectory directory(systemFontsPath.Path());
			BEntry entry;
			while (directory.GetNextEntry(&entry) == B_OK) {
				if (!entry.IsDirectory())
					continue;
				BPath fontDirectory;
				if (entry.GetPath(&fontDirectory) != B_OK)
					continue;
				setGlobalParameter("fontDir", fontDirectory.Path());
			}
		}
		
		// CID fonts
		BMessage msg;
		mSettings->GetDisplayCIDFonts(msg);
		DisplayCIDFonts displayNames(msg);
		
		// record new names
		bool foundNewName = false;
		GList* list = globalParams->getCIDToUnicodeNames();
		for (int i = 0; i < list->getLength(); i ++) {
			GString* name = (GString*)list->get(i);
			if (displayNames.Contains(name->getCString())) {
				continue;
			}
			// record name
			displayNames.Set(name->getCString());
			foundNewName = true;
		}
		
		// store in settings
		if (foundNewName) {
			msg.MakeEmpty();
			displayNames.Archive(msg);
			mSettings->SetDisplayCIDFonts(msg);
		} 
		
		// set CID fonts
		for (int i = 0; i < list->getLength(); i ++) {
		    GString* name = (GString*)list->get(i);
			BString file;
			DisplayCIDFonts::Type type;
			
			displayNames.Get(name->getCString(), file, type);
			if (type == DisplayCIDFonts::kUnknownType ||
				file.Length() == 0) {
				continue;
			}
			
			if (type == DisplayCIDFonts::kTrueType) {
				setGlobalParameter("displayCIDFontTT", name->getCString(), file.String());
			} else {
				setGlobalParameter("displayCIDFontT1", name->getCString(), file.String());
			}
		}
		
		deleteGList(list, GString);		
		
		// hinting
		globalParams->setFreeTypeHinting(
			(char*)(mSettings->GetHinting() ? "yes" : "no"));
	}
}

///////////////////////////////////////////////////////////
BepdfApplication::~BepdfApplication()
{	
	SaveSettings();
	
	delete mSettings; mSettings = NULL;

	delete linkCursor;          linkCursor = NULL;
	delete handCursor;          handCursor = NULL;
	delete grabCursor;          grabCursor = NULL;
	delete textSelectionCursor; textSelectionCursor = NULL;
	delete zoomCursor;          zoomCursor = NULL;
	delete splitVCursor;        splitVCursor = NULL;
	delete resizeCursor;        resizeCursor = NULL;
	
	FreeImages(mAttachmentImages, FileAttachmentAnnot::no_of_types);
	FreeImages(mTextAnnotImages,  TextAnnot::no_of_types);

	ExitBePDF();
}


///////////////////////////////////////////////////////////
void BepdfApplication::ReadyToRun()
{
#if 1
	mStdoutTracer = new OutputTracer(1, "stdout", GetSettings());
	mStderrTracer = new OutputTracer(2, "stderr", GetSettings());
#else
	mStdoutTracer = mStderrTracer = NULL;
#endif

	Initialize();
	InstallMimeTypes();
	if (! mGotSomething) {
		// open start document
		entry_ref defaultDocument;
		BMessage msg(B_REFS_RECEIVED);
		get_ref_for_path (mDefaultPDF.Path(), &defaultDocument);
		msg.AddRef ("refs", &defaultDocument);		
		RefsReceived (&msg);
		
		if (!mGotSomething) {
			// on error open file open dialog
			OpenFilePanel();
		}
	}
}

///////////////////////////////////////////////////////////
void BepdfApplication::AboutRequested()
{
	BString version;
	BString str("BePDF Version ");
	str += GetVersion(version);
	str += "\n";
	
	str += bePDFCopyright;

	str += bePDFLocalization;
	str += TRANSLATE("CatalogLanguage");
	str += bePDFLocalizationBy;
	str += TRANSLATE("CatalogAuthor");
	str += "\n";

	str += "\nBePDF is based on xpdf ";
	str += xpdfVersion;
	str += "\n";
	str += xpdfCopyright;

	str += GPLCopyright;

	BAlert *about = new BAlert("About", str.String(), "OK");
	BTextView *v = about->TextView();
	if (v) {
		rgb_color red = {255, 0, 51, 255};
		rgb_color blue = {0, 102, 255, 255};

		v->SetStylable(true);
		char *text = (char*)v->Text();
		char *s = text;
		// set all Be in BePDF in blue and red
		while ((s = strstr(s, "BePDF")) != NULL) {
			int32 i = s - text;
			v->SetFontAndColor(i, i+1, NULL, 0, &blue);
			v->SetFontAndColor(i+1, i+2, NULL, 0, &red);
			s += 2;
		}
		// first text line 
		s = strchr(text, '\n');
		BFont font;
		v->GetFontAndColor(0, &font);
		font.SetSize(12);
		v->SetFontAndColor(0, s-text+1, &font, B_FONT_SIZE);
	};
	about->Go();
}

/*
	open a file panel and ask for a PDF file 
	the file panel will tell by itself if openning have been cancelled 
	or not.
*/
void BepdfApplication::OpenFilePanel ()
{
	if (mOpenFilePanel == NULL) {
		mOpenFilePanel = new BFilePanel (B_OPEN_PANEL,
						NULL, NULL, B_FILE_NODE, true, NULL, NULL);
		mOpenFilePanel->SetRefFilter(&pdfFilter);
	}
	mOpenFilePanel->SetPanelDirectory(mSettings->GetPanelDirectory());
	mReadyToQuit = true;
	mOpenFilePanel->Show();
}

/*
	open a file panel and ask for a PDF file 
	the file panel will tell by itself if openning have been cancelled 
	or not.
*/
void BepdfApplication::OpenSaveFilePanel(BHandler* handler, bool fileMode, BRefFilter* filter, BMessage* msg, const char* name) {
	BFilePanel* panel = NULL;

	// lazy construct file panel
	if (fileMode) {
		// file panel for selection of file
		if (mSaveFilePanel == NULL) {
			mSaveFilePanel = new BFilePanel (B_SAVE_PANEL,
							NULL, NULL, B_FILE_NODE, false, NULL, NULL, true);
		}

		// hide other file panel
		if (mSaveToDirectoryFilePanel != NULL && mSaveToDirectoryFilePanel->IsShowing()) {
			mSaveToDirectoryFilePanel->Hide();
		}
		
		panel = mSaveFilePanel;
	} else {
		// file panel for selection of directory
		if (mSaveToDirectoryFilePanel == NULL) {
			mSaveToDirectoryFilePanel = new BFilePanel (B_OPEN_PANEL,
							NULL, NULL, B_DIRECTORY_NODE, false, NULL, NULL, true);
		}
		
		// hide other file panel
		if (mSaveFilePanel != NULL && mSaveFilePanel->IsShowing()) {
			mSaveFilePanel->Hide();
		}

		panel = mSaveToDirectoryFilePanel;
	}	
	
	// (re)-set to directory of currently opened PDF file
	// TODO decide if directory should be independent from PDF file
	panel->SetPanelDirectory(mSettings->GetPanelDirectory());		

	if (name != NULL) {
		panel->SetSaveText(name);
	}
	else if (fileMode) {
		panel->SetSaveText("");
	}

	// set/reset filter
	panel->SetRefFilter(filter);
	
	// add kind to message	
	BMessage message(B_SAVE_REQUESTED);
	if (msg == NULL) {
		msg = &message;
	}
	panel->SetMessage(msg);
	
	// set target
	BMessenger msgr(handler);
	panel->SetTarget(msgr);
	
	panel->Refresh();
	
	panel->Show();
}

void BepdfApplication::OpenSaveFilePanel(BHandler* handler, BRefFilter* filter, BMessage* msg, const char* name) {
	OpenSaveFilePanel(handler, true, filter, msg, name);
}

void BepdfApplication::OpenSaveToDirectoryFilePanel(BHandler* handler, BRefFilter* filter, BMessage* msg, const char* name) {
	OpenSaveFilePanel(handler, false, filter, msg, name);
}


/*
  NOTIFY_QUIT_MSG:
  Or to quit all BePDF applications.
*/
void BepdfApplication::Notify(uint32 cmd) {
	BList list;
	be_roster->GetAppList(BEPDF_APP_SIG, &list);
	const int n = list.CountItems()-1;
	BMessage msg(cmd);
	// notify all but this team
	for (int i = n; i >= 0; i --) {
		team_id who = (team_id)list.ItemAt(i);
		if (who == mTeamID) continue; // skip own team
		status_t status;
		BMessenger app(BEPDF_APP_SIG, who, &status);
		if (status == B_OK) {
			app.SendMessage(&msg, (BHandler*)NULL, 0);
		}
	}
	// notify ourself
	PostMessage(&msg, (BHandler*)NULL, 0);
}		

bool BepdfApplication::QuitRequested() {
	delete mStdoutTracer; mStdoutTracer = NULL;
	delete mStderrTracer; mStderrTracer = NULL;

	bool shortcut;
	if (B_OK == CurrentMessage()->FindBool("shortcut", &shortcut) && shortcut) {
		Notify(NOTIFY_QUIT_MSG);
	}
	return BApplication::QuitRequested();
}

///////////////////////////////////////////////////////////
/*
	Opens everything.
*/
void BepdfApplication::RefsReceived ( BMessage * msg )
{
	uint32 type;
	int32 count;
	int32 i;
	entry_ref ref;
	
	mReadyToQuit = false;
		
	msg->GetInfo ( "refs", &type, &count );
	if ( type != B_REF_TYPE ) {
		return;
	}
	
	BString ownerPassword, userPassword;
	const char *owner = NULL;
	const char *user  = NULL;
	if (B_OK == msg->FindString("ownerPassword", &ownerPassword)) {
		owner = ownerPassword.String();
	}
	if (B_OK == msg->FindString("userPassword", &userPassword)) {
		user = userPassword.String();
	}
	
	Initialize();
	
	for ( i = --count ; i >= 0; i-- ) {
		if ( msg->FindRef("refs", i, &ref ) == B_OK ) {
			/*
				Open the document...
				WARNING: The application thread is used to open a file!
			*/
			PDFWindow *win;
			BRect rect(mSettings->GetWindowRect());
			bool ok;
			bool encrypted = false;
			if (mWindow == NULL) {
				win = new PDFWindow(&ref, rect, false, owner, user, &encrypted);
				ok = win->IsOk();
			} else {
				win = mWindow; 
				win->Lock();
				ok = mWindow->LoadFile(&ref, owner, user, &encrypted);
				win->Unlock();
			} 
			
			if (!ok) {
				if (!encrypted) {
			 		BAlert *error = new BAlert(TRANSLATE("Error"), TRANSLATE("BePDF: Error opening file!"), TRANSLATE("Close"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
			 		error->Go();
		 		}
		 		
		 		if (mWindow == NULL) delete win;
		 		
		 		if (encrypted) {
		 			new PasswordWindow(&ref, rect, this);
		 		}
			} else if (mWindow == NULL) {
				mWindow = win;
				win->Show();
			}
			// stop after first document
			mGotSomething = true;
			break;
		} 
	}	
}



///////////////////////////////////////////////////////////
void
BepdfApplication::MessageReceived (BMessage * msg)
{
	if (msg == NULL) {
		fprintf (stderr, "xpdf: message NULL received\n");
		return;
	}

	switch (msg->what) {
	case NOTIFY_QUIT_MSG:
		if (mWindow) {
			BWindow* w = mWindow;
			w->Lock(); 
			w->PostMessage(B_QUIT_REQUESTED);
			w->Unlock();
		}
		break;
	case NOTIFY_CLOSE_MSG:
		if (mWindow) {
			mWindow->Lock(); 
			mWindow->UpdateWindowsMenu();
			mWindow->Unlock();
		}
		break;
	case B_CANCEL:
		if (!mWindow && mReadyToQuit) {
			PostMessage(B_QUIT_REQUESTED);
		}
		break;
	default:
		BApplication::MessageReceived(msg);
	}
}





///////////////////////////////////////////////////////////
void
BepdfApplication::ArgvReceived (int32 argc, char **argv)
{
	GBool ok;
	int pg;
	entry_ref fileToOpen;

	// copy args because parseArgs might be change it
	char **argvCopy = new char*[argc];
	for (int i = 0; i < argc; i ++) {
		argvCopy[i] = argv[i];
	}
	
	int intArgc = argc;
	ok = parseArgs (GetGlobalArgDesc(), &intArgc, argvCopy);
	argc = intArgc;
		
	// check command line
	if (!(argc == 2 || argc == 3) || GetPrintHelp()) {
		printUsage(argvCopy[0], "[<PDF-file> [<page>]]", GetGlobalArgDesc());
		exit(1);
	}
	if (argc == 3) {
		pg = atoi(argvCopy[2]);
	} else {
		pg = 1;
	}

	// print banner
//	fprintf(errFile, "BePDF version %s\n", pdfViewerVersion);
//	fprintf(errFile, "based on xpdf %s %s\n", xpdfVersion, xpdfCopyright);
//	fprintf(errFile, "and based on BePDF %s\n%s\n", bePDFVersion, bePDFCopyright);
//	fprintf(errFile, "%s%s\n", pdfViewerCopyright, GPLCopyright);

	BMessage msg(B_REFS_RECEIVED);
	msg.AddInt32 (PAGE_NUM_MSG, pg);
	get_ref_for_path (argvCopy[1], &fileToOpen);
	msg.AddRef ("refs", &fileToOpen);
	PostMessage (&msg);
	mGotSomething = true;
	delete argvCopy;
}


///////////////////////////////////////////////////////////
void BepdfApplication::LoadSettings() {
BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK &&
		path.Append(settingsFilename) == B_OK ) {
		mSettings->Load(path.Path());
	}
}

///////////////////////////////////////////////////////////
void BepdfApplication::SaveSettings() {
BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK &&
		path.Append(settingsFilename) == B_OK) {
		mSettings->Save(path.Path());
	}
}


const int32 kLargeIconWidth = 32;
const int32 kLargeIconHeight = 32;
const color_space kLargeIconColorSpace = B_COLOR_8_BIT;

const unsigned char kLargeIconBits [] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x3f,0x3f,0x3f,0x00,0x3f,0x3f,0x3f,0x00,0x3f,0x3f,0x3f,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x3f,0x00,0x3f,0x00,0x3f,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x3f,0x3f,0x3f,0x00,0x3f,0x00,0x3f,0x00,0x3f,0x3f,0x3f,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x3f,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x3f,0x3f,0x1c,
	0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x1c,0x3f,0x1c,0x3f,
	0x1c,0x3f,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x3f,0x3f,0x09,0x1d,0x1c,
	0x09,0x1d,0x3f,0x1c,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x2a,0x2a,0x09,0x1d,0x1d,0x1c,0x3f,
	0x1c,0x1d,0x09,0x3f,0x1c,0x3f,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x2a,0x1e,0x1e,0x2a,0x2a,0x1e,0x62,0x1c,
	0x3f,0x1c,0x3f,0x1c,0x09,0x1c,0x1e,0x1c,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x2a,0x2a,0x1e,0x1e,0x2a,0x2a,0x1e,0x1e,
	0x1c,0x3f,0x1c,0x1e,0x2a,0x2a,0x2a,0x1e,0x1c,0x3f,0x00,0x00,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x00,0x3f,0x3f,0x1c,0x3f,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,
	0x2a,0x2a,0x2a,0x2a,0x2a,0x1e,0x2a,0x2a,0x1e,0x1c,0x3f,0x1c,0x00,0x00,0xff,0xff,
	0xff,0xff,0xff,0x00,0x3f,0x3f,0x1c,0x3f,0x09,0x1d,0x1c,0x2a,0x2a,0x2a,0x2a,0x2a,
	0x2a,0x2a,0x2a,0x2a,0x1e,0x1e,0x2a,0x2a,0x1c,0x3f,0x1c,0x3f,0x1c,0x1c,0x00,0xff,
	0xff,0xff,0x00,0x3f,0x3f,0x1c,0x3f,0x1c,0x1d,0x1c,0x1c,0x1c,0x1c,0x2a,0x2a,0x2a,
	0x1e,0x1e,0x2a,0x2a,0x2a,0x2a,0x2a,0x1c,0x1e,0x1c,0x3f,0x1c,0x1c,0x00,0xff,0xff,
	0xff,0x00,0x3f,0x1c,0x1c,0x1c,0x1c,0x1c,0x09,0x1c,0x1c,0x1c,0x1c,0x1c,0x2a,0x2a,
	0x1e,0x2a,0x2a,0x2a,0x1c,0x1c,0x09,0x1c,0x1c,0x1c,0x1c,0x1c,0x29,0xb0,0xb0,0xb0,
	0x00,0x00,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x2a,
	0x2a,0x2a,0x2a,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1f,0x00,0x00,0xb0,0xb0,0xff,
	0xff,0xb0,0x00,0x00,0x1c,0x1c,0x1c,0x1c,0x1c,0x09,0x1c,0x1c,0x1c,0x1c,0x1c,0x2a,
	0x2a,0x2a,0x1c,0x1c,0x1c,0x1c,0x09,0x1c,0x1c,0x1c,0x29,0x16,0x16,0x00,0xb0,0xff,
	0xff,0xff,0xb0,0x00,0x00,0x00,0x1a,0x1a,0x1a,0x1a,0x1c,0x1a,0x1a,0x1a,0x1a,0x2a,
	0x2a,0x2a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x29,0x16,0x16,0x17,0x00,0x00,0xb0,
	0xff,0xff,0xff,0xb0,0x00,0x12,0x00,0x00,0x1a,0x1a,0x1a,0x09,0x1a,0x1a,0x1a,0x2a,
	0x3f,0x2a,0x1a,0x1a,0x09,0x1a,0x1a,0x1a,0x00,0x16,0x16,0x29,0x00,0xb0,0xb0,0xb0,
	0xff,0xff,0xff,0xff,0xb0,0x00,0x12,0x12,0x00,0x00,0x18,0x18,0x18,0x18,0x2a,0x2a,
	0x3f,0x2a,0x09,0x18,0x18,0x18,0x18,0x00,0x16,0x00,0x00,0xb0,0xb0,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x16,0x12,0x12,0x00,0x00,0x18,0x18,0x2a,0x2a,
	0x3f,0x2a,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x16,0x16,0x12,0x12,0x00,0x00,0x17,0x2a,
	0x2a,0x2a,0x17,0x17,0x17,0x00,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x17,0x16,0x16,0x12,0x12,0x00,0x00,
	0x17,0x17,0x17,0x17,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x17,0x16,0x16,0x16,0x12,0x12,
	0x00,0x00,0x17,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x17,0x17,0x17,0x17,0x00,
	0x00,0xb0,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0x17,0x17,0x00,0xb0,
	0xb0,0xb0,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0xb0,0xb0,0xff,
	0xff,0xff,0xb0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb0,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

const int32 kSmallIconWidth = 16;
const int32 kSmallIconHeight = 16;
const color_space kSmallIconColorSpace = B_COLOR_8_BIT;

const unsigned char kSmallIconBits [] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x00,0x3f,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x00,0x1e,0x1d,0x1d,0x1d,0x00,0x00,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0x00,0x1e,0x2a,0x1d,0x1d,0x1d,0x08,0x1e,0x00,0x00,0xff,0xff,0xff,
	0xff,0xff,0x00,0x1e,0x2a,0x2a,0x2a,0x1d,0x1d,0x1d,0x2a,0x1e,0x1d,0x00,0x00,0xff,
	0xff,0x00,0x1d,0x1d,0x1d,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x1e,0x1d,0x00,0x00,
	0x00,0x1d,0x1d,0x1a,0x1a,0x1a,0x1d,0x2a,0x2a,0x2a,0x1a,0x1a,0x1a,0x00,0x00,0xb0,
	0xb0,0x00,0x00,0x1a,0x1a,0x1a,0x1a,0x2a,0x2a,0x1a,0x1a,0x1a,0x29,0x89,0x00,0x00,
	0xff,0xb0,0x00,0x00,0x00,0x17,0x2a,0x2a,0x2a,0x17,0x17,0x00,0x89,0x00,0xb0,0xb0,
	0xff,0xff,0xb0,0x00,0x89,0x00,0x00,0x1e,0x2a,0x17,0x00,0x00,0x00,0xb0,0xff,0xff,
	0xff,0xff,0xff,0xb0,0x00,0x62,0x89,0x00,0x17,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xb0,0x00,0x00,0x00,0x00,0xb0,0xb0,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xb0,0x00,0xb0,0xff,0xb0,0xff,0xff,0xff,0xff,0xff,0xff
};


static struct {
	const char *name;
	const char *public_name;
	const char *pdf_name;
	int32 type_code;
} gAttrInfo[] = {
	{"pdf:subject",     "Subject",     "Subject",      B_STRING_TYPE},
	{"pdf:title",       "Title",       "Title",        B_STRING_TYPE},
	{"pdf:creator",     "Creator",     "Creator",      B_STRING_TYPE},
	{"pdf:author",      "Author",      "Author",       B_STRING_TYPE},
	{"pdf:keywords",    "Keywords",    "Keywords",     B_STRING_TYPE},
	{"pdf:producer",    "Producer",    "Producer",     B_STRING_TYPE},
	{"pdf:created",     "Created",     "CreationDate", B_TIME_TYPE},
	{"pdf:modified",    "Modified",    "ModDate",      B_TIME_TYPE},
	{"pdf:pages",       "Pages",       NULL,           B_INT32_TYPE},
	{"pdf:version",     "Version",     NULL,           B_DOUBLE_TYPE},
	{"pdf:linearized",  "Linearized",  NULL,           B_BOOL_TYPE},
	{NULL, NULL, NULL, 0}
};


static void AddIfMissing(BMessage* m, const char* key, const char* value) {
	BString v;
	for (int i = 0; B_OK == m->FindString(key, i, &v); i++) {
		if (strcmp(v.String(), value) == 0) return; // found
	}
	m->AddString(key, value);
}

///////////////////////////////////////////////////////////
void BepdfApplication::InstallMimeType(const char *mime) {
	BMimeType type(mime);
	BMessage info;
	bool has_changed = false;

	// install mime type of PDF file
	if (!type.IsInstalled()) {
		type.Install();
	}
	// get attr info
	if (B_OK != type.GetAttrInfo(&info)) {
		info.MakeEmpty();
	}
	// add missing attr info 
	for (int i = 0; gAttrInfo[i].name; i++) {
		BString name;
		bool name_found = false;
		for (int j = 0; info.FindString("attr:name", j, &name) == B_OK; j ++) {
			if (strcmp(gAttrInfo[i].name, name.String()) == 0) {
				name_found = true; 
				break;
			}
		}
		if (!name_found) {
			has_changed = true;
			info.AddString("attr:public_name", gAttrInfo[i].public_name);
			info.AddString("attr:name",        gAttrInfo[i].name);
			info.AddInt32 ("attr:type",        gAttrInfo[i].type_code);
			info.AddBool  ("attr:viewable",    true);
			info.AddBool  ("attr:editable",    true);
			info.AddInt32 ("attr:width",       120);
			info.AddInt32 ("attr:alignment",   B_ALIGN_LEFT);
			info.AddBool  ("attr:extra",       false);
		}
	}
	// set attr info 
	if (has_changed) {
		BBitmap	large_icon(BRect(0, 0, B_LARGE_ICON - 1, B_LARGE_ICON - 1), B_COLOR_8_BIT);
		BBitmap	mini_icon(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_COLOR_8_BIT);
		large_icon.SetBits(kLargeIconBits, large_icon.BitsLength(), 0, B_COLOR_8_BIT);
		mini_icon.SetBits(kSmallIconBits, mini_icon.BitsLength(), 0, B_COLOR_8_BIT);

		type.SetShortDescription("PDF");
		type.SetLongDescription("Portable Document Format.");
		type.SetIcon(&large_icon, B_LARGE_ICON);
		type.SetIcon(&mini_icon, B_MINI_ICON);
		type.SetPreferredApp(BEPDF_APP_SIG);
		type.SetAttrInfo(&info);	
	}

	BMessage exts;
	if (B_OK != type.GetFileExtensions(&exts)) {
		exts.MakeEmpty();
	}
	AddIfMissing(&exts, "extensions", "pdf");
	AddIfMissing(&exts, "extensions", "PDF");
	type.SetFileExtensions(&exts);
}

///////////////////////////////////////////////////////////
void BepdfApplication::InstallMimeTypes() {
	InstallMimeType("application/pdf");
	InstallMimeType("application/x-pdf");
}


///////////////////////////////////////////////////////////
void 
BepdfApplication::UpdateAttr(BNode &node, const char *name, type_code type, off_t offset, void *buffer, size_t length) {
	char dummy[10];
	if (B_ENTRY_NOT_FOUND == node.ReadAttr(name, type, offset, (char*)dummy, sizeof(dummy))) {
		node.WriteAttr(name, type, offset, buffer, length);
	}
}


///////////////////////////////////////////////////////////
void 
BepdfApplication::UpdateFileAttributes(PDFDoc *doc, entry_ref *ref) {
	BNode node(ref);
	if (node.InitCheck() != B_OK) return;
		
	const bool force_overwrite = (modifiers() & B_COMMAND_KEY) == B_COMMAND_KEY;

	if (force_overwrite) {
		for (int i = 0; gAttrInfo[i].name; i++) {
			node.RemoveAttr(gAttrInfo[i].name);
		}
	}

	int32 pages = (int32)doc->getNumPages();
	UpdateAttr(node, "pdf:pages", B_INT32_TYPE, 0, &pages, sizeof(int32));
	bool b = doc->isLinearized();
	UpdateAttr(node, "pdf:linearized", B_BOOL_TYPE, 0, &b, sizeof(b));
	double d = doc->getPDFVersion();
	UpdateAttr(node, "pdf:version", B_DOUBLE_TYPE, 0, &d, sizeof(d));
	
	Object obj; 
	if (doc->getDocInfo(&obj) && obj.isDict()) {
		Dict *dict = obj.getDict();
		for (int i = 0; gAttrInfo[i].name; i++) {
			time_t time;
			if (gAttrInfo[i].pdf_name == NULL) continue;
			BString *s = FileInfoWindow::GetProperty(dict, gAttrInfo[i].pdf_name, &time);
			if (s) {
				if (gAttrInfo[i].type_code == B_TIME_TYPE) {
					if (time != 0) {
						UpdateAttr(node, gAttrInfo[i].name, B_TIME_TYPE, 0, &time, sizeof(time));
					}
				} else {
					UpdateAttr(node, gAttrInfo[i].name, B_STRING_TYPE, 0, (void*)s->String(), s->Length()+1);
				}
				delete s;
			}
		}
	}
	obj.free();	
}


const char* BepdfApplication::GetVersion(BString &version) {
	version = "?.?.?";
	if (be_app == NULL) {
		return version.String();
	}
	
	app_info info;
	if (be_app->GetAppInfo(&info) != B_OK) {
		return version.String();
	}
	
	BFile file(&info.ref, B_READ_ONLY);
	if (file.InitCheck() != B_OK) {
		return version.String();
	}

	BAppFileInfo appFileInfo(&file);
	version_info appVersion;
	if (appFileInfo.GetVersionInfo(&appVersion, B_APP_VERSION_KIND) != B_OK) {
		return version.String();
	}

	BString variety = "Unknown";
	switch (appVersion.variety) {
		case 0: variety = "Development";
			break;
		case 1: variety = "Alpha";
			break;
		case 2: variety = "Beta";
			break;
		case 3: variety = "Gamma";
			break;
		case 4: variety = "Golden Master";
			break;
		case 5: 
			if (appVersion.internal == 0) {
				// hide variety
				variety = "";
			}
			else {
				variety = "Final";
			}
			break;
	};
	version = "";
	version << appVersion.major << "." 
		<< appVersion.middle << "."
		<< appVersion.minor
		<< " " << variety;
	if (appVersion.internal != 0) {
		version << " "<< appVersion.internal;
	}
	return version.String();
}

