/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RFlatBlock.h
	Partie	: Storage

	Auteur	: RM
	Date		: 180298
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFLATBLOCK_
#define _H_CFLATBLOCK_

#include "RString.h"
#include "RDate.h"

//------------------------------------------------------------

#define K_FLATBLOCK_GRANULARITY  256
#define K_FLATBLOCK_HEAD_SIZE    12
#define K_FLATBLOCK_EXCPT_EOF    "read index too big"

//------------------------------------------------------------

class RFlattenable;

//------------------------------------------------------------

//******************************************
class RFlatBlock : virtual public RInterface
//******************************************
{
public:
	// create empty to be filled by hand
  RFlatBlock(void);

	// create non-empty to be filled by hand
  RFlatBlock(ui32 type, ui32 revision = 0);
 
  // recreate from existing data to be read
  RFlatBlock(void *data);

  virtual ~RFlatBlock(void);

	// change content (empty or not with new data) to be read
	void			setData(void *data);

  const ui8 *data(void)     { setSizeRevType(); return mData; }
  ui32      dataSize(void) 	{ return mDataSize; }
  void  		makeEmpty(void)	{ erase(); setSizeRevType(); rewind(); }

  void	rewind(void)				{ mReadIndex = K_FLATBLOCK_HEAD_SIZE; }
  rbool	eof(void)						{ return mReadIndex >= mDataSize; }

  void  setRevision(ui32 rev) { mRevision = rev;  }
  void 	setType    (ui32 typ) { mType = typ;      }
  ui32 	revision   (void)     { return mRevision; }
  ui32 	type       (void)     { return mType;     }

  void   addBool   (rbool b);
  void   addSi32   (si32 i);
  void   addUi32   (ui32 u);
  void   addCString(sptr s);
  void   addString (sptr s)			{ addCString((sptr)s); }
  void   addString (RString s)	{ addCString((sptr)s); }
  void   addRString(RString s)	{ addCString((sptr)s); }
  void	 addDate   (RDate d);
  void	 addRaw    (vptr data, ui32 size);
  void	 addFlat   (RFlattenable *flat);

  rbool	getBool   (void);
  si32	getSi32   (void);
  ui32	getUi32   (void);
  sptr	getString (void);
  void	getPString(RString &s)	{ s = getString(); }
  void	getDate   (RDate d);
  ui32	getRaw    (vptr *data);		// returns the size. Don't touch *data if size==0
  void	getFlat   (RFlattenable *flat);

	void printToStream(rbool dump = true);

protected:
  void resize(ui32 size, rbool keepold = true);
  void setSizeRevType(void);
  void erase(void);

  ui8 * mData;
  ui32 mDataSize;
  ui32 mAllocSize;

  ui32 mRevision;
  ui32 mType;

  ui32 mReadIndex;
};

//------------------------------------------------------------

#endif // of _H_CFLATBLOCK_

// eof
