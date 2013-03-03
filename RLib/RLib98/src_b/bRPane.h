/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRPane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 270698
	Format	: tabs==2

	Leaf panes that reimplemented existing Be widgets should multiple inherit
	from the base class RPane and from their Be counterpart.

*****************************************************************************/

#ifndef _H_BRPANE_
#define _H_BRPANE_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "TList.h"
#include "RRect.h"
#include "RPoint.h"
#include "gMacros.h"
#include "gErrors.h"

#include <Window.h>
#include <View.h>

//---------------------------------------------------------------------------

class RPane;
class RPaneWindow;
typedef TList<RPane *> RPaneList;

//---------------------------------------------------------------------------
// Interface for embedding Panes into Panes
//
// (for an explanation on using interfaces in C++ :
//  Dr. Dobbs #288 August 98, "C++ Interaces, page 18).

//******************
class RPaneInterface
//******************
{
public:
	// add a sub-pane to this parent
	virtual void		addPane(RPane *p) = 0;
	
	// remove a sub-pane from this parent (and delete the instance of the sub pane)
	virtual void		removePane(RPane *p) = 0;

	// return the nth sub-pane of this parent
	virtual RPane*	paneAt(si32 index) = 0;

	// count number of sub-panes, 0 if none.
	virtual si32		countPane(void) = 0;

	// callback suited to notify that the pane has been attached or detached
	virtual void		attached(rbool _attached)	= 0;
};


//---------------------------------------------------------------------------


//************************************************************
class RPane : public RPaneInterface, virtual public RInterface
//************************************************************
{
public:
	RPane(void);
	virtual ~RPane(void);

	virtual RRect bounds(void) = 0;
	virtual RRect frame(void)	 = 0;

	virtual void	moveTo(si32 x, si32 y)		= 0;
	virtual void	moveTo(RPoint p)					= 0;
	virtual void	resizeTo(si32 x, si32 y)	= 0;
	virtual void	resizeTo(RPoint p)				= 0;

	virtual void	update(void)	{ /* nop */ }

	// the five next methods implements the RPaneInterface
	virtual void		addPane(RPane *p);
	virtual void		removePane(RPane *p);
	virtual RPane*	paneAt(si32 index)				{ return mPaneList.itemAt(index); }
	virtual si32		countPane(void)						{ return mPaneList.size();				}
	virtual void		attached(rbool _attached)	{ /* nop */ }

	// services...

	RPane				*parent(void)	{ return mParent; }
	RPaneWindow	*window(void)	{ return mWindow; }

	// more to come :
	// detach - attach - isAttached
	// window
	// show - hide - isVisible
	// callbacks : moved - resized
	// keep cached bounds rectangle -- used for update/Draw

//----
protected:

	RPaneWindow	*mWindow;
	RPane				*mParent;
	RPaneList	 	mPaneList;

	friend class RPaneWindow;

}; // end of class defs for RPane


//---------------------------------------------------------------------------
// Glue between BView-derived and pane

#define M_WIN_LOCK() 		if (Window() && Window()->Lock())
#define M_WIN_UNLOCK()	Window()->Unlock()

#define M_GLUE_PANE_TO_VIEW(this_class, parent_class)						\
	RRect bounds(void)							{ M_WIN_LOCK() { BRect r=Bounds();	M_WIN_UNLOCK(); return RRect(r); } else M_THROW(kErrNotAttached); return RRect(); } \
	RRect frame(void)								{ M_WIN_LOCK() { BRect r=Frame(); 	M_WIN_UNLOCK(); return RRect(r); } else M_THROW(kErrNotAttached); return RRect(); } \
	void	moveTo(si32 x, si32 y)		{ M_WIN_LOCK() { MoveTo(x,y); 			M_WIN_UNLOCK(); } }	\
	void	moveTo(RPoint p)					{ M_WIN_LOCK() { MoveTo(p.x, p.y);	M_WIN_UNLOCK(); } }	\
	void	resizeTo(si32 x, si32 y)	{ M_WIN_LOCK() { ResizeTo(x,y); 		M_WIN_UNLOCK(); } }	\
	void	resizeTo(RPoint p)				{ M_WIN_LOCK() { ResizeTo(p.x,p.y);	M_WIN_UNLOCK(); } }	\
	void	update(void)							{ M_WIN_LOCK() { Draw(Bounds()); 		M_WIN_UNLOCK(); } else M_THROW(kErrNotAttached); }	\
	void	addPane(RPane *p)					{ this_class *p2 = M_CAST_AS(p, this_class); if (p2) { M_WIN_LOCK() { AddChild(p2); 		M_WIN_UNLOCK(); }} parent_class::addPane(p);		} \
	void	removePane(RPane *p)			{ this_class *p2 = M_CAST_AS(p, this_class); if (p2) { M_WIN_LOCK() { RemoveChild(p2);	M_WIN_UNLOCK(); }} parent_class::removePane(p);	}



//---------------------------------------------------------------------------
// RPaneView is the default implementation for BView objects


//**********************************************************
class RPaneView : virtual public RPane, virtual public BView
//**********************************************************
{
public:
	RPaneView(RRect frame, sptr name, bool follow_all = false)
		: RPane(),
		  BView(frame, (const char *)name,
		  			(follow_all ? B_FOLLOW_ALL : B_FOLLOW_NONE),
		  			B_NAVIGABLE|B_WILL_DRAW)
		{ SetViewColor(140,140,140); }
	virtual ~RPaneView(void) { }

	M_GLUE_PANE_TO_VIEW(RPaneView, RPane);

}; // end of class defs for RPaneView


//---------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRPANE_

// eoh
