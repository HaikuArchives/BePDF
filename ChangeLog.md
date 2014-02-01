BePDF ChangeLog
==========================
Complete version history for BePDF. Most of it comes from when BePDF was on SourceForge; the last SourceForge release was 1.1.1b5.

### BePDF 1.2.0 (unreleased master)
 - Various fixes to the docs.
 - Switch to using Make instead of Jam for building.
 - Removed the OptionalPackageDescription file.
 - Moved ChangeLog out of the docs and into its own file.
 - Use the RDEF by default instead of the RSRC.
 - Tweaks to the menus, about box & file info dialog.
 - Drop BeOS support from the code and the docs.
 - Modify start PDF to reflect Haiku being the only supported platform.
 - Update GhostScript fonts to 9.10 (was 8.14).
 - Move docs to a separate branch.
 - Large number of fixes to the build script.
 - Removed libsanta, liblayout, freetype, t1lib from the source tree, using packages now

### BePDF 1.1.1 Beta 5 - May 6, 2011
 - Updated/corrected Hungarian translation by Robert Dancso (dsjonny).

### BePDF 1.1.1 Beta 4 - May 6, 2011
 - Do not use hard coded path to system fonts directory.

### BePDF 1.1.1 Beta 3 - May 2nd, 2010
 - Disabled automatic file reload after file contents change. The mechanism to detect changes is broken in Haiku, as now notifications are also sent from BFS when attributes have changed.

### BePDF 1.1.1 Beta 1 - March 21, 2009
 - Documentation converted from GoBE Productive documents to HTML by Humdinger.
 - Español; Castellano documentation updated by Miguel Zúñiga González.
 - German documentation created by Humdinger.
 - Italian documentation created by Miguel Zúñiga González.
 - Vector version of the BePDF icon by Humdinger.

### BePDF 1.1.0 Beta 1 - October 23, 2008
 - Updated to xpdf 3.02 and Freetype 2.3.7.

### BePDF 1.0.0 Beta 2 - August 28, 2005
 - Bug fix: Find TrueType fonts in BePDFs fonts directory.

### BePDF 1.0.0 Beta 1 - August 26, 2005
 - New: Added support for additional font encodings and CID fonts. Included are Arabic, Cyrillic, Chinese simplified and traditional, Greek, Hebrew, Japanese, Korean, Latin2, Thai and Turkish encoding tables.
 - New: Case-sensitive search and search backwards.
 - New: New layout of print settings dialog. Enabled monochrome printing mode.
 - New: Added list of attachments. Saving selected attachment to file or multiple selected attachments into directory.
 - New: Saving copy of PDF file in separate thread.
 - New: Saving of File Attachment Annotations.
 - New: Use installed system fonts for 13 of base 14 fonts instead of URW fonts (to improve font rendering quality).
 - New: Upgraded to xpdf 3.01.
 - New: Use FreeType2 for rendering of Type 1 fonts instead of T1lib (to improve font rendering quality).
 - New: Upgrade to FreeType 2.1.10.
 - New: Removed alternative Ghostscript renderer. The rendering issues of previous versions should have been fixed by using xpdfs own PDF renderer.
 - Bug fix: Restore position of page list after starting of BePDF.
 - Bug fix: Use native renderer for annotations to avoid "ghost" annotations after they have been changed.
 - Bug fix: Crash during printing a page.
 - Bug fix: Tooltip should be displayed under Zeta.

### BePDF 1.0.0 Alpha 1 - July 25, 2005
 - Ported and integrated xpdf 3.0.
 - Included Base-14 fonts (URW fonts) from Ghostscript 8.14.

### BePDF 0.9.7 Beta 1 - March 20, 2005
 - New: Korean font support implemented by Park Sukyouk.

### BePDF 0.9.6 Beta 2 - July 30, 2004
 - Bug fixed: Automatically reload PDF file if file has changed did not work for the file opened at startup.

### BePDF 0.9.6 Beta 1 - June 29, 2004
 - New: Automatically reload PDF file if file has changed.
 - Bug fixed in outline view.
 - Updated for Ghostscript 8.14.
 - Updated to FreeType 2.1.9.

