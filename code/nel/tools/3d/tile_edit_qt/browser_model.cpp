// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "common.h"
#include "browser_model.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

extern CTileBank tileBankBrowser;
static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};

static bool loadPic(const string &path, std::vector<NLMISC::CBGRA> &tampon, uint &width, uint &height)
{
	try
	{
		NLMISC::CIFile file;
		if (file.open(path.c_str()))
		{
			NLMISC::CBitmap bitmap;
			bitmap.load(file);
			width = bitmap.getWidth();
			height = bitmap.getHeight();
			tampon.resize(width * height);
			bitmap.convertToType(NLMISC::CBitmap::RGBA);
			for (uint y = 0; y < height; ++y)
			{
				for (uint x = 0; x < width; ++x)
				{
					NLMISC::CRGBA c = bitmap.getPixelColor(x, y, 0);
					c.R = (c.R * c.A) / 255;
					c.G = (c.G * c.A) / 255;
					c.B = (c.B * c.A) / 255;
					tampon[(y * width) + x] = c;
				}
			}
			return true;
		}
	}
	catch (const NLMISC::Exception& ) { }
	return false;
}

bool RemovePath (std::string& path, const char* absolutePathToRemplace);

// Rotate a buffer
void rotateBuffer (std::vector<NLMISC::CBGRA> &Buffer, uint &Width, uint &Height)
{
	// Make a copy
	std::vector<NLMISC::CBGRA> copy = Buffer;

	// Rotate
	for (uint y=0; y<Width; y++)
	{
		// Line offset
		uint tmp=y*Width;
		for (uint x=0; x<Width; x++)
		{
			Buffer[y+(Width-x-1)*Height]=copy[x+tmp];
		}
	}

	// New size
	uint tmp=Width;
	Width=Height;
	Height=tmp;
}

/////////////////////////////////////////////////////////////////////////////
// Load a Pic and apply Alpha & Rotation
int loadPixmapBuffer(const std::string& path, std::vector<NLMISC::CBGRA>& Buffer, std::vector<NLMISC::CBGRA>* Alpha, int rot)
{	
	uint Width;
	uint Height;
	if (loadPic(path, Buffer, Width, Height))
	{
		while (rot)
		{
			// Rotate the buffer
			rotateBuffer (Buffer, Width, Height);
			rot--;
		}

		if ( (Alpha) && (Alpha->size()==Buffer.size()) )
		{
			// Pre mul RGB componates by Alpha one
			int nPixelCount=(int)(Width*Height);
			for (int p=0; p<nPixelCount; p++)
			{
				// Invert alpha ?
				int alpha=(*Alpha)[p].A;
				Buffer[p].R=(uint8)(((int)Buffer[p].R*alpha)>>8);
				Buffer[p].G=(uint8)(((int)Buffer[p].G*alpha)>>8);
				Buffer[p].B=(uint8)(((int)Buffer[p].B*alpha)>>8);
			}
		}

		return 1;
	}
	else
		return 0;
}


/////////////////////////////////////////////////////////////////////////////
//TileInfo


TileInfo::TileInfo():tileType(UnSet)
{
	this->loaded = 0;
	this->nightLoaded = 0;
	this->alphaLoaded = 0;
}



TileInfo::TileInfo(int id, TileType tileType):id(id), tileType(tileType)
{
	this->loaded = 0;
	this->nightLoaded = 0;
	this->alphaLoaded = 0;
}

void TileInfo::Init(int id, TileType tileType)
{
	this->id = id;
	this->tileType = tileType;
	this->loaded = 0;
	this->nightLoaded = 0;
	this->alphaLoaded = 0;
}


