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

#include "CachedPage.h"

/////////////////////////////////////////////////////////////////////////
CachedPage::CachedPage() :
	mState(EMPTY),
	mBitmap(NULL),
	mText(NULL),
	mLinks(NULL),
	mAnnotations(NULL) {
}

CachedPage::~CachedPage() {
	delete mBitmap; delete mText; delete mLinks;
}

/////////////////////////////////////////////////////////////////////////
void CachedPage::InitCTM(OutputDev *outputDev) {
	for (int i = 0; i < 6; i++) mCtm[i] = outputDev->getDefCTM()[i];
	for (int i = 0; i < 6; i++) mIctm[i] = outputDev->getDefICTM()[i];
}

void CachedPage::CvtDevToUser(int dx, int dy, double *ux, double *uy) {
  *ux = mIctm[0] * dx + mIctm[2] * dy + mIctm[4];
  *uy = mIctm[1] * dx + mIctm[3] * dy + mIctm[5];
}

void CachedPage::CvtUserToDev(double ux, double uy, int *dx, int *dy) {
  *dx = (int)(mCtm[0] * ux + mCtm[2] * uy + mCtm[4] + 0.5);
  *dy = (int)(mCtm[1] * ux + mCtm[3] * uy + mCtm[5] + 0.5);
}

/////////////////////////////////////////////////////////////////////////
void CachedPage::SetLinks(Links *links) {
	// ASSERT(mLinks == NULL);
	mLinks = links;
}

LinkAction *CachedPage::FindLink(double x, double y) {
	if (mLinks) {
		return mLinks->find(x, y);
	} else {
		return NULL;
	}
}

GBool CachedPage::OnLink(double x, double y) {
	if (mLinks) {
		return mLinks->onLink(x, y);
	} else {
		return false;
	}
}
/////////////////////////////////////////////////////////////////////////
#define xoutRound(x) ((int)(x + 0.5))

void CachedPage::SetText(TextPage *text) {
	// ASSERT(mText == NULL);
	mText = text;
}

GBool CachedPage::FindText(Unicode *s, int len,
		 GBool startAtTop, GBool stopAtBottom,
		 GBool startAtLast, GBool stopAtLast,
		 GBool caseSensitive, GBool backward,
		double *xMin, double *yMin, double *xMax, double *yMax) {
	if (mText && mText->findText(s, len, startAtTop, stopAtBottom,
		startAtLast, stopAtLast,
		caseSensitive, backward, false, // wordwise -- TODO/FIXME
		xMin, yMin, xMax, yMax)) {
		return gTrue;
	}
	return gFalse;
}

GString *CachedPage::GetText(int xMin, int yMin, int xMax, int yMax) {
	if (mText) {
		return mText->getText((double)xMin, (double)yMin,
							   (double)xMax, (double)yMax);
	} else {
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////
void CachedPage::SetBitmap(BBitmap *bitmap, int32 width, int32 height) {
	mBitmap = bitmap;
	mWidth = width; mHeight = height;
}

void CachedPage::MakeEmpty() {
	delete mLinks; mLinks = NULL;
	delete mText; mText = NULL;
	// don't delete mBitmap
}