### BePDF 0.9.5 Beta 2 - March 2, 2003
 - Bug fix: Some images in the resources of the BePDF executable file were in a format that caused at least one user installed image translators to crash and when another image translator was installed the image could not be loaded from the resources which later cause BePDF to crash. Thanks to Dr. Hartmut Reh for helping to fix this bug.
 - Update: Uses FreeType 2.1.7.
 - Localization update: Dutch and Slovak.
 - Missing localization updates: Bulgarian, Czech, Finnish, French, Italian, Japanese, Korean, Norwegian, Polish, Russian, Swedish, Turkish.

### BePDF 0.9.5 Beta 1 - February 20, 2003
 - New: Added basic support for stroke/fill opacity (=transparency)
 - New: FreeText annotation is now fully supported. This allows to add text to a PDF file.
 - New: Allow to change the default values of annotations.
 - New: Replaced annotation menu with a vertical annotation tool bar.
 - New: Show file icon and PDF title if available in "Open" and "Open in Window" menu.
 - New: Bookmarks are displayed in specified style, color and open state (=child bookmarks are collapsed or expanded)
 - New: Clicking a bookmark respects the destination position on page.
 - Changed handling of links to PDF file:
   - Clicking a link, opens the file in the same window.
   - COMMAND key + clicking a link, opens the file in a new window.
   - Right clicking a link, opens a popup menu that allows to open the file in a new window.
   - Case insensitive search for PDF file.
 - New: History is not deleted when a new file is opened.
 - Bug fix: Printing annotations showed them in the state stored in the PDF file. Now the current state is printed.
 - Localization update: Brasilian Portuguese, Castellano, English, German, Hungarian, Traditional Chinese.

### BePDF 0.9.4 Beta 2 - February 1, 2003
 - User manual is now available in PDF only. It is accessible via the help menu in BePDF. A localized version of the documentation is opened if it exists. Default language is English. The main distribution package includes the English documentation only. Translations of the documentations are available separately and the .pdf file should be placed into the docs folder of the BePDF installation folder to be found from BePDF when requested to show the help file.
 - BiPolar made the translation of the documentation to Castellano (Spanish). Some bugs fixed: full screen mode, FreeType rendering.
 - Distributed as All-In-One package only. Self-contained, no libraries are added to system paths.
 - Removed online update feature.
 - Removed show online help from menu.
 - Added option to reverse vertical scrolling direction of document view when dragged with mouse.

### BePDF 0.9.4 Beta 1 - August 4, 2002
 - Added ability to Create and Edit PDF Annotations
 -  - Supported annotation types: Text, Square, Circle, Highlight, Underline, Squiggly, Strike-out, and Pop-up
 -  - Unsupported annotation types: Link, FreeText, Line, Stamp, Ink, FileAttachement, Sound Movie, Widget, PrinterMark, TrapNet
   - Limitations:
     - PDFfiles must not be encrypted
     - Pages must not be rotated
     - Must be in Native Rendering Mode
 - Added "Save File As..." dialog to retain PDF annotations
 - Implemented a "Close" path work-around

### BePDF 0.9.3 - May 12, 2002

 - Added document loading status bar
 - Added support for document attributes to be viewable and editable from Tracker windows
 - Fixed bug that caused program crashes on exit
 - Implemented a workaround for a bug in StrokeShape
 - Postponed certain commands that would block updating of the display, until page rendering has finished

### BePDF 0.9.2 - March 11, 2002
 - Fixed bug where rendering certain images caused BePDF to crash
 - Fixed bug where displaying the bookmarks view in Dano caused BePDF to crash.
 - There are still other problems with Dano that have not yet been addressed.
 - Officially, however, BePDF does not support Dano
 - Fixed bug where sporadically embedded fonts could not be loaded, and text was then displayed in one of the default BeOS fonts.
 - Entered passwords are now hidden

### BePDF 0.9.1 - January 28, 2002
 - Added support for annotations and associated text
 - Added mouse scroll wheel zoom feature
 - Added a window for displaying error messages and warnings
 - Incorporated several bug fixes

