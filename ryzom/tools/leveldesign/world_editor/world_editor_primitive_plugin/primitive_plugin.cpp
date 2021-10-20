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

#include "primitive_plugin.h"

#include "../world_editor/world_editor.h"
#include "../world_editor/display.h"
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/load_form.h>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

CFileDisplayer		*PrimitivePluginLogDisplayer= NULL;

extern "C"
{
	void *createPlugin()
	{
		return new CPrimitivePlugin();
	}
}

CPrimitivePlugin::CPrimitivePlugin()
{
	NLMISC::createDebug();
	PrimitivePluginLogDisplayer= new CFileDisplayer("world_editor_primitive_plugin.log", true, "WORLD_EDITOR_PRIMITIVE_PLUGIN.LOG");
	DebugLog->addDisplayer (PrimitivePluginLogDisplayer);
	InfoLog->addDisplayer (PrimitivePluginLogDisplayer);
	WarningLog->addDisplayer (PrimitivePluginLogDisplayer);
	ErrorLog->addDisplayer (PrimitivePluginLogDisplayer);
	AssertLog->addDisplayer (PrimitivePluginLogDisplayer);

	nlinfo("Starting primitive plugin...");

	_PluginActive = false;
	_PluginAccess = NULL;
}


bool		CPrimitivePlugin::isActive()
{
	return _PluginActive;
}

bool		CPrimitivePlugin::activatePlugin()
{
	_PluginActive = true;

	// TODO : open the 'rebuild npc data' dialog box

	return true;
}

bool		CPrimitivePlugin::closePlugin()
{
	_PluginActive = false;
	return true;
}

std::string& CPrimitivePlugin::getName()
{
	static string name("Primitive displayer");
	return name;
}


void	CPrimitivePlugin::TCreatureInfo::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	const NLGEORGES::UFormElm &item=form->getRootNode();

	// the form was found so read the true values from Georges
	item.getValueByName(Radius, "Collision.CollisionRadius");
	HaveRadius = Radius != 0.0f;
	item.getValueByName(Length, "Collision.Length");
	item.getValueByName(Width, "Collision.Width");
	HaveBox = (Length != 0 && Width != 0);
}

void	CPrimitivePlugin::TCreatureInfo::serial (NLMISC::IStream &s)
{
	s.serial(HaveRadius);
	s.serial(Radius);
	s.serial(HaveBox);
	s.serial(Width);
	s.serial(Length);
}

uint CPrimitivePlugin::TCreatureInfo::getVersion ()
{
	return 1;
}



// @{
// \name Overload for IPluginCallback
void		CPrimitivePlugin::init(IPluginAccess *pluginAccess)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();
	_PluginAccess = pluginAccess;

	string packedFileName("primitive_plugin.packed_sheets");

	vector<string>	paths;
	string sheetIdPath;

	// add the search path
	CConfigFile &cf = pluginAccess->getConfigFile();
	CConfigFile::CVar *pv = cf.getVarPtr("PrimitivePluginPath");
	if (pv)
	{
		for (uint i=0; i<pv->size(); ++i)
			paths.push_back(pv->asString(i));
	}
	// add the sheetId file
	pv = cf.getVarPtr("PrimitivePluginSheetId");
	sheetIdPath = pv->asString();

	// Init the sheet id
	CPath::addSearchFile(sheetIdPath);
	CSheetId::init(false);

	// Read the sheets
	if (NLMISC::CFile::fileExists(packedFileName))
		loadForm("creature", packedFileName, _CreatureInfos, false, false);
	else
	{
		for (uint i=0; i<paths.size(); ++i)
		{
			CPath::addSearchPath(paths[i], true, false);
		}

		// build the packed sheet
		loadForm("creature", packedFileName, _CreatureInfos, true, false);
	}

	vector<string>	classNames;
	classNames.push_back("npc_bot");
	_PluginAccess->registerPrimitiveDisplayer(this, classNames);
}

/// The current region has changed.
void		CPrimitivePlugin::primitiveChanged(const NLLIGO::IPrimitive *root)
{
}

