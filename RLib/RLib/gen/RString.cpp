/*****************************************************************************

	Projet	: RLib

	Fichier	: RString.h
	Partie	: Portable Lib

	Auteur	: RM
	Date	: 061499 (MacOS)
	Date	: 300798 (operator+=)
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#include "RLib.h"
#include "RString.h"

//------------------------------------------------------------
// public implementation

const char RString::mEmpty = '\0';

//------------------------------------------------------------


//***************************
RString::RString(void)
	  	:mString(NULL),
		 mStrLen(0)	   	   
		 #ifdef RLIB_MACOS
			, mIsPascal(false)
		 #endif
//***************************
{
}


//*************************
void RString::Cut(sptr pos)
//*************************
{
	// this operation can't be done on a Pascal string
	#ifdef RLIB_MACOS
		ThrowIf_(mIsPascal);
		ThrowIfNULL_(mString);
	#endif

	if (   mString
		&& mStrLen
		&& pos>=mString
		&& pos<mString+mStrLen)
		*pos = '\0';

	mStrLen = ::strlen(mString);
}


//****************************************
void RString::SetData(sptr ptr, ui32 size)
//****************************************
{
	_free();
	if (ptr && size>0)
	{
		mString = new char[size+1];
		M_ASSERT_PTR(mString);
		memcpy(mString, ptr, size);
		mStrLen = size;
		mString[size] = '\0';
	}
}


//***********************************
char& RString::operator[](ui32 index)
//***********************************
{
	M_ASSERT_PTR(mString);

	if (index >= mStrLen)
	{
		return *((sptr)&mEmpty);
		M_THROW(kErrBadIndex);
	}
	return mString[index]; 
}

//------------------------------------------------------------
// protected implementation

//***********************
void RString::_free(void)
//***********************
{
	if (mString) delete [] mString;
	mString = NULL;
	mStrLen = 0;

	#ifdef RLIB_MACOS
		mIsPascal = false;
	#endif
}


//***********************
void RString::use(sptr s)
//***********************
{
	_free(); 

	if (s)
	{
		mStrLen = ::strlen(s); 
		mString = new char[mStrLen+1];
		M_ASSERT_PTR(mString);
		if (mString) strcpy(mString, s);
	} 
}


//********************************
RString& RString::concat(sptr rhs)
//********************************
{
	ui32 len;

	M_ASSERT_PTR(mString);

	// this operation can't be done on a Pascal string
	#ifdef RLIB_MACOS
		ThrowIf_(mIsPascal);
	#endif

	if (rhs)
	{
		len = ::strlen(rhs);
		if (len)
		{
			sptr old = mString;	// keep old string
			ui32 rightlen = this->Strlen();

			if (!old || !rightlen) use(rhs);
			else
			{
				len += rightlen;
				mString = new char[len+1];
				M_ASSERT_PTR(mString);
				if (mString)
				{
					strcpy(mString, old);			// concat old
					strcat(mString, rhs);			// and new
					mStrLen = ::strlen(mString);	// RM 012999 -- was missing
				}
				delete [] old;						// delete old string
			}
		}
	}	
	return *this;

} // end of operator += on sptr for RString


#ifdef RLIB_MACOS

	//*********************
	void RString::c2p(void)
	//*********************
	{
		// convert the ASCIIZ C string into a pascal string
		// this WILL fail if the str is more than 255 characters
		if (mIsPascal) return;
		M_ASSERT_PTR(mString);
		ThrowIf_(mStrLen > 255);
		mIsPascal = true;

		// shift up the string one byte up in memory
		memmove(mString+1, mString, mStrLen);
		// put the string len at the 0th position
		ui8 *head = (ui8 *)mString;
		*head = mStrLen;

		// now look in the string and replace '\n' by '\r'
		head++;
		for(si32 n=mStrLen; n>0; n--, head++)
			if (*head == '\n') *head = '\r';
	}

	//*********************
	void RString::p2c(void)
	//*********************
	{
		// convert the pascal string back into a ASCIIZ C string
		// this fails if the string is not in pascal mode and
		// than the pascal length does not match the strlen
		if (!mIsPascal) return;
		M_ASSERT_PTR(mString);
		ui8 *head = (ui8 *)mString;
		ThrowIf_(mStrLen != *head);
		mIsPascal = false;

		// shift the string one byte down in memory
		memmove(mString, mString+1, mStrLen);
		// put back the trailing zero
		mString[mStrLen] = '\0';

		// now look in the string and replace '\n' by '\r'
		for(si32 n=mStrLen; n>0; n--, head++)
			if (*head == '\n') *head = '\r';
	}

	//******************************************
	void RString::LStr255toC(const LStr255 &str)
	//******************************************
	{
		ConstStringPtr ptr = str;
		SetData((sptr)(&ptr[1]), str.Length());
	}

	//**********************************
	RString::RString(const LStr255 &str)
					:mString(NULL),
					 mStrLen(0),
					 mIsPascal(false)
	//**********************************
	{
		LStr255toC(str);
	}
	
#endif


//------------------------------------------------------------

/****************************************************************

	$Log: RString.cpp,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eof
