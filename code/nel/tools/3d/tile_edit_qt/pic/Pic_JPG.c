#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "jpeglib.h"
#include "pic_private.h"
#include "pic.h"

/* ---------------------------------------------------------------------------------------------------------------------------------- */

struct my_error_mgr 
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;

/* ---------------------------------------------------------------------------------------------------------------------------------- */

static unsigned char	error;

/* ---------------------------------------------------------------------------------------------------------------------------------- */

void my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	error=1;
	longjmp(myerr->setjmp_buffer, 1);
}

/* ---------------------------------------------------------------------------------------------------------------------------------- */

unsigned long Pic_JPG_Read(const char *FileName, unsigned char **ppPal, unsigned char **ppDatas, unsigned long *w, unsigned long *h)
{
	struct jpeg_decompress_struct	cinfo;
	struct my_error_mgr				jerr;
	FILE							*file;
	JSAMPARRAY						buffer;		
	int								row_stride,i;		
	unsigned char					*pDatas,*pPal;
	unsigned long					ptr;
	
	error=0;
	ptr=0;
	file=fopen(FileName, "rb");
	if (!file)
	{
		Pic_SetError("JPG_Read, unable to open %s",FileName);
		return(0);
	}
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	setjmp(jerr.setjmp_buffer);
	if (error)
	{
		Pic_SetError("JPG_Read, internal decompression error");
		jpeg_destroy_decompress(&cinfo);
		return(0);
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, file);
	(void) jpeg_read_header(&cinfo, TRUE);
	*w=cinfo.image_width;
	*h=cinfo.image_height;
	if (!ppPal)
	{
		pDatas=Pic_calloc(1,(*w)*(*h)*3);
	}
	else
	{
		pDatas=Pic_calloc(1,(*w)*(*h));
		pPal=Pic_calloc(1,256*3);
		if (!pPal)
		{
			Pic_SetError("JPG_Read, not enough memory for palette");
			return(0);
		}
		cinfo.desired_number_of_colors = 256;
		cinfo.quantize_colors = TRUE;
		cinfo.dither_mode = JDITHER_ORDERED;
	}
	if (!pDatas)
	{
		Pic_SetError("JPG_Read, not enough memory for pic");
		return(0);
	}
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
	((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(&pDatas[ptr],buffer[0],row_stride);
		ptr+=row_stride;
	}
	*ppDatas=pDatas;
	if (ppPal)
	{
		for(i=0 ; i<256 ; i++)
		{
			pPal[i*3+0]=cinfo.colormap[2][i];
			pPal[i*3+1]=cinfo.colormap[1][i];
			pPal[i*3+2]=cinfo.colormap[0][i];
		}
		*ppPal=pPal;
	}	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(file);
	return(1);
}

/* ---------------------------------------------------------------------------------------------------------------------------------- */

unsigned long Pic_JPG_Write(const char *FileName, unsigned long Qual, unsigned char *pDatas, unsigned long w, unsigned long h)
{
	struct jpeg_compress_struct	cinfo;
	struct my_error_mgr			jerr;
	FILE						*file;		
	JSAMPROW					row_pointer[1];
	int							row_stride;		

	error=0;
	file=fopen(FileName,"wb");
	if (!file)
	{
		Pic_SetError("JPG_Write, unable to open %s",FileName);
		return(0);
	}
	jpeg_create_compress(&cinfo);
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	setjmp(jerr.setjmp_buffer);
	if (error)
	{
		Pic_SetError("JPG_Write, internal compression error");
		jpeg_destroy_compress(&cinfo);
		return(0);
	}
	jpeg_stdio_dest(&cinfo, file);
	cinfo.image_width = w; 	
	cinfo.image_height = h;
	cinfo.input_components = 3;	
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, Qual, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = w * 3;
	while(cinfo.next_scanline<cinfo.image_height) 
	{
		row_pointer[0] = & pDatas[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(file);
	jpeg_destroy_compress(&cinfo);
	return(1);
}
