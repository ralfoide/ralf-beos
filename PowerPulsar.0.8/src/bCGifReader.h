/**************************************
	Gif Reader
	140297
	R'alf
**************************************/

#ifndef _H_BCGIFREADER_
#define _H_BCGIFREADER_

#include "PP_Be.h"

#define ERR_OPEN        0x001
#define ERR_READ        0x002
#define ERR_NOT_GIF     0x004
#define ERR_BAD_STRUC   0x008
#define ERR_END         0x010
#define ERR_STK_OVER    0x020
#define ERR_DECOD       0x040

#define GIF_EXTENSION	0x21
#define GIF_END			0x3B
#define GIF_STRUCT		0x2C


#define NULL_CODE       -1

#define PSTK_SIZE       4200
#define STAB_SIZE       4200

#define COLOR			1
#define LUMA			2
#define GRIS			3

#define REFRESH_COEF	50


//**************
class CGifReader
//**************
{
public:
	CGifReader(void);
  ~CGifReader();
	
	bool decoder(uchar *input, long inLen,
							 uchar * &output, long &outLen,
							 uchar *palette=NULL);

protected:

	uchar *inputBuf, *outputBuf;
	long inputLen, outputLen;
	long pointeurInput;

	long Read(uchar **_buf, long size);


//--- org

	void		Init();
	void		CalcCmap();
	void		DecodeEntrelace();
	void		DecodeNonEntrelace();
	long		OpenScreen();
	long		OpenImage();
	long		LoadColormap(rgb_color *loadcmap, long ncolors);
	long		SautExtension();
	long		ReadDataBlock();
	long		NoError(long num);
	long		GetPixel(long *pixel);
	void		PushPixel(long p);
	long		PushString(long code);
	void		AddString(long p, long e);
	
	//BBitmap		*bmp;
	uchar		*cmap;
	uchar		*buf;
	uchar		*line_buf;
	long		line_buf_size;
	int			buf_cnt;
	int			buf_idx;

	int			root_size;
	int			clr_code;
	int			eoi_code;
	int			code_size;
	int			code_mask;
	int			prev_code;
	long		work_data;
	int			work_bits;

	int			table_size;
	int			*prefix;
	int			*extnsn;

	int			*pstk;
	int			pstk_size;
	int			pstk_idx;

	int			rast_width;
	int			rast_height;
	int			color_bits;

	int			img_left;
	int			img_top;
	int			img_width;
	int			img_height;

	long		delay;
	long		repeat;

	rgb_color	*g_cmap;
	rgb_color	*l_cmap;
	rgb_color	*r_cmap;
	short		g_ncolors;
	short		l_ncolors;
	uchar		interlace_flag;	
	uchar		bg_color;
	uchar		tr_color;
	uchar		g_pixel_bits;
	uchar		l_pixel_bits;

	bool		use_g_cmap;
	bool		use_l_cmap;
	bool		need_tr_color;
	bool		screen_header_ok;
	bool		image_header_ok;

	//char		couleur;
	char		annee;

	bool		keep_order;
};


#endif /* _H_BCGIFREADER_ */

// eoh
