/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLayerPane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 040798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RLAYABLEPANE_
#define _H_RLAYABLEPANE_

#include "gMachine.h"
#include "RPane.h"
#include "gErrors.h"

//---------------------------------------------------------------------------

//********************
enum ELayerHConstraint
//********************
{
	kLayerHFull,
	kLayerHMinimize,
	kLayerHLeft,
	kLayerHCenter,
	kLayerHRight
};

//********************
enum ELayerVConstraint
//********************
{
	kLayerVFull,
	kLayerVMinimize,
	kLayerVTop,
	kLayerVCenter,
	kLayerVBottom
};


//---------------------------------------------------------------------------


//***************************************
class RLayerPane : virtual public RPane
//***************************************
{
public:
	RLayerPane(ELayerHConstraint hConstraint = kLayerHMinimize,
							 ELayerVConstraint vConstraint = kLayerVMinimize);
	virtual ~RLayerPane(void);
	
	// getIdealSize() request the pane to compute its own min/max limits...
	// getIdealSize returns if rbool maxIsInfinite (in which case maxRect should be ignored)
	// If the pane has only one size, and adjustSize() has been called before, the pane
	// may want to cache the bound and return it in minRect and return false.
	//
	// Porting Alert : you may want to fill mMinRect, mMaxRect, mHasIdealSize & mHasMaxRect

	virtual rbool getIdealSize(RRect &minRect, RRect &maxRect) = 0;

	// adjustSize is a callback used by layers to indicate to this pane that it
	// should adjust it's own size right now...

	virtual void adjustSize(void);

	void	setConstraint(ELayerHConstraint hConstraint, ELayerVConstraint vConstraint);
	void	getConstraint(ELayerHConstraint &hConstraint, ELayerVConstraint &vConstraint);

	// HOOK TBDL...	
	// void constraintChanged()

//----
protected:

	ELayerHConstraint mHConstraint;
	ELayerVConstraint mVConstraint;

	RRect mMinRect;
	RRect mMaxRect;
	rbool	mHasIdealSize;
	rbool	mHasMaxRect;

}; // end of class defs for RLayerPane


//---------------------------------------------------------------------------

#endif // of _H_RLAYABLEPANE_

// eoh