bool TileInfo::Load (int index, std::vector<NLMISC::CBGRA>* Alpha)
{
	bool bRes=true;
	if (!loaded && !getRelativeFileName (Diffuse, index).empty())
	{
		path = fixPath(tileBankBrowser.getAbsPath() + getRelativeFileName (Diffuse, index));
		
		
	
		if ( !loadPixmapBuffer( path, Bits, Alpha, 0))
		{
			bRes=false;
			QMessageBox::information( NULL, QObject::tr("Can't load Diffuse file"),  QString( (path.c_str()) ));

		}
		else
			loaded=1;
	}
	if (!nightLoaded && !getRelativeFileName (Additive, index).empty())
	{
		nightPath = fixPath(tileBankBrowser.getAbsPath() + getRelativeFileName (Additive, index));
		if (!loadPixmapBuffer( nightPath, nightBits, Alpha, 0))
		{
			bRes=false;
			QMessageBox::information( NULL, QObject::tr("Can't load Additive file"),  QString( nightPath.c_str() ) );
		}
		else
			nightLoaded=1;
	}
	if (!alphaLoaded && !getRelativeFileName (::Alpha, index).empty())
	{
		alphaPath = fixPath(tileBankBrowser.getAbsPath() + getRelativeFileName (::Alpha, index));
		if (!loadPixmapBuffer( alphaPath, alphaBits, NULL, tileBankBrowser.getTile (index)->getRotAlpha ()))
		{
			bRes=false;
			QMessageBox::information( NULL, QObject::tr("Can't load Alpha file"),  QString( alphaPath.c_str() ) );

		}
		else
			alphaLoaded=1;
	}

	return bRes;
}

std::string TileInfo::fixPath(const std::string &path) {
	// Fix the Windows path issues for subpaths.
	std::string searchStr("\\");
	std::string replaceStr("/");
	string::size_type pos = 0;
	std::string pathStr = path;

	while( (pos = pathStr.find(searchStr, pos)) != std::string::npos) {
		pathStr.replace(pos, searchStr.size(), replaceStr);
		pos++;
	}
	return pathStr;
}

void TileInfo::Delete ()
{
	loaded=0;
	Bits.resize(0);

	nightLoaded=0;
	nightBits.resize(0);
	
	alphaLoaded=0;
	alphaBits.resize(0);
}

const std::string TileInfo::getRelativeFileName (TileTexture texture, int index)
{
	nlassert(this->tileType != UnSet);

	std::string currentPath;
	if (tileType != Displace)
	{
		currentPath = tileBankBrowser.getTile(index)->getRelativeFileName ((CTile::TBitmap)texture);
	}
	else
	{
		if (texture == Diffuse)
		{
			currentPath = tileBankBrowser.getDisplacementMap(index);

			//TODO titegus: remove MAGIC STRING "EmptyDisplacementMap" in Nel \tile_bank.cpp\void CTileNoise::setEmpty () or add an IsEmpty() method !!!!
			if (currentPath == "EmptyDisplacementMap")
				currentPath.clear();
		}	
	}

	return currentPath;
}



//TileList
TileList::TileList()
{
	_tileSet = -1;

	// Add 48 transitions
	int i;
	for (i=0; i<CTileSet::count; i++)
	{
		TileInfo info(i, Transition);
		theListTransition.push_back (info);
	}

	// Add 16 displacements
	for (i=0; i<CTileSet::CountDisplace; i++)
	{
		TileInfo info(i, Displace);
		theListDisplacement.push_back (info);
	}
}


int TileList::addTile128 ()
{

	int index;
	tileBankBrowser.getTileSet (_tileSet)->addTile128 (index, tileBankBrowser);
	nlassert (index==(sint)theList128.size());

	TileInfo info(index, _128x128);
	theList128.push_back (info);

	return index;
}

int TileList::addTile256 ()
{
	int index;
	tileBankBrowser.getTileSet (_tileSet)->addTile256 (index, tileBankBrowser);
	nlassert (index==(sint)theList256.size());

	TileInfo info(index, _256x256);
	theList256.push_back (info);

	return index;
}

