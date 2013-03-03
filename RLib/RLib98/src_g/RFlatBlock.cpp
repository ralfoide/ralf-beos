/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: RFlatBlock.cpp
	Partie	: Storage

	Auteur	: RM
	Date		: 180298
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"
#include "gMacros.h"
#include "RFlatBlock.h"
#include "RFlattenable.h"

//------------------------------------------------------------
// addString() with a NULL ptr will convert it to the empty string

#define K_FLATBLOCK_FORBID_NULL_STRING

//------------------------------------------------------------


//************************************************************
RFlatBlock::RFlatBlock(void)
//************************************************************
{
	mData = NULL;
	makeEmpty();
} // end of default constructor for RFlatBlock


//************************************************************
RFlatBlock::RFlatBlock(vptr data)
//************************************************************
{
	mData = NULL;
	makeEmpty();
	setData(data);
} // end of existing-content constructor for RFlatBlock


//************************************************************
RFlatBlock::RFlatBlock(ui32 type, ui32 revision)
//************************************************************
{
	mData = NULL;
	makeEmpty();
	mRevision = revision;
	mType = type;
	setSizeRevType();
	rewind();
} // end of constructor for RFlatBlock


//************************************************************
RFlatBlock::~RFlatBlock(void)
//************************************************************
{
   makeEmpty();
} // end of destructor for RFlatBlock


//------------------------------------------------------------


//************************************************************
void RFlatBlock::setData(vptr data)
//************************************************************
{
ui32 *p;

	if (!data) throw("RFlatBlock::setData null ptr");
	erase();

	p = (ui32 *)data;
  mDataSize = (*p++);
  mDataSize = M_FROM_BIG_UINT32(mDataSize);
  if (mDataSize < K_FLATBLOCK_HEAD_SIZE) throw("RFlatBlock::setData block too small");

	resize(mDataSize, false);
	memcpy(mData, data, mDataSize);

	p = (ui32 *)mData;
	mRevision = M_FROM_BIG_UINT32(p[1]);
	mType     = M_FROM_BIG_UINT32(p[2]);

	rewind();

} // end of setData for RFlatBlock


//------------------------------------------------------------


//************************************************************
void RFlatBlock::erase(void)
//************************************************************
{
   if (mData) delete [] mData;
   mData 			= NULL;
   mDataSize 	= 0;
   mAllocSize = 0;
   mRevision 	= 0;
   mType 			= 0;
} // end of erase for RFlatBlock


//************************************************************
void RFlatBlock::resize(ui32 size, rbool keepold)
//************************************************************
{
   if (size == 0)
   {
      // remove buffer
      if (mData) delete [] mData;
      mDataSize = 0;
      mAllocSize = 0;
   }
   else if (mData && size < mDataSize)
   {
      // just change virtual size
      mDataSize = size;
   }
   else
   {
      // must have a non null size
      // has no mData and/or alloc size is unsufficient

      if (size > mAllocSize)
      {
         ui8 * data;
         ui32 s = (size+K_FLATBLOCK_GRANULARITY) / K_FLATBLOCK_GRANULARITY;
         mAllocSize = s*K_FLATBLOCK_GRANULARITY;
         data = new ui8[mAllocSize];
         M_ASSERT_PTR(data);

         if (keepold) memcpy(data, mData, mDataSize);
         if (mData) delete [] mData;
         mData = data;
      }

      mDataSize = size;
   }

} // end of resize for RFlatBlock


//************************************************************
void RFlatBlock::setSizeRevType(void)
//************************************************************
{
  if (mDataSize < K_FLATBLOCK_HEAD_SIZE) resize(K_FLATBLOCK_HEAD_SIZE);

	ui32 *d = (ui32 *)mData;
  d[0] = M_TO_BIG_UINT32(mDataSize);
  d[1] = M_TO_BIG_UINT32(mRevision);
  d[2] = M_TO_BIG_UINT32(mType);

	//printf("setSizeRevType %08x - %08x //  %08x - %08x //  %08x - %08x\n",
	// 	mDataSize, d[0], mRevision, d[1], mType, d[2]);

} // end of setSizeRevType for RFlatBlock


//------------------------------------------------------------


//************************************************************
void RFlatBlock::addBool(rbool b)
//************************************************************
{
rbool *p;
ui32 size = mDataSize;

   resize(size+sizeof(rbool));
   p = (rbool *)(mData+size);
   *p = b;

} // end of addBool for RFlatBlock


//************************************************************
void RFlatBlock::addSi32(si32 i)
//************************************************************
{
si32 *p;
ui32 size = mDataSize;

   resize(size+sizeof(si32));
   p = (si32 *)(mData+size);
   i = M_TO_BIG_UINT32(i);
   *p = i;

} // end of addSi32 for RFlatBlock


//************************************************************
void RFlatBlock::addUi32(ui32 u)
//************************************************************
{
ui32 *p;
ui32 size = mDataSize;

   resize(size+sizeof(ui32));
   p = (ui32 *)(mData+size);
   u = M_TO_BIG_UINT32(u);
   *p = u;

} // end of addUi32 for RFlatBlock


//************************************************************
void RFlatBlock::addCString(sptr str)
//************************************************************
{
ui32 size = mDataSize;
sptr s;
ui32 len;


#ifdef K_FLATBLOCK_FORBID_NULL_STRING
	
	if (!str) s = "";
	else s = (sptr)str;
	len = strlen(s)+1;
	resize(size+len);
	strcpy((sptr)(mData+size), s);

#else

	len = 0;
	if (str)
	{
		s = (sptr)str;
		len = strlen(s)+1;
	}
	resize(size+len+1);
	mData[size] = (len == 0);	// true bool byte to say if string is NULL ptr
	if (len) strcpy(mData+size+1, s);

#endif

} // end of addCString for RFlatBlock


