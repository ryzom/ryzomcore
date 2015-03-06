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

#include "stdpch.h"
#include "main_loop_temp.h"

#include "global.h"

// tempDumpValidPolys
#include <nel/gui/interface_element.h>
#include <nel/gui/ctrl_polygon.h>
#include "interface_v3/interface_manager.h"

// tempDumpColPolys
#include <nel/3d/packed_world.h>
#include "r2/editor.h"
#include "user_entity.h"
#include <nel/3d/driver_user.h>

using namespace NLMISC;
using namespace NL3D;

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

// TMP TMP
void tempDumpValidPolys()
{
	struct CPolyDisp : public CInterfaceElementVisitor
	{
		virtual void visitCtrl(CCtrlBase *ctrl)
		{
			CCtrlPolygon *cp = dynamic_cast<CCtrlPolygon *>(ctrl);
			if (cp)
			{
				sint32 cornerX, cornerY;
				cp->getParent()->getCorner(cornerX, cornerY, cp->getParentPosRef());
				for(sint32 y = 0; y < (sint32) Screen.getHeight(); ++y)
				{
					for(sint32 x = 0; x < (sint32) Screen.getWidth(); ++x)
					{
						if (cp->contains(CVector2f((float) (x - cornerX), (float) (y - cornerY))))
						{
							((CRGBA *) &Screen.getPixels()[0])[x + (Screen.getHeight() - 1 - y) * Screen.getWidth()] = CRGBA::Magenta;
						}
					}
				}
			}
		}
		CBitmap Screen;
	} polyDisp;
	Driver->getBuffer(polyDisp.Screen);
	CInterfaceManager::getInstance()->visit(&polyDisp);
	COFile output("poly.tga");
	polyDisp.Screen.writeTGA(output);
}

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

// TMP TMP
static void viewportToScissor(const CViewport &vp, CScissor &scissor)
{
	scissor.X = vp.getX();
	scissor.Y = vp.getY();
	scissor.Width = vp.getWidth();
	scissor.Height = vp.getHeight();
}

