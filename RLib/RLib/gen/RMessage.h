/*****************************************************************************

	Projet	: RLib

	Fichier	: RMessage.h
	Partie	: Portable Lib

	Auteur	: RM
	Date	: 032900 (rebuild msgstruct)
	Date	: 181198 (initial)
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RMESSAGE_
#define _H_RMESSAGE_

#include "RIFlat.h"

//------------------------------------------------------------

/**	The base interface for TMessage, a message wrapping a structure.
	The message derives from a given class or structure and
	can be flattened and unflattened in order to be transmited
	since the message knows its own size.
*/

//****************************
class RMessage : public RIFlat
//****************************
{
public:
				RMessage(ui32 w=0)	{ _mWhat = w; _mSize = sizeof(RMessage) };

				// Inside members

inline	ui32	What(void) { return _mWhat; }
inline	ui32	Size(void) { return _mSize; }

				// (un)flatten the message (inherited from RIFlat)

virtual inline RFlatBlock*	Flatten(void);
virtual inline void			Unflatten(RFlatBlock *data);

protected:

		ui32	_mWhat;
		ui32	_mSize;

};	// end of RMessage



/**	A message containing a structure.
	Actual messages are based on the TMessage template.
*/

//****************************************************
template<class T> TMessage : public T, public RMessage
//****************************************************
{
public:

	TMessage(ui32 w=0) : RMessage(w) { _mSize = sizeof(TMessage<T>) };

};	// end of TMessage





//------------------------------------------------------------
// inline implementation

//****************************************
inline RFlatBlock* RMessage::Flatten(void)
//****************************************
{
	RFlatBlock *block = new RFlatBlock(_mWhat);
	M_ASSERT_PTR(block);

	block->AddRaw(this, sizeof(_mSize));

	return block;

} // end of Flatten for RMessage


//***********************************************
inline void RMessage::Unflatten(RFlatBlock *data)
//***********************************************
{
	M_ASSERT_PTR(data);

	vptr ptr = NULL;
	ui32 size = data->GetRaw(ptr);

	M_ASSERT(!size || size == sizeof(*this));

	if (size == sizeof(*this) && ptr) memcpy(this, ptr, size);

} // end of Unflatten for RMessage


//------------------------------------------------------------

#endif // of _H_RMESSAGE_

// eof
