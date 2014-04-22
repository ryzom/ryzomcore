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


#include "color_modifier.h"
#include "color_mask.h"
#include "hls_bank_texture_info.h"
#define HAS_INFO_GENERATION 0
#if HAS_INFO_GENERATION
#include "info_color_generation.h"
#include "info_mask_generation.h"
#endif

#include <nel/misc/types_nl.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/debug.h>

#include <time.h>

using namespace NLMISC;
using namespace std;


string	DivideBy2Dir= "/d4/";
//string	HlsInfoDir= "hlsInfo/";


// ========================================================================================================
// This tool is for creating various colored texture from a base texture.
// Parts of a base texture can have hue, contrast, luminosity shifting etc.
// Each part is defined by a mask. The red component of it is considered as an alpha value (not the alpha, because it is faster to create a grey texture with photoshop..)
// The result is serialized in png or tga files.
// ========================================================================================================
// why this tool ? : it is useful to create various colored cloth and skin textures
// Not all hardware allow it to manage that at runtime (lack for palettized textures or pixel shaders...)
//=========================================================================================================


/// describes the building infos
struct CBuildInfo
{
	std::string					 InputPath;
	std::string					 OutputPath;
	std::string					 HlsInfoPath;
	std::string					 CachePath;
	std::vector<std::string>     BitmapExtensions; // the supported extension for bitmaps
	std::string					 OutputFormat; // png or tga
	std::string					 DefaultSeparator;
	TColorMaskVect				 ColorMasks;
	// how to shift right the size of the src Bitmap for the .hlsinfo
	uint						 LowDefShift;
};


/// Temporary
static void validateCgiInfo();
static void validateGtmInfo();
/// Temporary


/** Build the infos we need from a config file
  * It build a list of masks infos
  */
static void BuildMasksFromConfigFile(NLMISC::CConfigFile &cf,
									 TColorMaskVect &colorMasks);

/// Build the colored versions
static void BuildColoredVersions(const CBuildInfo &bi);

///
static void BuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension, bool mustDivideBy2);
/** Check if building if reneeded by looking in the cache directory.
  * If the texture is found in the cache it is just copied
  */
static bool CheckIfNeedRebuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension, bool mustDivideBy2);

											

/// replace slashes by the matching os value in a file name
static std::string replaceSlashes(const std::string &src)
{
	std::string result = src;
	for(uint k = 0; k < result.size(); ++k)
	#ifdef NL_OS_WINDOWS			
		if (result[k] == '/') result[k] = '\\';
	#else
		if (result[k] == '\\') result[k] = '/';
	#endif
	return result;
}