### BePDF 0.9 - November 25, 2001
 - Updated back-end to xpdf 0.93
 - Implemented PDF 1.4 (128-bit) decryption
 - Allows acceptance and verification of the document owner password; if the correct password is entered, BePDF makes all document actions available
 - BePDF now respects security settings for text/graphics copying and printing
 - BePDF allows user defined bookmarks (to a maximum of one per page) stored in file attributes, not in PDF file
 - Incorporated several bug fixes
 - Reformatted and extended documentation by Celerick Stephens
 - Added links to BePDF related sites to help menu
 - Separated packages for external libraries (libxpdf, libfreetype) to reduce the size of the BePDF distribution package

### BePDF 0.7.11 - September 9, 2001
 - Added options for FreeType 2 renderer
 - Implemented image memory size protection
 - Made handling of images/image mask similar to handling performed by xpdf
 - Added "Open in Workspace" option to preferences window
 - Fixed interruption of rendering process.

### BePDF 0.7.10 Alpha 6 - August 14, 2001
 - Replaced development version of FreeType 2 library with latest stable version 2.0.4. The development version caused a crash when rendering a page (thanks to Fabrice Guix for reporting the bug)

### BePDF 0.7.10 Alpha 5 - August 13, 2001
 - Vertical text positioning bug fixed by NAITHO Jun
 - Limited text selection rectangle to page
 - Fixed buffer overrun bug in printing progress window (BePDF crashed e.g. when Japanese language is selected). Thanks to Eberhard Hafermalz for reporting the bug

### BePDF 0.7.10 Alpha 4 - July 19, 2001
 - Added ability to reload the current file
 - Enter key now has the same function as spacebar
 - Changed behavior of tooltips

### BePDF 0.7.10 Alpha 3 Localization Update 1 - July 2, 2001
 - The following languages have been updated:
 - Finnish, French, Italian, Japanese, Slovak, Traditional Chinese
 - These languages were previously current:
 - English, German
 - These languages have not yet been updated:
 - Hungarian, Bulgarian, Spanish, Polish, Brazilian Portuguese, Russian, Swedish, Czechoslovakian

### BePDF 0.7.10 Alpha 3 - June 24, 2001
 - Ghostscript fonts can now be used for PDF base 14 fonts
 - Added option for gray scale printing
 - Fixed bug affecting handling of password protected PDF files
 - Corrected display of password protected files with Ghostscript renderer enabled

### BePDF 0.7.10 Alpha 2 - June 12, 2001
 - Improved speed up to 43% for rendering embedded fonts
 - Added sub-pixel precise positioning (2x2) of glyphs of embedded fonts
 - Disabled auto-hinter of FreeType 2 renderer
 - Fixed bug in Type 1 character code mapping
 - Implemented many other miscellaneous bug fixes

### BePDF 0.7.10 Alpha 1 - June 7, 2001
 - FreeType 2 is now used to render embedded Type 1 and TrueType fonts. This replaces the FreeType 1.x and Type 1 renderer
 - The decoding tables for Japanese, Traditional Chinese, and Simplified Chinese are moved to add-ons to reduce the size of the executable
 - Fixed bug to allow searching for Japanese or Chinese text all pages instead of only on the current displayed page

### BePDF 0.7.9 Alpha 3 - May 31, 2001
 - Updated Japanese font support
 - Added minor modifications to the Graphical User Interface

### BePDF 0.7.9 Alpha 2 - May 29, 2001
 - Added Chinese font support (traditional and simplified)
 - Added customization option for Japanese, Traditional Chinese, and Simplified Chinese fonts
 - Made widths of page list and bookmarks side bars independent

### BePDF 0.7.8 Alpha 5 - May 22, 2001
 - Chinese by Chen Yi-feng
 - Fixed calculation of link rectangles

### BePDF 0.7.8 Alpha 4 Localization Update 1 - May 21, 2001
 - Added Slovak localization by Martin Paucula
 - Added Bulgarian localization by Philip Petev (MadMax)

