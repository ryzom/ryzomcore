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

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>

#include "camera.h"
#include "landscape.h"
#include "snowballs_client.h"
#include "mouse_listener.h"

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;

namespace SBCLIENT {

/// If axis segment is longer than this value then no lens flare is displayed
static const float _MaxLensFlareLenght = 0.4f;

//
// Functions
//

/**
 * A lens-flare class
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CLensFlare
{
	float _AlphaCoef;

	/// flare
	struct _CFlare
	{
		NL3D::UMaterial Material;

		float Width;
		float Height;

		float Location;

		float Scale;
		
		_CFlare(NL3D::UTexture *texture, float width, float height, float location, float scale)
		{
			// -- -- nicely re-usable class, but why should it know about the snowballs specific driver global
			Material = Driver->createMaterial ();
			Material.initUnlit ();
			Material.setTexture (texture);
			Material.setBlendFunc (UMaterial::srcalpha, UMaterial::one);
			Material.setBlend(true);
			Material.setZFunc (UMaterial::always);
			Material.setZWrite (false);

			// quad dimension
			Width = width;
			Height = height;

			// location on the lens-flare ray
			Location = location;

			// texture scale
			Scale = scale;
		}

		~_CFlare()
		{
			Driver->deleteMaterial(Material);
		}
	};

	/// flares due to light
	std::vector<_CFlare *> _Flares;

public:

	/// constructor
	CLensFlare()
	{
		_AlphaCoef = 1.0f;
	}

	~CLensFlare()
	{
		for (std::vector<_CFlare *>::iterator it(_Flares.begin()), end(_Flares.end()); it != end; ++it)
			delete (*it);
		_Flares.clear();
	}

	/// add a flare to the flare list
	void addFlare(NL3D::UTexture * texture, float width, float height, float location = 1.f, float scale = 1.f);

	void setAlphaCoef(float coef)
	{
		_AlphaCoef = coef;
	}

	// -- -- random note: show actually shows and is used within update
	/// lens flare display function
	void show();
};


/*********************************************************\
					addFlare()
\*********************************************************/
void CLensFlare::addFlare(UTexture * texture, float width, float height, float location, float scale)
{
	_Flares.push_back(new _CFlare(texture, width, height, location, scale));
}


/*********************************************************\
						show()
\*********************************************************/
void CLensFlare::show()
{
	CMatrix mtx;
	mtx.identity();

	nlassert(Driver!=NULL && !Camera.empty());

	Driver->setMatrixMode2D11 ();

	// Determining axis "screen center - light" vector
	CMatrix cameraMatrix = Camera.getMatrix();
	cameraMatrix.invert();
	CVector light = (-100000 * SunDirection);
	light = cameraMatrix * light;
	light = Camera.getFrustum().project(light);
	
	CVector screenCenter(0.5f,0.5f,0);
	CVector axis = light - screenCenter;

	if(axis.norm()>_MaxLensFlareLenght)
	{
		return;
	}

	// rendering flares
	vector<_CFlare *>::iterator itflr;
	for(itflr = _Flares.begin(); itflr!=_Flares.end(); itflr++)
	{
		(*itflr)->Material.setColor(CRGBA(255,255,255,(uint8)(_AlphaCoef*255)));
			
		CQuadUV quad;
		
		float xCenterQuad = screenCenter.x + (*itflr)->Location * axis.x;
		float yCenterQuad = screenCenter.y + (*itflr)->Location * axis.y;
		
		float x,y;


		x = xCenterQuad - (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad - (*itflr)->Height * (*itflr)->Scale / 2.f;
		quad.V0.set (x, y, 0);
		
		x = xCenterQuad + (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad - (*itflr)->Height * (*itflr)->Scale / 2.f;
		quad.V1.set (x, y, 0);
		
		x = xCenterQuad + (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad + (*itflr)->Height * (*itflr)->Scale / 2.f;
		quad.V2.set (x, y, 0);
		
		x = xCenterQuad - (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad + (*itflr)->Height * (*itflr)->Scale / 2.f;
		quad.V3.set (x, y, 0);

		quad.Uv0.U = 0.0f; quad.Uv0.V = 1.0f;
		quad.Uv1.U = 1.0f; quad.Uv1.V = 1.0f;
		quad.Uv2.U = 1.0f; quad.Uv2.V = 0.0f;
		quad.Uv3.U = 0.0f; quad.Uv3.V = 0.0f;

		Driver->drawQuad (quad, (*itflr)->Material);
	}
}

static CLensFlare *LensFlare = NULL;
static UTextureFile *flareTexture1 = NULL;
static UTextureFile *flareTexture3 = NULL;
static UTextureFile *flareTexture4 = NULL;
static UTextureFile *flareTexture5 = NULL;
static UTextureFile *flareTexture6 = NULL;
static UTextureFile *flareTexture7 = NULL;

void initLensFlare ()
{
	// -- -- getting this from a config file would be more re-usable

	LensFlare = new CLensFlare ();

	flareTexture1 = Driver->createTextureFile("flare01.tga");
	flareTexture3 = Driver->createTextureFile("flare03.tga");
	flareTexture4 = Driver->createTextureFile("flare04.tga");
	flareTexture5 = Driver->createTextureFile("flare05.tga");
	flareTexture6 = Driver->createTextureFile("flare06.tga");
	flareTexture7 = Driver->createTextureFile("flare07.tga");

	float w = 30/800.0f;
	float h = 30/600.0f;

	// shine
	LensFlare->addFlare (flareTexture3, w, h, 1.f, 16.f);

	LensFlare->addFlare (flareTexture1, w, h, 1.f, 6.f);
	LensFlare->addFlare (flareTexture6, w, h, 1.3f, 1.2f);
	LensFlare->addFlare (flareTexture7, w, h, 1.0f, 3.f);
	LensFlare->addFlare (flareTexture6, w, h, 0.5f, 4.f);
	LensFlare->addFlare (flareTexture5, w, h, 0.2f, 2.f);
	LensFlare->addFlare (flareTexture7, w, h, 0.0f, 0.8f);
	LensFlare->addFlare (flareTexture7, w, h, -0.25f, 2.f);
	LensFlare->addFlare (flareTexture1, w, h, -0.4f, 1.f);
	LensFlare->addFlare (flareTexture4, w, h, -1.0f, 12.f);
	LensFlare->addFlare (flareTexture5, w, h, -0.6f, 6.f);
}

void updateLensFlare ()
{
	// -- -- todo: see how much of this can be modified to depend on nel 
	//       things only, and moved into the lensflare class

	// vector to sun
	//==============
	CVector userLook = MouseListener->getViewDirection ();

	CVector sunDirection = (-100000 * SunDirection);

	// cosinus between the two previous vectors
	//=========================================
	float cosAngle = sunDirection*userLook/sunDirection.norm();	

	// alpha
	//======
	float alphaf;
	if(cosAngle<0) 
	{
		alphaf = 0;
	}
	else
	{
		alphaf = 255*(float)(pow(cosAngle,20));
	}

	// landscape's masking sun ?
	//==========================
	CMatrix camMatrix;
	camMatrix = Camera.getMatrix();
	camMatrix.setPos(CVector::Null);
	camMatrix.invert();
	CVector tmp = camMatrix * sunDirection;
	tmp = Camera.getFrustum().project(tmp);
	uint32	w,h;
	Driver->getWindowSize(w,h);
	float sunRadius = 24; // -- -- why 24
	CRect rect((uint32)(tmp.x*w)-(uint32)sunRadius,(uint32)(tmp.y*h)-(uint32)sunRadius,2*(uint32)sunRadius,2*(uint32)sunRadius);
	vector<float> zbuff;
	Driver->getZBufferPart(zbuff, rect);
	float view = 0.f;
	float sum = 0;
	sint i; // -- -- signed?
	for(i=0; i<(sint)zbuff.size(); i++)
	{
		if(zbuff[i]>=0.99999f) sum ++;
	}
	view = sum/(sunRadius*2*sunRadius*2);

	Driver->setMatrixMode2D11 ();

	// quad for dazzle 
	//================
	uint8 alpha = (uint8)(alphaf*view/2.0f);
	if(alpha!=0)
	{
		Driver->drawQuad(0,0,1,1,CRGBA(255,255,255,alpha));
	}

	// Display lens-flare
	LensFlare->setAlphaCoef( 1.f - (float)cos(alphaf*view*Pi/(2.f*255.f)) );
	LensFlare->show();
}

void releaseLensFlare ()
{
	delete LensFlare; LensFlare = NULL;
	Driver->deleteTextureFile(flareTexture1); flareTexture1 = NULL;
	Driver->deleteTextureFile(flareTexture3); flareTexture3 = NULL;
	Driver->deleteTextureFile(flareTexture4); flareTexture4 = NULL;
	Driver->deleteTextureFile(flareTexture5); flareTexture5 = NULL;
	Driver->deleteTextureFile(flareTexture6); flareTexture6 = NULL;
	Driver->deleteTextureFile(flareTexture7); flareTexture7 = NULL;
}

} /* namespace SBCLIENT */

/* end of file */
