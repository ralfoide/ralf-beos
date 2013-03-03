/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
*/

#ifndef _adpcm_h_
#define _adpcm_h_

#include "gMachine.h"
#include "gMacros.h"

struct adpcm_state
{
    short	valprev;	/* Previous output value */
    char	index;		/* Index into stepsize table */
};

// ANSI C protos
//void adpcm_coder(short indata[], char outdata[], int len, struct adpcm_state *state);
//void adpcm_decoder(char indata[], short outdata[], int len, struct adpcm_state *state);

extern void adpcm_coder(si16 *indata, ui8 *outdata, ui32 len, adpcm_state *state);
extern void adpcm_decoder(ui8 *indata, si16 *outdata, ui32 len, adpcm_state *state);


#endif // _adpcm_h_