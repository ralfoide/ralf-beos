/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RFlatBlock.h
	Partie	: Storage

	Auteur	: RM
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_CFLATBLOCK_
#define _H_CFLATBLOCK_

#include "RString.h"
#include "RDate.h"

//------------------------------------------------------------

#define K_FLATBLOCK_GRANULARITY  256
#define K_FLATBLOCK_HEAD_SIZE    12
#define K_FLATBLOCK_EXCPT_EOF    kErrFlatIndexRange

//------------------------------------------------------------

class RIFlat;

//------------------------------------------------------------

/**	A flat blocks, used to encode a given structure in a piece of bytes
	that can be stored or transmitted and later unflattened.
	@see	RIFlat
	@see	TMessage
*/

//**************
class RFlatBlock
//**************
{
public:
		// create empty to be filled by hand
		RFlatBlock(void);

		// create non-empty to be filled by hand
		RFlatBlock(ui32 type, ui32 revision = 0);
 
		// recreate from existing data to be read
		RFlatBlock(void *data);

virtual	~RFlatBlock(void);

		// change content (empty or not with new data) to be read
		void	SetData(void *data);

const	ui8 *	Data(void)		{ setSizeRevType(); return mData;		}
		ui32	DataSize(void)	{ return mDataSize;						}
		void	MakeEmpty(void)	{ erase(); setSizeRevType(); Rewind();	}

		void	Rewind(void)	{ mReadIndex = K_FLATBLOCK_HEAD_SIZE;	}
		rbool	Eof(void)		{ return mReadIndex >= mDataSize;		}

		void	SetRevision(ui32 rev) { mRevision = rev;  }
		void	SetType    (ui32 typ) { mType = typ;      }
		ui32	Revision   (void)     { return mRevision; }
		ui32	Type       (void)     { return mType;     }

		void	AddBool   (rbool b);
		void	AddSi32   (si32 i);
		void	AddUi32   (ui32 u);
		void	AddCString(sptr s);
		void	AddString (sptr s)		{ AddCString((sptr)s); }
		void	AddString (RString s)	{ AddCString((sptr)s); }
		void	AddRString(RString s)	{ AddCString((sptr)s); }
		void	AddDate   (RDate d);
		void	AddRaw    (vptr data, ui32 size);
		void	AddFlat   (RIFlat *flat);

		rbool	GetBool   (void);
		si32	GetSi32   (void);
		ui32	GetUi32   (void);
		sptr	GetString (void);
		void	GetPString(RString &s)	{ s = GetString(); }
		void	GetDate   (RDate d);
		ui32	GetRaw    (vptr *data);		// returns the size. Don't touch &data if size==0
		void	GetFlat   (RIFlat *flat);

		void	PrintToStream(rbool dump = true);

protected:

		void	resize(ui32 size, rbool keepold = true);
		void	setSizeRevType(void);
		void	erase(void);

		ui8 *	mData;
		ui32	mDataSize;
		ui32	mAllocSize;

		ui32	mRevision;
		ui32	mType;

		ui32	mReadIndex;
};

//------------------------------------------------------------

#endif // of _H_CFLATBLOCK_

// eof