///=====================================================
int main(int argc, char* argv[])
{	
	// Filter addSearchPath
	NLMISC::createDebug();

	//"panoply.cfg" "gtm" "fyros"

#if HAS_INFO_GENERATION
	if(!strcmp(argv[2], "gtm") || !strcmp(argv[2], "cgi"))
	{
		NLMISC::CConfigFile cf;

		std::string _Path_Input_TexBases;
		std::string _Path_Input_Masks;
		std::string _Path_Output_MaksOptimized;
		std::string _Path_Output_Gtm;
		std::string _Path_Output_Cgi;

		try
		{
			/// load the config file
			cf.load(argv[1]);

			/// look paths
			try
			{
				NLMISC::CConfigFile::CVar &additionnal_paths = cf.getVar ("additionnal_paths");
				for (uint k = 0; k < (uint) additionnal_paths.size(); ++k)
				{
					NLMISC::CPath::addSearchPath(NLMISC::CPath::standardizePath(additionnal_paths.asString(k)),true, false);
				}
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

			/// repertory of textures bases (no-colorized)
			try
			{
				_Path_Input_TexBases = NLMISC::CPath::standardizePath(cf.getVar ("input_path_texbase").asString());
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

			/// repertory of masks (original)
			try
			{
				_Path_Input_Masks = NLMISC::CPath::standardizePath(cf.getVar ("input_path_mask").asString());
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

			/// repertory output of masks optimized created 
			try
			{
				_Path_Output_MaksOptimized = NLMISC::CPath::standardizePath(cf.getVar ("output_path_mask_optimized").asString());
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

			/// file of infos about colorization average for the client
			try
			{
				_Path_Output_Cgi = NLMISC::CPath::standardizePath(cf.getVar ("output_path_cgi").asString());
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

			/// file of infos about multiplexing texture for the client
			try
			{
				_Path_Output_Gtm = NLMISC::CPath::standardizePath(cf.getVar ("output_path_gtm").asString());
			}
			catch (const NLMISC::EUnknownVar &)
			{
			}

		}
		catch (const std::exception &e)
		{
			nlwarning("Panoply building failed.");
			nlwarning(e.what());
			return -1;
		}

		/// oriented program

		if( !strcmp(argv[2], "gtm"))	/// masks optimized
		{
			CInfoMaskGeneration infoMaskGen(_Path_Input_TexBases,
											_Path_Input_Masks,
											_Path_Output_MaksOptimized,
											_Path_Output_Gtm,
											argv[3],
											1);
			infoMaskGen.init();
			infoMaskGen.process();
		}
		else if( !strcmp(argv[2], "cgi")) /// colorized information
		{
			CInfoColorGeneration infoColor(_Path_Input_TexBases,
										   _Path_Input_Masks,
										   _Path_Output_Cgi,
										   argv[3],
										   1);
			infoColor.init();
			infoColor.process();
		}
	}
	else
	{
#endif
		NLMISC::InfoLog->addNegativeFilter ("adding the path");

		if (argc != 2)
		{
			nlwarning("usage : %s [config_file name]", argv[0]);
			return -1;
		}

		CBuildInfo bi;	

		/////////////////////////////////////////
		// reads infos from the config files   //
		/////////////////////////////////////////

			NLMISC::CConfigFile cf;
			try
			{
				/// load the config file
				cf.load(argv[1]);

				/// colors masks
				BuildMasksFromConfigFile(cf, bi.ColorMasks);

				/// look paths
				try
				{
					NLMISC::CConfigFile::CVar &additionnal_paths = cf.getVar ("additionnal_paths");
					for (uint k = 0; k < (uint) additionnal_paths.size(); ++k)
					{
						NLMISC::CPath::addSearchPath(NLMISC::CPath::standardizePath(additionnal_paths.asString(k)));
					}
				}
				catch (const NLMISC::EUnknownVar &)
				{
				}
				
				/// input
				try
				{
					bi.InputPath = NLMISC::CPath::standardizePath(cf.getVar ("input_path").asString());
				}
				catch (const NLMISC::EUnknownVar &)
				{
				}

				/// output
				try
				{
					bi.OutputPath = NLMISC::CPath::standardizePath(cf.getVar ("output_path").asString());
				}
				catch (const NLMISC::EUnknownVar &)
				{
				}

				/// hls info path
				try
				{
					bi.HlsInfoPath = NLMISC::CPath::standardizePath(cf.getVar("hls_info_path").asString());
				}
				catch (const NLMISC::EUnknownVar &)
				{
					bi.HlsInfoPath = "hlsInfo/";
				}

				/// output
				try
				{
					bi.CachePath = NLMISC::CPath::standardizePath(cf.getVar ("cache_path").asString());
				}
				catch (const NLMISC::EUnknownVar &)
				{
				}

				/// output format
				try
				{
					bi.OutputFormat = "." + cf.getVar ("output_format").asString();
				}
				catch (const NLMISC::EUnknownVar &)
				{
					bi.OutputFormat = ".tga";
				}

				/// default ascii character for unused masks
				try
				{
					bi.DefaultSeparator = cf.getVar ("default_separator").asString();								
				}
				catch (const NLMISC::EUnknownVar &)
				{
					bi.DefaultSeparator = '_';
				}
				/// extension for bitmaps
				try
				{
					NLMISC::CConfigFile::CVar &bitmap_extensions = cf.getVar ("bitmap_extensions");				
					for (uint k = 0; k < (uint) bitmap_extensions.size(); ++k)
					{
						std::string ext = "." + bitmap_extensions.asString(k);
						ext = NLMISC::strupr(ext);
						if (std::find(bi.BitmapExtensions.begin(), bi.BitmapExtensions.end(), ext) == bi.BitmapExtensions.end())
						{
							bi.BitmapExtensions.push_back(ext);
						}					
					}				
				}
				catch (const NLMISC::EUnknownVar &)
				{
					bi.BitmapExtensions[0].resize(1);
					bi.BitmapExtensions[0] = bi.OutputFormat;
				}

				try
				{
					bi.LowDefShift = cf.getVar ("low_def_shift").asInt();								
				}
				catch (const NLMISC::EUnknownVar &)
				{
					// tranform 512*512 to 64*64 by default
					bi.LowDefShift= 3;
				}

			}
			catch (const std::exception &e)
			{
				nlwarning("Panoply building failed.");
				nlwarning(e.what());
				return -1;
			}

		////////////////////////////////
		// Build the colored versions //
		////////////////////////////////
		try
		{
			BuildColoredVersions(bi);
		}
		catch (const std::exception &e)
		{
			nlwarning("Something went wrong while building bitmap : %s", e.what());
			return -1;
		}
		return 0;
#if HAS_INFO_GENERATION
	}
#endif
}

///======================================================
#if HAS_INFO_GENERATION
static void validateCgiInfo()
{
	NLMISC::CIFile f;
	

	vector<StrInfoTexColor> temp;
	uint version;

	try
	{
		f.open(CPath::lookup("info_color_texbase_fyros.cgi"));

		f.serialCont(temp);

	}
	catch(const std::exception &e)
	{
		nlwarning("Panoply building failed.");
	}

	uint16 a = temp.size();

	f.close();
}

///======================================================

static void validateGtmInfo()
{

}
#endif
///======================================================
static void BuildMasksFromConfigFile(NLMISC::CConfigFile &cf,
									 TColorMaskVect &colorMasks)
									 
{
	/// get a list of the alpha mask extensions	
	NLMISC::CConfigFile::CVar &mask_extensions = cf.getVar ("mask_extensions");
	colorMasks.resize(mask_extensions.size());

	/// For each kind of mask, build a list of the color modifiers
	for (uint k = 0; k < (uint) mask_extensions.size(); ++k)
	{			
		colorMasks[k].MaskExt = mask_extensions.asString(k);
		NLMISC::CConfigFile::CVar &luminosities    = cf.getVar (colorMasks[k].MaskExt + "_luminosities");
		NLMISC::CConfigFile::CVar &contrasts	   = cf.getVar (colorMasks[k].MaskExt + "_constrasts");
		NLMISC::CConfigFile::CVar &hues			   = cf.getVar (colorMasks[k].MaskExt + "_hues");
		NLMISC::CConfigFile::CVar &lightness	   = cf.getVar (colorMasks[k].MaskExt + "_lightness");
		NLMISC::CConfigFile::CVar &saturation	   = cf.getVar (colorMasks[k].MaskExt + "_saturations");
		NLMISC::CConfigFile::CVar &colorIDs		   = cf.getVar (colorMasks[k].MaskExt + "_color_id");

		if (luminosities.size() != contrasts.size()
			|| luminosities.size() != hues.size()	
			|| luminosities.size() != lightness.size()
			|| luminosities.size() != saturation.size()
			|| luminosities.size() != colorIDs.size()
			)
		{
			throw NLMISC::Exception("All color descriptors must have the same number of arguments");
		}
		colorMasks[k].CMs.resize(luminosities.size());
		for (uint l = 0; l < (uint) luminosities.size(); ++l)
		{
			CColorModifier &cm = colorMasks[k].CMs[l];
			cm.Contrast		   = contrasts.asFloat(l);
			cm.Luminosity      = luminosities.asFloat(l);
			cm.Hue		       = hues.asFloat(l);
			cm.Lightness       = lightness.asFloat(l);
			cm.Saturation      = saturation.asFloat(l);

			cm.ColID = colorIDs.asString(l);
		}
	}
}

///======================================================
static void BuildColoredVersions(const CBuildInfo &bi)
{
	if (!NLMISC::CFile::isExists(bi.InputPath))
	{
		nlwarning(("Path not found : " + bi.InputPath).c_str());
		return;
	}
	for(uint sizeVersion= 0; sizeVersion<2; sizeVersion++)
	{
		std::vector<std::string> files;
		if(sizeVersion==0)
			// get the original (not to dvide) dir
			NLMISC::CPath::getPathContent (bi.InputPath, false, false, true, files);
		else
			// get the dir content with texture that must be divided by 2.
			NLMISC::CPath::getPathContent (bi.InputPath+DivideBy2Dir, false, false, true, files);

		// For all files found
		for (uint k = 0;  k < files.size(); ++k)
		{
			for (uint l = 0; l < bi.BitmapExtensions.size(); ++l)
			{
				std::string fileExt = "." + NLMISC::strupr(NLMISC::CFile::getExtension(files[k]));						
				if (fileExt == bi.BitmapExtensions[l])
				{
					//nlwarning("Processing : %s ", files[k].c_str());				
					try
					{
						if (CheckIfNeedRebuildColoredVersionForOneBitmap(bi, NLMISC::CFile::getFilename(files[k]),
																		sizeVersion==1) )
						{					
							BuildColoredVersionForOneBitmap(bi,											
															NLMISC::CFile::getFilename(files[k]),
														   sizeVersion==1);
						}
						else
						{
							//nlwarning(("No need to rebuild " + NLMISC::CFile::getFilename(files[k])).c_str());
						}
					}
					catch (const std::exception &e)
					{
						nlwarning("Processing of %s failed : %s \n", files[k].c_str(), e.what());					
					}
				}
			}
		}
	}
}


/// used to loop throiugh the process, avoiding unused masks 
struct CLoopInfo
{
	NLMISC::CBitmap		Mask;
	uint        Counter;
	uint        MaskID;
};


///======================================================
static bool CheckIfNeedRebuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension, 
	bool mustDivideBy2)
{		
	if (bi.CachePath.empty()) return true;
	uint32 srcDate = (uint32) NLMISC::CFile::getFileModificationDate(replaceSlashes(bi.InputPath + fileNameWithExtension));		
	static std::vector<CLoopInfo> masks;
	/// check the needed masks
	masks.clear();

	std::string fileName = NLMISC::CFile::getFilenameWithoutExtension(fileNameWithExtension);
	std::string fileExt  = NLMISC::strupr(NLMISC::CFile::getExtension(fileNameWithExtension));

	for (uint k = 0; k < bi.ColorMasks.size(); ++k)
	{
		std::string maskName = fileName + "_" + bi.ColorMasks[k].MaskExt + "." + fileExt;
		std::string maskFileName = NLMISC::CPath::lookup(maskName,
														 false, false);
		if (!maskFileName.empty()) // found the mask ?
		{			
			CLoopInfo li;
			li.Counter = 0;
			li.MaskID = k;

			if (NLMISC::CFile::fileExists(maskFileName))
			{
				srcDate = std::max(srcDate, (uint32) NLMISC::CFile::getFileModificationDate(replaceSlashes(maskFileName)));			
				masks.push_back(li);	
			}			
		}
	}

	// get hls info version that is in the cache. if not possible, must rebuild
	std::string outputHLSInfo = bi.HlsInfoPath + fileName + ".hlsinfo";
	std::string cacheHLSInfo = bi.CachePath + fileName + ".hlsinfo";
	if (!NLMISC::CFile::fileExists(cacheHLSInfo.c_str()) )
		return true;
	else
	{
		// Must now if was moved beetween normal dir and d4/ dir. 
		CHLSBankTextureInfo		hlsInfo;
		// read .hlsInfo cache
		CIFile		f;
		if(!f.open(cacheHLSInfo))
			return true;
		f.serial(hlsInfo);
		f.close();
		// check if same DividedBy2 Flag.
		if(hlsInfo.DividedBy2!=mustDivideBy2)
			return true;

		// ok, can move the cache
		if (!NLMISC::CFile::moveFile(outputHLSInfo.c_str(), cacheHLSInfo.c_str()))
		{
			nlwarning(("Couldn't move " + cacheHLSInfo + " to " + outputHLSInfo).c_str());
			return true;
		}
	}


	/// check is each generated texture has the same date or is more recent
	for(;;)
	{			
		uint l;
		std::string outputFileName = fileName;
		
		/// build current tex name
		for (l  = 0; l < masks.size(); ++l)
		{
			uint maskID = masks[l].MaskID;
			uint colorID = masks[l].Counter;			
			/// complete the file name
			outputFileName += bi.DefaultSeparator + bi.ColorMasks[maskID].CMs[colorID].ColID;
		}

		// compare date							
		std::string searchName = replaceSlashes(bi.CachePath + outputFileName + bi.OutputFormat);
		if ((uint32) NLMISC::CFile::getFileModificationDate(searchName) < srcDate) 
		{					
			return true; // not found or more old => need rebuild			
		}

		// get version that is in the cache
		std::string cacheDest = bi.OutputPath + outputFileName + bi.OutputFormat;
		if (!NLMISC::CFile::moveFile(cacheDest.c_str(), searchName.c_str()))
		{
			nlwarning(("Couldn't move " + searchName + " to " + cacheDest).c_str());
			return true;
		}		

		/// increment counters
		for (l  = 0; l < (uint) masks.size(); ++l)
		{
			++ (masks[l].Counter);

			/// check if we have done all colors for this mask
			if (masks[l].Counter == bi.ColorMasks[masks[l].MaskID].CMs.size())
			{
				masks[l].Counter = 0;
			}
			else
			{
				break;
			}
		}
		if (l == masks.size()) break; // all cases dones
	}
	return false; // nothing to rebuild
}



///======================================================
static void BuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension, 
	bool mustDivideBy2)
{		
	uint32 depth;
	NLMISC::CBitmap srcBitmap;
	NLMISC::CBitmap resultBitmap;

	/// **** load the src bitmap
	{
		// where to load it.
		string	actualInputPath;
		if(mustDivideBy2)
			actualInputPath= bi.InputPath + DivideBy2Dir;
		else
			actualInputPath= bi.InputPath;

		// load
		NLMISC::CIFile is;
		try
		{
			if (is.open(actualInputPath + fileNameWithExtension))
			{
				depth = srcBitmap.load(is);
				if (depth == 0 || srcBitmap.getPixels().empty())
				{
					throw NLMISC::Exception(std::string("Failed to load bitmap ") + actualInputPath + fileNameWithExtension);
				}
				if (srcBitmap.PixelFormat != NLMISC::CBitmap::RGBA)
				{
					srcBitmap.convertToType(NLMISC::CBitmap::RGBA);
				}
			}
			else
			{
				nlwarning("Unable to open %s. Processing next", (actualInputPath + fileNameWithExtension).c_str());
				return;
			}			
		}
		catch (const NLMISC::Exception &)
		{
			nlwarning("File or format error with : %s. Processing next...", fileNameWithExtension.c_str());
			return;
		}
	}
	
	/// **** Build and prepare build of the .hlsinfo to write.
	CHLSBankTextureInfo		hlsInfo;
	CBitmap					hlsInfoSrcBitmap;
	hlsInfoSrcBitmap= srcBitmap;
	// reduce size of the bitmap of LowDef shift
	uint	reduceShift= bi.LowDefShift;
	if(reduceShift>0)
	{
		uint	w= hlsInfoSrcBitmap.getWidth()>>reduceShift;
		uint	h= hlsInfoSrcBitmap.getHeight()>>reduceShift;
		w= max(w, 1U);
		h= max(h, 1U);
		hlsInfoSrcBitmap.resample(w, h);
	}
	// Compress DXTC5 src bitmap
	hlsInfo.SrcBitmap.build(hlsInfoSrcBitmap);
	// Store info about if where in d4/ dir or not
	hlsInfo.DividedBy2= mustDivideBy2;


	/// **** check the needed masks
	static std::vector<CLoopInfo> masks;
	masks.clear();

	std::string fileName = NLMISC::CFile::getFilenameWithoutExtension(fileNameWithExtension);
	std::string fileExt  = NLMISC::strupr(NLMISC::CFile::getExtension(fileNameWithExtension));

	uint	k;
	for (k = 0; k < bi.ColorMasks.size(); ++k)
	{
		std::string maskName = fileName + "_" + bi.ColorMasks[k].MaskExt + "." + fileExt;
		std::string maskFileName = NLMISC::CPath::lookup(maskName,
														 false, false);
		if (!maskFileName.empty()) // found the mask ?
		{			
			CLoopInfo li;
			li.Counter = 0;
			li.MaskID = k;

			/// try to load the bitmap
			NLMISC::CIFile is;
			try
			{

				if (is.open(maskFileName))
				{				
					if (li.Mask.load(is) == 0)
					{
						throw NLMISC::Exception(std::string("Failed to load mask ") + maskFileName);
					}
					if (li.Mask.getPixels().empty())
					{
						throw NLMISC::Exception(std::string("Failed to load mask ") + maskFileName);
					}

					if (li.Mask.PixelFormat != NLMISC::CBitmap::RGBA)
					{
						li.Mask.convertToType(NLMISC::CBitmap::RGBA);
					}

					/// make sure the mask has the same size
					if (li.Mask.getWidth() != srcBitmap.getWidth()
						|| li.Mask.getHeight() != srcBitmap.getHeight())
					{
						throw NLMISC::Exception("Bitmap and mask do not have the same size");
					}
					masks.push_back(li);
				}
				else
				{
					nlwarning("Unable to open %s. Processing next", maskFileName.c_str());
					return;
				}
			}
			catch (const std::exception &e)
			{
				nlwarning("Error with : %s : %s. Aborting this bitmap processing", maskFileName.c_str(), e.what());				
				return;
			}
		}
	}

	// **** Add the masks to the .hlsInfo
	hlsInfo.Masks.resize(masks.size());
	for (k = 0; k < masks.size(); ++k)
	{
		CLoopInfo &li= masks[k];
		CBitmap		tmp= li.Mask;
		tmp.resample(hlsInfoSrcBitmap.getWidth(), hlsInfoSrcBitmap.getHeight());
		hlsInfo.Masks[k].build(tmp);
	}


	// **** generate each texture 
	// NB : if there are no masks the texture just will be copied
	for(;;)
	{	
		resultBitmap = srcBitmap;
		uint l;
		std::string outputFileName = fileName;
		
		// Add an instance entry to the hlsInfo
		uint	instId= (uint)hlsInfo.Instances.size();
		hlsInfo.Instances.resize(instId+1);
		CHLSBankTextureInfo::CTextureInstance	&hlsTextInstance= hlsInfo.Instances[instId];
		hlsTextInstance.Mods.resize(masks.size());

		/// build current tex
		for (l  = 0; l < masks.size(); ++l)
		{
			uint maskID = masks[l].MaskID;
			uint colorID = masks[l].Counter;

			/// get the color modifier
			const CColorModifier &cm = bi.ColorMasks[maskID].CMs[colorID];

			/// apply the mask
			float	deltaHueApplied;
			cm.convertBitmap(resultBitmap, resultBitmap, masks[l].Mask, deltaHueApplied);

			/// save the setup in hlsInfo
			hlsTextInstance.Mods[l].DHue= deltaHueApplied;
			hlsTextInstance.Mods[l].DLum= cm.Lightness;
			hlsTextInstance.Mods[l].DSat= cm.Saturation;

			/// complete the file name
			outputFileName += bi.DefaultSeparator + bi.ColorMasks[maskID].CMs[colorID].ColID;
		}
		
		// save good hlsInfo instance name
		hlsTextInstance.Name= outputFileName + bi.OutputFormat;

		nlwarning("Writing %s", outputFileName.c_str());
		/// Save the result. We let propagate exceptions (if there's no more space disk it useless to continue...)
		{
			try
			{
				NLMISC::COFile os;
				if (os.open(bi.OutputPath + "/" + outputFileName + bi.OutputFormat))
				{
					// divide by 2 when needed.
					if(mustDivideBy2)
						resultBitmap.resample( (resultBitmap.getWidth()+1)/2, (resultBitmap.getHeight()+1)/2 );
					// write the file
					if (bi.OutputFormat == ".png")
					{
						resultBitmap.writePNG(os, depth);
					}
					else
					{
						resultBitmap.writeTGA(os, depth);
					}
				}
				else
				{
					nlwarning(("Couldn't open " + bi.OutputPath + outputFileName + bi.OutputFormat + " for writing").c_str());
				}
			}
			catch(const NLMISC::EStream &e)
			{
				nlwarning(("Couldn't write " + bi.OutputPath + outputFileName + bi.OutputFormat + " : " + e.what()).c_str());
			}
		}
		

		/// increment counters		
		for (l  = 0; l < (uint) masks.size(); ++l)
		{
			++ (masks[l].Counter);

			/// check if we have done all colors for this mask
			if (masks[l].Counter == bi.ColorMasks[masks[l].MaskID].CMs.size())
			{
				masks[l].Counter = 0;
			}
			else
			{
				break;
			}
		}
		if (l == masks.size()) break; // all cases dones
	}

	// **** save the TMP hlsInfo
	NLMISC::COFile os;
	if (os.open(bi.HlsInfoPath + fileName + ".hlsinfo"))
	{
		os.serial(hlsInfo);
	}
	else
	{
		nlwarning(("Couldn't write " + bi.HlsInfoPath + fileName + ".hlsinfo").c_str());
	}

}