### BePDF 0.7.8 Alpha 4 - May 9, 2001
 - Added options to preferences window to set fullscreen mode or quasi-fullscreen mode and to show selection rectangle as filled rectangle or as stroked rectangle.
 - Added shell scripts to add/remove a replicant to/from the deskbar
 - Added filter to file open window to show only PDF files
 - Added possibility to close sub windows using the escape key
 - Right clicking on link now allows copying the link text to the clipboard
 - Clicking on a link to another PDF file now opens that file in a new window
 - Status bar now shows more meaningful information for internal links
 - Disabled menu items and buttons in main window when they have no effect
 - Changed and added some keyboard shortcuts (fullscreen, fit to width, fit to page)
 - Bookmarks can now be displayed in place of page list

### BePDF 0.7.7 Alpha 10 - March 21, 2001
 - Added new translations of user interface strings to Brazilian Portuguese by Bruno G. Albuquerque
 - Disabled usage of SpLocale

### BePDF 0.7.7 Alpha 9 - March 1, 2001
 - Added new translations of user interface strings to Swedish by Daniel Wesslaen
 - Location and orientation of Deskbar is accounted for during zooming
 - Completed SpLocale integration. SpLocale is used when HAVE_SPLOCALE macro is defined at the time of program compiling

### BePDF 0.7.7 Alpha 8 - February 26, 2001
 - Added new translations of user interface strings to Russian by Oleg V. Kourapov
 - Fixed a bug which crashed the PPC version (thanks to Michael Pieper for submitting a bug report and testing)

### BePDF 0.7.7 Alpha 7 - February 20, 2001
 - New translations for Czechoslovakian by Jan Polzer
 - Fixed a number of major bugs

### BePDF 0.7.7 Alpha 6 - February 16, 2001
 - Updated Localization
 - Added minor bug fixes

### BePDF 0.7.7 Alpha 5 - February 14, 2001
 - Added dynamic loading of Ghostscript library if available. This should also work with the PPC version of Ghostscript when it is ported
 - Added minor bug fixes

### BePDF 0.7.7 Alpha 4 - February 6, 2001
 - Added quasi fullscreen/window mode and show/hide page list (the current implementation of quasi fullscreen mode does not allow opening of other windows in BePDF, e.g. opening a file, searching, etc. does not work; this is to be resolved in a later version).

### BePDF 0.7.7 Alpha 3 - January 28, 2001
 - Added new translations for Spanish by Carlos Hasan
 - Added new translations for Finnish by Jaakko Leikas
 - Added UI improvements; more buttons in toolbar
 - Bug fixed (opening a defect PDF file crashed BePDF)
 - BePDF now requires Santa's Gift Bag (included)
 - Features a 'Get BePDF' button for the web by Kevin H. Patterson

### BePDF 0.7.7 Alpha 2 - January 16, 2001
 - Updated backend to XPDF 0.92
 - Updated localization

