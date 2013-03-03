/************************************************************
	Author	: Jonathan Perret
	1997	: JP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Jonathan Perret <jperret@cybercable.fr>

************************************************************/

#include "PP_Be.h"
#include "balls_addon.h"

// PutSprite() PROFILING SWITCHES

#define method 1         // 1,2,3 - higher is better - 4 turns off sprites
#define looptestmethod 1 // 1,2 - 1 (compare with 0) is best
#define zerotestmethod 2 // for method 2 - 1,2 - 2 is best
#define unroll 1         // unrolling in method 3 - 0,1,2 - 1 is best

#if zerotestmethod==1
#define zerotest(X) rlwinm. X,X,0,24,31 // one of 2^32151 ways to test for 0 ... :/
#else
#define zerotest(X) cmplwi cr0,X,0 // this is 20% faster... go figure :/
#endif


#if method==2

// method-2 implementation
// pass source-1 and dest-1 (because do_sprite uses pre-increment)
static inline asm void do_sprite
(register void *source,register int srcdelta,register void *dest,
 register int destdelta,register int w,
#if looptestmethod==1
 register int h)
#else
 register void *endsource)
#endif
 // r3..r8 used by args
{
yloop:

	mtctr w // load counter for xloop
xloop:

	lbzu r9,1(source)
	zerotest(r9)
	beq skip
	
	stbu r9,1(dest)
	b xloop_end
skip:
	addi dest,dest,1
	
xloop_end:
	bdnz xloop // dec test and loop :)
	
	// dest+=destdelta
	add dest,dest,destdelta
	// source+=srcdelta
	add source,source,srcdelta
	
yloop_end:
#if looptestmethod==1
	subi h,h,1    // dec
	cmpwi cr0,h,0 // test
	bne yloop     // and loop
#else	
	cmpw cr0,source,endsource
	blt yloop
#endif

	blr
}

// method-3 implementation
// pass source-4, dest-0, w/4 (w/8 with unroll 1, w/12 with unroll 2)
static inline asm void do_sprite_2
(register void *source,register int srcdelta,register void *dest,
 register int destdelta,register int w,
#if looptestmethod==1
 register int h)
#else
 register void *endsource)
#endif
 // r3..r8 used by args
{
yloop:
	
	mtctr w // load counter for xloop
xloop:

	lwzu r9,4(source) // load 1 full word
	
	rlwinm. r10,r9,8,24,31 // get MSB in r10
	beq skip1
	stb r10,0(dest)
skip1:
	
	rlwinm. r10,r9,16,24,31 // get byte2 in r10
	beq skip2
	stb r10,1(dest)
skip2:
	
	rlwinm. r10,r9,24,24,31 // get byte3 in r10
	beq skip3
	stb r10,2(dest)
skip3:
	
	rlwinm. r10,r9,0,24,31 // get LSB in r10
	beq skip4
	stb r10,3(dest)
skip4:

#if unroll>=1
	lwzu r9,4(source) // load 1 full word
	
	rlwinm. r10,r9,8,24,31 // get MSB in r10
	beq skip1b
	stb r10,4(dest)
skip1b:
	
	rlwinm. r10,r9,16,24,31 // get byte2 in r10
	beq skip2b
	stb r10,5(dest)
skip2b:
	
	rlwinm. r10,r9,24,24,31 // get byte3 in r10
	beq skip3b
	stb r10,6(dest)
skip3b:
	
	rlwinm. r10,r9,0,24,31 // get LSB in r10
	beq skip4b
	stb r10,7(dest)
skip4b:

#if unroll>=2

	lwzu r9,4(source) // load 1 full word
	
	rlwinm. r10,r9,8,24,31 // get MSB in r10
	beq skip1c
	stb r10,8(dest)
skip1c:
	
	rlwinm. r10,r9,16,24,31 // get byte2 in r10
	beq skip2c
	stb r10,9(dest)
skip2c:
	
	rlwinm. r10,r9,24,24,31 // get byte3 in r10
	beq skip3c
	stb r10,10(dest)
skip3c:
	
	rlwinm. r10,r9,0,24,31 // get LSB in r10
	beq skip4c
	stb r10,11(dest)
skip4c:

#endif

#endif

#if unroll==0

	addi dest,dest,4

#elif unroll==1

	addi dest,dest,8

#elif unroll==2
	
	addi dest,dest,12
	
#endif
	
xloop_end:
	bdnz xloop // dec test and loop :)
	
	// dest+=destdelta
	add dest,dest,destdelta
	// source+=srcdelta
	add source,source,srcdelta
	
yloop_end:
#if looptestmethod==1
	subi h,h,1    // dec
	cmpwi cr0,h,0 // test
	bne yloop     // and loop
#else	
	cmpw cr0,source,endsource
	blt yloop
#endif
	
	blr
}

#endif

// PutSprite draws a sprite at x y, skipping pixels of color zero.
void balls_addon::PutSprite(PixelSprite *spr,int32 x,int32 y)
{
	ASSERT(spr);
	ASSERT(spr->data);
	
	// eliminate trivial cases
	// need to cast width and height to signed !!!
	if (x>=int32(fWidth) || y>=int32(fHeight)) return;

	register uint32 srcdelta;
	register uchar *source = spr->data;
	register int32 w = spr->width, h = spr->height;
		
	if (x+w<=0 || y+h<=0) return;
	
	int32 over;
	
	// row clipping
	if(y<0) {
		source -= y*w;
		h += y;
		y = 0;
		if(h>fHeight)	h = fHeight;
		}
	else {
		over = y+h-fHeight;
		if(over>0)
			h -= over;
		}
		
	// column clipping
	srcdelta = 0;
	bool colclip=false;
	if(x<0) {
		source += -x;
		w -= -x;
		srcdelta += -x;
		x = 0;
		if(w>fWidth) w = fWidth;
		colclip = true;
		}
	else {
		over = x+w-fWidth;
		if(over>0) {
			w -= over;
			srcdelta += over;
			colclip = true;
			}
		}
	
#pragma warn_possunwant off // sometimes I know what I'm doing

#if method==1
	register uchar c;
	register uchar *dest = fBits+y*fBpr+x;
	register uint32 destdelta = fBpr-w;
	register uint32 i;
	
	while(h)
	{
		i=w;
		while(i)
		{
			if (c=*++source) *++dest=c; // hurry up !
			else ++dest;
			i--;
		}
		dest+=destdelta;
		source+=srcdelta;
		h--;
	}
#elif method==2
	do_sprite(source-1,srcdelta,fBits+y*fBpr+x-1,fBpr-w,w,
	#if looptestmethod==1
		h);
	#else
		source+h*(spr->width));
	#endif
#elif method==3
	if(colclip||(w&7)) // column clipping, or width not multiple of 8 : 
					// fall back on method 2
		do_sprite(source-1,srcdelta,fBits+y*fBpr+x-1,fBpr-w,w,
		#if looptestmethod==1
			h);
		#else
			source+h*(spr->width));
		#endif
	else
		do_sprite_2(source-4,srcdelta,fBits+y*fBpr+x,fBpr-w,
		#if !unroll
			w>>2,
		#elif unroll==1
			w>>3,
		#elif unroll==2
			w/12,
		#endif		
		#if looptestmethod==1
			h);
		#else
			source+h*(spr->width));
		#endif
#endif
	
#pragma warn_possunwant reset 		
}

