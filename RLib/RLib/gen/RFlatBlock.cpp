/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: RFlatBlock.cpp
	Partie	: Storage

	Auteur	: RM
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#include "RLib.h"
#include "RFlatBlock.h"
#include "RIFlat.h"

//------------------------------------------------------------
// AddString() with a NULL ptr will convert it to the empty string

#define K_FLATBLOCK_FORBID_NULL_STRING

//------------------------------------------------------------


//**************************
RFlatBlock::RFlatBlock(void)
//**************************
{
	mData = NULL;
	MakeEmpty();
} // end of default constructor for RFlatBlock


//*******************************
RFlatBlock::RFlatBlock(vptr data)
//*******************************
{
	mData = NULL;
	MakeEmpty();
	SetData(data);
} // end of existing-content constructor for RFlatBlock


//**********************************************
RFlatBlock::RFlatBlock(ui32 type, ui32 revision)
//**********************************************
{
	mData = NULL;
	MakeEmpty();
	mRevision = revision;
	mType = type;
	setSizeRevType();
	Rewind();
} // end of constructor for RFlatBlock


//***************************
RFlatBlock::~RFlatBlock(void)
//***************************
{
	MakeEmpty();
} // end of destructor for RFlatBlock


//------------------------------------------------------------
// public methods


//*********************************
void RFlatBlock::SetData(vptr data)
//*********************************
{
	ui32 *p;

	if (!data) M_THROW_ERR("RFlatBlock::setData null ptr");
	erase();

	p = (ui32 *)data;
	mDataSize = (*p++);
	mDataSize = M_FROM_BIG_UI32(mDataSize);
	if (mDataSize < K_FLATBLOCK_HEAD_SIZE) M_THROW_ERR("RFlatBlock::setData block too small");

	resize(mDataSize, false);
	memcpy(mData, data, mDataSize);

	p		  = (ui32 *)mData;
	mRevision = M_FROM_BIG_UI32(p[1]);
	mType     = M_FROM_BIG_UI32(p[2]);

	Rewind();

} // end of SetData for RFlatBlock


//------------------------------------------------------------
// private methods


//**************************
void RFlatBlock::erase(void)
//**************************
{
	delete [] mData;
	mData 		= NULL;
	mDataSize	= 0;
	mAllocSize	= 0;
	mRevision 	= 0;
	mType 		= 0;

} // end of erase for RFlatBlock


//***********************************************
void RFlatBlock::resize(ui32 size, rbool keepold)
//***********************************************
{
	if (size == 0)
	{
		// remove buffer
		delete [] mData;
		mData		 = NULL;
		mDataSize	 = 0;
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
			delete [] mData;
			mData = data;
		}

		mDataSize = size;
	}

} // end of resize for RFlatBlock


//***********************************
void RFlatBlock::setSizeRevType(void)
//***********************************
{
	if (mDataSize < K_FLATBLOCK_HEAD_SIZE) resize(K_FLATBLOCK_HEAD_SIZE);

	ui32 *d = (ui32 *)mData;
	d[0] = M_TO_BIG_UI32(mDataSize);
	d[1] = M_TO_BIG_UI32(mRevision);
	d[2] = M_TO_BIG_UI32(mType);

	//DPRINTF(("setSizeRevType %08x - %08x //  %08x - %08x //  %08x - %08x\n",
	// 	mDataSize, d[0], mRevision, d[1], mType, d[2]));

} // end of setSizeRevType for RFlatBlock


//------------------------------------------------------------
// Public Methods


//*******************************
void RFlatBlock::AddBool(rbool b)
//*******************************
{
	rbool *p;
	ui32 size = mDataSize;

	resize(size+sizeof(rbool));
	p = (rbool *)(mData+size);
	*p = b;

} // end of AddBool for RFlatBlock


//******************************
void RFlatBlock::AddSi32(si32 i)
//******************************
{
	si32 *p;
	ui32 size = mDataSize;

	resize(size+sizeof(si32));
	p = (si32 *)(mData+size);
	i = M_TO_BIG_UI32(i);
	*p = i;

} // end of AddSi32 for RFlatBlock


//******************************
void RFlatBlock::AddUi32(ui32 u)
//******************************
{
	ui32 *p;
	ui32 size = mDataSize;

	resize(size+sizeof(ui32));
	p = (ui32 *)(mData+size);
	u = M_TO_BIG_UI32(u);
	*p = u;

} // end of AddUi32 for RFlatBlock


//***********************************
void RFlatBlock::AddCString(sptr str)
//***********************************
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

} // end of AddCString for RFlatBlock


//*******************************
void RFlatBlock::AddDate(RDate d)
//*******************************
{
	sptr p = d;
	ui32 size = mDataSize;
	ui32 len;

	len = d.Len()+1;
	resize(size+len);
	strcpy((sptr)(mData+size), p);

} // end of AddDate for RFlatBlock


//*********************************************
void RFlatBlock::AddRaw(vptr data, ui32 taille)
//*********************************************
{
	ui32 *p;
	ui32 u;
	ui32 size = mDataSize;

	if (!data) taille = 0;

	resize(size+sizeof(ui32)+taille);
	p = (ui32 *)(mData+size);
	u = M_TO_BIG_UI32(taille);
	*(p++) = u;

	if (taille > 0) memcpy(p, data, taille);

} // end of AddRaw for RFlatBlock


//************************************
void RFlatBlock::AddFlat(RIFlat *flat)
//************************************
{
	M_ASSERT_PTR(flat);
	RFlatBlock *block = flat->Flatten();
	M_ASSERT_PTR(block);
	AddRaw((vptr)block->Data(), block->DataSize());
} // end of AddFlat for RFlatBlock


//------------------------------------------------------------
//------------------------------------------------------------
#pragma mark -


//*****************************
rbool RFlatBlock::GetBool(void)
//*****************************
{
	rbool b;

	if (mReadIndex > mDataSize+sizeof(rbool)) M_THROW(K_FLATBLOCK_EXCPT_EOF);
	b = *((rbool *)(mData+mReadIndex));
	mReadIndex+=sizeof(rbool);
	return b;

} // end of GetBool for RFlatBlock


//****************************
si32 RFlatBlock::GetSi32(void)
//****************************
{
	si32 i;

	if (mReadIndex > mDataSize+sizeof(si32)) M_THROW(K_FLATBLOCK_EXCPT_EOF);
	i = *((si32 *)(mData+mReadIndex));
	i = M_FROM_BIG_UI32(i);
	mReadIndex+=sizeof(si32);
	return i;

} // end of GetSi32 for RFlatBlock


//****************************
ui32 RFlatBlock::GetUi32(void)
//****************************
{
	ui32 i;

	if (mReadIndex > mDataSize+sizeof(ui32)) M_THROW(K_FLATBLOCK_EXCPT_EOF);
	i = *((ui32 *)(mData+mReadIndex));
	i = M_FROM_BIG_UI32(i);
	mReadIndex+=sizeof(ui32);
	return i;

} // end of GetUi32 for RFlatBlock


//******************************
sptr RFlatBlock::GetString(void)
//******************************
{
	sptr p;
	ui32 len;

	if (mReadIndex > mDataSize+1) M_THROW(K_FLATBLOCK_EXCPT_EOF);

#ifdef K_FLATBLOCK_FORBID_NULL_STRING

	p = (sptr)(mData+mReadIndex);
	len = strlen(p);

#else

	p = mData+mReadIndex;
	mReadIndex++;
	if (*(p++)) return NULL; // this was a NULL string

#endif

	mReadIndex += len+1;
	if (mReadIndex > mDataSize) M_THROW(K_FLATBLOCK_EXCPT_EOF);
	return p;

} // end of GetString for RFlatBlock


//*******************************
void RFlatBlock::GetDate(RDate d)
//*******************************
{
	sptr p;
	ui32 len;

	len = d.Len()+1;
	if (mReadIndex > mDataSize+len) M_THROW(K_FLATBLOCK_EXCPT_EOF);

	p = (sptr)(mData+mReadIndex);
	d = p;
	mReadIndex += len;

} // end of GetDate for RFlatBlock


//*********************************
ui32 RFlatBlock::GetRaw(vptr *data)
//*********************************
{
	sptr p;
	ui32 i;

	if (mReadIndex > mDataSize+sizeof(ui32)) M_THROW(K_FLATBLOCK_EXCPT_EOF);
	i = *((ui32 *)(mData+mReadIndex));
	i = M_FROM_BIG_UI32(i);
	mReadIndex+=sizeof(ui32);

	// do nothing more if size is null
	if (!i) return 0;

	M_ASSERT_PTR(data);
	p = (sptr)(mData+mReadIndex);
	*data = p;
	mReadIndex += i;
	return i;

} // end of GetRaw for RFlatBlock


//************************************
void RFlatBlock::GetFlat(RIFlat *flat)
//************************************
{
vptr data;
ui32 size;

	M_ASSERT_PTR(flat);

	size = GetRaw(&data);
	M_ASSERT_PTR(size);

	flat->Unflatten(data);
} // end of GetFlat for RFlatBlock


//------------------------------------------------------------



//****************************************
void RFlatBlock::PrintToStream(rbool dump)
//****************************************
{
	setSizeRevType();
	DPRINTF(("RFlatBlock\n"
	       "  type $%08x (%d)\n"
	       "  revision $%08x (%d)\n"
	       "  size $08x (%d) (alloc %d)\n"
	       "  data %p\n", mType, mType, mRevision, mRevision, mDataSize, mDataSize, mAllocSize, mData));
	if (dump && mData)
	{
		ui32 i;
		for(i=0; i<mDataSize; i++)
		{
			DPRINTF(("%02x ", mData[i]));
			if (i % 16 == 15) DPRINTF(("\n"));
		}
		if (i % 16 != 0) DPRINTF(("\n"));
	}
} // end of PrintToStream for RFlatBlock


// eof