bool TileList::setTile128 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated = name;
	if (RemovePath(troncated, tileBankBrowser.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!loadPic(tileBankBrowser.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't load bitmap."),  QString( ((tileBankBrowser.getAbsPath ()+troncated)+"\nContinue ?").c_str() ), QMessageBox::Yes | QMessageBox::No));
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);

			CTileSet::TError error;
			int pixel=-1;
			int composante=4;
			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBankBrowser.getTileSet(_tileSet)->checkTile128 (type, border, pixel, composante);

			if ((error!=CTileSet::ok)&&(error!=CTileSet::addFirstA128128))
			{
				QString pixelMessage = QObject::tr("%1\nPixel: %2(%3).\nContinue ?").arg(CTileSet::getErrorMessage (error)).arg(pixel).arg(comp[composante]);
				return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't set Bitmap"),  pixelMessage, QMessageBox::Yes | QMessageBox::No) );
			}
			else
			{
				if (error==CTileSet::addFirstA128128)
					tileBankBrowser.getTileSet(_tileSet)->setBorder (type, border);

				tileBankBrowser.getTileSet(_tileSet)->setTile128 (tile, troncated, type, tileBankBrowser);
				switch (type)
				{
				case CTile::diffuse:
					theList128[tile].loaded=0;
					break;
				case CTile::additive:
					theList128[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theList128[tile].alphaLoaded=0;
					break;
				default:
					break;
				}
				Reload(tile, tile + 1, _128x128);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		QString notInAbsolutePathMessage = QObject::tr("The bitmap %1 is not in the absolute path %2.\nContinue ?").arg(name.c_str()).arg(tileBankBrowser.getAbsPath ().c_str());
		return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Load error"),  notInAbsolutePathMessage, QMessageBox::Yes | QMessageBox::No));
	}

	return true;
}

bool TileList::setTile256 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBankBrowser.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!loadPic(tileBankBrowser.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't load bitmap."),  QString( ((tileBankBrowser.getAbsPath ()+troncated)+"\nContinue ?").c_str() ), QMessageBox::Yes | QMessageBox::No) );

		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);
			
			CTileSet::TError error;
			int pixel=-1;
			int composante=4;

			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBankBrowser.getTileSet(_tileSet)->checkTile256 (type, border, pixel, composante);
			if ((error!=CTileSet::ok))
			{
				QString pixelMessage = QObject::tr("%1\nPixel: %2(%3).\nContinue ?").arg(CTileSet::getErrorMessage (error)).arg(pixel).arg(comp[composante]);
				return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't set Bitmap"),  pixelMessage, QMessageBox::Yes | QMessageBox::No) );
			}
			else
			{
				tileBankBrowser.getTileSet(_tileSet)->setTile256 (tile, troncated, type, tileBankBrowser);
						switch (type)
				{
				case CTile::diffuse:
					theList256[tile].loaded=0;
					break;
				case CTile::additive:
					theList256[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theList256[tile].alphaLoaded=0;
					break;
				default:
					break;
				}
				Reload(tile, tile + 1, _256x256);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		QString notInAbsolutePathMessage = QObject::tr("The bitmap %1 is not in the absolute path %2.\nContinue ?").arg(name.c_str()).arg(tileBankBrowser.getAbsPath ().c_str());
		return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Load error"),  notInAbsolutePathMessage, QMessageBox::Yes | QMessageBox::No));
	}

	return true;
}

bool TileList::setTileTransition (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBankBrowser.getAbsPath ().c_str()))
	{
		// No alpha, use setTileTransitionAlpha
		nlassert (CTile::alpha!=type);

		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!loadPic(tileBankBrowser.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't load bitmap."),  QString( ((tileBankBrowser.getAbsPath ()+troncated)+"\nContinue ?").c_str() ), QMessageBox::Yes | QMessageBox::No) );
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);
			
			CTileSet::TError error;
			int pixel=-1;
			int composante=4;
			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBankBrowser.getTileSet(_tileSet)->checkTile128 (type, border, pixel, composante);
			if ((error!=CTileSet::ok)&&(error!=CTileSet::addFirstA128128))
			{
				QString pixelMessage = QObject::tr("%1\nPixel: %2(%3).\nContinue ?").arg(CTileSet::getErrorMessage (error)).arg(pixel).arg(comp[composante]);
				return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't set Bitmap"),  pixelMessage, QMessageBox::Yes | QMessageBox::No) );
			}
			else
			{
				if (error==CTileSet::addFirstA128128)
					tileBankBrowser.getTileSet(_tileSet)->setBorder (type, border);
				tileBankBrowser.getTileSet(_tileSet)->setTileTransition ((CTileSet::TTransition)tile, troncated, type, tileBankBrowser, border);
				switch (type)
				{
				case CTile::diffuse:
					theListTransition[tile].loaded=0;
					break;
				case CTile::additive:
					theListTransition[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theListTransition[tile].alphaLoaded=0;
					break;
				default:
					break;
				}
				Reload(tile, tile + 1, Transition);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		QString notInAbsolutePathMessage = QObject::tr("The bitmap %1 is not in the absolute path %2.\nContinue ?").arg(name.c_str()).arg(tileBankBrowser.getAbsPath ().c_str());
		return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Load error"),  notInAbsolutePathMessage, QMessageBox::Yes | QMessageBox::No) );
	}

	return true;
}

bool TileList::setDisplacement (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBankBrowser.getAbsPath ().c_str()))
	{

		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!loadPic(tileBankBrowser.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't load bitmap"),  QString( ((tileBankBrowser.getAbsPath ()+troncated)+"\nContinue ?").c_str() ), QMessageBox::Yes | QMessageBox::No) );
		}
		else
		{
			// Check the size
			if ( (Width!=32) || (Height!=32) )
			{
				// Error message
				return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't set Bitmap"), QString( (troncated+"\nInvalid size: displacement map must be 32x32 8 bits.\nContinue ?").c_str()), QMessageBox::Yes | QMessageBox::No) );
			}
			else
			{
				// change the file name of the displacement map
				tileBankBrowser.getTileSet(_tileSet)->setDisplacement ((CTileSet::TDisplacement)tile, troncated, tileBankBrowser);

				// Loaded
				theListDisplacement[tile].loaded=0;
				Reload(tile, tile + 1, Displace);
			}
		}

	}
	else
	{
		// Error: bitmap not in the absolute path..
		QString notInAbsolutePathMessage = QObject::tr("The bitmap %1 is not in the absolute path %2.\nContinue ?").arg(name.c_str()).arg(tileBankBrowser.getAbsPath ().c_str());
		return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Load error"),  notInAbsolutePathMessage, QMessageBox::Yes | QMessageBox::No) );
	}

	return true;
}

