/**************************************
	Gif Reader
	140297
	R'alf
	
	I got this from Pierre/Lois Team in Feb 97 and hacked it to serve my purposes.
	
**************************************/

#include "bCGifReader.h"


//*********************************************************
CGifReader::CGifReader(void)
//*********************************************************
{
	inputBuf = outputBuf = NULL;
	inputLen = outputLen = pointeurInput = 0;
  pstk = new int[PSTK_SIZE];
  prefix = new int[STAB_SIZE];
  extnsn = new int[STAB_SIZE];
  need_tr_color = FALSE;
	//buf = new uchar[256];
  cmap = new uchar[256];
  g_cmap = NULL;
  l_cmap = NULL;
  screen_header_ok = FALSE;
  image_header_ok = FALSE;
	repeat = -1;
	keep_order = false;
}


//*********************************************************
CGifReader::~CGifReader()
//*********************************************************
{
  delete pstk;
  delete prefix;
  delete extnsn;
  //delete buf;
  delete cmap;
  if (g_cmap) delete g_cmap;
  if (l_cmap) delete l_cmap;
}


//*********************************************************
long	CGifReader::Read(uchar **buf, long size)
//*********************************************************
{
	if (!inputBuf) return 0;
	if (pointeurInput + size > inputLen)
		size = inputLen-pointeurInput;

	*buf = inputBuf+pointeurInput;
	pointeurInput += size;
	return size;
}


//*********************************************************
bool	CGifReader::decoder(uchar *input, long inLen,
													uchar * &output, long &outLen,
													uchar *palette)
//*********************************************************
{
	//long		err;
	long		index = 0;

	inputBuf = input;
	inputLen = inLen;

	keep_order = (palette != NULL);

	if (!NoError(OpenScreen()) || !NoError(OpenImage()))
		return false;

	outLen = img_width*img_height;
	output = new uchar[img_width*img_height];
	if (!output) return false;

	outputBuf = output;
	outputLen = outLen;

	line_buf_size = img_width;

	// allocation du buffer
	line_buf = new uchar[line_buf_size];
	if (!line_buf) return false;

	//do		<-- invalide la gestion des images multiples
	//{
		// si ce n'est pas la premiere image
/*
		if (index)
		{
			bmp = new BBitmap(BRect(0, 0, img_width - 1, img_height - 1), screen_space);
			BRect	pos(img_left, img_top, img_width, img_height);
			_img->AddBitmap(bmp, 0, &pos, delay);
		}
*/
		if (!index)
			CalcCmap();
		else
			if (use_l_cmap)
				CalcCmap();
		if (interlace_flag)
			DecodeEntrelace();
		else
			DecodeNonEntrelace();
		// lit le sŽparateur d'image
		Read(&buf, 1);
		// une image de plus
		index++;
	//}
	//while (buf[0] != GIF_END && ((err = OpenImage()) & (ERR_END | ERR_READ)) && NoError(err));

	if (palette && g_cmap)
	{
		for(long x = 0; x < g_ncolors; x++)
		{
			*(palette++) = g_cmap[x].red;
			*(palette++) = g_cmap[x].green;
			*(palette++) = g_cmap[x].blue;
		}
	}

	delete line_buf;
	return true;
}


//*********************************************************
char	interlace_start[4] =
{ 0, 4, 2, 1 };

char	interlace_rate[4] =
{ 8, 8, 4, 2 };
//*********************************************************


//*********************************************************
void	CGifReader::DecodeEntrelace()
//*********************************************************
{
	long		x, y, pixel, l;
	uchar		*v4b = line_buf;
	long		line_size = line_buf_size;
	long		nb_line;

	for(long pass = 0; pass < 4; pass++)
	{
		y = interlace_start[pass];
		while (y < img_height)
		{
			for (x = 0; x < img_width; x++)
			{
				if (!NoError(GetPixel(&pixel)))
				{
					y = img_height;
					x = img_width;
				}
				else
				{
/*					if (screen_space == B_RGB_32_BIT)
					{
						*v4b++ = r_cmap[pixel].red;
						*v4b++ = r_cmap[pixel].green;
						*v4b++ = r_cmap[pixel].blue;
					}
					else
*/
						*v4b++ = cmap[pixel];
				}
			}
			v4b = line_buf;
			// nombre de ligne a remplir
			nb_line = (interlace_start[pass] ? interlace_start[pass] : 8);
			for (l = 0; (l < nb_line) && ((y + l) < img_height); l++)
				memcpy(outputBuf+(y + l) * line_size, v4b, line_buf_size);

			// on saute des lignes
			y += interlace_rate[pass];
		}
	}
}