void	CPrimitivePlugin::drawPrimitive(const NLLIGO::IPrimitive *primitive, const TRenderContext &ctx)
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	static const float POINT_ARROW_LINE_SIZE = 20.f;
	static const float POINT_ARROW_HEAD_SIZE = 8.f;
	static const float POINT_DOT_SIZE = 3.f;

	static const uint CIRCLE_SEGMENT_SIZE = 20;
	static const uint CIRCLE_MIN_SEGMENT_COUNT = 8;

	std::string *sheetName;
	primitive->getPropertyByName("sheet_client", sheetName);

	TCreatureInfo *info = NULL;
	if (!sheetName->empty())
	{
		// two step init of id to remove a flooding NeL warning
		CSheetId id;
		id.buildSheetId(*sheetName+".creature");
		std::map<NLMISC::CSheetId, TCreatureInfo>::iterator it(_CreatureInfos.find(id));
		if (it != _CreatureInfos.end())
		{
			info = &(it->second);
		}
	}


	const CPrimPoint *point = dynamic_cast<const CPrimPoint *>(primitive);
	if (point)
	{
		// Clip ?
		if (!ctx.Display->isClipped (&point->Point, 1))
		{
			// Position in world
			CVector center = point->Point;
			ctx.Display->worldToPixel (center);

			// Dot
			CVector dot0, dot1, dot2, dot3;
			dot0 = center;
			dot0.x += POINT_DOT_SIZE;
			dot0.y += POINT_DOT_SIZE;
			dot1 = center;
			dot1.x -= POINT_DOT_SIZE;
			dot1.y += POINT_DOT_SIZE;
			dot2 = center;
			dot2.x -= POINT_DOT_SIZE;
			dot2.y -= POINT_DOT_SIZE;
			dot3 = center;
			dot3.x += POINT_DOT_SIZE;
			dot3.y -= POINT_DOT_SIZE;

			// Transform primitive
			transformVector (dot0, point->Angle, center);
			transformVector (dot1, point->Angle, center);
			transformVector (dot2, point->Angle, center);
			transformVector (dot3, point->Angle, center);

			// In world space
			ctx.Display->pixelToWorld (center);
			ctx.Display->pixelToWorld (dot0);
			ctx.Display->pixelToWorld (dot1);
			ctx.Display->pixelToWorld (dot2);
			ctx.Display->pixelToWorld (dot3);

			// Draw it
			ctx.Display->triRenderProxy (ctx.MainColor, dot0, dot1, dot2, ctx.Selected?2:0);
			ctx.Display->triRenderProxy (ctx.MainColor, dot2, dot3, dot0, ctx.Selected?2:0);

			// Need detail?
			if (ctx.ShowDetail)
			{
				// Prim class available ?
				const CPrimitiveClass *primClass = ctx.PrimitiveClass;
				if (primClass != NULL)
				{
					// Draw an arraow ?
					if (primClass->ShowArrow)
					{
						// Position in world
						center = point->Point;
						ctx.Display->worldToPixel (center);
						CVector arrow = center;
						CVector arrow0 = center;
						arrow.x += POINT_ARROW_LINE_SIZE;
						CVector arrow1 = arrow;
						CVector arrow2 = arrow;
						arrow0.x += POINT_ARROW_LINE_SIZE + POINT_ARROW_HEAD_SIZE;
						arrow1.y += POINT_ARROW_HEAD_SIZE;
						arrow2.y -= POINT_ARROW_HEAD_SIZE;

						// Transform primitive
						transformVector (arrow, point->Angle, center);
						transformVector (arrow0, point->Angle, center);
						transformVector (arrow1, point->Angle, center);
						transformVector (arrow2, point->Angle, center);

						// In world space
						ctx.Display->pixelToWorld (center);
						ctx.Display->pixelToWorld (arrow);
						ctx.Display->pixelToWorld (arrow0);
						ctx.Display->pixelToWorld (arrow1);
						ctx.Display->pixelToWorld (arrow2);

						// Draw it
						ctx.Display->lineRenderProxy (ctx.MainColor, center, arrow, ctx.Selected?2:0);
						ctx.Display->triRenderProxy (ctx.ArrowColor, arrow0, arrow1, arrow2, ctx.Selected?2:0);
					}
				}

				// Have bounding info ?
				if (info != NULL)
				{
					if (info->HaveRadius)
					{
						// Get it
						float fRadius = info->Radius;

						// Get the perimeter
						float perimeter = 2.f*(float)Pi*fRadius;

						// Get the perimeter on the screen
						perimeter *= (float)ctx.Display->getWidth() / (ctx.Display->_CurViewMax.x - ctx.Display->_CurViewMin.x);
						
						// Get the segement count
						perimeter /= (float)CIRCLE_SEGMENT_SIZE;
						
						// Clamp
						if (perimeter < CIRCLE_MIN_SEGMENT_COUNT)
							perimeter = CIRCLE_MIN_SEGMENT_COUNT;

						// Segment count
						uint segmentCount = (uint)perimeter;

						// Draw a circle
						CVector posInit = center;
						posInit.x += fRadius;
						CVector posPrevious = posInit;
						for (uint i=1; i<segmentCount+1; i++)
						{
							CVector pos = posInit;
							transformVector (pos, (float)i*2.f*(float)Pi/(float)segmentCount, center);
							ctx.Display->lineRenderProxy (ctx.MainColor, pos, posPrevious, ctx.Selected?2:0);
							posPrevious = pos;
						}
					}
					if (info->HaveBox)
					{
						CVector center = point->Point;
//						ctx.Display->worldToPixel (center);

						// Dot
						CVector dot0, dot1, dot2, dot3;
						dot0 = center;
						dot0.x += info->Length/2;
						dot0.y += info->Width/2;
						dot1 = center;
						dot1.x -= info->Length/2;
						dot1.y += info->Width/2;
						dot2 = center;
						dot2.x -= info->Length/2;
						dot2.y -= info->Width/2;
						dot3 = center;
						dot3.x += info->Length/2;
						dot3.y -= info->Width/2;

						// Transform primitive
						transformVector (dot0, point->Angle, center);
						transformVector (dot1, point->Angle, center);
						transformVector (dot2, point->Angle, center);
						transformVector (dot3, point->Angle, center);

						// In world space
/*						ctx.Display->pixelToWorld (center);
						ctx.Display->pixelToWorld (dot0);
						ctx.Display->pixelToWorld (dot1);
						ctx.Display->pixelToWorld (dot2);
						ctx.Display->pixelToWorld (dot3);
*/
						// Draw it
						ctx.Display->lineRenderProxy (ctx.MainColor, dot0, dot1, ctx.Selected?2:0);
						ctx.Display->lineRenderProxy (ctx.MainColor, dot1, dot2, ctx.Selected?2:0);
						ctx.Display->lineRenderProxy (ctx.MainColor, dot2, dot3, ctx.Selected?2:0);
						ctx.Display->lineRenderProxy (ctx.MainColor, dot3, dot0, ctx.Selected?2:0);
					}
				}
			}
		}
	}
	
}