bool TileList::setTileTransitionAlpha (int tile, const std::string& name, int rot)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBankBrowser.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!loadPic(tileBankBrowser.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't load bitmap"),  QString( ((tileBankBrowser.getAbsPath ()+troncated)+"\nContinue ?").c_str() ), QMessageBox::Yes | QMessageBox::No) );
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);

			// rotate the border
			int rotBis=rot;
			while (rotBis)
			{
				border.rotate ();
				rotBis--;
			}
			
			CTileSet::TError error;
			int indexError;
			int pixel=-1;
			int composante=4;
			if (((error=tileBankBrowser.getTileSet(_tileSet)->checkTileTransition ((CTileSet::TTransition)tile, CTile::alpha, border, indexError,
				pixel, composante))!=CTileSet::ok))
			{
				QString pixelMessage;
				if ((error==CTileSet::topInterfaceProblem)||(error==CTileSet::bottomInterfaceProblem)||(error==CTileSet::leftInterfaceProblem)
					||(error==CTileSet::rightInterfaceProblem)||(error==CTileSet::topBottomNotTheSame)||(error==CTileSet::rightLeftNotTheSame)
					||(error==CTileSet::topInterfaceProblem))
				{
					if (indexError!=-1)
						pixelMessage = QObject::tr("%1.\nIncompatible with tile nb %4.\nPixel: %2(%3).\nContinue ?").arg(CTileSet::getErrorMessage (error)).arg(pixel).arg(comp[composante]).arg(indexError);
					else
						pixelMessage = QObject::tr("%1.\nIncompatible with the 128x128 tile.\nPixel: %2(%3).\nContinue ?").arg(CTileSet::getErrorMessage (error)).arg(pixel).arg(comp[composante]);

				}
				else
					pixelMessage = QObject::tr("%1.\nIncompatible filled tile.\nContinue ?").arg(CTileSet::getErrorMessage (error));
				
				return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Can't set Bitmap"),  pixelMessage, QMessageBox::Yes | QMessageBox::No) );
			}
			else
			{
				tileBankBrowser.getTileSet(_tileSet)->setTileTransitionAlpha ((CTileSet::TTransition)tile, troncated, tileBankBrowser, border, rot);
				theListTransition[tile].alphaLoaded=0;
				theListTransition[tile].Load (tileBankBrowser.getTileSet(_tileSet)->getTransition(tile)->getTile(), NULL);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		QString notInAbsolutePathMessage = QObject::tr("The bitmap %1 is not in the absolute path %2.\nContinue ?").arg(name.c_str()).arg(tileBankBrowser.getAbsPath ().c_str());
		return ( QMessageBox::Yes == QMessageBox::question( NULL, QObject::tr("Load error"),  notInAbsolutePathMessage, QMessageBox::Yes | QMessageBox::No) );
	}

	return true;
}