//*********************************************************
void	CGifReader::DecodeNonEntrelace()
//*********************************************************
{
	long		x, y, pixel;
	uchar		*v4b = line_buf;
	long		line_size = line_buf_size;

	for (y = 0; y < img_height; y++)
	{
		for (x = 0; x < img_width; x++)
		{
			if (!NoError(GetPixel(&pixel)))
			{
				y = img_height;
				x = img_width;
			}
			else
			{
/*
				if (screen_space == B_RGB_32_BIT)
				{
					*v4b++ = r_cmap[pixel].red;
					*v4b++ = r_cmap[pixel].green;
					*v4b++ = r_cmap[pixel].blue;
				}
				else
*/
					*v4b++ = cmap[pixel];
			}
		}
		v4b = line_buf;

		memcpy(outputBuf+y*line_size, v4b, line_buf_size);
	}
}



//*********************************************************
void	CGifReader::CalcCmap()
//*********************************************************
{
//	uchar	gray;
	short	ncolors;

	if (use_l_cmap)
	{
		r_cmap = l_cmap;
		ncolors = l_ncolors;
	}
	else
	{
		r_cmap = g_cmap;
		ncolors = g_ncolors;
	}
/*
	switch (couleur)
	{
		case COLOR:
			if (screen_space == B_COLOR_8_BIT)
*/
				for(long x = 0; x < ncolors; x++)
					cmap[x] = (keep_order ? x : index_for_color(r_cmap[x].red, r_cmap[x].green, r_cmap[x].blue));
/*
			break;

		case GRIS:
			for (long x = 0; x < ncolors; x++)
			{
				gray = (r_cmap[x].red * 333 + r_cmap[x].green * 334 + r_cmap[x].blue * 333) / 1000;
				if (screen_space == B_COLOR_8_BIT)
					cmap[x] = index_for_color(gray, gray, gray);
				else
				{
					r_cmap[x].red = gray;
					r_cmap[x].green = gray;
					r_cmap[x].blue = gray;
				}
			}
			break;
		case LUMA:
			for (long x = 0; x < ncolors; x++)
			{
				gray = (r_cmap[x].red * 299 + r_cmap[x].green * 587 + r_cmap[x].blue * 114) / 1000;
				if (screen_space == B_COLOR_8_BIT)
					cmap[x] = index_for_color(gray, gray, gray);
				else
				{
					r_cmap[x].red = gray;
					r_cmap[x].green = gray;
					r_cmap[x].blue = gray;
				}
			}
			break;
	}
*/	if (need_tr_color)
	{
			cmap[tr_color] = B_TRANSPARENT_8_BIT;
	}
}


//*********************************************************
long	CGifReader::OpenScreen()
//*********************************************************
/*-------------------------------------------------------*/
/* OPEN_SCREEN :                                         */
/* Cette fonction extrait les informations sur l'ecran   */ 
/*-------------------------------------------------------*/
{
	if (screen_header_ok)
		return 0;
	if (Read(&buf,1) != 1)
		return ERR_READ;
	if (*buf == 'G')
		Read(&buf, 1);
	if (Read(&buf + 1, 4) != 4)
		return ERR_READ;
	buf[6] = 0;
	if (!strcmp((char *) buf,"IF87a") && !strcmp((char *) buf,"IF89a"))
		return ERR_NOT_GIF;
	annee = buf[3];

	if (Read(&buf,7) != 7)
		return ERR_READ;

  /*--- Decodage du screen descriptor ---*/
	rast_width   = (buf[1] << 8) + buf[0];
	rast_height  = (buf[3] << 8) + buf[2];
	use_g_cmap   = ((buf[4] & 0x80) ? 1 : 0);
	color_bits   = ((buf[4] & 0x70) >> 4) + 1;
	g_pixel_bits = (buf[4] & 0x07) +1;
	bg_color     = buf[5]; 
  
	if (buf[6])
		return ERR_BAD_STRUC;
// cout << "Global ColorMap: " << (use_g_cmap ? "oui" : "non") << endl;
	if (use_g_cmap)
	{
		long err;
		g_ncolors = 1 << g_pixel_bits;
		g_cmap = new rgb_color[256];
		if ((err = LoadColormap(g_cmap, g_ncolors)) != 0)
			return err;
	}
	else
		g_ncolors = 0;
	screen_header_ok = TRUE;
	return 0;
}



