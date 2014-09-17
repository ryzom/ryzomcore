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

#include "screenshot_islands.h"

#include <game_share/scenario_entry_points.h>

#include <game_share/season.h>
#include <game_share/dir_light_setup.h>
#include <game_share/bmp4image.h>

#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/config_file.h>
#include <nel/misc/big_file.h>
#include <nel/misc/i18n.h>
#include <nel/misc/progress_callback.h>
#include <nel/misc/random.h>
#include <nel/misc/common.h>
#include <nel/3d/u_material.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_landscape.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/landscapeig_manager.h>
#include <nel/3d/u_material.h>

#include <nel/georges/u_form_loader.h>
#include <nel/georges/u_form.h>
#include <nel/georges/u_form_elm.h>

// AI share
#include <ai_share/world_map.h>

#include <nel/3d/material.h>

#include <math.h>

using namespace NLMISC;
using namespace NL3D;
using namespace std;
using namespace EGSPD;
using namespace NLGEORGES;
using namespace RYAI_MAP_CRUNCH;


// The 3d driver
UDriver        *driver = NULL;
CLandscapeIGManager		LandscapeIGManager;

uint			ScreenShotWidth;	
uint			ScreenShotHeight;

UMaterial sceneMaterial;

namespace R2
{

const TBufferEntry InteriorValue= (TBufferEntry)(~0u-1);
const TBufferEntry ValueBorder= (TBufferEntry)(~0u-2);
const uint32 BigValue= 15*5;
const float limitValue = 200.0;

//-------------------------------------------------------------------------------------------------
CScreenshotIslands::CScreenshotIslands()
{
	_BackColor = CRGBA(255, 255, 255, 255);
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::init()
{
	// Create a driver
	driver = UDriver::createDriver(0, true);
	nlassert(driver);

	sceneMaterial = driver->createMaterial();
	sceneMaterial.getObjectPtr()->setLighting(true);
	sceneMaterial.getObjectPtr()->setSpecular(CRGBA(255, 255, 255, 255));
	sceneMaterial.getObjectPtr()->setShininess(50);
	sceneMaterial.getObjectPtr()->setDiffuse(CRGBA(100, 100, 100, 255));
	sceneMaterial.getObjectPtr()->setEmissive(CRGBA(25, 25, 25, 255));

	// load and parse the configfile
	CConfigFile cf;
	cf.load("island_screenshots.cfg");

	// get the value of searchPaths
	CConfigFile::CVar * searchPaths = cf.getVarPtr("SearchPaths");
	if(searchPaths)
	{
		for(int i = 0; i < searchPaths->size(); i++)
		{
 			CPath::addSearchPath(searchPaths->asString(i).c_str(), true, false);
		}
	}
	CPath::remapExtension("dds", "tga", true);
	CPath::remapExtension("dds", "png", true);

	// get the scenario entry points file
	CConfigFile::CVar * epFile = cf.getVarPtr("CompleteIslandsFile");
	if(epFile)
	{
		_CompleteIslandsFile = epFile->asString();
	}

	// get the out directory path
	CConfigFile::CVar * outDir = cf.getVarPtr("OutDir");
	if(outDir)
	{
		_OutDirectory = outDir->asString();
	}

	// get the vegetation option
	CConfigFile::CVar * veget = cf.getVarPtr("Vegetation");
	if(veget)
	{
		_Vegetation = veget->asBool();
	}

	// get the vegetation option
	CConfigFile::CVar * inverseZTest = cf.getVarPtr("InverseZTest");
	if(inverseZTest)
	{
		_InverseZTest = inverseZTest->asBool();
	}

	// get list of continents
	CConfigFile::CVar * continents = cf.getVarPtr("Continents");
	vector<string> continentsName(continents->size());
	for(int i = 0; i < continents->size(); i++)
	{
 		continentsName[i] = continents->asString(i);	
	}
	
	// get continents data (light, coarseMesh,...)
	UFormLoader * formLoader;
	for(uint i=0; i<continentsName.size(); i++)
	{
		string georgeFileName = continentsName[i]+".continent";

		formLoader = UFormLoader::createLoader();
		CSmartPtr<UForm> form = formLoader->loadForm(CPath::lookup(georgeFileName).c_str());
		if(form)
		{
			CContinentData continentData;

			UFormElm &formRoot = form->getRootNode();

			const UFormElm *elm;
			if(formRoot.getNodeByName(&elm, "LightLandscapeDay") && elm)
			{
				CDirLightSetup	landscapeLightDay;
				landscapeLightDay.build(*elm);
				continentData.Ambiant = landscapeLightDay.Ambiant;
				continentData.Diffuse = landscapeLightDay.Diffuse;

				if(continentsName[i]=="r2_jungle" || continentsName[i]=="r2_forest" || continentsName[i]=="r2_roots")
				{
					continentData.Ambiant = CRGBA(255, 255, 255, 255);
					continentData.Diffuse = CRGBA(255, 255, 255, 255);
				}
			}

			formRoot.getValueByName(continentData.IGFile, "LandscapeIG");
			string zoneMin, zoneMax;
			formRoot.getValueByName(zoneMin, "ZoneMin");
			formRoot.getValueByName(zoneMax, "ZoneMax");

			getPosFromZoneName(zoneMin, continentData.ZoneMin);
			getPosFromZoneName(zoneMax, continentData.ZoneMax);

			string filename;
			if(formRoot.getValueByName(filename, "Ecosystem"))
			{
				UFormLoader *formLoaderEco = UFormLoader::createLoader();
				if(formLoaderEco)
				{		
					// Load the form
					CSmartPtr<UForm> formEco = formLoaderEco->loadForm(filename.c_str());

					if(formEco)
					{
						// Root node
						UFormElm &formRootEco = formEco->getRootNode();

						// Small bank.
						formRootEco.getValueByName(continentData.SmallBank, "SmallBank");

						// Far bank.
						formRootEco.getValueByName(continentData.FarBank, "FarBank");

						// Coarse mesh texture.
						formRootEco.getValueByName(continentData.CoarseMeshMap, "CoarseMeshMap");
					}
					else 
					{
						nlwarning("CScreenshotIslands::init : Can't load form %s.", filename.c_str());
					}
					UFormLoader::releaseLoader(formLoaderEco);
				}		
			}

			_ContinentsData[continentsName[i]] = continentData;
		}
		UFormLoader::releaseLoader(formLoader);
	}
	

	// load islands
	loadIslands();

	searchIslandsBorders();

	// get seasons
	CConfigFile::CVar * seasonSuffixes = cf.getVarPtr("SeasonSuffixes");
	if(seasonSuffixes)
	{
		for(uint i = 0; i < (uint)seasonSuffixes->size(); i++)
		{
 			_SeasonSuffixes.push_back(seasonSuffixes->asString(i));
		}
	}
	
	// get the meter size in pixels
	CConfigFile::CVar * meterSize = cf.getVarPtr("MeterPixelSize");
	if(meterSize)
	{
		_MeterPixelSize = meterSize->asInt();
	}
}

//-------------------------------------------------------------------------------------------------
bool CScreenshotIslands::getPosFromZoneName(const std::string &name, NLMISC::CVector2f &dest)
{
	if(name.empty())
	{
		nlwarning ("getPosFromZoneName(): empty name, can't getPosFromZoneName");
		return false;
	}

	static std::string zoneName;
	static string xStr, yStr;
	xStr.clear();
	yStr.clear();
	zoneName = CFile::getFilenameWithoutExtension(name);
	uint32 i = 0;
	while(zoneName[i] != '_')
	{
		if(!::isdigit(zoneName[i])) return false;
		yStr += zoneName[i]; ++i;
		if(i == zoneName.size())
			return false;
	}	
	++i;
	while(i < zoneName.size())
	{
		if(!::isalpha(zoneName[i])) return false;
		xStr += (char) ::toupper(zoneName[i]); ++i;
	}
	if(xStr.size() != 2) return false;
	dest.x = 160.f * ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	dest.y = 160.f * -atoi(yStr.c_str());
	return true;
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::searchIslandsBorders()
{
	vector<string> filenames;
	list<string> zonelFiles;

	map< CVector2f, bool> islandsMap;
	
	TContinentsData::iterator itCont(_ContinentsData.begin()), lastCont(_ContinentsData.end());
	for( ; itCont != lastCont ; ++itCont)
	{
		// for each continent we recover a map of zonel files whith position of 
		// left/bottom point of each zone for keys
		filenames.clear();
		zonelFiles.clear();

		string bnpFileName = itCont->first + ".bnp";
		CBigFile::getInstance().list(bnpFileName.c_str(), filenames);

		for(uint i=0; i<filenames.size(); i++)
		{
			if(CFile::getExtension(filenames[i]) == "zonel")
			{
				zonelFiles.push_back(filenames[i]);
			}
		}

		list<string>::iterator itZonel(zonelFiles.begin()), lastZonel(zonelFiles.end());
		for( ; itZonel != lastZonel ; ++itZonel)
		{	
			CVector2f position;
			getPosFromZoneName(*itZonel, position);
			
			islandsMap[position] = true;
		}

		// search for island borders
		CContinentData & continent = itCont->second;
		list< string >::const_iterator itIsland(continent.Islands.begin()), lastIsland(continent.Islands.end());
		for( ; itIsland != lastIsland ; ++itIsland)
		{
			if(_IslandsData.find(itIsland->c_str()) != _IslandsData.end())
			{
				const CProximityZone & islandData = _IslandsData[itIsland->c_str()];

				sint32 xmin = islandData.getBoundXMin();
				sint32 xmax = islandData.getBoundXMax();
				sint32 ymin = islandData.getBoundYMin();
				sint32 ymax = islandData.getBoundYMax();

				sint32 width = xmax-xmin;
				sint32 height = ymax-ymin;

				sint32 zonelXMin = ((uint)(xmin/160)) * 160;
				sint32 zonelYMin = ((uint)(ymin/160) - 1) * 160;
				sint32 zonelXMax = ((uint)(xmax/160)) * 160;
				sint32 zonelYMax = ((uint)(ymax/160) - 1) * 160;

				list< CVector2f > leftBorders, rightBorders, bottomBorders, topBorders;
				
				// search for left and right borders on lines
				for(sint32 y = zonelYMin; y<=zonelYMax; y+=160 )
				{
					sint32 x=zonelXMin;
					CVector2f vec((float)x, (float)y);
					bool lastZoneFull = (islandsMap.find(vec) != islandsMap.end());
					bool currentZoneFull;

					while(x<=zonelXMax)
					{
						vec = CVector2f((float)x, (float)y);
						currentZoneFull = (islandsMap.find(vec) != islandsMap.end());
						
						if(lastZoneFull && !currentZoneFull && vec.x-1 >= xmin)
						{
							rightBorders.push_back(CVector2f(vec.x-1, vec.y));
						}
						else if(!lastZoneFull && currentZoneFull)
						{
							leftBorders.push_back(vec);
						}
						x += 160;
						lastZoneFull = currentZoneFull;
					}
				}

				// search for bottom and top borders on columns
				for(sint32 x = zonelXMin; x<=zonelXMax; x+=160 )
				{
					sint32 y=zonelYMin;
					CVector2f vec((float)x, (float)y);
					bool lastZoneFull = (islandsMap.find(vec) != islandsMap.end());
					bool currentZoneFull;

					while(y<=zonelYMax)
					{
						vec = CVector2f((float)x, (float)y);
						currentZoneFull = (islandsMap.find(vec) != islandsMap.end());
						
						if(lastZoneFull && !currentZoneFull && vec.y-1 >= ymin)
						{
							topBorders.push_back(CVector2f(vec.x, vec.y-1));
						}
						else if(!lastZoneFull && currentZoneFull)
						{
							bottomBorders.push_back(vec);
						}
						y += 160;
						lastZoneFull = currentZoneFull;
					}
				}

				_BorderIslands[*itIsland + "/right"] = rightBorders;
				_BorderIslands[*itIsland + "/left"] = leftBorders;
				_BorderIslands[*itIsland + "/bottom"] = bottomBorders;
				_BorderIslands[*itIsland + "/top"] = topBorders;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::attenuateIslandBorders(const std::string & islandName, CBitmap & islandBitmap,
												const CProximityZone & islandData)
{
	list< CVector2f > leftBorders, rightBorders, bottomBorders, topBorders;
	rightBorders = _BorderIslands[islandName + "/right"];
	leftBorders = _BorderIslands[islandName + "/left"];
	bottomBorders = _BorderIslands[islandName + "/bottom"];
	topBorders = _BorderIslands[islandName + "/top"];

	sint32 xmin = islandData.getBoundXMin();
	sint32 xmax = islandData.getBoundXMax();
	sint32 ymin = islandData.getBoundYMin();
	sint32 ymax = islandData.getBoundYMax();

	sint32 width = xmax-xmin;
	sint32 height = ymax-ymin;
	uint8 *dest	= &(islandBitmap.getPixels(0)[0]);

	list< CVector2f >::iterator itBorder;

	for(itBorder=leftBorders.begin(); itBorder!=leftBorders.end(); itBorder++)
	{
		const CVector2f initPoint = *itBorder;
		sint32 x = (sint32)initPoint.x - xmin;
		sint32 y = (sint32)initPoint.y - ymin;
		sint32 maxBorder = 160;
		if(y<0)
		{
			maxBorder += y;
			y = 0;
		}
		sint32 inity = y;

		while(y<(inity+maxBorder) && y<(sint32)height)
		{
			double noiseValue = (1+cos(((y-inity)*2*Pi)/maxBorder))*5;
			double evalNoise = 10 + noiseValue;
			double diffAlpha = 255/evalNoise;

			CRGBA color = islandBitmap.getPixelColor(x, height-y-1);
			sint32 currentX = x-1;

			while((currentX>=x-evalNoise) && currentX>=0)
			{
				uint8 *pixel = &(islandBitmap.getPixels(0)[((height-y-1)*width + currentX)*4]);
				uint alpha = (uint)(255-diffAlpha*(x-currentX));
				uint invAlpha = 255-alpha;

				*pixel = (uint8) (((invAlpha * *pixel)		+ (alpha * color.R)) >> 8);
				*(pixel + 1) = (uint8) (((invAlpha * *(pixel + 1)) + (alpha * color.G)) >> 8);
				*(pixel + 2) = (uint8) (((invAlpha * *(pixel + 2)) + (alpha * color.B)) >> 8);
				*(pixel + 3)  = (uint8) 255;

				currentX--;
			}
			y++;
		}
	}

	for(itBorder=rightBorders.begin(); itBorder!=rightBorders.end(); itBorder++)
	{
		const CVector2f initPoint = *itBorder;
		sint32 x = (sint32)initPoint.x - xmin;
		sint32 y = (sint32)initPoint.y - ymin;
		sint32 maxBorder = 160;
		if(y<0)
		{
			maxBorder += y;
			y = 0;
		}
		sint32 inity = y;

		while(y<(inity+maxBorder) && y<(sint32)height)
		{
			double noiseValue = (1+cos(((y-inity)*2*Pi)/maxBorder))*5;
			double evalNoise = 10 + noiseValue;
			double diffAlpha = 255/evalNoise;

			CRGBA color = islandBitmap.getPixelColor(x, height-y-1);
			sint32 currentX = x+1;

			while((currentX<=x+evalNoise) && currentX<width)
			{
				uint8 *pixel = &(islandBitmap.getPixels(0)[((height-y-1)*width + currentX)*4]);
				uint alpha = (uint)(255-diffAlpha*(currentX-x));
				uint invAlpha = 255-alpha;

				*pixel = (uint8) (((invAlpha * *pixel)		+ (alpha * color.R)) >> 8);
				*(pixel + 1) = (uint8) (((invAlpha * *(pixel + 1)) + (alpha * color.G)) >> 8);
				*(pixel + 2) = (uint8) (((invAlpha * *(pixel + 2)) + (alpha * color.B)) >> 8);
				*(pixel + 3)  = (uint8) 255;

				currentX++;
			}

			y++;
		}
	}
	
	for(itBorder=bottomBorders.begin(); itBorder!=bottomBorders.end(); itBorder++)
	{
		const CVector2f initPoint = *itBorder;
		sint32 x = (sint32)initPoint.x - xmin;
		sint32 y = (sint32)initPoint.y - ymin;
		sint32 maxBorder = 160;
		if(x<0)
		{
			maxBorder += x;
			x = 0;
		}
		sint32 initx = x;
		
		while(x<(initx+maxBorder) && x<(sint32)width)
		{
			double noiseValue = (1+cos(((x-initx)*2*Pi)/maxBorder))*5;
			double evalNoise = 10 + noiseValue;
			double diffAlpha = 255/evalNoise;

			CRGBA color = islandBitmap.getPixelColor(x, height-y-1);
			sint32 currentY = y-1;

			while((currentY>=y-evalNoise) && currentY>=0)
			{
				uint8 *pixel = &(islandBitmap.getPixels(0)[((height-currentY-1)*width + x)*4]);
				uint alpha = (uint)(255-diffAlpha*(y-currentY));
				uint invAlpha = 255-alpha;

				*pixel = (uint8) (((invAlpha * *pixel)		+ (alpha * color.R)) >> 8);
				*(pixel + 1) = (uint8) (((invAlpha * *(pixel + 1)) + (alpha * color.G)) >> 8);
				*(pixel + 2) = (uint8) (((invAlpha * *(pixel + 2)) + (alpha * color.B)) >> 8);
				*(pixel + 3)  = (uint8) 255;

				currentY--;
			}

			x++;
		}
	}

	for(itBorder=topBorders.begin(); itBorder!=topBorders.end(); itBorder++)
	{
		const CVector2f initPoint = *itBorder;
		sint32 x = (sint32)initPoint.x - xmin;
		sint32 y = (sint32)initPoint.y - ymin;
		sint32 maxBorder = 160;
		if(x<0)
		{
			maxBorder += x;
			x = 0;
		}
		sint32 initx = x;
		
		while(x<(initx+maxBorder) && x<(sint32)width)
		{
			double noiseValue = (1+cos(((x-initx)*2*Pi)/maxBorder))*5;
			double evalNoise = 10 + noiseValue;
			double diffAlpha = 255/evalNoise;

			CRGBA color = islandBitmap.getPixelColor(x, height-y-1);
			sint32 currentY = y+1;

			while((currentY<=y+evalNoise) && currentY<height)
			{
				uint8 *pixel = &(islandBitmap.getPixels(0)[((height-currentY-1)*width + x)*4]);
				uint alpha = (uint)(255-diffAlpha*(currentY-y));
				uint invAlpha = 255-alpha;

				*pixel = (uint8) (((invAlpha * *pixel)		+ (alpha * color.R)) >> 8);
				*(pixel + 1) = (uint8) (((invAlpha * *(pixel + 1)) + (alpha * color.G)) >> 8);
				*(pixel + 2) = (uint8) (((invAlpha * *(pixel + 2)) + (alpha * color.B)) >> 8);
				*(pixel + 3)  = (uint8) 255;

				currentY++;
			}

			x++;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::buildScreenshots()
{
	init();
	buildIslandsTextures();
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::writeProximityBufferToTgaFile(const std::string& fileName,const TBuffer& buffer,uint32 scanWidth,uint32 scanHeight)
{
	uint	imageWidth = (scanWidth); // (scanWidth+15)&~15;
	uint	imageHeight = (scanHeight);

	CTGAImageGrey tgaImage;
	tgaImage.setup((uint16)imageWidth, (uint16)imageHeight, fileName, 0, 0);
	for (uint32 y=0;y<scanHeight;++y)
	{
		for (uint32 x=0; x<scanWidth; ++x)
		{
			uint32 value= buffer[y*scanWidth+x];
			tgaImage.set(x, (value>255*5)?255:value/5);
		}
		tgaImage.writeLine();
	}
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::processProximityBuffer(TBuffer & inputBuffer, uint32 lineLength, TBuffer& resultBuffer)
{
	// a couple of constants to control the range over which our degressive filter is to be applied
	const uint32 smallValue= 2*5;

	float a = 5*((255.0 - limitValue)/(float(100-BigValue)));
	float b = (float)(limitValue*5 - a*BigValue);

	// determine numer of lines in the buffer...
	uint32 numLines= inputBuffer.size()/ lineLength;

	// clear out the result buffer and reset all values to 5*255, remembering that this is the correct value for the image edges
	resultBuffer.clear();
	resultBuffer.resize(inputBuffer.size(),(TBufferEntry)5*255);

	for (uint32 y=1;y<numLines-1;++y)
	{
		uint32 lineOffset= y* lineLength;
		for (uint32 x=1;x<lineLength-1;++x)
		{
			uint32 offset= lineOffset+x;
			
			uint32 value=(uint32)inputBuffer[offset];

			// apply a clip and cosine function
			/*
			if (value<smallValue) value=0;
			else if (value>BigValue) value=5*255;
			else value= (uint32)(((1.0-cos(Pi*(float(value-smallValue)/(float)(BigValue-smallValue))))/2.0)*float(5*255));
			*/

			if (value==ValueBorder);
			else if ((value>=0) && (value<smallValue)) value=0;
			else if (value>BigValue) 
			{
				if(value==InteriorValue) 
				{
					value = (uint)(5*limitValue);	
				}
				else
				{
					value = (uint)(a*value+b);
					if(value > 5*255) value = 5*255; 
				}
			}
			else if((value>=smallValue) && (value<=BigValue))
			{
				value= (uint32)(((1.0-cos(Pi*(float(value-smallValue)/(float)(BigValue-smallValue))))/2.0)*float(5*limitValue));
			}

			// store the value into the result buffer
			resultBuffer[offset]= (TBufferEntry)value;
		}
	}

	// modify inputBuffer to store "bigValue" limit
	for (uint32 y=1;y<numLines-1;++y)
	{
		uint32 lineOffset= y* lineLength;
		for (uint32 x=1;x<lineLength-1;++x)
		{
			uint32 offset= lineOffset+x;

			uint32 value=(uint32)inputBuffer[offset];

			if(value==BigValue) value = 255*5;
			else value = 0;
			
			inputBuffer[offset]= (TBufferEntry)value;
		}
	}

	/*
	// lines
	for (uint32 y=0;y<numLines;++y)
	{
		uint32 lineOffset= y* lineLength;

		for (uint32 x=0;x<lineLength;++x)
		{
			uint32 offset = lineOffset+x;
			uint16 value = resultBuffer[offset];
			
			if(value==ValueBorder)
			{
				resultBuffer[offset]= (TBufferEntry)(5*limitValue);
				
				// increase x
				if(x+1<lineLength-1)
				{
					uint16 nextValue = resultBuffer[offset+1];
					uint16 prevValue = (TBufferEntry)(5*limitValue);
					if(nextValue!=(TBufferEntry)(5*limitValue))
					{
						uint32 count = x+1;	
						while((count<x+7) && (count<lineLength-1) && ((nextValue=resultBuffer[lineOffset+count])!=0) 
							&& nextValue!=(TBufferEntry)(5*limitValue) && (nextValue!=ValueBorder))
						{
							uint16 newValue = (TBufferEntry)(prevValue+5*7);
							if(newValue < nextValue)
							{
								resultBuffer[lineOffset+count] = newValue;
							}
							
							prevValue = newValue;
							count++;
						}

						x = count-1;
						break;
					}
				}

				// decrease x
				if(x-1>0)
				{
					uint16 nextValue = resultBuffer[offset-1];
					uint16 prevValue = (TBufferEntry)(5*limitValue);
					if(nextValue!=(TBufferEntry)(5*limitValue))
					{
						uint32 count = x-1;	
						while((count>x-7) && (count>0) && ((nextValue=resultBuffer[lineOffset+count])!=0) 
							&& nextValue!=(TBufferEntry)(5*limitValue) && (nextValue!=ValueBorder))
						{
							uint16 newValue = (TBufferEntry)(prevValue+5*7);
							if(newValue < nextValue)
							{
								resultBuffer[lineOffset+count] = newValue;
							}
							
							prevValue = newValue;
							count--;
						}
					}
				}
			}
		}
	}

	//columns
	for (uint32 x=0;x<lineLength;++x)
	{
		// setup start and end offsets marking first and last 'zero' value pixels in the column
		uint32 startOffset=x, endOffset=x+(numLines-1)*lineLength;
	
		for (uint32 offset=startOffset; offset<=endOffset; offset+=lineLength)
		{
			uint16 value = resultBuffer[offset];
			
			if(value==ValueBorder)
			{
				resultBuffer[offset]= (TBufferEntry)(5*limitValue);

				// increase y
				if(offset+lineLength<=endOffset)
				{
					uint16 nextValue = resultBuffer[offset+lineLength];
					uint16 prevValue = (TBufferEntry)(5*limitValue);
					if(nextValue!=(TBufferEntry)(5*limitValue))
					{
						uint32 count = offset+lineLength;	
						while((count<offset+7*lineLength) && (count<=endOffset) && 
							((nextValue=resultBuffer[count])!=0) && nextValue!=(TBufferEntry)(5*limitValue) && (nextValue!=ValueBorder))
						{
							uint16 newValue = (TBufferEntry)(prevValue+5*7);
							if(newValue < nextValue)
							{
								resultBuffer[count] = newValue;
							}
							
							prevValue = newValue;
							count += lineLength;
						}

						offset = count-lineLength;
						break;
					}
				}

				// decrease y
				if(offset-lineLength>=startOffset)
				{
					uint16 nextValue = resultBuffer[offset-lineLength];
					uint16 prevValue = (TBufferEntry)(5*limitValue);
					if(nextValue!=(TBufferEntry)(5*limitValue))
					{
						uint32 count = offset-lineLength;	
						while((count>offset-7*lineLength) && (count>=startOffset) && 
							((nextValue=resultBuffer[count])!=0) && nextValue!=(TBufferEntry)(5*limitValue) && (nextValue!=ValueBorder))
						{
							uint16 newValue = (TBufferEntry)(prevValue+5*7);
							if(newValue < nextValue)
							{
								resultBuffer[count] = newValue;
							}
							
							prevValue = newValue;
							count -= lineLength;
						}
					}
				}
			}
		}
	}
	*/

	//-----------------------------------------------------------------------------
	// search for pixels of borders
	map< CVector2f, bool > bordersPixels;
	for (uint32 y=0;y<numLines;++y)
	{
		uint32 lineOffset= y* lineLength;

		for (uint32 x=0;x<lineLength;++x)
		{
			uint32 offset = lineOffset+x;
			uint16 value = resultBuffer[offset];
			
			if(value==ValueBorder)
			{
				bordersPixels[CVector2f((float)x, (float)y)] = true;
				resultBuffer[offset] = (TBufferEntry)(5*limitValue);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// search for global borders
	// search on lines for top and bottom borders
	map< CVector2f, uint32 > leftBorders, rightBorders, bottomBorders, topBorders;
	for (uint32 y=0;y<numLines;++y)
	{
		uint32 lineOffset= y* lineLength;

		bool lastValue = false;
		CVector2f firstPixelBorder;
		uint32 nbPixelsBorder = 0;

		for (uint32 x=0;x<lineLength;++x)
		{
			bool value = (bordersPixels.find(CVector2f((float)x, (float)y))!=bordersPixels.end());
			
			if(value)
			{
				if(!lastValue)
				{
					firstPixelBorder = CVector2f((float)x, (float)y);
				}
				nbPixelsBorder ++;
			}
			else if(lastValue)
			{
				// store border line
				if(nbPixelsBorder !=1) //column instead of line
				{
					bool top=false, bottom=false;
					if(y>0)
					{
						for(uint32 xc=(uint32)firstPixelBorder.x; xc<(uint32)(firstPixelBorder.x+nbPixelsBorder); xc++)
						{
							if(resultBuffer[(y-1)*lineLength+xc]==(TBufferEntry)(5*255))
							{
								bottom = true;
								break;
							}				
						}
					}

					if(y+1<numLines)
					{
						for(uint32 xc=(uint32)firstPixelBorder.x; xc<(uint32)(firstPixelBorder.x+nbPixelsBorder); xc++)
						{
							if(resultBuffer[(y+1)*lineLength+xc]==(TBufferEntry)(5*255))
							{
								top = true;
								break;
							}				
						}
					}

					// TOP
					if(top)
						topBorders[firstPixelBorder] =nbPixelsBorder;
					
					// BOTTOM
					if(bottom)
						bottomBorders[firstPixelBorder] =nbPixelsBorder;
				}
				nbPixelsBorder =0;
			}
			lastValue = value;
		}
	}

	// search on columns for left and right borders
	for (uint32 x=0;x<lineLength;++x)
	{
		bool lastValue = false;
		CVector2f firstPixelBorder;
		uint32 nbPixelsBorder = 0;
	
		for(uint32 y=0; y<numLines; y++)
		{
			bool value = (bordersPixels.find(CVector2f((float)x, (float)y))!=bordersPixels.end());
			
			if(value)
			{
				if(!lastValue)
				{
					firstPixelBorder = CVector2f((float)x, (float)y);
				}
				nbPixelsBorder ++;
			}
			else if(lastValue)
			{
				// store border line
				if(nbPixelsBorder !=1) //line instead of column
				{
					bool left = false;
					bool right = false;

					if(x>0)
					{
						for(uint32 yc=(uint32)firstPixelBorder.y; yc<(uint32)(firstPixelBorder.y+nbPixelsBorder); yc++)
						{
							if(resultBuffer[yc*lineLength+x-1]==(TBufferEntry)(5*255))
							{
								left = true;
								break;				
							}
						}
					}

					if(x+1<lineLength)
					{
						for(uint32 yc=(uint32)firstPixelBorder.y; yc<(uint32)(firstPixelBorder.y+nbPixelsBorder); yc++)
						{
							if(resultBuffer[yc*lineLength+x+1]==(TBufferEntry)(5*255))
							{
								right = true;
								break;								
							}
						}
					}

					// LEFT
					if(left)
						leftBorders[firstPixelBorder] =nbPixelsBorder;

					// RIGHT
					if(right)
						rightBorders[firstPixelBorder] =nbPixelsBorder;
				}
				nbPixelsBorder =0;
			}
			lastValue = value;
		}
	}

	//-----------------------------------------------------------------------------
	// attenuate borders
	// bottom and top borders (lines)
	for (uint32 y=0;y<numLines;++y)
	{
		uint32 lineOffset= y* lineLength;
		
		for (uint32 x=0;x<lineLength;++x)
		{
			uint32 offset = lineOffset+x;
			CVector2f firstPixelBorder((float)x, (float)y);

			// BOTTOM
			if(bottomBorders.find(CVector2f((float)x, (float)y))!=bottomBorders.end())
			{
				uint32 nbPixelsBorder = bottomBorders[firstPixelBorder];
				uint32 xc;
				for(xc=(uint32)firstPixelBorder.x; xc<(uint32)(firstPixelBorder.x+nbPixelsBorder); xc++)
				{
					uint nbZones = (uint)(nbPixelsBorder/160)+1;
					uint period = (uint)(nbPixelsBorder/(2*nbZones));
					double noiseValue = (1-cos(((xc-firstPixelBorder.x)*Pi)/period))*5;
					double evalNoise = 7 + noiseValue;
					double diffAlpha = (255.0-limitValue)/evalNoise;

					uint32 yc = y-1;
					while((yc>=(uint32)(y-evalNoise)) && yc>=0)
					{
						uint16 newValue = (TBufferEntry)(5*(limitValue + diffAlpha*(y-yc)));
						uint16 currentValue = (TBufferEntry)resultBuffer[yc*lineLength+xc];
						if(newValue<currentValue)
							resultBuffer[yc*lineLength+xc] = (TBufferEntry)(5*(limitValue + diffAlpha*(y-yc)));
						yc--;
					}
				}
				x=xc;
			}

			// TOP
			if(topBorders.find(CVector2f((float)x, (float)y))!=topBorders.end())
			{
				uint32 nbPixelsBorder = topBorders[firstPixelBorder];
				uint32 xc;
				for(xc=(uint32)firstPixelBorder.x; xc<=(uint32)(firstPixelBorder.x+nbPixelsBorder); xc++)
				{
					uint nbZones = (uint)(nbPixelsBorder/160)+1;
					uint period = (uint)(nbPixelsBorder/(2*nbZones));
					double noiseValue = (1-cos(((xc-firstPixelBorder.x)*Pi)/period))*5;
					double evalNoise = 7 + noiseValue;
					double diffAlpha = (255.0-limitValue)/evalNoise;

					uint32 yc = y+1;
					while((yc<=(uint32)(y+evalNoise)) && yc<numLines)
					{
						uint16 newValue = (TBufferEntry)(5*(limitValue + diffAlpha*(yc-y)));
						uint16 currentValue = (TBufferEntry)resultBuffer[yc*lineLength+xc];
						if(newValue<currentValue)
							resultBuffer[yc*lineLength+xc] = (TBufferEntry)(5*(limitValue + diffAlpha*(yc-y)));
						yc++;
					}
				}
				x=xc;
			}
		}
	}

	// left and right borders (columns)
	for (uint32 x=0;x<lineLength;++x)
	{
		for (uint32 y=0; y<numLines; y++)
		{
			CVector2f firstPixelBorder((float)x, (float)y);

			// LEFT
			if(leftBorders.find(CVector2f((float)x, (float)y))!=leftBorders.end())
			{
				uint32 nbPixelsBorder = leftBorders[firstPixelBorder];
				uint32 yc;
				for(yc=(uint32)firstPixelBorder.y; yc<(uint32)(firstPixelBorder.y+nbPixelsBorder); yc++)
				{
					uint nbZones = (uint)(nbPixelsBorder/160)+1;
					uint period = (uint)(nbPixelsBorder/(2*nbZones));
					double noiseValue = (1-cos(((yc-firstPixelBorder.y)*Pi)/period))*5;
					double evalNoise = 7 + noiseValue;
					double diffAlpha = (255.0-limitValue)/evalNoise;

					uint32 xc = x-1;
					while((xc>=(uint32)(x-evalNoise)) && xc>=0)
					{
						uint16 newValue = (TBufferEntry)(5*(limitValue + diffAlpha*(x-xc)));
						uint16 currentValue = (TBufferEntry)resultBuffer[yc*lineLength+xc];
						if(newValue<currentValue)
							resultBuffer[yc*lineLength+xc] = (TBufferEntry)(5*(limitValue + diffAlpha*(x-xc)));
						xc--;
					}
				}
				y=yc;
			}

			// RIGHT
			if(rightBorders.find(CVector2f((float)x, (float)y))!=rightBorders.end())
			{
				uint32 nbPixelsBorder = rightBorders[firstPixelBorder];
				uint32 yc;
				for(yc=(uint32)firstPixelBorder.y; yc<=(uint32)(firstPixelBorder.y+nbPixelsBorder); yc++)
				{
					uint nbZones = (uint)(nbPixelsBorder/160)+1;
					uint period = (uint)(nbPixelsBorder/(2*nbZones));
					double noiseValue = (1-cos(((yc-firstPixelBorder.y)*Pi)/period))*5;
					double evalNoise = 7 + noiseValue;
					double diffAlpha = (255.0-limitValue)/evalNoise;

					uint32 xc = x+1;
					while((xc<=(uint32)(x+evalNoise)) && xc<lineLength)
					{
						uint16 newValue = (TBufferEntry)(5*(limitValue + diffAlpha*(xc-x)));
						uint16 currentValue = (TBufferEntry)resultBuffer[yc*lineLength+xc];
						if(newValue<currentValue)
							resultBuffer[yc*lineLength+xc] = (TBufferEntry)(5*(limitValue + diffAlpha*(xc-x)));
						xc++;
					}
				}
				y=yc;
			}
		}
	}
}

//--------------------------------------------------------------------------------
void CScreenshotIslands::loadIslands()
{
	// load entryPoints
	map< string, CVector2f > islands;
	CScenarioEntryPoints scenarioEntryPoints = CScenarioEntryPoints::getInstance();

	scenarioEntryPoints.loadFromFile();
	const CScenarioEntryPoints::TEntryPoints& entryPoints =  scenarioEntryPoints.getEntryPoints();

	CScenarioEntryPoints::TEntryPoints::const_iterator entry(entryPoints.begin()), entryPoint(entryPoints.end());
	for( ; entry != entryPoint ; ++entry)
	{	
		islands[entry->Island] = CVector2f((float)entry->X, (float)entry->Y);
	}

	// search islands of each continent
	map< string, CVector2f >::iterator itIsland(islands.begin()), lastIsland(islands.end());
	for( ; itIsland != lastIsland ; ++itIsland)
	{	
		const CVector2f & entryPoint = itIsland->second;

		// search continent of this island
		TContinentsData::iterator itCont(_ContinentsData.begin()), lastCont(_ContinentsData.end());
		for( ; itCont != lastCont ; ++itCont)
		{
			CContinentData & continent = itCont->second;
			CVector2f zoneMax = continent.ZoneMax;
			CVector2f zoneMin = continent.ZoneMin;

			if((zoneMin.x <= entryPoint.x) && (entryPoint.x <= zoneMax.x)
				&& (zoneMax.y <= entryPoint.y) && (entryPoint.y <= zoneMin.y))
			{
				continent.Islands.push_back(itIsland->first);
				break;
			}
		}
	}

	// search data of each island
	TContinentsData::iterator itCont(_ContinentsData.begin()), lastCont(_ContinentsData.end());
	for( ; itCont != lastCont ; ++itCont)
	{
		string aiFileName = itCont->first+"_0.cwmap2";

		const CContinentData & continent = itCont->second;

		CProximityMapBuffer continentBuffer;
		continentBuffer.load(CPath::lookup(aiFileName));

		CProximityMapBuffer::TZones zones;
		continentBuffer.calculateZones(zones);

		for (uint32 i=0;i<zones.size();++i)
		{
			TBuffer zoneBuffer;
			continentBuffer.generateZoneProximityMap(zones[i], zoneBuffer); 

			TBuffer cleanBuffer;
			processProximityBuffer(zoneBuffer, zones[i].getZoneWidth(), cleanBuffer);

			string fileName = string("");
			list< string >::const_iterator itIsland(continent.Islands.begin()), lastIsland(continent.Islands.end());
			for( ; itIsland != lastIsland ; ++itIsland)
			{
				const CVector2f & entryPoint = islands[*itIsland];
				sint32 xmin = zones[i].getBoundXMin();
				sint32 xmax = zones[i].getBoundXMax();
				sint32 ymin = zones[i].getBoundYMin();
				sint32 ymax = zones[i].getBoundYMax();

				if((xmin <= entryPoint.x) && (entryPoint.x <= xmax)
					&& (ymin <= entryPoint.y) && (entryPoint.y <= ymax))
				{
					fileName = _OutDirectory + "/" + *itIsland + "_prox.tga";
					_IslandsData[*itIsland] = zones[i];
					break;
				}
			}
		
			// write the processed proximity map to an output file
			if(fileName != "")
			{
				writeProximityBufferToTgaFile(fileName, cleanBuffer, zones[i].getZoneWidth(), zones[i].getZoneHeight());
				_TempFileNames.push_back(fileName);

				fileName = _OutDirectory + "/" + *itIsland + "_limit.tga";
				writeProximityBufferToTgaFile(fileName, zoneBuffer, zones[i].getZoneWidth(), zones[i].getZoneHeight());
				_TempFileNames.push_back(fileName);
			}
			else
			{
				nlinfo("Zone of island not found, tga not build");
			}
		}
	}


	CScenarioEntryPoints::TCompleteIslands completeIslands(entryPoints.size());

	uint completeIslandsNb = 0;
	for(uint e=0; e<entryPoints.size(); e++)
	{	
		const CScenarioEntryPoints::CEntryPoint & entry = entryPoints[e];

		CScenarioEntryPoints::CCompleteIsland completeIsland;
		completeIsland.Island = entry.Island;
		completeIsland.Package = entry.Package;

		for(itCont=_ContinentsData.begin(); itCont!=_ContinentsData.end(); ++itCont)
		{
			list< string >::const_iterator itIsland(itCont->second.Islands.begin()), lastIsland(itCont->second.Islands.end());
			for( ; itIsland != lastIsland ; ++itIsland)
			{
				if(*itIsland == entry.Island)
				{
					completeIsland.Continent = CSString(itCont->first);
					if(_IslandsData.find(entry.Island)!=_IslandsData.end())
					{
						completeIsland.XMin = _IslandsData[entry.Island].getBoundXMin();
						completeIsland.YMin = _IslandsData[entry.Island].getBoundYMin();
						completeIsland.XMax = _IslandsData[entry.Island].getBoundXMax();
						completeIsland.YMax = _IslandsData[entry.Island].getBoundYMax();
						completeIslands[completeIslandsNb] = completeIsland;
						completeIslandsNb++;
					}
					break;
				}
			}
		}
	}
	completeIslands.resize(completeIslandsNb);
	
	CScenarioEntryPoints::getInstance().saveXMLFile(completeIslands, _CompleteIslandsFile);
}

//--------------------------------------------------------------------------------

void CScreenshotIslands::getBuffer(UScene * scene, ULandscape * landscape, CBitmap &btm)
{		
	//
	if (ScreenShotWidth && ScreenShotHeight)
	{		
		UCamera camera = scene->getCam();

		// Destination image
		CBitmap dest;
		btm.resize(ScreenShotWidth, ScreenShotHeight, CBitmap::RGBA);

		uint windowWidth = driver->getWindowWidth();
		uint windowHeight = driver->getWindowHeight();
		
		uint top;
		uint bottom = min(windowHeight, ScreenShotHeight);
		for (top=0; top<ScreenShotHeight; top+=windowHeight)
		{
			uint left;
			uint right = std::min(windowWidth, ScreenShotWidth);
			for(left=0; left<ScreenShotWidth; left+=windowWidth)
			{
				driver->clearBuffers(_BackColor);
				
				// store initial frustum and viewport
				CFrustum Frustum = scene->getCam().getFrustum();
				CViewport Viewport = scene->getViewport();

				// Build a new frustum
				CFrustum frustumPart;
				frustumPart.Left = Frustum.Left+(Frustum.Right-Frustum.Left)*((float)left/(float)ScreenShotWidth);
				frustumPart.Right = Frustum.Left+(Frustum.Right-Frustum.Left)*((float)right/(float)ScreenShotWidth);
				frustumPart.Top = ceil(Frustum.Top+(Frustum.Bottom-Frustum.Top)*((float)top/(float)ScreenShotHeight));
				frustumPart.Bottom = ceil(Frustum.Top+(Frustum.Bottom-Frustum.Top)*((float)bottom/(float)ScreenShotHeight));
				frustumPart.Near = Frustum.Near;
				frustumPart.Far = Frustum.Far;
				frustumPart.Perspective = Frustum.Perspective;

				// Build a new viewport
				CViewport viewport;
				viewport.init(0, 0, (float)(right-left)/windowWidth, 
					(float)(bottom-top)/windowHeight);
				
				// Activate all this
				scene->getCam().setFrustum(frustumPart);
				scene->setViewport(viewport);
				
				scene->setMaxSkeletonsInNotCLodForm(1000000);
				scene->setPolygonBalancingMode(UScene::PolygonBalancingOff);

				if(_InverseZTest)
				{
					// render scene with inversed ZBuffer test (keep greater distances)
					driver->setColorMask(false, false, false, false);
					sceneMaterial.setZFunc(UMaterial::less);

					// initialize ZBuffer with leak value
					driver->setMatrixMode2D11();
					CQuad quad;
					quad.V0 = CVector(0.0, 0.0, 0.0);
					quad.V1 = CVector(1.0, 0.0, 0.0);
					quad.V2 = CVector(1.0, 1.0, 0.0);
					quad.V3 = CVector(0.0, 1.0, 0.0);

					driver->drawQuad(quad, sceneMaterial);
					driver->setMatrixMode3D(camera);
					driver->setColorMask(true, true, true, true);

					scene->enableElementRender(UScene::FilterWater, false);
				}
				
				scene->render();
				
				// display vegetables with normal ZBuffer test
				if(_InverseZTest && _Vegetation)
				{
					scene->enableElementRender(UScene::FilterWater, false);
					scene->enableElementRender(UScene::FilterLandscape, false);
					scene->enableElementRender(UScene::FilterWater, true);
					scene->render();
					scene->enableElementRender(UScene::FilterLandscape, true);
				}

				// Get the bitmap
				driver->getBuffer(dest);
				btm.blit(dest, 0, windowHeight-(bottom-top), right-left, bottom-top, left, top);

				// restore camera
				scene->getCam().setFrustum(Frustum);
				scene->setViewport(Viewport);

				driver->flush();
				driver->swapBuffers();	

				// Next
				right = std::min(right+windowWidth, ScreenShotWidth);
			}

			// Next
			bottom = std::min(bottom+windowHeight, ScreenShotHeight);
		}
	}
	else
	{		
		driver->getBuffer(btm);
	}
}



void CScreenshotIslands::buildIslandsTextures()
{
	int loop = 0;
	int maxLoop = 6;

	// Create the window with config file values
	driver->setDisplay(UDriver::CMode(512, 512, 32, true));

	// Create a scene
	UScene	* scene = driver->createScene(true);
	scene->animate(CTime::ticksToSecond(CTime::getPerformanceTime()));
	scene->setMaxSkeletonsInNotCLodForm(1000000);
	scene->setPolygonBalancingMode(UScene::PolygonBalancingOff);
	
	// Create a camera
	UCamera camera = scene->getCam();
	camera.setTransformMode(UTransformable::DirectMatrix);

	// Create and load landscape
	ULandscape * landscape = scene->createLandscape();
	landscape->setThreshold(0.0005);
	if(_InverseZTest)
	{
		landscape->setZFunc(UMaterial::greaterequal);
	}
	else
	{
		landscape->setZFunc(UMaterial::less);
	}
	
	//Iteration on seasons
	list< string >::iterator itSeason(_SeasonSuffixes.begin()), lastSeason(_SeasonSuffixes.end());
	for( ; itSeason != lastSeason ; ++itSeason)
	{
		string seasonSuffix = *itSeason;

		int season;
		if(seasonSuffix=="_sp") season = CSeason::Spring;
		else if(seasonSuffix=="_su") season = CSeason::Summer;
		else if(seasonSuffix=="_au") season = CSeason::Autumn;
		else if(seasonSuffix=="_wi") season = CSeason::Winter;

		// Iterations on Continents
		TContinentsData::iterator itCont(_ContinentsData.begin()), lastCont(_ContinentsData.end());
		for( ; itCont != lastCont ; ++itCont)
		{	
			const CContinentData & continent = itCont->second;

			// Light init
			landscape->setupStaticLight(continent.Diffuse, continent.Ambiant, 1.0f);

			string coarseMeshFile = continent.CoarseMeshMap;
			string coarseMeshWithoutExt = CFile::getFilenameWithoutExtension(coarseMeshFile);
			string coarseMeshExt = CFile::getExtension(coarseMeshFile);
			coarseMeshFile = coarseMeshWithoutExt + seasonSuffix + "." + coarseMeshExt;
			nldebug("Coarse mesh texture: '%s'", coarseMeshFile.c_str());
			scene->setCoarseMeshManagerTexture(coarseMeshFile.c_str()); 

			// Load the landscape	
			string farBank = continent.FarBank;
			string farBankWithoutExt = CFile::getFilenameWithoutExtension(farBank);
			string farBankExt = CFile::getExtension(farBank);
			farBank = farBankWithoutExt + seasonSuffix + "." + farBankExt;
			landscape->loadBankFiles(continent.SmallBank, farBank);

			// Load vegatables
			LandscapeIGManager.initIG(scene, continent.IGFile, driver, season, NULL);
		
			// Iterations on Islands
			list< string >::const_iterator itIsland(continent.Islands.begin()), lastIsland(continent.Islands.end());
			for( ; itIsland != lastIsland ; ++itIsland)
			{
				loop = 0;

				if(_IslandsData.find(itIsland->c_str()) != _IslandsData.end())
				{
					const CProximityZone & islandData = _IslandsData[itIsland->c_str()];

					sint32 xmin = islandData.getBoundXMin();
					sint32 xmax = islandData.getBoundXMax();
					sint32 ymin = islandData.getBoundYMin();
					sint32 ymax = islandData.getBoundYMax();

					float width = (float)(xmax-xmin);
					float height = (float)(ymax-ymin);
					ScreenShotWidth = (uint)(width*_MeterPixelSize);	
					ScreenShotHeight = (uint)(height*_MeterPixelSize);

					// position in island center
					float posx = ((float)(xmax+xmin))/2;
					float posy = ((float)(ymax+ymin))/2;
					CVector entryPos(posx, posy, 400);
					
					// Setup camera
					CMatrix startCamMatrix;
					startCamMatrix.setPos(entryPos);
					startCamMatrix.rotateX(-(float)Pi/2);
					camera.setMatrix(startCamMatrix);
					camera.setFrustum(width, height, -10000.0f, 10000.0f, false);
					
					// init lanscape
					landscape->postfixTileFilename(seasonSuffix.c_str());
					
					while(loop<maxLoop)
					{
						// refresh lanscape
						vector<string>		zonesAdded;
						vector<string>		zonesRemoved;
						IProgressCallback progress;
						landscape->refreshAllZonesAround(camera.getMatrix().getPos(), 1000, zonesAdded, zonesRemoved, progress);
						if(_Vegetation)
						{
							LandscapeIGManager.unloadArrayZoneIG(zonesRemoved);
							LandscapeIGManager.loadArrayZoneIG(zonesAdded);
						
							vector<string>::iterator itName(zonesAdded.begin()), lastName(zonesAdded.end());
							for( ; itName != lastName ; ++itName)
							{
								UInstanceGroup	* instanceGr = LandscapeIGManager.getIG(*itName);
								if(instanceGr)
								{
									uint nbInst = instanceGr->getNumInstance();
									for(uint i=0; i<instanceGr->getNumInstance(); i++)
									{
										instanceGr->setCoarseMeshDist(i, 100000);
										instanceGr->setDistMax(i, 100000);
									}
								}
							}
						}
						scene->animate(CTime::ticksToSecond(CTime::getPerformanceTime()));
						
						// Clear all buffers
						driver->clearBuffers(_BackColor);

						if(_InverseZTest)
						{
							// render scene with inversed ZBuffer test (keep greater distances)
							driver->setColorMask(false, false, false, false);
							sceneMaterial.setZFunc(UMaterial::less);

							// initialize ZBuffer with leak value
							driver->setMatrixMode2D11();
							CQuad quad;
							quad.V0 = CVector(0.0, 0.0, 0.0);
							quad.V1 = CVector(1.0, 0.0, 0.0);
							quad.V2 = CVector(1.0, 1.0, 0.0);
							quad.V3 = CVector(0.0, 1.0, 0.0);

							driver->drawQuad(quad, sceneMaterial);
							driver->setMatrixMode3D(camera);
							driver->setColorMask(true, true, true, true);

							scene->enableElementRender(UScene::FilterWater, false);
						}

						scene->render();

						// display vegetables with normal ZBuffer test
						if(_InverseZTest && _Vegetation)
						{
							scene->enableElementRender(UScene::FilterLandscape, false);
							scene->enableElementRender(UScene::FilterWater, true);
							scene->render();
							scene->enableElementRender(UScene::FilterLandscape, true);
						}
						
						// Swap 3d buffers
						driver->flush();
						driver->swapBuffers();

						// Pump user input messages
						driver->EventServer.pump();

						loop += 1;

						// Screenshot
						if(loop==maxLoop-1)
						{
							CBitmap islandBitmap;
							getBuffer(scene, landscape, islandBitmap);
						
							buildBackTextureHLS(*itIsland, islandBitmap);
						}
						if(loop==maxLoop)
						{
							// create srcennshot bitmap of full island
							CBitmap islandBitmap;
							getBuffer(scene, landscape, islandBitmap);

							attenuateIslandBorders(*itIsland, islandBitmap, islandData);

							// load proximity bitmap
							CBitmap proxBitmap;
							std::string proxFileName = _OutDirectory + "/" + *itIsland + "_prox.tga";
							CIFile proxFS(proxFileName.c_str());
							proxBitmap.load(proxFS);

							
							// resize proximity bitmap
							CBitmap tempBitmap;
							int newWidth = islandBitmap.getWidth();
							int newHeight = islandBitmap.getHeight();
							tempBitmap.resize(newWidth, newHeight, islandBitmap.PixelFormat);
							// blit src bitmap
							//tempBitmap.blit(proxBitmap, 0, 0, newWidth, newHeight, 0, 0);
							{
								const uint8 *prox = &(proxBitmap.getPixels(0)[0]);
								uint8 *temp = &(tempBitmap.getPixels(0)[0]);
								for (uint y = 0; y < newHeight; ++y)
									for (uint x = 0; x < newWidth; ++x)
								{
									uint ys = (y * proxBitmap.getHeight()) / newHeight;
									uint xs = (x * proxBitmap.getWidth()) / newWidth;
									uint addr = ((y * newWidth) + x) * 4;
									uint addrs = ((ys * proxBitmap.getWidth()) + xs) * 4;
									temp[addr] = prox[addrs];
									temp[addr+1] = prox[addrs+1];
									temp[addr+2] = prox[addrs+2];
									temp[addr+3] = prox[addrs+3];
								}
							}

							// swap them
							proxBitmap.resize(newWidth, newHeight, proxBitmap.PixelFormat);
							proxBitmap.swap(tempBitmap);
							
							//proxBitmap.resample(newWidth, newHeight);


							// create final bitmap
							CBitmap bitmapDest;
							bitmapDest.resize(islandBitmap.getWidth(), islandBitmap.getHeight(), islandBitmap.PixelFormat);

							
							// mix black and full island bitmaps with blend factor of proximity bitmap pixels
							uint numPix = islandBitmap.getWidth() * islandBitmap.getHeight(); 

							const uint8 *alphaProx = &(proxBitmap.getPixels(0)[0]);
							const uint8 *srcIsland		= &(islandBitmap.getPixels(0)[0]);
							uint8 *dest					= &(bitmapDest.getPixels(0)[0]);


							const uint8 *srcBack		= &(_BackBitmap.getPixels(0)[0]);
							
							uint8 *endDest = dest + (numPix << 2);

							do
							{
								uint invblendFact = (uint) alphaProx[0];
								uint blendFact    = 256 - invblendFact;
								
								// blend 4 component at each pass
								*dest = (uint8) (((blendFact * *srcIsland)		+ (invblendFact * *srcBack)) >> 8);
								*(dest + 1) = (uint8) (((blendFact * *(srcIsland + 1)) + (invblendFact * *(srcBack + 1))) >> 8);
								*(dest + 2) = (uint8) (((blendFact * *(srcIsland + 2)) + (invblendFact * *(srcBack + 2))) >> 8);
								*(dest + 3)  = (uint8) 255;

								alphaProx = alphaProx + 4;
								srcIsland = srcIsland + 4;
								dest = dest + 4;

								srcBack = srcBack + 4;
							}
							while(dest != endDest);
							
							
							// create tga file of avoidable place in island
							string textureName = _OutDirectory + "/" + *itIsland + seasonSuffix + ".tga";

							CBitmap bitmapLittle;
							bitmapLittle.resize(bitmapDest.getWidth(), bitmapDest.getHeight(), bitmapDest.PixelFormat);
							bitmapLittle = bitmapDest;
							if(!isPowerOf2(bitmapDest.getWidth()) || !isPowerOf2(bitmapDest.getHeight()) )
							{
								uint pow2w = NLMISC::raiseToNextPowerOf2(bitmapDest.getWidth());
								uint pow2h = NLMISC::raiseToNextPowerOf2(bitmapDest.getHeight());
								CBitmap enlargedBitmap;
								enlargedBitmap.resize(pow2w, pow2h, bitmapDest.PixelFormat);
								// blit src bitmap
								enlargedBitmap.blit(&bitmapDest, 0, 0);
								// swap them
								bitmapDest.swap(enlargedBitmap);
							}

							COFile fsDest(textureName.c_str());
							bitmapDest.writeTGA(fsDest,32);


							// little tga
							bitmapLittle.resample(bitmapLittle.getWidth()/10, bitmapLittle.getHeight()/10);
							if(!isPowerOf2(bitmapLittle.getWidth()) || !isPowerOf2(bitmapLittle.getHeight()) )
							{
								uint pow2w = NLMISC::raiseToNextPowerOf2(bitmapLittle.getWidth());
								uint pow2h = NLMISC::raiseToNextPowerOf2(bitmapLittle.getHeight());
								CBitmap enlargedBitmap;
								enlargedBitmap.resize(pow2w, pow2h, bitmapLittle.PixelFormat);
								// blit src bitmap
								enlargedBitmap.blit(&bitmapLittle, 0, 0);
								// swap them
								bitmapLittle.swap(enlargedBitmap);
							}

							textureName = _OutDirectory + "/" + *itIsland + seasonSuffix + "_little.tga";
							COFile fsLittle(textureName.c_str());
							bitmapLittle.writeTGA(fsLittle,32);

							_BackColor = CRGBA(255, 255, 255, 255);
						}
					}
				}
			}

			LandscapeIGManager.reset();
			landscape->removeAllZones();
		}
	}

	// remove proximity tga
	list<string>::iterator itProx(_TempFileNames.begin()), lastProx(_TempFileNames.end());
	for( ; itProx != lastProx ; ++itProx)
	{
		CFile::deleteFile(*itProx);
	};
}

//--------------------------------------------------------------------------------
inline bool RGB2HSV(const CRGBA & rgba, uint & Hue, uint & Sat, uint & Val)
{
	double Min_, Max_, Delta, H, S, V;
	
	H = 0.0;
	Min_ = min(min(rgba.R, rgba.G), rgba.B);
	Max_ = max(max(rgba.R, rgba.G), rgba.B);
	Delta = ( Max_ - Min_);
	V = Max_;

	if(Max_ != 0.0)
	{
		S = 255.0*Delta/Max_;
	}
	else
	{
		S = 0.0;
		H = -1;
		return false;
	}

	if(rgba.R == Max_) 
	{
		H = (rgba.G - rgba.B) / Delta;
	}
	else if(rgba.G == Max_)
	{
		H = 2.0 + (rgba.B - rgba.R) / Delta;
	}
	else 
	{
		H = 4.0 + (rgba.R - rgba.G) / Delta;
	}

	H = H * 60;
	if(H < 0.0)
	{
		H = H + 360.0;
	}

	Hue = (uint)H ;             // Hue -> 0..360
	Sat = (uint)S * 100 / 255; // Saturation -> 0..100 %
	Val = (uint)V * 100 / 255; // Value - > 0..100 %

	return true;
}

//-------------------------------------------------------------------------------------------------
inline bool infHLS(const CRGBA & rgba1, const CRGBA & rgba2)
{
	uint h1, s1, v1, h2, s2, v2;
	RGB2HSV(rgba1, h1, s1, v1);  
	RGB2HSV(rgba2, h2, s2, v2);  
	
	if(h1 != h2)
	{
		return (h1 < h2);
	}
	else if(s1 != s2)
	{
		return (s1 < s2);
	}
	else 
	{
		return (v1 < v2);
	}
}

//-------------------------------------------------------------------------------------------------
void CScreenshotIslands::buildBackTextureHLS(const std::string & islandName, const CBitmap & islandBitmap)
{
	// load limit bitmap
	CBitmap limBitmap;
	std::string limFileName = _OutDirectory + "/" + islandName + "_limit.tga";

	CIFile limFS(limFileName.c_str());
	limBitmap.load(limFS);

	list< CRGBA > limitPixels;

	// search for colors of limit pixels
	for(uint x=0; x<islandBitmap.getWidth(); x++)
	{
		for(uint y=0; y<islandBitmap.getHeight(); y++)
		{
			CRGBA lim = limBitmap.getPixelColor(x, y);
			if((lim == CRGBA::White) && (islandBitmap.getPixelColor(x, y)!=CRGBA::White))
			{
				limitPixels.push_back(islandBitmap.getPixelColor(x, y));	
			}
		}
	}


	// HLS order
	list< CRGBA > sortedHLS;
	list< CRGBA >::iterator itCol, itHLS;
	bool inserted = false;
	for(itCol=limitPixels.begin(); itCol!=limitPixels.end(); itCol++)
	{
		inserted = false;
		for(itHLS=sortedHLS.begin(); itHLS!=sortedHLS.end(); ++itHLS)
		{
			if(infHLS(*itCol, *itHLS))
			{
				sortedHLS.insert(itHLS, *itCol);
				inserted = true;
				break;
			}
		}
		if(inserted==false) sortedHLS.push_back(*itCol);
	}


	// keep more filled eighth of circle
	itHLS = sortedHLS.begin();
	uint h, s, v;
	RGB2HSV(*itHLS, h, s, v);
	list< CRGBA > currentList, maxList;

	for(uint i=0; i<8; i++)
	{
		while(itHLS!=sortedHLS.end() && h<i*45)
		{
			currentList.push_back(*itHLS);
			RGB2HSV(*itHLS, h, s, v);
			++itHLS;
		}

		if(currentList.size() > maxList.size())
		{
			maxList.clear();
			maxList = currentList;
			currentList.clear();
		}
	}

	vector< CRGBA > sortedColors(maxList.size());
	uint colorsNb = 0;
	CRGBA lastColor(0, 0, 0, 0);
	CRGBA maxColor;
	uint maxColorNb = 0;
	uint currentColorNb = 0;
	for(itHLS=maxList.begin(); itHLS!=maxList.end(); ++itHLS)
	{
		if(lastColor==*itHLS)
		{
			currentColorNb++;	
		}
		else
		{
			currentColorNb = 1;
		}

		if(currentColorNb>maxColorNb)
		{
			maxColorNb = currentColorNb;
			maxColor = *itHLS;
		}
		
		lastColor = *itHLS;

		RGB2HSV(*itHLS, h, s, v); 
		if(v>25 && v<75 && s>25 && s<75)
		{
			sortedColors[colorsNb] = *itHLS;
			colorsNb++;
		}
	}

	if(colorsNb < 5)
	{
		colorsNb = 0;
		for(itHLS=maxList.begin(); itHLS!=maxList.end(); ++itHLS)
		{
			sortedColors[colorsNb] = *itHLS;
			colorsNb++;
		}			
	}

	sortedColors.resize(colorsNb);

	_BackBitmap.resize(islandBitmap.getWidth(), islandBitmap.getHeight(), islandBitmap.PixelFormat);

	if(sortedColors.size()!=0)
	{
		_BackColor = maxColor;

		CRandom	randomGenerator;

		uint8 * backPixels = &(_BackBitmap.getPixels(0)[0]);

		for(uint x=0; x<_BackBitmap.getWidth(); x++)
		{
			for(uint y=0; y<_BackBitmap.getHeight(); y++)
			{
				sint32 randomVal = randomGenerator.rand(colorsNb-1);
				const CRGBA & color = sortedColors[randomVal];

				*backPixels = (uint8) color.R;
				*(backPixels + 1) = (uint8) color.G;
				*(backPixels + 2) = (uint8) color.B;
				*(backPixels + 3)  = (uint8) 255;
				
				backPixels = backPixels+4;
			}
		}
	}

	/*
	//TEST
	CBitmap HLSBitmap;
	HLSBitmap.resize(640, sortedColors.size()*4, islandBitmap.PixelFormat);

	uint8 * hlsPixels = &(HLSBitmap.getPixels(0)[0]);

	for(uint i=0; i < sortedColors.size(); i++)
	{
		uint count = 0;
		while(count<640*4)
		{
			*hlsPixels = (uint8) sortedColors[i].R;
			*(hlsPixels + 1) = (uint8) sortedColors[i].G;
			*(hlsPixels + 2) = (uint8) sortedColors[i].B;
			*(hlsPixels + 3)  = (uint8) sortedColors[i].A;
			
			hlsPixels = hlsPixels+4;
			count++;
		}
	}


	string textureName = _OutDirectory + "/" + islandName + "_HLS2.tga";
	COFile fsHLS(textureName.c_str());
	HLSBitmap.writeTGA(fsHLS,32);
	*/
}


//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// methods CProximityMapBuffer
//-------------------------------------------------------------------------------------------------

void CProximityMapBuffer::load(const std::string& name)
{
	// load the AI collision map file
	CWorldMap worldMap;
	CIFile	f(name);
	f.serial(worldMap);

	// lookup the map bounds
	CMapPosition	min, max;
	worldMap.getBounds(min, max);

	// calculate a handful of constants relating to the bounds of the image...
	_ScanWidth = max.x()-min.x();
	_ScanHeight = max.y()-min.y();
	_XOffset= min.x();
	_YOffset= (sint16)min.y();

	// redimension buffer to correct size
	_Buffer.resize(_ScanWidth*_ScanHeight);

	// setup a position variable to mark the start point of each line
	CMapPosition	scanpos(min.x(),min.y());

	// iterate over the scan area looking for points that are accessible
	for (uint32 y=0; y<_ScanHeight; ++y, scanpos = scanpos.getStepN())
	{
		CMapPosition pos(scanpos);

		// scan a line of the map
		for (uint32 x=0; x<_ScanWidth; ++x, pos = pos.getStepE())
		{
			bool isAccessible= false;
			// if the cell pointer is NULL it means that the 16x16 cell in question is inaccessible
			if (worldMap.getRootCellCst(pos) != NULL)
			{
				// run through the surfaces in the cell looking for a match for this position (may be as many as 3 surfaces per cell max)
				for (uint32 ns=0; ns<3; ++ns)
				{
					isAccessible |= worldMap.getSafeWorldPosition(pos, CSlot(ns)).isValid();
				}
			}
			// setup the next pixel in the output buffers...
			_Buffer[y*_ScanWidth+x]= (isAccessible? 0: (TBufferEntry)~0u);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void CProximityMapBuffer::calculateZones(TZones& zones)
{
	// clear out the result buffer before starting work
	zones.clear();

	// setup a container to hold the accessible points within this buffer
	typedef std::set<uint32> TAccessiblePoints;
	TAccessiblePoints accessiblePoints;

	// start by building the set of all accessible points
	for (uint32 i=0;i<_Buffer.size();++i)
	{
		if (_Buffer[i]==0)
			accessiblePoints.insert(i);
	}

	// while there are still points remaining in the set we must have another zone to process
	while (!accessiblePoints.empty())
	{
		// append a new zone to the zones vector and get a refference to it
		zones.push_back( CProximityZone(_ScanWidth,_ScanHeight,_XOffset,_YOffset) );
		CProximityZone& theZone= zones.back();

		// setup a todo list representing points that are part of the surface that we are dealing with
		// that haven't yet been treated to check for neighbours, etc
		std::vector<uint32> todo;

		// get hold of the first point in the accessilbe points set and push it onto the todo list
		todo.push_back(*accessiblePoints.begin());
		accessiblePoints.erase(todo.back());

		// while we have more points to deal with ...
		while (!todo.empty())
		{
			// pop the next point off the todo list
			uint32 thePoint= todo.back();
			todo.pop_back();

			// add the point to the zone
			theZone.add(thePoint);

			// a little macro for the code to perform for each movement test...
			#define TEST_MOVE(xoffs,yoffs)\
				{\
					TAccessiblePoints::iterator it= accessiblePoints.find(thePoint+xoffs+_ScanWidth*yoffs);\
					if (it!=accessiblePoints.end())\
					{\
						todo.push_back(*it);\
						accessiblePoints.erase(it);\
					}\
				}

			// N, S, W, E moves
			TEST_MOVE( 0, 1);
			TEST_MOVE( 0,-1);
			TEST_MOVE( 1, 0);
			TEST_MOVE(-1, 0);

			// NW, NE, WS, SE moves
			TEST_MOVE( 1, 1);
			TEST_MOVE(-1, 1);
			TEST_MOVE( 1,-1);
			TEST_MOVE(-1,-1);

			#undef TEST_MOVE
		}
	}

	nlinfo("Found %u zones",zones.size());
}

//-------------------------------------------------------------------------------------------------
void CProximityMapBuffer::_prepareBufferForZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer,TOffsetsVector& accessiblePoints)
{
	// the length of runs that we consider too short to deal with...
	const uint32 shortRunLength=5;

	// redimention and initialise the zone buffer
	uint32 zoneWidth= zone.getZoneWidth();
	uint32 zoneHeight= zone.getZoneHeight();
	zoneBuffer.clear();
	zoneBuffer.resize(zoneWidth*zoneHeight,(TBufferEntry)~0u);

	// setup the buffer's accessible points and prime vects[0] with the set of accessible points in the zone buffer
	for (uint32 i=0;i<zone.getOffsets().size();++i)
	{
		// lookup the next offset in the zone's offsets vector and remap to a zone-relative offset
		uint32 zoneOffset= zone.remapOffset(zone.getOffsets()[i]);
		// flag the appropriate entry in the zoneBuffer as 'distance=0'
		zoneBuffer[zoneOffset]= 0;
		// add the zone offset to the accessible points vector
		accessiblePoints.push_back(zoneOffset);
	}

	// run through the zone buffer looking for points that are surrounded that we can just throw out...
	// start by flagging all points that belong to a short run in the y direction
	for (uint32 i=0;i<zoneWidth;++i)
	{
		// setup start and end offsets marking first and last 'zero' value pixels in the column
		uint32 startOffset=i, endOffset=i+(zoneHeight-1)*zoneWidth;
		for (; startOffset<endOffset && zoneBuffer[startOffset]!=0; startOffset+= zoneWidth) {}
		for (; endOffset>startOffset && zoneBuffer[endOffset]!=0;   endOffset-= zoneWidth) {}

		for (uint32 offset=startOffset, marker=startOffset;offset<=endOffset;offset+=zoneWidth)
		{
			// see if this is an accessible position 
			if (zoneBuffer[offset]!=0)
			{
				zoneBuffer[offset]= InteriorValue;

				if(offset-1>=startOffset && zoneBuffer[offset-1]==(TBufferEntry)~0u)
				{
					zoneBuffer[offset-1] = ValueBorder;
				}
				if(offset+1<=endOffset && zoneBuffer[offset+1]==(TBufferEntry)~0u)
				{
					zoneBuffer[offset+1] = ValueBorder;
				}
			}
		}
	}

	// continue by dealing with all points that belong to a short run in the x direction
	for (uint32 i=0;i<zoneHeight;++i)
	{
		// setup start and end offsets marking first and last 'zero' value pixels in the column
		uint32 startOffset= i*zoneWidth;
		uint32 endOffset= startOffset+zoneWidth-1;

		uint32 startOffsetInit = startOffset;
		uint32 endOffsetInit = endOffset;

		for (; startOffset<endOffset && zoneBuffer[startOffset]!=0; ++startOffset) {}
		for (; endOffset>startOffset && zoneBuffer[endOffset]!=0; --endOffset) {}

		for (uint32 offset=startOffset, marker=startOffset;offset<=endOffset;++offset)
		{
			// see if this is an accessible position 
			if (zoneBuffer[offset]!=0)
			{
				zoneBuffer[offset]= InteriorValue;

				if(offset>zoneWidth && zoneBuffer[offset-zoneWidth]==(TBufferEntry)~0u)
				{
					zoneBuffer[offset-zoneWidth] = ValueBorder;
				}
				if(offset+zoneWidth<zoneHeight*zoneWidth && zoneBuffer[offset+zoneWidth]==(TBufferEntry)~0u)
				{
					zoneBuffer[offset+zoneWidth] = ValueBorder;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void CProximityMapBuffer::generateZoneProximityMap(const CProximityZone& zone,TBuffer& zoneBuffer)
{
	// a set of vectors to hold sets of points that need to be treated
	TOffsetsVector vects[16];
	// a counter that should always contain sum of all vects[i].size()
	uint32 entriesToTreat= 0;

	// setup the buffer's accessible points and prime vects[0] with the set of accessible points in the zone buffer
	_prepareBufferForZoneProximityMap(zone, zoneBuffer, vects[0]);
	entriesToTreat= vects[0].size();

	// lookup the buffer dimentions
	uint32 zoneWidth= zone.getZoneWidth();
	uint32 zoneHeight= zone.getZoneHeight();

	// for dist=0 to ? treat points with distance 'dist' from centre, iterating until all vects are empty
	for (TBufferEntry dist=0; entriesToTreat!=0; ++dist)
	{
		// setup refference to the vector that we are supposed to be iterating over for this dist
		TOffsetsVector &vect= vects[dist&15];

		// iterate over contents of points for this distance, treating NSWE neighbours
		for(TOffsetsVector::iterator it=vect.begin(); it!=vect.end(); ++it)
		{
			uint32 val=(*it);

			// deal with the case where this point has already been refferenced via a better route
			if (zoneBuffer[val]<dist || (zoneBuffer[val]==InteriorValue && dist > BigValue) 
				|| (zoneBuffer[val]==ValueBorder && dist > BigValue))
				continue;

			// write the new distance into this buffer entry
			zoneBuffer[val]=dist;

			// decompose into x and y in order to manage identification of neighbour cells correctly
			uint32 x= val% zoneWidth;
			uint32 y= val/ zoneWidth;

			#define TEST_MOVE(xoffs,yoffs,newDist)\
				{\
					if (((uint32)(x+(xoffs))<zoneWidth) && ((uint32)(y+(yoffs))<zoneHeight))\
					{\
						uint32 newVal= val+(xoffs)+((yoffs)*zoneWidth);\
						bool isInterior= ((zoneBuffer[newVal]==InteriorValue && newDist > BigValue) || (zoneBuffer[newVal]==ValueBorder && newDist > BigValue));\
						if (zoneBuffer[newVal]>(newDist) && !isInterior)\
						{\
							zoneBuffer[newVal]=(newDist);\
							vects[(newDist)&15].push_back(newVal);\
							++entriesToTreat;\
						}\
					}\
				}

			// N, S, W, E moves
			TEST_MOVE( 0, 1,dist+5);
			TEST_MOVE( 0,-1,dist+5);
			TEST_MOVE( 1, 0,dist+5);
			TEST_MOVE(-1, 0,dist+5);

			// NW, NE, WS, SE moves
			TEST_MOVE( 1, 1,dist+7);
			TEST_MOVE(-1, 1,dist+7);
			TEST_MOVE( 1,-1,dist+7);
			TEST_MOVE(-1,-1,dist+7);

			// NNW, NNE, SSW, SSE moves
			TEST_MOVE( 1, 2,dist+11);
			TEST_MOVE(-1, 2,dist+11);
			TEST_MOVE( 1,-2,dist+11);
			TEST_MOVE(-1,-2,dist+11);

			// WNW, WSW, ENE, ESE moves
			TEST_MOVE( 2, 1,dist+11);
			TEST_MOVE(-2, 1,dist+11);
			TEST_MOVE( 2,-1,dist+11);
			TEST_MOVE(-2,-1,dist+11);

			#undef TEST_MOVE
		}

		// clear out the vector
		entriesToTreat-= vect.size();
		vect.clear();
	}
}

//-------------------------------------------------------------------------------------------------
const TBuffer& CProximityMapBuffer::getBuffer() const
{
	return _Buffer;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityMapBuffer::getScanHeight() const
{
	return _ScanHeight;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityMapBuffer::getScanWidth() const
{
	return _ScanWidth;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// methods CProximityZone
//-------------------------------------------------------------------------------------------------

CProximityZone::CProximityZone(uint32 scanWidth,uint32 scanHeight,sint32 xOffset, sint32 yOffset)
{
	_ScanWidth	= scanWidth;
	_ScanHeight	= scanHeight;
	_XOffset	= xOffset;
	_YOffset	= yOffset;

	_MaxOffset	= scanWidth * scanHeight -1;

	_XMin = ~0u;
	_YMin = ~0u;
	_XMax = 0;
	_YMax = 0;

	_BorderPixels = 30;
}

//-------------------------------------------------------------------------------------------------
bool CProximityZone::add(uint32 offset)
{
	// make sure the requested point is in the zone
	if (offset>_MaxOffset)
		return false;

	// calculate the x and y coordinates of the point
	uint32 y= offset/ _ScanWidth;
	uint32 x= offset% _ScanWidth;

	// update the bounding coordinates for this zone
	if (x<_XMin) _XMin= x;
	if (x>_XMax) _XMax= x;
	if (y<_YMin) _YMin= y;
	if (y>_YMax) _YMax= y;
			 
	// add the point to the vector of points
	_Offsets.push_back(offset);
	return true;
}

//-------------------------------------------------------------------------------------------------
const CProximityZone::TOffsets& CProximityZone::getOffsets() const
{
	return _Offsets;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getZoneWidth() const
{
	return getZoneXMax()- getZoneXMin() +1;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getZoneHeight() const
{
	return getZoneYMax()- getZoneYMin() +1;
}

//-------------------------------------------------------------------------------------------------
sint32 CProximityZone::getZoneXMin() const
{
	return _XMin-_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
sint32 CProximityZone::getZoneYMin() const
{
	return _YMin-_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getZoneXMax() const
{
	return _XMax+_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getZoneYMax() const
{
	return _YMax+_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getBoundXMin() const
{
	return _XMin+_XOffset-_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getBoundYMin() const
{
	return _YMin+_YOffset-_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getBoundXMax() const
{
	return _XMax+_XOffset+_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::getBoundYMax() const
{
	return _YMax+_YOffset+_BorderPixels;
}

//-------------------------------------------------------------------------------------------------
uint32 CProximityZone::remapOffset(uint32 bufferOffset) const
{
	// decompose input coordinates into x and y parts
	uint32 bufferX= bufferOffset% _ScanWidth;
	uint32 bufferY= bufferOffset/ _ScanWidth;

	// remap the offset from a _Buffer-relative offset to a zone-relative offset
	return bufferX-getZoneXMin()+ (bufferY-getZoneYMin())*getZoneWidth();
}

}