void TileList::removeTile128 (int index)
{
	tileBankBrowser.getTileSet (_tileSet)->removeTile128 (index, tileBankBrowser);
	theList[0].erase (theList[0].begin()+index);
	for (int i=0; i<(sint)theList[0].size(); i++)
	{
		theList[0][i].setId(i);
	}
}

void TileList::removeTile256 (int index)
{
	tileBankBrowser.getTileSet (_tileSet)->removeTile256 (index, tileBankBrowser);
	theList[1].erase (theList[1].begin()+index);
	for (int i=0; i<(sint)theList[1].size(); i++)
	{
		theList[1][i].setId(i);
	}
}

void TileList::clearTile128 (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
		case CTile::diffuse:
			theList128[index].loaded=0;
			theList128[index].Bits.resize(0);
			break;
		case CTile::additive:
			theList128[index].nightLoaded=0;
			theList128[index].nightBits.resize(0);
			break;
		case CTile::alpha:
			nlassert(0);
			break;
		default:
			break;
	}
	tileBankBrowser.getTileSet (_tileSet)->clearTile128 (index, bitmap, tileBankBrowser);
}

void TileList::clearTile256 (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
		case CTile::diffuse:
			theList256[index].loaded=0;
			theList256[index].Bits.resize(0);
			break;
		case CTile::additive:
			theList256[index].nightLoaded=0;
			theList256[index].nightBits.resize(0);
			break;
		case CTile::alpha:
			nlassert(0);
			break;
		default:
			break;
	}
	tileBankBrowser.getTileSet (_tileSet)->clearTile256 (index, bitmap, tileBankBrowser);
}

void TileList::clearTransition (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
		case CTile::diffuse:
			theListTransition[index].loaded=0;
			theListTransition[index].Bits.resize(0);
			break;
		case CTile::additive:
			theListTransition[index].nightLoaded=0;
			theListTransition[index].nightBits.resize(0);
			break;
		case CTile::alpha:
			theListTransition[index].alphaLoaded=0;
			theListTransition[index].alphaBits.resize(0);
			break;
		default:
			break;
	}
	tileBankBrowser.getTileSet (_tileSet)->clearTransition ((CTileSet::TTransition)index, bitmap, tileBankBrowser);
}

void TileList::clearDisplacement (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
		case CTile::diffuse:
			theListDisplacement[index].loaded=0;
			theListDisplacement[index].Bits.resize(0);
			break;
		case CTile::additive:
			nlassert(0);
			break;
		case CTile::alpha:
			nlassert(0);
			break;
		default:
			break;
	}

	tileBankBrowser.getTileSet (_tileSet)->clearDisplacement ((CTileSet::TDisplacement)index, tileBankBrowser);
}

tilelist::iterator TileList::GetFirst(int n)
{
	return theList[n].begin();
}

tilelist::iterator TileList::GetLast(int n)
{
	return theList[n].end();
}

tilelist::iterator TileList::Get(int i, int n)
{
	return theList[n].begin()+i;
}


int TileList::GetSize(int n)
{
	return (int)theList[n].size();
}


void TileList::Reload(int first, int last, TileType n) //recharge en memoire une tranche de tiles
{
	for (int i=first; i<last; i++)
	{
		switch (n)
		{
			case _128x128:
			{
				theList[n][i].Load (tileBankBrowser.getTileSet(_tileSet)->getTile128 (i), NULL);
				break;
			}
			case _256x256:
			{
				theList[n][i].Load (tileBankBrowser.getTileSet(_tileSet)->getTile256 (i), NULL);
				break;
			}
			case Transition:
			{
				int index=tileBankBrowser.getTileSet(_tileSet)->getTransition (i)->getTile();
				if (index!=-1)
					theList[n][i].Load (index, &theListTransition[i].alphaBits);
				break;
			}
			case Displace:
			{
				int index=tileBankBrowser.getTileSet(_tileSet)->getDisplacementTile ((CTileSet::TDisplacement)i);
				if (index!=-1)
					theList[n][i].Load (index, NULL);
				break;
			}
			default:
				break;
		}
	}
}
