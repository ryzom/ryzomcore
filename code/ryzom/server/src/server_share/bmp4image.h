// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef RY_BMP4IMAGE_H
#define RY_BMP4IMAGE_H

#include <math.h>

#include <vector>
#include <string>

#include <nel/misc/file.h>

template <unsigned int W, unsigned int H>
class CBMP4Image
{
public:

	CBMP4Image(): hdr(W,H)
	{
		// nlassert((W&0xf)==0);
		memset(Data,0,sizeof(Data));
	}

	void circle()
	{
		// a rough circle round the centre point of the grid
		for (int j=0;j<H;j++)
			for (int i=0;i<W;i++)
			{
				double pi=atan2(1.0,0.0)*2;
				double angle=atan2(double(i)-double(W-1)/2,double(j)-double(H-1)/2);
				double angle01=7.5*angle/pi;
				if (angle01<0) angle01-=0.5;
					else	   angle01+=0.5;
				unsigned char colour=char(angle01+8)&0xf;
				putpix(i,j,colour);
			}
	}

	void gradient()
	{
		// colour progression
		for (int j=0;j<H;j++)
			for (int i=0;i<W;i++)
			{
				putpix(i,j,j*16/H);
			}
	}

	/*bool write(char *filename)
	{
		FILE *outf=fopen(filename,"wb");
		if (outf==NULL)
			return false;

		short BM=0x4D42;
		fwrite((void *)&BM,1,sizeof(BM),outf);
		fwrite((void *)&image,1,sizeof(image),outf); // ??
		fclose(outf);

		return true;
	}*/

	void putpix(int x, int y, unsigned char val)
	{
		int shift= ((x&1)^1)*4;
		char mask= 0x0f<<(4-shift);
		unsigned int idx= (W/2)*y+x/2;

		Data[idx]=(Data[idx]&mask)|((val&0xf)<<shift);
	}

	// the image header data buffer
	struct SHdr
	{
		SHdr(int w, int h)
		{
//			magicBM=	0x4D42;
			width=		w;
			height=		h;
			bitdepth=	0x0004;
			hdrsize=	sizeof(CBMP4Image::SHdr)+sizeof(CBMP4Image::SPalette); // 0x00000076;
			datasize=	w*h/2;
			filesize=	hdrsize + datasize;
			paloffs=	0x00000028;

			unknown0=	0x00000000;
			unknown1=	0x0001;
			unknown2=	0x00000000;
			unknown3=	0x00000EC4;
			unknown4=	0x00000EC4;
			unknown5=	0x00000000;
			unknown6=	0x00000000;
		}

//		unsigned short magicBM;		// must have value "BM" (0x4D42) if it's a .bmp
		unsigned long filesize;		// hdr size + data size
		unsigned long unknown0;		// 0x00000000 (00 00 00 00)
		unsigned long hdrsize;		// size of the header (0x76)
		unsigned long paloffs;		// offset from here to the start of the palette (0x28)
		unsigned long width;		// width in pixels
		unsigned long height;		// height in pixels
		unsigned short unknown1;	// 0x0001 (01 00)
		unsigned short bitdepth;	// number of bits per pixel (0x04)
		unsigned long unknown2;		// 0x00000000 (00 00 00 00)
		unsigned long datasize;		// size of data block (0x00000080 for 16x16 4bit image)
		unsigned long unknown3;		// 0x00000EC4 (C4 0E 00 00)
		unsigned long unknown4;		// 0x00000EC4 (C4 0E 00 00)
		unsigned long unknown5;		// 0x00000000 (00 00 00 00)
		unsigned long unknown6;		// 0x00000000 (00 00 00 00)
	} hdr;

	// the palette for the image
	struct SPalette
	{
		SPalette()
		{
			setupHueCircle();
		}

		void setupHueCircle()
		{
			setEntry(0x0, 0x80, 0x00, 0x80, 0x00);
			setEntry(0x1, 0xB0, 0x00, 0x50, 0x00);
			setEntry(0x2, 0xE0, 0x00, 0x20, 0x00);
			setEntry(0x3, 0xF0, 0x10, 0x00, 0x00);
			setEntry(0x4, 0xC0, 0x40, 0x00, 0x00);
			setEntry(0x5, 0x90, 0x70, 0x00, 0x00);
			setEntry(0x6, 0x60, 0xA0, 0x00, 0x00);
			setEntry(0x7, 0x30, 0xD0, 0x00, 0x00);
			setEntry(0x8, 0x00, 0xFF, 0x00, 0x00);
			setEntry(0x9, 0x00, 0xD0, 0x30, 0x00);
			setEntry(0xa, 0x00, 0xA0, 0x60, 0x00);
			setEntry(0xb, 0x00, 0x70, 0x90, 0x00);
			setEntry(0xc, 0x00, 0x40, 0xC0, 0x00);
			setEntry(0xd, 0x00, 0x10, 0xF0, 0x00);
			setEntry(0xe, 0x20, 0x00, 0xE0, 0x00);
			setEntry(0xf, 0x50, 0x00, 0xB0, 0x00);
		}

