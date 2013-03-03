/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CStuff.cpp
	Partie	: Misc

	Auteur	: RM
	Date		: 170697
	Format	: tabs==2

*****************************************************************************/

#include "PP_Be.h"
#include "CStuff.h"


//--------------------------------------------------------

//******************
CStuff::CStuff(void)
//******************
{
	printf("CStuff constructor\n");
	makeFondEcran();
}


//*******************
CStuff::~CStuff(void)
//*******************
{
}


//--------------------------------------------------------


//******************************
void CStuff::makeFondEcran(void)
//******************************
{
const char name[] = "pulsar1.gif";
CGifReader gifreader;
uchar *input;
long inLen;
uchar *output;
long outLen;
uchar *palette=NULL;
FILE *f;

	f = fopen(name, "rb");
	if (!f)
	{
		printf("can't open %s\n", name);
		return;
	}

	fseek(f, 0, SEEK_END);
	inLen = ftell(f);
	rewind(f);
	input = new uchar[inLen];
	if (!input)
	{
		printf("malloc fail %d uchar\n", inLen);
		return;
	}
	fread(input,inLen,sizeof(uchar),f);
	fclose(f);

	if (!gifreader.decoder(input, inLen, output, outLen, palette))
	{
		printf("gif decode fail\n");
		return;
	}

	printf("ouput decode : %p -- size %d\n", output, outLen);

int liste[] =		// <x,y> -- <sx,sy>
{
	0,0,					532,5,
	10,100+5,			532-20,10,
	10,200+15,		532-20,10,
	10,300+25,		532-20,5,
	0,5,					5,100,
	256+5,5,			10,100,
	512+15,5,			5,100,
	0,100+5,			10,200+25,
	512+15,100+5,	10,200+25,
};

int total;
int nb;
int i;
int *li = liste;

	nb = sizeof(liste)/sizeof(int)/4;
	printf("nb %d\n", nb);

	for(i=0, total=0; i<nb; i++, li+=4)
	{
		total += li[2] * li[3];
	}
	
	printf("total bytes %d\n", total);

	if (input) delete input;
	if (output) delete output;
}


//--------------------------------------------------------

// eoc