### BePDF 0.7.7 Alpha 1 - January 9, 2001
 - Added new toolbar button bitmaps by Adam Rosser
 - Added PDF doc encoding
 - Fixed some memory leak bugs (in Bookmarks window and Preferences window)
 - Added page list which shows page labels of PDF 1.3 files, if available
 - Reorganized preferences window and added some settings
 - Added 'Fonts' tab to file info window, which shows the fonts in the PDF file
 - Help documentation is now opened with the preferred application for HTML
 - (Experimental) BePDF now uses Ghostscript for page rendering. The following items do not work when the Ghostscript renderer is enabled:
   - Printing (uses always the native renderer)
   - Rotation
   - Selecting & copying text (graphics works)
   - Link position (position is sometimes incorrect)
   - Searching (starts at the next page, not at the current page, and does not display the found text

### BePDF 0.7.6 Alpha 3 - November 25, 2000
 - Fixed a bug which caused BePDF to crash

### BePDF 0.7.6 Alpha 2 - November 25, 2000
 - Added localization. Translations for Japanese, Italian and German are included in this distribution
 - Added Drag and Drop support for copying text or images inside the selection to another application (including Tracker); works with Japanese text also
 - Added zoom options: Fit to Page Width, Fit to Page, Zoom In and Zoom Out
 - The contents of the File Info and Bookmarks window are updated when a new document is opened
 - Color space for bitmap is now customizable
 - Update frequency is now customizable
 - Page number, zoom factor, rotation, window position, and window size are now stored in the file attributes of the PDF document
 - Line mode for drawing lines is now used when printing
 - Fixed some bugs

### BePDF 0.7.6 Alpha 1 - November 13, 2000
 - Fixed two bugs which crashed BePDF (switching between workspaces; opening a document which has no info dictionary)
 - Added tooltips to buttons in toolbar
 - Added a bookmarks window (this is experimental; the contents of the window are not updated when a new document is loaded; the window has to be closed and reopened)

### BePDF 0.7.5 - November 4, 2000
 - Reduced amount of memory used while printing (up to 400%)
 - Moved printer and display settings in to windows
 - Can display password protected PDF files (not tested)
 - UTF8 to ASCII conversion is done when necessary and vice versa (copied text; search text; password)
 - Application resources used for cursors and bitmaps
 - Added bitmap buttons to toolbar
 - Added file info window

### BePDF 0.7.4 - September 30, 2000
 - Fixed a bug which crashed BePDF after opening a file
 - Added go to next/previous page with mouse wheel
 - Added zoom to selection with third mouse button
 - Window size is now independent of page size
 - Type 1 and FreeType font renderers can now be disabled
 - Anti-aliasing is now disabled when printing
 - Page history stores zoom factor, position, and rotation

### BePDF 0.7.3 - September 25, 2000
 - Added printer settings (print odd/even pages, normal/reverse order)
 - Added a page rendering thread
 - A page is now rendered in its own thread and the rendering process can now be interrupted; this also allows faster navigation between pages

### BePDF 0.7.2 - September 21, 2000
 - Type 3 font renderer can now be enabled at runtime (Warning: Type 3 font renderer is very slow!)
 - Fixed some minor bugs (window size, file name in window title)

### BePDF 0.7.1 - September 19, 2000
 - Important menu settings and window position/size are now persistent.
 - BePDF now remembers path of last opened file in the file open dialog
 - Added encoding table for ZapfDingbats font
 - Fixed bug in true type font renderer
 - Added 'Find Next' menu item
 - Added Keyboard Shortcuts menu item

### BePDF 0.7 - September 3, 2000
 - Renamed PDFViewer 0.2.2 to BePDF 0.7
 - Added TrueType font support for embedded fonts for X86 only (uses FreeType 1.3.1; PPC Crosscompiler generates error messages when compiling FreeType, because of the old parameter declaration style used by the FreeType library)
 - Added menu item to change the DPI for printing
 - Fixed bug when printing with BinkJet driver
 - Separated BePDF source files from the required libraries

### PDFViewer 0.2.1 - August 27, 2000
 - Added Type 1 font support for embedded fonts (uses t1lib 1.0.1)
 - Fixed some minor bugs

### PDFViewer 0.2 - August 22, 2000
 - Japanese font support added by NAITOH Jun
 - Included first PPC release
 - Switched from xpdf 0.9 to xpdf 0.91
 - Added real time dragging of the view
 - Added searching for text
 - Added text copy
 - Fixed another image bug--hope this is the last :)

### PDFViewer 0.1.5 - August 15, 2000
 - Fixed masked image bug

### PDFViewer 0.1.4 - August 13, 2000
 - Added printing support
 - Added page rotation
 - Fixed image bug (image was not rotated)

### PDFViewer 0.1.3 - August 11, 2000
 - Added character decoding for Symbol font
 - Improved character decoding
 - Added an open dialog box
 - Added more navigation buttons
 - Added drag and drop support to open a document in the current window

### PDFViewer 0.1.2 - July 30, 2000
 - Added handling of document internal links and URL links

### PDFViewer 0.1.1 - July 30, 2000
 - Fixed image color bug

### PDFViewer 0.1.0 - July 29, 2000
 - First release of PDFViewer which is based on BePDF 0.6
 - Fixed scrollbar bug
 - Added navigation with the keyboard
 - Added moving the view with the mouse
 - Added code for displaying masked images
 - Improved code for drawing images
 - Some special characters are now displayed
 - Fixed bug in stroke function (line width was not set correctly)
 - Added a status window
