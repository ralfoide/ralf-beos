/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RVertLayout.cpp
	Partie	: GUI kit

	Auteur	: RM
	Date		: 090798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"
#include "RVertLayout.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RVertLayout::RVertLayout(EVertLayoutAlign align, rbool hAdjust)
//***************************************************************************
{
	mAlign = align;
	mHAdjust = hAdjust;

	mHBorder = K_LAYOUT_DEFAULT_BORDER;
	mVBorder = K_LAYOUT_DEFAULT_BORDER;
} // end of constructor for RVertLayout


//***************************************************************************
RVertLayout::~RVertLayout(void)
//***************************************************************************
{
} // end of destructor for RVertLayout


//---------------------------------------------------------------------------


//***************************************************************************
void RVertLayout::setBorder(si32 hBorder, si32 vBorder)
//***************************************************************************
{
	mHBorder = hBorder;
	mVBorder = vBorder;
} // end of setBorder for RVertLayout


//***************************************************************************
void RVertLayout::adjustPane(RPane &parent)
//***************************************************************************
{
si32 i,n;
si32 height = 0;
si32 height_max = 0;
si32 width_max = 0;
si32 current_v, current_h;
si32 position_v, position_h;
si32 size_v, size_h;
RRect minRect;
RRect maxRect;
RRect r;

	// get parent pane info...
	r = parent.bounds();
	
	current_v = r.height();
	current_h = r.width();
	n = parent.countPane();

	// ask every pane its preferred size
	for(i=0; i<n; i++)
	{
		RLayerPane *pane = M_CAST_AS(parent.paneAt(i), RLayerPane);
		if (!pane) continue;
		
		pane->getIdealSize(minRect, maxRect);
		pane->adjustSize();

		si32 h = minRect.height();
		if (h > height_max) height_max = h;
		height += h + mVBorder;

		width_max = max(minRect.width(), width_max);
	}

	if (i > 0) height -= mVBorder;

	// reposition panes...
	if (mHAdjust) current_h = width_max;
	else if (current_h > 2*mHBorder) current_h -= 2*mHBorder;

	if (mAlign == kVertLayoutAlignCenter)
		position_v = (current_v - height)/2;
	else if (mAlign == kVertLayoutAlignBottom)
		position_v = current_v - height - mVBorder;
	else
		position_v = mVBorder;

	for(i=0; i<n; i++)
	{
		RLayerPane *pane = M_CAST_AS(parent.paneAt(i), RLayerPane);
		if (!pane) continue;

		ELayerHConstraint hConstraint;
		ELayerVConstraint vConstraint;
		pane->getConstraint(hConstraint, vConstraint);
		
		if (		mAlign != kVertLayoutAlignEqual
				|| (hConstraint != kLayerHFull && hConstraint != kLayerHLeft))
			pane->getIdealSize(minRect, maxRect);
		
		position_h = mHBorder;

		r = parent.bounds();
		size_v = r.height();
		size_h = current_h;

		// horizontal size/position adjustment
		if (hConstraint == kLayerHMinimize)
			size_h = minRect.width();
		else if (hConstraint == kLayerHCenter)
		{
			size_h = minRect.width();
			position_h = mHBorder+(current_h - size_h)/2;
		}
		else if (hConstraint == kLayerHRight)
		{
			size_h = minRect.width();
			position_h = mHBorder + current_h - size_h;
		}

		// vertical size/position adjustement
		if (mAlign == kVertLayoutAlignEqual) size_v = height_max;
		
		// adjust size & position
		pane->moveTo(position_h, position_v);
		pane->resizeTo(size_h, size_v);

		position_v += size_v+mVBorder;
	}

	// adjust parent as necessary

	if (mHAdjust)
	{
		r = parent.bounds();
		parent.resizeTo(r.width(), position_v);
	}
	else parent.resizeTo(current_h+2*mHBorder, position_v);

} // end of adjustPane for RVertLayout



//---------------------------------------------------------------------------

// eoc
