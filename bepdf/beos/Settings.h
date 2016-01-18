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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <String.h>
#include <Archivable.h>
#include <Window.h> // for B_CURRENT_WORKSPACE
#include <Message.h>

#define DECLARE_VARIABLE(type, Type, method, name, value) type _##name;
#define DEFINE_VARIABLE(type, Type, method, name, value)  _##name = value;
#define DECLARE_SETTER(type, Type, method, name, value)   void Set##method(type name);
#define DECLARE_GETTER(type, Type, method, name, value)   type Get##method() const;
#define DEFINE_SETTER(type, Type, method, name, value)    void GlobalSettings::Set##method(type name) { mChanged = mChanged || (_##name == name); _##name = name; }
#define DEFINE_GETTER(type, Type, method, name, value)    type GlobalSettings::Get##method() const { return _##name; }
#define STORE_SETTINGS(type, Type, method, name, value)   archive->Add##Type(#name, _##name);
#define LOAD_SETTINGS(type, Type, method, name, value)  \
  if (B_OK != archive->Find##Type(#name, &_##name))           \
    _##name = value;

//  c-type, msg-type,  name,                  attribute-name,                              default
#define SETTINGS(do)                                                                                \
  do(int16,   Int16,  Zoom,                   zoom,                                              7) \
                                                                                                    \
  do(float,   Float,  Rotation,               rotation,                                        0.0) \
                                                                                                    \
  do(int16,   Int16,  ZoomPrinter,            zoomPrinter,                                     100) \
                                                                                                    \
  do(int32,   Int32,  DPI,                    dpi,                                               0) \
                                                                                                    \
  do(float,   Float,  RotationPrinter,        rotationPrinter,                                 0.0) \
                                                                                                    \
  do(int16,   Int16,  PrintSelection,         printSelection,                                    0) \
  do(int16,   Int16,  PrintOrder,             printOrder,                                        0) \
                                                                                                    \
  do(BPoint,  Point,  WindowPosition,         winPos,                               BPoint(10, 23)) \
  do(float,   Float,  WindowWidth,            winWidth,                                        780) \
  do(float,   Float,  WindowHeight,           winHeight,                                       740) \
                                                                                                    \
  do(BPoint,  Point,  PrefsWindowPosition,    prefsWinPos,                        BPoint(100, 100)) \
  do(float,   Float,  PrefsWinWidth,          prefsWinWidth,                                   600) \
  do(float,   Float,  PrefsWinHeight,         prefsWinHeight,                                   80) \
                                                                                                    \
  do(BPoint,  Point,  PrinterWindowPosition,  printWinPos,                        BPoint(100, 100)) \
  do(float,   Float,  PrintWinWidth,          printWinWidth,                                   100) \
  do(float,   Float,  PrintWinHeight,         printWinHeight,                                   80) \
                                                                                                    \
  do(BPoint,  Point,  FileInfoWindowPosition, fileInfoWinPos,                     BPoint(100, 100)) \
  do(float,   Float,  FileInfoWinWidth,       fileInfoWinWidth,                                440) \
  do(float,   Float,  FileInfoWinHeight,      fileInfoWinHeight,                               200) \
                                                                                                    \
  do(BPoint,  Point,  FindWindowPosition,     findWinPos,                         BPoint(100, 100)) \
  do(float,   Float,  FindWinWidth,           findWinWidth,                                    440) \
  do(float,   Float,  FindWinHeight,          findWinHeight,                                   404) \
  do(bool,    Bool,   FindIgnoreCase,         findIgnoreCase,                                 true) \
  do(bool,    Bool,   FindBackward,           findBackward,                                  false) \
                                                                                                    \
  do(BPoint,  Point,  OutlinesWindowPosition, outlinesWinPos,                     BPoint(100, 100)) \
  do(float,   Float,  OutlinesWinWidth,       outlinesWinWidth,                                440) \
  do(float,   Float,  OutlinesWinHeight,      outlinesWinHeight,                               404) \
                                                                                                    \
  do(BPoint,  Point,  AnnotationWindowPosition, annotWinPos,                      BPoint(100, 100)) \
  do(float,   Float,  AnnotationWinWidth,       annotWinWidth,                                 440) \
  do(float,   Float,  AnnotationWinHeight,      annotWinHeight,                                404) \
                                                                                                    \
  do(BPoint,  Point,  TraceWindowPosition,    traceWinPos,                        BPoint(100, 100)) \
  do(float,   Float,  TraceWinWidth,          traceWinWidth,                                   440) \
  do(float,   Float,  TraceWinHeight,         traceWinHeight,                                  404) \
                                                                                                    \
  do(bool,    Bool,   RestorePageNumber,      restorePageNumber,                              true) \
  do(bool,    Bool,   RestoreWindowFrame,     restoreWindowFrame,                            false) \
  do(bool,    Bool,   ShowLeftPanel,          showLeftPanel,                                  true) \
  do(bool,    Bool,   QuasiFullscreenMode,    quasiFullscreenMode,                            true) \
  do(bool,    Bool,   FilledSelection,        filledSelection,                                true) \
  do(int8,    Int8,   LeftPanel,              leftPanel,                                         0) \
                                                                                                    \
  do(int8,    Int8,   PrintColorMode,	      printColorMode,                                    0) \
  do(int32,  Int32,   Workspace,              workspace,                                         1) \
  do(bool,    Bool,   OpenInWorkspace,        wrkspcOpen,                                    false) \
  do(bool,    Bool,   InvertVerticalScrolling, invertVerticalScrolling,                      false) \
                                                                                                    \
  do(bool,    Bool,   TraceAutoOpen,          traceAutoOpen,                                  true) \
  do(bool,    Bool,   TraceShowStdout,        traceStdout,                                   false) \
  do(bool,    Bool,   TraceShowStderr,        traceStderr,                                    true) \
  do(bool,    Bool,   TraceFloating,          traceFloating,                                  true) \
                                                                                                    \
  do(float,   Float,  AttachmentFileNameColumnWidth,     attachmentFileNameColumnWidth,      170.0) \
  do(float,   Float,  AttachmentDescriptionColumnWidth,  attachmentDescriptionColumnWidth,   120.0)



#define WINDOW_SETTINGS(do)          \
  do(Window,         Window)         \
  do(PrefsWindow,    PrefsWin)       \
  do(PrinterWindow,  PrintWin)       \
  do(FindWindow,     FindWin)        \
  do(FileInfoWindow, FileInfoWin)    \
  do(OutlinesWindow, OutlinesWin)    \
  do(AnnotationWindow, AnnotationWin) \
  do(TraceWindow,    TraceWin)

#define DECLARE_STRING_VARIABLE(method, name, value) BString _##name;
#define DEFINE_STRING_VARIABLE(method, name, value)  _##name = value;
#define DECLARE_STRING_SETTER(method, name, value)   void Set##method(const char *string);
#define DECLARE_STRING_GETTER(method, name, value)   const char* Get##method() const;
#define DEFINE_STRING_SETTER(method, name, value)    void GlobalSettings::Set##method(const char *string) { mChanged = mChanged || (_##name == string); _##name = string; }
#define DEFINE_STRING_GETTER(method, name, value)    const char * GlobalSettings::Get##method() const { return _##name.String(); }
#define STORE_STRING_SETTINGS(method, name, value)   archive->AddString(#name, _##name.String());
#define LOAD_STRING_SETTINGS(method, name, value)  \
  if (B_OK != archive->FindString(#name, &_##name))     \
    _##name = value;

#define STRING_SETTINGS(do)                     \
  do(Language,           language,   "Default.catalog") \
  do(Author,             author,     "")

#define SIZE_ACCESSOR(method, accessor) \
  void Get##method##Size(float &w, float &h) const { w = Get##accessor##Width(); h = Get##accessor##Height(); } \
  void Set##method##Size(float w, float h)   { Set##accessor##Width(w);    Set##accessor##Height(h); }

// Renamed class Settings to GlobalSettings because of Linker warning
// in PPC Crosscompiler (libtracker)
class _EXPORT GlobalSettings : public BArchivable {
	bool mChanged;

	// file open settings
	BString mPanelDirectory;
	BString mDefaultPanelDirectory;
	// CID display names
	BMessage mDisplayCIDFonts;

	SETTINGS(DECLARE_VARIABLE)
	STRING_SETTINGS(DECLARE_STRING_VARIABLE)

public:
	GlobalSettings();
	bool HasChanged() const;

	SETTINGS(DECLARE_SETTER)
	SETTINGS(DECLARE_GETTER)
	STRING_SETTINGS(DECLARE_STRING_SETTER)
	STRING_SETTINGS(DECLARE_STRING_GETTER)

	WINDOW_SETTINGS(SIZE_ACCESSOR)

	void SetPanelDirectory(const char *dir);
	const char *GetPanelDirectory() const;

	void SetDisplayCIDFonts(const BMessage& fonts);
	void GetDisplayCIDFonts(BMessage& fonts) const;

	// helper functions
	BRect GetWindowRect() const;

	// BArchivable:
	GlobalSettings(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;
	static BArchivable *Instantiate(BMessage *archive);

	enum {
		NORMAL_PRINT_ORDER = 0,
		REVERSE_PRINT_ORDER,

		PRINT_ALL_PAGES = 0,
		PRINT_ODD_PAGES = 1,
		PRINT_EVEN_PAGES = 2,

		PRINT_COLOR_MODE = 0,
		PRINT_MONOCHROME_MODE = 1,
	};
	// load settings from file
	void Load(const char* filename);
	// save settings to file (if force is true, write settings even there are no changes)
	void Save(const char* filename, bool force = false);
};

class FileAttributes {
	int32    page;
	float    left, top;
	BMessage bookmarks;

public:
	void SetPage(int32 page);
	int32 GetPage() const;
	void SetLeftTop(float left, float top);
	void GetLeftTop(float &left, float &top);
	void SetBookmarks(BMessage *bookmarks) { this->bookmarks = *bookmarks; }
	BMessage *GetBookmarks()               { return &bookmarks; }

	// read/write settings from/to attributes
	bool Read(entry_ref *ref, GlobalSettings *s);
	bool Write(entry_ref *ref, GlobalSettings *s);
};

#endif
