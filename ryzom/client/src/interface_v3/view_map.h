// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Winch Gate Property Limited
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

#ifndef CL_VIEW_MAP_CL
#define CL_VIEW_MAP_CL

#include "nel/gui/view_base.h"
namespace NL3D
{
	class UMaterial;
	class UTextureFile;
}

class CViewMap : public CViewBase
{
public:
	CViewMap(const TCtorParam &param);
	virtual ~CViewMap();

	virtual void updateCoords();
	virtual void draw();

	bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);

	void setMap(const std::string &texture, NLMISC::CVector2f minCorner, NLMISC::CVector2f maxCorner);

	void setPosition(NLMISC::CVector2f pos) { m_Position = pos; }
	void setRotateZ(float z) { m_RotateZ = z; }

	// set/get map offset in meters
	void setOffset(NLMISC::CVector2f offset) { m_Offset = offset; }
	NLMISC::CVector2f getOffset() const { return m_Offset; }

	void setWorldSize(float ws);
	float getWorldSize() const { return m_WorldSize; }

	REFLECT_EXPORT_START(CViewMap, CViewBase)
		REFLECT_FLOAT ("world_size", getWorldSize, setWorldSize);
	REFLECT_EXPORT_END

private:
	// update real vision based on gui widget size
	void updateVision();
	// update map ingame area size and min/max corners
	void updateMapSize();

	void loadTexture();
	void unloadTexture();

	void loadTextureMask();
	void unloadTextureMask();

	void computeUVRect(NLMISC::CUV &minUV, NLMISC::CUV &maxUV, sint32 &dX, sint32 &dY, sint32 &dW, sint32 &dH) const;

private:
	// center position (in meters)
	NLMISC::CVector2f m_Position;
	// position offset (in meters)
	NLMISC::CVector2f m_Offset;
	// visible area (in meters), ie 500 is 250m radar radius
	float m_WorldSize;
	// visible area centered on gui window (in meters)
	NLMISC::CVector2f m_VisionReal;

	float m_RotateZ;

	// min/max corners (in meters)
	NLMISC::CVector2f m_MinCorner;
	NLMISC::CVector2f m_MaxCorner;
	float m_MapWidth;
	float m_MapHeight;

	// map texture
	bool m_ReloadTexture;
	std::string m_Texture;
	// map texture size in next pow2
	uint32 m_TextureWidth;
	uint32 m_TextureHeight;
	float m_MapURatio;
	float m_MapVRatio;
	NL3D::UTextureFile *m_TextureFile;
	NL3D::UMaterial m_Material;

	// mask to get circular radar
	bool m_ReloadTextureMask;
	std::string m_TextureMask;
	NL3D::UTextureFile *m_TextureMaskFile;
	NL3D::UMaterial m_MaskMaterial;
};

#endif // CL_VIEW_MAP_CL