		void setupFrom0To4()
		{
			setEntry(0x0, 0x00, 0x00, 0x00, 0x00);
			setEntry(0x1, 0x00, 0x00, 0xFF, 0x00);
			setEntry(0x2, 0x00, 0xFF, 0xFF, 0x00);
			setEntry(0x3, 0xFF, 0xFF, 0xFF, 0x00);
			setEntry(0x4, 0xFF, 0xFF, 0x00, 0x00);
			setEntry(0x5, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0x6, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0x7, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0x8, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0x9, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xa, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xb, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xc, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xd, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xe, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xf, 0xFF, 0x00, 0x00, 0x00);
		}

		void setupForCol()
		{
			setEntry(0x0, 0x00, 0x00, 0x00, 0x00);
			setEntry(0x1, 0x00, 0x00, 0x80, 0x00);
			setEntry(0x2, 0x00, 0x80, 0xFF, 0x00);
			setEntry(0x3, 0x00, 0xFF, 0xFF, 0x00);
			setEntry(0x4, 0x40, 0x00, 0x00, 0x00);
			setEntry(0x5, 0x40, 0x00, 0x80, 0x00);
			setEntry(0x6, 0x40, 0x80, 0xFF, 0x00);
			setEntry(0x7, 0x40, 0xFF, 0xFF, 0x00);
			setEntry(0x8, 0xA0, 0x00, 0x00, 0x00);
			setEntry(0x9, 0xA0, 0x00, 0x80, 0x00);
			setEntry(0xa, 0xA0, 0x80, 0xFF, 0x00);
			setEntry(0xb, 0xA0, 0xFF, 0xFF, 0x00);
			setEntry(0xc, 0xFF, 0x00, 0x00, 0x00);
			setEntry(0xd, 0xFF, 0x00, 0x80, 0x00);
			setEntry(0xe, 0xFF, 0x80, 0xFF, 0x00);
			setEntry(0xf, 0x00, 0x40, 0x00, 0x00);
		}

		void setupGreyScale()
		{
			setEntry(0x0, 0x00, 0x00, 0x00, 0x00);
			setEntry(0x1, 0x11, 0x11, 0x11, 0x00);
			setEntry(0x2, 0x22, 0x22, 0x22, 0x00);
			setEntry(0x3, 0x33, 0x33, 0x33, 0x00);
			setEntry(0x4, 0x44, 0x44, 0x44, 0x00);
			setEntry(0x5, 0x55, 0x55, 0x55, 0x00);
			setEntry(0x6, 0x66, 0x66, 0x66, 0x00);
			setEntry(0x7, 0x77, 0x77, 0x77, 0x00);
			setEntry(0x8, 0x88, 0x88, 0x88, 0x00);
			setEntry(0x9, 0x99, 0x99, 0x99, 0x00);
			setEntry(0xa, 0xaa, 0xaa, 0xaa, 0x00);
			setEntry(0xb, 0xbb, 0xbb, 0xbb, 0x00);
			setEntry(0xc, 0xcc, 0xcc, 0xcc, 0x00);
			setEntry(0xd, 0xdd, 0xdd, 0xdd, 0x00);
			setEntry(0xe, 0xee, 0xee, 0xee, 0x00);
			setEntry(0xf, 0xff, 0xff, 0xff, 0x00);
		}

		void setEntry(int idx, int r, int g, int b, int a)
		{
			rgba[4*idx+0]=char(r);
			rgba[4*idx+1]=char(g);
			rgba[4*idx+2]=char(b);
			rgba[4*idx+3]=char(a);
		}

		char rgba[16*4];
	} ColourPalette;

	// the main image data buffer
	char Data[W*H/2];

};


class CTGAImage
{
public:

	// Header
	uint8	IdLength;			// 0
	uint8	ColourMapType;		// 1
	uint8	DataTypeCode;		// 2
	uint16	ColourMapOrigin;	// 3
	uint16	ColourMapLength;	// 5
	uint8	ColourMapDepth;		// 7
	uint16	XOrigin;			// 8
	uint16	YOrigin;			// 10
	uint16	Width;				// 12
	uint16	Height;				// 16
	uint8	BitsPerPixel;		// 18
	uint8	ImageDescriptor;	// 19

	std::vector<uint16>	Line;

	std::vector<uint16>	ColorMap;