//*********************************************************
long	CGifReader::OpenImage()
//*********************************************************
/*-------------------------------------------------------*/
/* OPEN_IMAGE :                                          */
/* Cette fonction extrait les informations sur l'image   */ 
/*-------------------------------------------------------*/
{
	long	err;

// cout << "OpenImage\n";
	if (Read(&buf,1) != 1)
		return ERR_READ;
	while (buf[0] == GIF_EXTENSION)
	{
		if ((err = SautExtension()) != 0)
			return err;
		if (Read(&buf,1) != 1)
			return ERR_READ;
	}
	if (buf[0] == GIF_END)
		return ERR_END;
	if (buf[0] != 0x2C)
		return ERR_BAD_STRUC;
	if (Read(&buf, 9) != 9)
		return ERR_READ;
  
  /*--- Decodage de l'image descriptor ---*/
	img_left       = (buf[1] << 8) + buf[0];
	img_top        = (buf[3] << 8) + buf[2];
	img_width      = (buf[5] << 8) + buf[4];
	img_height     = (buf[7] << 8) + buf[6];
	use_l_cmap     = (buf[8] & 0x80) ? 1 : 0;
	interlace_flag = (buf[8] & 0x40) ? 1 : 0;
	l_pixel_bits   = (buf[8] & 0x07) +1;

//cout << "width: " << img_width << ", height: " << img_height << endl;

	if (use_l_cmap)
	{
		long	err;
	
    	l_ncolors = (1 << l_pixel_bits);
		l_cmap = new rgb_color[256];
		if ((err = LoadColormap(l_cmap, l_ncolors)) != 0)
			return err;
	}
	else
		l_ncolors = 0;
  
	if (!Read(&buf,1))
		return ERR_READ;

  root_size = buf[0];
  clr_code  = 1 << root_size;
  eoi_code  = clr_code + 1;
  code_size = root_size + 1;
  code_mask = (1 << code_size) - 1;
  work_bits = 0;
  work_data = 0;
  buf_cnt   = 0;
  buf_idx   = 0;
  
  for(long i = 0; i < STAB_SIZE; i++)
  {
    prefix[i] = NULL_CODE;
    extnsn[i] = i;
  }
   
  pstk_idx = 0;
  image_header_ok = TRUE;
  return 0;
}

    

//*********************************************************
long	CGifReader::LoadColormap(rgb_color *loadcmap, long ncolors)
//*********************************************************
/*-------------------------------------------------------*/
/* LOAD_COLORMAP :                                       */
/* Cette fonction extrait la palette de couleurs         */ 
/*-------------------------------------------------------*/
{
	for(long i = 0; i < ncolors; i++)
	{
		if (Read(&buf, 3) != 3)
			return ERR_READ;
		loadcmap[i].red = buf[0];
		loadcmap[i].green = buf[1];
		loadcmap[i].blue = buf[2];
	}
	return 0;
}



//*********************************************************
long	CGifReader::SautExtension()
//*********************************************************
/*-------------------------------------------------------*/
/* SAUT_EXTENSION :                                      */
/* Cette fonction saute les blocs d'extension d'info.    */ 
/*-------------------------------------------------------*/
{
	uchar	*numero;
	uchar	nb;

	buf_cnt = 1;
	if (!Read(&numero, 1))
		return ERR_READ;
	nb = *numero;
	if (ReadDataBlock())
		return ERR_BAD_STRUC;
	switch (nb)
	{
		case 0xF9:
			if (buf[0] & 1)
			{
				need_tr_color = TRUE;
				tr_color = buf[3];
			}
			delay = (buf[2] << 8) + buf[1];
			break;
		case 0xFF:
			if (buf_cnt != 0x0B)
				break;
			if (strncmp("NETSCAPE2.0", (char *)buf, 11))
				break;
			if (ReadDataBlock())
				return ERR_BAD_STRUC;
			repeat = (buf[1] << 8) + buf[2];
			if (!repeat)
				repeat = -1;
			break;
//	default:
// cout << "Unknown Extension: " << (long)buf[0] << endl;
	}
	while (buf_cnt > 0)
		if (ReadDataBlock())
			return ERR_BAD_STRUC;
	return 0;
}



