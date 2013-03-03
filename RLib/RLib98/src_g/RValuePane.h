/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RValuePane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 270698
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RVALUABLEPANE_
#define _H_RVALUABLEPANE_

#include "gMachine.h"
#include "RPane.h"
#include "TAssoc.h"
#include "gErrors.h"

//---------------------------------------------------------------------------
//
// Should become :
// - EValType
// - RValue
// - RValue::RValue
// - RValueInterface (assoc, set/get, etc.)
// - RValuePane::virtual RPane, RValueInterface


//***************
enum EValType
//***************
{
	EValVoid,
	EValString,
	EValInt,
	EValFloat
};

//******************************************
class RValue : virtual public RInterface
//******************************************
{
public:
	RValue(void)				{ mType = EValVoid; mP = NULL; }
	RValue(EValType t)	{ mType = t; if (t == EValFloat) mF = 0.0; else mP = NULL; }
	~RValue(void)				{ }

	inline RValue(const RValue &v)						{ mType = v.mType; if (mType == EValFloat) mF = v.mF; else mP = v.mP; }
	inline RValue	&operator=(const RValue &v)	{ mType = v.mType; if (mType == EValFloat) mF = v.mF; else mP = v.mP; return *this; }
	
	inline void	setVoid  (vptr p) { mType = EValVoid;   mP = p; }
	inline void	setString(sptr s) { mType = EValString; mS = s; }
	inline void	setInt   (si32 i) { mType = EValInt;    mI = i; }
	inline void	setFloat (fp64 f) { mType = EValFloat;  mF = f; }

	inline vptr getVoid  (void)		{ if (mType != EValVoid  ) M_THROW(kErrClassMismatch); return mP; }
	inline sptr getstring(void)		{ if (mType != EValString) M_THROW(kErrClassMismatch); return mS; }
	inline si32 getInt   (void)		{ if (mType != EValInt   ) M_THROW(kErrClassMismatch); return mI; }
	inline fp64 getFloat (void)		{ if (mType != EValFloat ) M_THROW(kErrClassMismatch); return mF; }

protected:

	EValType	mType;

	union 
	{
		vptr	mP;
		sptr	mS;
		si32	mI;
		fp64	mF;
	};

};


//---------------------------------------------------------------------------
// associations lists for Panes
// key is of type RPaneKey [ui32 'code' for Be, maybe char[4] "code" for windoze]
// value is of type RValue

typedef ui32 RPaneKey;
typedef TAssoc<RPaneKey,RValue> RPaneAssoc;

#define K_NO_PANEKEY (RPaneKey)0

//---------------------------------------------------------------------------

//*************************************
class RValuePane : virtual public RPane
//*************************************
{
public:
	RValuePane(RPaneKey key = K_NO_PANEKEY);
	virtual ~RValuePane(void);

	virtual void setValue(RValue &v);
	virtual RValue getValue(void);
	virtual void getValue(RValue &v);

	virtual void setAssoc(RPaneAssoc *a);
	RPaneAssoc *getAssoc(void) { return mAssoc; }

	RPaneKey	getKey(void) { return mKey; }

	// inherited from RPane
	void attached(rbool _attached);

//----
protected:

	void removeAssoc(void);

	RPaneAssoc	*mAssoc;
	RPaneKey		mKey;

}; // end of class defs for RLayerPane


//---------------------------------------------------------------------------

#endif // of _H_RVALUABLEPANE_

// eoh
