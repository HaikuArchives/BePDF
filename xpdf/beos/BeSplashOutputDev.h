//========================================================================
//
// BeSplashOutputDev.h
//
// Based on XSplashOutputDev copyright 2003 Glyph & Cog, LLC
// Copyright 2004-2005 Michael W. Pfeiffer
//
//========================================================================

#ifndef BESPLASHOUTPUTDEV_H
#define BESPLASHOUTPUTDEV_H

#include <View.h>
#include <Bitmap.h>
#include "aconf.h"
#include "SplashOutputDev.h"
#include "SplashTypes.h"
#include "TextOutputDev.h"


//------------------------------------------------------------------------
// BeSplashOutputDev
//------------------------------------------------------------------------

typedef void (rgb24_to_color_space)(uchar r, uchar g, uchar b, uchar* dest);
typedef void (gray8_to_color_space)(uchar gray, uchar* dest);

typedef void (*BeSplashOutRedrawCbk)(void *data, int left, int top, int right, int bottom,
	bool composited);

class BeSplashOutputDev: public SplashOutputDev {
public:

  enum ColorMode {
  	kColorMode,
  	kGrayScaleMode
  };

  BeSplashOutputDev(GBool reverseVideoA, SplashColor paperColorA,
		   GBool incrementalUpdateA,
		   BeSplashOutRedrawCbk redrawCbkA,
		   void *redrawCbkDataA,
		   ColorMode colorMode = kColorMode); 

  virtual ~BeSplashOutputDev();

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  // Dump page contents to display.
  virtual void dump();

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- text drawing
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);

  //----- special access

  // Clear out the document (used when displaying an empty window).
  void clear();

  // Copy the rectangle (srcX, srcY, width, height) to (destX, destY)
  // in BView.
  void redraw(int srcX, int srcY,
	      BView *view,
	      int destX, int destY,
	      int width, int height,
	      bool composited);

  // Find a string.  If <startAtTop> is true, starts looking at the
  // top of the page; else if <startAtLast> is true, starts looking
  // immediately after the last find result; else starts looking at
  // <xMin>,<yMin>.  If <stopAtBottom> is true, stops looking at the
  // bottom of the page; else if <stopAtLast> is true, stops looking
  // just before the last find result; else stops looking at
  // <xMax>,<yMax>.
  GBool findText(Unicode *s, int len,
		 GBool startAtTop, GBool stopAtBottom,
		 GBool startAtLast, GBool stopAtLast,
		 GBool caseSensitive, GBool backward,
		 double *xMin, double *yMin,
		 double *xMax, double *yMax);

  // Get the text which is inside the specified rectangle.
  GString *getText(double xMin, double yMin, double xMax, double yMax);

  // Returns the text page and sets member variable to NULL.
  TextPage *acquireText();

private:
  color_space getColorSpace();
  bool viewsSupportDrawBitmap(color_space cs);
  rgb24_to_color_space* getRGB24ToColorSpace(color_space cs);
  gray8_to_color_space* getGray8ToColorSpace(color_space cs);
  void blend24(uchar r, uchar g, uchar b, uchar alpha, uchar* dest);

  ColorMode fColorMode;
  GBool fIncrementalUpdate;      // incrementally update the display?
  BeSplashOutRedrawCbk fRedrawCallback;
  void *fRedrawCallbackData;
  TextPage *fText;               // text from the current page
};

#endif
