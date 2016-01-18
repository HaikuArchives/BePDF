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

#include <stdlib.h>
#include <ctype.h>

#include <locale/Catalog.h>
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BepdfApplication"

static const char * bePDFCopyright =
	"© 1997 Benoit Triquet\n"
	"© 1998-2000 Hubert Figuiere\n"
    "© 2000-2011 Michael Pfeiffer\n"
    "© 2013-2016 waddlesplash\n";

static const char * GPLCopyright =
    "\n\n"
    "This program is free software under the GNU GPL v2, or any later version.\n";

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

// Implementation of PDFFilter
class PDFFilter : public BRefFilter {
	static const char *valid_filetypes[];

public:
	bool Filter(const entry_ref *ref, BNode *node, struct stat_beos *st, const char *filetype);
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

bool PDFFilter::Filter(const entry_ref *ref, BNode *node, struct stat_beos *st, const char *filetype) {
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
	pointerCursor = new BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
	linkCursor = new BCursor(B_CURSOR_ID_CREATE_LINK);
	handCursor = new BCursor(B_CURSOR_ID_GRAB);
	grabCursor = new BCursor(B_CURSOR_ID_GRABBING);
	textSelectionCursor = new BCursor(B_CURSOR_ID_I_BEAM);
	zoomCursor = new BCursor(B_CURSOR_ID_ZOOM_IN);
	splitVCursor = new BCursor(B_CURSOR_ID_RESIZE_NORTH_SOUTH);
	resizeCursor = new BCursor(B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST);

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
	BString str("BePDF\n");
	str += B_TRANSLATE("Version");
	str += " ";
	str += GetVersion(version);
	str += "\n";

	str += bePDFCopyright;

	str += "\n";
	str += B_TRANSLATE_COMMENT("Language 'English', translated by the BePDF authors.",
		"Replace 'English' with the language you're translating to, and 'the BePDF authors' with your name or your translation group's name.");
	str += "\n\n";

	str += BString().SetToFormat(B_TRANSLATE_COMMENT("BePDF is based on XPDF %s, %s.", "XPDF version, copyright"),
		xpdfVersion, xpdfCopyright);

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
		font.SetSize(16);
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
				win = new PDFWindow(&ref, rect, owner, user, &encrypted);
				ok = win->IsOk();
			} else {
				win = mWindow;
				win->Lock();
				ok = mWindow->LoadFile(&ref, owner, user, &encrypted);
				win->Unlock();
			}

			if (!ok) {
				if (!encrypted) {
			 		BAlert *error = new BAlert(B_TRANSLATE("Error"), B_TRANSLATE("BePDF: Error opening file!"), B_TRANSLATE("Close"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
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
	ok = parseArgs(GetGlobalArgDesc(), &intArgc, argvCopy);
	argc = intArgc;

	// check command line
	if (!ok || !(argc == 2 || argc == 3) || GetPrintHelp()) {
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

	BString variety = B_TRANSLATE("Unknown");
	switch (appVersion.variety) {
		case 0: variety = B_TRANSLATE("Development");
			break;
		case 1: variety = B_TRANSLATE("Alpha");
			break;
		case 2: variety = B_TRANSLATE("Beta");
			break;
		case 3: variety = B_TRANSLATE("Gamma");
			break;
		case 4: variety = B_TRANSLATE("Golden Master");
			break;
		case 5:
			if (appVersion.internal == 0) {
				// hide variety
				variety = "";
			}
			else {
				variety = B_TRANSLATE("Final");
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