// TMP TMP
void tempDumpColPolys()
{
	CPackedWorld *pw = R2::getEditor().getIslandCollision().getPackedIsland();
	if (pw)
	{
		static CMaterial material;
		static CMaterial wiredMaterial;
		static CMaterial texturedMaterial;
		static CVertexBuffer vb;
		static bool initDone = false;
		if (!initDone)
		{
			vb.setVertexFormat(CVertexBuffer::PositionFlag);
			vb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
			material.initUnlit();
			material.setDoubleSided(true);
			material.setZFunc(CMaterial::lessequal);
			wiredMaterial.initUnlit();
			wiredMaterial.setDoubleSided(true);
			wiredMaterial.setZFunc(CMaterial::lessequal);
			wiredMaterial.setColor(CRGBA(255, 255, 255, 250));
			wiredMaterial.texEnvOpAlpha(0, CMaterial::Replace);
			wiredMaterial.texEnvArg0Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
			wiredMaterial.setBlend(true);
			wiredMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
			texturedMaterial.initUnlit();
			texturedMaterial.setDoubleSided(true);
			texturedMaterial.setZFunc(CMaterial::lessequal);
			initDone = true;
		}
		// just add a projected texture
		R2::getEditor().getIslandCollision().loadEntryPoints();
		R2::CScenarioEntryPoints &sep = R2::CScenarioEntryPoints::getInstance();
		CVectorD playerPos = UserEntity->pos();
		R2::CScenarioEntryPoints::CCompleteIsland *island = sep.getCompleteIslandFromCoords(CVector2f((float) playerPos.x, (float) playerPos.y));
		static CSString currIsland;
		if (island && island->Island != currIsland)
		{
			currIsland = island->Island;
			CTextureFile *newTex = new CTextureFile(currIsland + "_sp.tga");
			newTex->setWrapS(ITexture::Clamp);
			newTex->setWrapT(ITexture::Clamp);
			texturedMaterial.setTexture(0, newTex);
			texturedMaterial.texEnvOpRGB(0, CMaterial::Replace);
			texturedMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
			texturedMaterial.setTexCoordGen(0, true);
			texturedMaterial.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);
			CMatrix mat;
			CVector scale((float) (island->XMax -  island->XMin),
						  (float) (island->YMax -  island->YMin), 0.f);
			scale.x = 1.f / favoid0(scale.x);
			scale.y = 1.f / favoid0(scale.y);
			scale.z = 0.f;
			mat.setScale(scale);
			mat.setPos(CVector(- island->XMin * scale.x, - island->YMin * scale.y, 0.f));
			//
			CMatrix uvScaleMat;
			//
			uint texWidth = (uint) (island->XMax -  island->XMin);
			uint texHeight = (uint) (island->YMax -  island->YMin);
			float UScale = (float) texWidth / 	raiseToNextPowerOf2(texWidth);
			float VScale = (float) texHeight / raiseToNextPowerOf2(texHeight);
			//
			uvScaleMat.setScale(CVector(UScale, - VScale, 0.f));
			uvScaleMat.setPos(CVector(0.f, VScale, 0.f));
			//
			texturedMaterial.enableUserTexMat(0, true);
			texturedMaterial.setUserTexMat(0, uvScaleMat * mat);
		}
		const CFrustum &frust = MainCam.getFrustum();

		//
		IDriver *driver = ((CDriverUser  *) Driver)->getDriver();

		driver->enableFog(true);
		const CRGBA clearColor = CRGBA(0, 0, 127, 0);
		driver->setupFog(frust.Far * 0.8f, frust.Far, clearColor);
		CViewport vp;
		vp.init(0.f, 0.f, 1.f, 1.f);
		driver->setupViewport(vp);
		CScissor scissor;
		viewportToScissor(vp, scissor);
		driver->setupScissor(scissor);
		//
		driver->setFrustum(frust.Left, frust.Right, frust.Bottom, frust.Top, frust.Near, frust.Far, frust.Perspective);
		driver->setupViewMatrix(MainCam.getMatrix().inverted());
		driver->setupModelMatrix(CMatrix::Identity);
		//
		//
		const CVector localFrustCorners[8] =
		{
			CVector(frust.Left, frust.Near, frust.Top),
			CVector(frust.Right, frust.Near, frust.Top),
			CVector(frust.Right, frust.Near, frust.Bottom),
			CVector(frust.Left, frust.Near, frust.Bottom),
			CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
			CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
			CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near),
			CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near)
		};
		// roughly compute covered zones
		//
		/*
		sint frustZoneMinX = INT_MAX;
		sint frustZoneMaxX = INT_MIN;
		sint frustZoneMinY = INT_MAX;
		sint frustZoneMaxY = INT_MIN;
		for(uint k = 0; k < sizeofarray(localFrustCorners); ++k)
		{
			CVector corner = camMat * localFrustCorners[k];
			sint zoneX = (sint) (corner.x / 160.f) - zoneMinX;
			sint zoneY = (sint) floorf(corner.y / 160.f) - zoneMinY;
			frustZoneMinX = std::min(frustZoneMinX, zoneX);
			frustZoneMinY = std::min(frustZoneMinY, zoneY);
			frustZoneMaxX = std::max(frustZoneMaxX, zoneX);
			frustZoneMaxY = std::max(frustZoneMaxY, zoneY);
		}
		*/

		const uint TRI_BATCH_SIZE = 10000; // batch size for rendering
		static std::vector<TPackedZoneBaseSPtr> zones;
		zones.clear();
		pw->getZones(zones);
		for(uint k = 0; k < zones.size(); ++k)
		{
			zones[k]->render(vb, *driver, texturedMaterial, wiredMaterial, MainCam.getMatrix(), TRI_BATCH_SIZE, localFrustCorners);
		}
	}
}

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

/* end of file */