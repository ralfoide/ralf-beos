/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CInputMethod.h
	Partie	: Misc

	Auteur	: RM
	Date		: 180797
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CINPUTMETHOD_
#define _H_CINPUTMETHOD_

//--------------------------------------------------------

class CTrack;

//--------------------------------------------------------

//****************
class CInputMethod
//****************
// this is the base class for input methods
// yet to be defined
{
public:
	CInputMethod(void);
	virtual ~CInputMethod(void);

	virtual void nextConfig(BMessage *data, CTrack *track);
	virtual void start(BMessage *data) = 0;
	virtual void stop(void) = 0;

protected:
	BMessage	*mCurrentData;
	CTrack		*mCurrentTrack;

}; // end of class def CInputMethod


//--------------------------------------------------------

class CAudioCD;

//**********************************
class CInputCD : public CInputMethod
//**********************************
{
public:
	CInputCD(CAudioCD *cdinterface);
	~CInputCD(void);

	virtual void nextConfig(BMessage *data, CTrack *track);
	virtual void start(BMessage *data);
	virtual void stop(void);

private:

}; // end of CInputCD


//--------------------------------------------------------
#endif // of _H_CINPUTMETHOD_
// eoh