//************************************************************
void RFlatBlock::addDate(RDate d)
//************************************************************
{
sptr p = d;
ui32 size = mDataSize;
ui32 len;

	len = d.len()+1;
	resize(size+len);
	strcpy((sptr)(mData+size), p);

} // end of addDate for RFlatBlock


//************************************************************
void RFlatBlock::addRaw(vptr data, ui32 taille)
//************************************************************
{
ui32 *p;
ui32 u;
ui32 size = mDataSize;

	if (!data) taille = 0;

  resize(size+sizeof(ui32)+taille);
  p = (ui32 *)(mData+size);
  u = M_TO_BIG_UINT32(taille);
  *(p++) = u;

	if (taille > 0) memcpy(p, data, taille);

} // end of addRaw for RFlatBlock


//************************************************************
void RFlatBlock::addFlat(RFlattenable *flat)
//************************************************************
{
	M_ASSERT_PTR(flat);
	RFlatBlock *block = flat->flatten();
	M_ASSERT_PTR(block);
	addRaw((vptr)block->data(), block->dataSize());
} // end of addFlat for RFlatBlock


//------------------------------------------------------------
//------------------------------------------------------------
#pragma mark -


//************************************************************
rbool RFlatBlock::getBool(void)
//************************************************************
{
rbool b;

   if (mReadIndex > mDataSize+sizeof(rbool)) throw(K_FLATBLOCK_EXCPT_EOF);
   b = *((rbool *)(mData+mReadIndex));
   mReadIndex+=sizeof(rbool);
   return b;

} // end of getBool for RFlatBlock


//************************************************************
si32 RFlatBlock::getSi32(void)
//************************************************************
{
si32 i;

   if (mReadIndex > mDataSize+sizeof(si32)) throw(K_FLATBLOCK_EXCPT_EOF);
   i = *((si32 *)(mData+mReadIndex));
   i = M_FROM_BIG_UINT32(i);
   mReadIndex+=sizeof(si32);
   return i;

} // end of getSi32 for RFlatBlock


//************************************************************
ui32 RFlatBlock::getUi32(void)
//************************************************************
{
ui32 i;

   if (mReadIndex > mDataSize+sizeof(ui32)) throw(K_FLATBLOCK_EXCPT_EOF);
   i = *((ui32 *)(mData+mReadIndex));
   i = M_FROM_BIG_UINT32(i);
   mReadIndex+=sizeof(ui32);
   return i;

} // end of getUi32 for RFlatBlock


//************************************************************
sptr RFlatBlock::getString(void)
//************************************************************
{
sptr p;
ui32 len;

	if (mReadIndex > mDataSize+1) throw(K_FLATBLOCK_EXCPT_EOF);

#ifdef K_FLATBLOCK_FORBID_NULL_STRING

	p = (sptr)(mData+mReadIndex);
	len = strlen(p);

#else

	p = mData+mReadIndex;
	mReadIndex++;
	if (*(p++)) return NULL; // this was a NULL string

#endif

	mReadIndex += len+1;
	if (mReadIndex > mDataSize) throw(K_FLATBLOCK_EXCPT_EOF);
	return p;

} // end of getString for RFlatBlock


//************************************************************
void RFlatBlock::getDate(RDate d)
//************************************************************
{
sptr p;
ui32 len;

	len = d.len()+1;
	if (mReadIndex > mDataSize+len) throw(K_FLATBLOCK_EXCPT_EOF);

	p = (sptr)(mData+mReadIndex);
	d = p;
	mReadIndex += len;

} // end of getDate for RFlatBlock


//************************************************************
ui32 RFlatBlock::getRaw(vptr *data)
//************************************************************
{
sptr p;
ui32 i;

  if (mReadIndex > mDataSize+sizeof(ui32)) throw(K_FLATBLOCK_EXCPT_EOF);
  i = *((ui32 *)(mData+mReadIndex));
  i = M_FROM_BIG_UINT32(i);
	mReadIndex+=sizeof(ui32);

	// do nothing more if size is null
	if (!i) return 0;

	M_ASSERT_PTR(data);
	p = (sptr)(mData+mReadIndex);
	*data = p;
	mReadIndex += i;
	return i;

} // end of getRaw for RFlatBlock


//************************************************************
void RFlatBlock::getFlat(RFlattenable *flat)
//************************************************************
{
vptr data;
ui32 size;

	M_ASSERT_PTR(flat);

	size = getRaw(&data);
	M_ASSERT_PTR(size);

	flat->unflatten(data);
} // end of getFlat for RFlatBlock


//------------------------------------------------------------



//************************************************************
void RFlatBlock::printToStream(rbool dump)
//************************************************************
{
	setSizeRevType();
	printf("RFlatBlock\n"
	       "  type $%08lx (%ld)\n"
	       "  revision $%08lx (%ld)\n"
	       "  size %08lx (%ld) (alloc %ld)\n"
	       "  data %p\n", mType, mType, mRevision, mRevision, mDataSize, mDataSize, mAllocSize, mData);
	if (dump && mData)
	{
		ui32 i;
		for(i=0; i<mDataSize; i++)
		{
			printf("%02x ", mData[i]);
			if (i % 16 == 15) printf("\n");
		}
		if (i % 16 != 0) printf("\n");
	}
} // end of printToStream for RFlatBlock


// eof