	NLMISC::COFile	File;


	CTGAImage() :	IdLength(0), ColourMapType(0), DataTypeCode(2),
					ColourMapOrigin(0), ColourMapLength(0), ColourMapDepth(0),
					XOrigin(0), YOrigin(0), BitsPerPixel(16),
					ImageDescriptor(0)
	{
	}

	void	setup(uint16 width, uint16 height, const std::string &filename, uint16 xorigin, uint16 yorigin)
	{
		Width = width;
		Height = height;
		XOrigin = xorigin;
		YOrigin = yorigin;

		Line.resize(Width);

		File.open(filename);

		File.serial(IdLength);
		File.serial(ColourMapType);
		File.serial(DataTypeCode);
		File.serial(ColourMapOrigin);
		File.serial(ColourMapLength);
		File.serial(ColourMapDepth);
		File.serial(XOrigin);
		File.serial(YOrigin);
		File.serial(Width);
		File.serial(Height);
		File.serial(BitsPerPixel);
		File.serial(ImageDescriptor);
	}

	static uint16	getColor16(uint blue, uint green, uint red)
	{
		return ((red>>3)<<10) + ((green>>3)<<5) + (blue>>3);
	}

	static uint16	getBlue( uint16 color16 )
	{
		return (color16 & 31) << 3;
	}

	static uint16	getGreen( uint16 color16 )
	{
		return ((color16 & 0x3E0) >> 5) << 3;
	}

	static uint16	getRed( uint16 color16 )
	{
		return ((color16 & 0x7C00) >> 10) << 3;
	}

	void setupForCol()
	{
		ColorMap.resize(16);
		ColorMap[0] = getColor16(0x00, 0x00, 0x00);
		ColorMap[1] = getColor16(0x00, 0x00, 0x80);
		ColorMap[2] = getColor16(0x00, 0x80, 0xFF);
		ColorMap[3] = getColor16(0x00, 0xFF, 0xFF);
		ColorMap[4] = getColor16(0x40, 0x00, 0x00);
		ColorMap[5] = getColor16(0x40, 0x00, 0x80);
		ColorMap[6] = getColor16(0x40, 0x80, 0xFF);
		ColorMap[7] = getColor16(0x40, 0xFF, 0xFF);
		ColorMap[8] = getColor16(0xA0, 0x00, 0x00);
		ColorMap[9] = getColor16(0xA0, 0x00, 0x80);
		ColorMap[10] = getColor16(0xA0, 0x80, 0xFF);
		ColorMap[11] = getColor16(0xA0, 0xFF, 0xFF);
		ColorMap[12] = getColor16(0xFF, 0x00, 0x00);
		ColorMap[13] = getColor16(0xFF, 0x00, 0x80);
		ColorMap[14] = getColor16(0xFF, 0x80, 0xFF);
		ColorMap[15] = getColor16(0x00, 0x40, 0x00);
	}

	void setup4BitGreyScale()
	{
		ColorMap.clear();
		for (uint32 i=0;i<0x10;++i)
		{
			uint32 value= 0x11*i;
			ColorMap.push_back(getColor16(value, value, value));
		}
	}

	void setup8BitGreyScale()
	{
		ColorMap.clear();
		for (uint32 i=0;i<256;++i)
		{
			ColorMap.push_back(getColor16(i, i, i));
		}
	}

	void	set(uint col, uint idx)
	{
		nlassert(!ColorMap.empty());
		Line[col] = ColorMap[idx%ColorMap.size()];
	}

	void	set(uint col, uint r, uint g, uint b)
	{
		Line[col] = getColor16(r, g, b);
	}

	void	set16(uint col, uint16 color16)
	{
		Line[col] = color16;
	}

	void	writeLine()
	{
		File.serialBuffer((uint8*)(&(Line[0])), (uint)Line.size()*2);
	}
};

class CTGAImageGrey: public CTGAImage
{
public:

	struct SGreyPixel { uint8 r,g,b; };
	std::vector<SGreyPixel>	GreyLine;

	CTGAImageGrey()
	{
		BitsPerPixel= 24;
//		ImageDescriptor= 32;
	}

	void set(uint32 x, uint32 greyValue)
	{
		if (GreyLine.size()<Line.size())
			GreyLine.resize(Line.size());

		nlassert(GreyLine.size()>x);

		GreyLine[x].r = (uint8)greyValue;
		GreyLine[x].g = (uint8)greyValue;
		GreyLine[x].b = (uint8)greyValue;
	}

	void writeLine()
	{
		File.serialBuffer((uint8*)(&(GreyLine[0])), (uint)GreyLine.size()*sizeof(GreyLine[0]));
	}
};

#endif // RY_BMP4IMAGE_H