//*********************************************************
long	CGifReader::ReadDataBlock()
//*********************************************************
/*-------------------------------------------------------*/
/* READ_DATA_BLOCK :                                     */
/* Cette fonction extrait un bloc de données du fichiers */ 
/*-------------------------------------------------------*/
{
	if (!Read(&buf, 1))
		return ERR_READ;
	buf_cnt = buf[0];
	if (Read(&buf, buf_cnt) != buf_cnt)
		return ERR_READ;
	buf_idx = 0;
	return 0;
}



//*********************************************************
long	CGifReader::GetPixel(long *pel)
//*********************************************************
/*-------------------------------------------------------*/
/* GET_PIXEL :                                           */
/* Cette fonction decode l'image selon l'algo. LZW       */ 
/*-------------------------------------------------------*/
{
  long	code, first, place;

  while (pstk_idx == 0)
  {
    while (work_bits < code_size)
	{
	  if (buf_idx == buf_cnt)
	  {
	    if (ReadDataBlock())
		  return ERR_READ;
	    if (buf_cnt == 0)
		  return ERR_BAD_STRUC;
	  } 
	  work_data |= ((long) buf[buf_idx++]) << work_bits;
	  work_bits += 8;
	}
    code        = work_data & code_mask;
    work_data >>= code_size;
    work_bits  -= code_size;
    if (code == clr_code)
	{
	  code_size  = root_size + 1;
	  code_mask  = (1 << code_size) - 1;
	  prev_code  = NULL_CODE;
	  table_size = eoi_code + 1;
	}
    else
	  if (code == eoi_code)
	    return ERR_BAD_STRUC;
      else
	    if (prev_code == NULL_CODE)
	      PushString(prev_code = code);
        else
	    {
	      if (code < table_size)
	        first = PushString(code);
	      else
	      {
	        place = pstk_idx;
	        PushPixel(NULL_CODE);
	        first = PushString(prev_code);
	        pstk[place] = first;
	      }
	      AddString(prev_code,first);
	      prev_code = code;
	    }
  }
  *pel = pstk[--pstk_idx];
  return 0;
}



//*********************************************************
void	CGifReader::PushPixel(long p)
//*********************************************************
/*-------------------------------------------------------*/
/* PUSH_PIXEL :                                          */
/* Cette fonction stocke le pixel dans la pile           */ 
/*-------------------------------------------------------*/
{
  int	*tmp;

  if (pstk_idx == pstk_size)
  {
// cout << "Gif Stack Realloc\n";
	tmp = new int[pstk_size + PSTK_SIZE];
	memcpy(tmp, pstk, pstk_size * 4);
	pstk_size += PSTK_SIZE;
	//** delete pstk;
	pstk = tmp;
  }
  pstk[pstk_idx++] = p;
}



//*********************************************************
long	CGifReader::PushString(long code)
//*********************************************************
/*-------------------------------------------------------*/
/* PUSH_STRING :                                         */
/* Cette fonction extrait la chaine                      */ 
/*-------------------------------------------------------*/
{
  while(prefix[code]!= NULL_CODE)
  {
    PushPixel(extnsn[code]);
    code = prefix[code];
  }
  PushPixel(extnsn[code]);
  return extnsn[code];
}



//*********************************************************
void	CGifReader::AddString(long p, long e)
//*********************************************************
/*-------------------------------------------------------*/
/* ADD_STRING :                                          */
/* Cette fonction ajoute une chaine dans la table        */ 
/*-------------------------------------------------------*/
{
  prefix[table_size] = p;
  extnsn[table_size] = e;
  if ((table_size == code_mask) && (code_size < 12))
  {
    code_size += 1;
    code_mask  = (1 << code_size) - 1;
  }
  table_size += 1;
}


//*********************************************************
long	CGifReader::NoError(long num)
//*********************************************************
{
  char		*err_str;

  if (!num) return 1;
  switch(num)
  {
    case ERR_READ :
    	return 0; // ?
      err_str = "Missing GIF Data";
      break; 
    case ERR_NOT_GIF :
      err_str = "Not a GIF";
      break;   
    case ERR_BAD_STRUC : 
      err_str = "GIF Struct Error";
      break; 
    case ERR_END :
      err_str = "GIF End Of File";
      break; 
    case ERR_STK_OVER :
      err_str = "GIF Stack Overflow";
      break; 
    case ERR_DECOD :
	  err_str = "GIF Decoding Error";
	  	break;
		default :
      err_str = "GIF Unknown Error";
      break; 
  }

	// please never get here...
	printf("CGifReader / error \"%s\" at offset %d\n", err_str, pointeurInput);
  return 0;
}
