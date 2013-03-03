/*****************************************************************

	Projet	: RLib

	Fichier	: RIBin.h
	Partie	: file access

	Auteur	: RM
	Date	: 180298
	Date	: 090999
	Date	: 180100 - Win32
	Format	: tabs==4

	An abstract base class for handling a stream of data
	with read/write/seek capabilities.

****************************************************************/

#ifndef _H_RBININTERFACE_
#define _H_RBININTERFACE_

#include <stdio.h>
#include <fcntl.h>

#include "RPath.h"

//-------------------------------------------------------------
// file access mode

/** Open File Mode for RIBin.
	Maps the standard fnctl access modes.
	@see	RIBin
	@param	kFileReadOnly		Read only file.	
	@param	kFileWriteOnly		Write only file.
	@param	kFileReadWrite		Read and write file.
	@param	kFileInvalidMode	Not to be used, represents the non
								initialized state of the RIBin,
								used for file not already open or
								already closed.
*/

//************
enum EFileMode
//************
{
	kFileReadOnly	= O_RDONLY,		// read only -- cf bits/fnctl.h
	kFileWriteOnly	= O_WRONLY,		// write only
	kFileReadWrite	= O_RDWR,  		// read and write
	kFileInvalidMode				// don't use -- non initialized state
}; // end of EFileMode


/** File Seek Mode for RIBin.
	Maps the standard stdio seek modes.
	@see	RIBin
	@param	kFileSeekSet	Seek from beginning of file
	@param	kFileSeekCur	Seek from current position
	@param	kFileSeekEnd	Seek from end of file
*/

//************
enum EFileSeek
//************
{
	kFileSeekSet	= SEEK_SET,		// Seek from beginning of file -- cf stdio.h
	kFileSeekCur	= SEEK_CUR,		// Seek from current position
	kFileSeekEnd	= SEEK_END		// Seek from end of file
}; // end of EFileSeek



//-------------------------------------------------------------


/** Interface for manipulating a seekable stream device.
	Currently only used by RBinFile.
	Should be used by RBinMemory too.
*/


//*********
class RIBin
//*********
{
public:

		/** Opens a file for reading or writing.
			@param	path	The ASICZ string path to the file.
			@param	mode	An open mode.
			@return	true on success, false on failure to locate
					or open the file.
		*/
virtual	rbool	Open(const sptr   path, EFileMode mode) = 0;

		/** Opens a file for reading or writing.
			@param	path	The RPath path to the file.
			@param	mode	An open mode.
			@return	true on success, false on failure to locate
					or open the file.
		*/
virtual	rbool	Open(const RPath &path, EFileMode mode);

		/** Closes a previously open file.
			Must not fail if the file was not open.
		*/
virtual void	Close(void) = 0;

		// --- random access ---

		/// Rewinds an open file (i.e. seeks to the beginning)
virtual	void	Rewind(void) = 0;

		/**	Seeks to a given position in the file.
			@param	position	The position to seek, in bytes.
			@param	mode		The relative or absolute seek reference mode.
			@return	true if seeked successfully, false otherwise.
		*/
virtual	rbool	Seek  (si32 position, EFileSeek mode = kFileSeekSet) = 0;

		/**	Reports the current position in the file stream.
			@return	The number of bytes from the beginning of the file.
		*/
virtual	si32	Tell  (void) = 0;

		/**	Reports if the end of while was reached.
			Eof is always reached on an invalid file.
			@return	true if the current position is the end of file. false otherwise.
		*/
virtual	rbool	Eof	  (void) = 0;

		/**	Reports the size of the file.
			Gets the file size using seek/tell (valid for read-only).
			@return	The size of the file, in bytes.
		*/
virtual	si32	Size  (void) = 0;

		// --- read/write sequential access ---

		/**	Reads a bunch of bytes.
			@param	buffer	The pointer onto the buffer to put the read data.
							Must be preallocated onto a storage big enough to
							accomodate 'size' bytes; non-NULL pointer.
			@param	size	The number of bytes to be read from the stream.
			@return	true if the requested number of bytes was correctly read,
					false if the file was not open, or nothing or less than
					the amount of requested data was actually read.
		*/
virtual	rbool	Read(char *buffer, si32 size) = 0;

		/**	Writes a bunch of bytes.
			@param	buffer	The pointer onto the buffer to get the written data.
							Must be preallocated onto a storage big enough holding
							at least 'size' bytes; non-NULL pointer.
			@param	size	The number of bytes to be written to the stream.
			@return	true if the requested number of bytes was correctly written,
					false if the file was not open, or nothing or less than
					the amount of requested data was actually written.
		*/
virtual	rbool	Write(char *buffer, si32 size) = 0;

		// utilities to read/write bytes, words, long endian-free
		// these are based on top of read/write() implementation.

		// for the read-routines, the function will return 0 on failure
		// you need to check eof() before if you care.
		
				/// reads a byte
		ui8		Read8(void);
				/// reads a word stored as Little-Endian
		ui16	Read16LE(void);
				/// reads a word stored as Big-Endian
		ui16	Read16BE(void);
				/// reads a long stored as Little-Endian
		ui32	Read32LE(void);
				/// reads a long stored as Big-Endian
		ui32	Read32BE(void);
				/// reads a float stored as Little-Endian
		fp32	ReadFpLE(void);
				/// reads a float stored as Big-Endian
		fp32	ReadFpBE(void);

		// there is also another bunch of read function when you want to be
		// sure that the read occured correctly

				/// reads a byte
		rbool	ReadCheck8(ui8 &b);
				/// reads a word stored as Little-Endian
		rbool	ReadCheck16LE(ui16 &w);
				/// reads a word stored as Big-Endian
		rbool	ReadCheck16BE(ui16 &w);
				/// reads a long stored as Little-Endian
		rbool	ReadCheck32LE(ui32 &w);
				/// reads a long stored as Big-Endian
		rbool	ReadCheck32BE(ui32 &w);
				/// reads a float stored as Little-Endian
		rbool	ReadCheckFpLE(fp32 &f);
				/// reads a float stored as Big-Endian
		rbool	ReadCheckFpBE(fp32 &f);

				/// writes a byte
		rbool	Write8(ui8);				
				/// writes a word stored as Little-Endian
		rbool	Write16LE(ui16);
				/// writes a word stored as Big-Endian
		rbool	Write16BE(ui16);
				/// writes a long stored as Little-Endian
		rbool	Write32LE(ui32);
				/// writes a long stored as Big-Endian
		rbool	Write32BE(ui32);
				/// writes a float stored as Little-Endian
		rbool	WriteFpLE(fp32);
				/// writes a float stored as Big-Endian
		rbool	WriteFpBE(fp32);

		/** Debug printToStream
			@param name	A comment string
		*/

virtual	void	PrintToStream(const sptr name = NULL);

protected:

	EFileMode	mMode;
};

//-------------------------------------------------------------


#endif // _H_RBININTERFACE_


/****************************************************************

	$Log: RIBin.h,v $
	Revision 1.1  2000/04/01 21:50:20  Ralf
	Using new RIClass convention for interfaces
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoh
