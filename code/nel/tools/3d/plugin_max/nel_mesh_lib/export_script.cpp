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

#include "stdafx.h"
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/compiler/parser.h>
#	include <maxscript/foundation/strings.h>
#else
#	include <MaxScrpt/parser.h>
#	include <MaxScrpt/strings.h>
#endif
#include "export_nel.h"
#include "export_appdata.h"

// ***************************************************************************

bool CExportNel::scriptEvaluate (const char *script, void *out, TNelScriptValueType type)
{
	BOOL result=TRUE;
	init_thread_locals();

	four_typed_value_locals(Parser* parser,Value* code,Value* result,StringStream* source);

	vl.parser = new Parser;
	vl.source = new StringStream(MaxTStrFromUtf8(script));
	vl.source->log_to(NULL);

#if MAX_VERSION_MAJOR < 19
	save_current_frames();
#endif

	try
	{
#if MAX_VERSION_MAJOR >= 19
		ScopedSaveCurrentFrames currentFrames;
#endif

		vl.source->flush_whitespace();
		vl.code = vl.parser->compile_all(vl.source);
		vl.result = vl.code->eval();
		
		vl.source->flush_whitespace();
		vl.source->close();

		// No prb ?
		if (vl.result!=FALSE)
		{
			switch (type)
			{
			case scriptFloat:
				*(float*)out=vl.result->to_float();
				break;
			case scriptBool:
				*(bool*)out=vl.result->to_bool()!=FALSE;
				break;
			case scriptNode:
				*(INode**)out= vl.result->to_node();
			}
		}
	}
	catch (...)
	{
#if MAX_VERSION_MAJOR < 19
		restore_current_frames();
#endif
		result=FALSE;
		vl.source->close();
	}

#if MAX_VERSION_MAJOR < 19
	pop_value_locals();
#endif
	return (result!=FALSE);
}

// ***************************************************************************

int CExportNel::getScriptAppData (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (NLMISC::fromString(std::string((const char*)ap->data), value))
		return value;
	else
		return def;
}

// ***************************************************************************

void CExportNel::setScriptAppData (Animatable *node, uint32 id, int value)
{
	// Int to string
	std::string block = NLMISC::toString(value);

	// Remove data
	node->RemoveAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Copy data
	char *copy = (char *)MAX_malloc(block.size() + 1);
	strcpy(copy, block.c_str());

	// Add data
	node->AddAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id, block.size() + 1, copy);

}

// ***************************************************************************

float CExportNel::getScriptAppData (Animatable *node, uint32 id, float def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	float value = 0.f;
	if (toFloatMax((const TCHAR*)ap->data, value))
		return value;
	else
		return def;
}

// ***************************************************************************

void CExportNel::setScriptAppData (Animatable *node, uint32 id, float value)
{
	std::string str = toStringMax(value);	

	// Remove data
	node->RemoveAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Copy data
	char *copy = (char *)MAX_malloc(str.length() + 1);
	strcpy (copy, str.c_str());

	// Add data
	node->AddAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id, str.length() + 1, copy);
}

// ***************************************************************************

std::string CExportNel::getScriptAppData (Animatable *node, uint32 id, const std::string& def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	if (((const char*)ap->data)[ap->length - 1] == 0)
		return (const char*)ap->data;
	else
		return def;
}

// ***************************************************************************

void CExportNel::setScriptAppData (Animatable *node, uint32 id, const std::string& value)
{
	// Remove data
	node->RemoveAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Copy data
	char *copy = (char *)MAX_malloc(value.length() + 1);
	strcpy(copy, value.c_str());

	// Add data
	node->AddAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id, value.size() + 1, copy);
}


// ***************************************************************************

NLMISC::CRGBA CExportNel::getScriptAppData (Animatable *node, uint32 id, NLMISC::CRGBA def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);
	
	// Not found ? return default
	if (ap == NULL)
		return def;
	
	// String to RGBA
	if (((const char*)ap->data)[ap->length - 1] == 0)
	{
		const	char *ptr= (const char*)ap->data;
		int r = 255, g = 255, b = 255, a = 255;	
		sscanf (ptr, "%d %d %d %d", &r, &g, &b, &a);
		NLMISC::clamp (r, 0, 255);
		NLMISC::clamp (g, 0, 255);
		NLMISC::clamp (b, 0, 255);
		NLMISC::clamp (a, 0, 255);
		return NLMISC::CRGBA(r,g,b,a);
	}
	else
		return def;
}

// ***************************************************************************

void CExportNel::setScriptAppData (Animatable *node, uint32 id, NLMISC::CRGBA val)
{
	// Remove data
	node->RemoveAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);
	
	// RGBA to string
	std::string	value = NLMISC::toString("%d %d %d %d", val.R, val.G, val.B, val.A);

	// Copy data
	char *copy = (char *)MAX_malloc(value.length() + 1);
	strcpy(copy, value.c_str());
	
	// Add data
	node->AddAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id, value.size() + 1, copy);
}


// ***************************************************************************
void CExportNel::getScriptAppDataVPWT (Animatable *node, CVPWindTreeAppData &apd)
{
	nlassert(NEL3D_APPDATA_VPWT_LEVELMAX == CVPWindTreeAppData::HrcDepth);

	apd.FreqScale= getScriptAppData(node, NEL3D_APPDATA_VPWT_FREQ_SCALE, 2.f);
	apd.DistScale= getScriptAppData(node, NEL3D_APPDATA_VPWT_DIST_SCALE, 2.f);
	apd.SpecularLighting= getScriptAppData(node, NEL3D_APPDATA_VPWT_USE_SPEC, BST_UNCHECKED);

	for(uint i=0; i<CVPWindTreeAppData::HrcDepth; i++)
	{
		// Default frequence of 2*0.1= 0.2f; Level0 only
		int		defFreq= i==0?(CVPWindTreeAppData::NumTicks)/10:0;
		// Default Amplitude of 2*0.5= 1.f; Level0 only
		int		defDistXY= i==0?(CVPWindTreeAppData::NumTicks)/2:0;
		// get appData.
		apd.Frequency[i]= getScriptAppData(node, NEL3D_AppDataVPWTFreq[i], defFreq);
		apd.FrequencyWindFactor[i]= getScriptAppData(node, NEL3D_AppDataVPWTFreqWD[i], 0);
		apd.DistXY[i]= getScriptAppData(node, NEL3D_AppDataVPWTDistXY[i], defDistXY);
		apd.DistZ[i]= getScriptAppData(node, NEL3D_AppDataVPWTDistZ[i], 0);
		apd.Bias[i]= getScriptAppData(node, NEL3D_AppDataVPWTBias[i], CVPWindTreeAppData::NumTicks/2);
	}
}

// ***************************************************************************
void CExportNel::setScriptAppDataVPWT (Animatable *node, const CVPWindTreeAppData &apd)
{
	nlassert(NEL3D_APPDATA_VPWT_LEVELMAX == CVPWindTreeAppData::HrcDepth);

	setScriptAppData(node, NEL3D_APPDATA_VPWT_FREQ_SCALE, apd.FreqScale);
	setScriptAppData(node, NEL3D_APPDATA_VPWT_DIST_SCALE, apd.DistScale);
	setScriptAppData(node, NEL3D_APPDATA_VPWT_USE_SPEC, apd.SpecularLighting);

	for(uint i=0; i<CVPWindTreeAppData::HrcDepth; i++)
	{
		// set appData.
		setScriptAppData(node, NEL3D_AppDataVPWTFreq[i], apd.Frequency[i]);
		setScriptAppData(node, NEL3D_AppDataVPWTFreqWD[i], apd.FrequencyWindFactor[i]);
		setScriptAppData(node, NEL3D_AppDataVPWTDistXY[i], apd.DistXY[i]);
		setScriptAppData(node, NEL3D_AppDataVPWTDistZ[i], apd.DistZ[i]);
		setScriptAppData(node, NEL3D_AppDataVPWTBias[i], apd.Bias[i]);
	}
}



