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

#ifndef WE_EXPORT_H
#define WE_EXPORT_H

// ---------------------------------------------------------------------------


#include <string>
#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/zone.h"

// ---------------------------------------------------------------------------

namespace NLMISC
{
	class IStream;
	class CBitmap;
	class CMatrix;
	class CQuat;
	class CVector;
}

namespace NL3D
{
	class CZone;
	struct CPatchInfo;
	class CBezierPatch;
	class CTileBank;
	class CInstanceGroup;
}

namespace NLLIGO
{
	class CZoneRegion;
	class CZoneBank;
	struct SPiece;
}

namespace NLPACS
{
	class CCollisionMeshBuild ;
}


namespace NLGEORGES
{
	class UForm;
	class UFormLoader;
}


// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------
struct SExportOptions
{
	// Options saved
	std::string		OutZoneDir;
	std::string		OutIGDir;
	std::string		RefZoneDir;
	std::string		RefIGDir;
	bool			ExportCollisions;
	bool			ExportAdditionnalIGs;
	std::string     RefCMBDir;
	std::string     OutCMBDir;
	std::string		DFNDir;
	std::string     AdditionnalIGInDir; // villages..
	std::string     AdditionnalIGOutDir;
	
	std::string		ContinentFile;
	std::string		ContinentsDir;
	
	std::string		LigoBankDir;
	std::string		TileBankFile;
	
	// The colormap file name
	std::string		ColorMapFile;

	std::string		HeightMapFile;
	float			ZFactor;
	std::string		HeightMapFile2;
	float			ZFactor2;

	uint8			Light; // Roughly light the zone (0-none, 1-patch, 2-noise)

	std::string		ZoneMin;
	std::string		ZoneMax;

	
	
	// Options not saved
	std::string				ZoneRegionFile;
	NLLIGO::CZoneRegion		*ZoneRegion; // The region to make
	float					CellSize;
	float					Threshold;

	// =======================================================================
	
	SExportOptions ();
	void serial (NLMISC::IStream& s);
};

// ---------------------------------------------------------------------------
// Export callback
// ---------------------------------------------------------------------------
// The user of CExport can be informed of what's happen in the export process with
// this class and can cancel the process by returning true in the isCanceled method.
class IExportCB
{
public:
	virtual bool isCanceled () = 0; // Tell the exporter if it must end as quick as possible
	// Display callbacks
	virtual void dispPass (const std::string &Text) = 0; // Pass (generate land, vegetable, etc...)
	virtual void dispPassProgress (float percentage) = 0; // [ 0.0 , 1.0 ]
	virtual void dispInfo (const std::string &Text) = 0; // Verbose
	virtual void dispWarning (const std::string &Text) = 0; // Error but not critical
	virtual void dispError (const std::string &Text) = 0; // Should block (misfunction)
};

// ---------------------------------------------------------------------------

class CExport
{

public:

	CExport ();
	~CExport ();
	
	// EXPORT :
	bool export_ (SExportOptions &options, IExportCB *expCB = NULL);

	static std::string getZoneNameFromXY (sint32 x, sint32 y);
	static sint32 getXFromZoneName (const std::string &ZoneName);
	static sint32 getYFromZoneName (const std::string &ZoneName);

private:	
	SExportOptions		*_Options;
	IExportCB			*_ExportCB;
	NLLIGO::CZoneBank	*_ZeZoneBank;

	NL3D::CTileBank		*_ZeTileBank;
	NLMISC::CBitmap		*_HeightMap;
	NLMISC::CBitmap		*_HeightMap2;
	sint32				_ZoneMinX, _ZoneMinY, _ZoneMaxX, _ZoneMaxY;	

	// The colormap
	NLMISC::CBitmap		*_ColorMap;


	NLGEORGES::UFormLoader			*_FormLoader;
private:
	/// export all cmb (directory is given in the options)
	void exportCMBs();
	void exportAdditionnalIGs();	
	void treatPattern (sint32 nPosX, sint32 nPosY, 
						std::vector<bool> &ZoneTreated, sint32 nMinX, sint32 nMinY, sint32 nStride);
	/// build a non mirroring transfo (for mesh, collision mesh builds)
	void buildTransfo(sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip, NLMISC::CMatrix &posTransfo, NLMISC::CQuat &rotTranfo);
	void transformZone (NL3D::CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip, bool computeHeightmap);
	void transformIG (NL3D::CInstanceGroup &ig, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip);
	void transformCMB(NLPACS::CCollisionMeshBuild &cmb, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip);
	
	/* 
	   Add the Colormap to the user color of the zone
	   Performs USERCOLOR = USERCOLOR * (1-colormap.A) + colormap.RGB * colormap.A
	*/
	void addColorMap (NL3D::CZone &zeZone);


	void cutZone (NL3D::CZone &bigZone, NL3D::CZone &bigZoneNoHeightmap, NL3D::CZone &unitZone, NL3D::CZone &unitZoneNoHeightmap, 
					   sint32 nPosX, sint32 nPosY, std::vector<bool> &PatchTransfered, const std::vector<NLMISC::CAABBox> &bb, std::vector<NL3D::CPatchInfo> &SrcPI, 
					   std::vector<NL3D::CPatchInfo> &SrcPINoHeightmap, NLLIGO::SPiece &sMask, std::vector<NL3D::CBorderVertex> &BorderVertices, 
					   std::vector<NL3D::CBorderVertex> &BorderVerticesNoHeightmap, sint32 baseX, sint32 baseY);

	void cutIG	(NL3D::CInstanceGroup &bigIG, NL3D::CInstanceGroup &unitIG, sint32 nPosX, sint32 nPosY, NLLIGO::SPiece &sMask, bool first, 
		sint32 baseX, sint32 baseY);

	float	getHeight (float x, float y);

	// Get a filtred RGBA color from _ColorMap
	NLMISC::CRGBAF	getColor (float x, float y);

	// return the normalized height normal.
	NLMISC::CVector	getHeightNormal (float x, float y);

	void light (NL3D::CZone &zoneOut, NL3D::CZone &zoneIn);		

	NLGEORGES::UForm  *loadContinent(const std::string &name) const;
	void			  transformCMB (const std::string &cmbName, const NLMISC::CMatrix &transfo, bool verbose) const;
	void			  transformAdditionnalIG (const std::string &igName, const NLMISC::CMatrix &transfo, const NLMISC::CQuat &rotTransfo) const;
	void			  exportCMBAndAdditionnalIGs();


	// used by transformZone. compute sub tangents along an edge
	void computeSubdividedTangents(uint numBinds, const NL3D::CBezierPatch &patch, uint edge, NLMISC::CVector subTangents[8]);

	// used by transformZone. return false if the vertex was same, and nop in this case. Else change the vertex, 
	// and move related tangent/interiors too
	bool applyVertexBind(NL3D::CPatchInfo &pa, NL3D::CPatchInfo &oldPa, uint edgeToModify, bool startEdge, 
		const NLMISC::CMatrix &oldTgSpace, const NLMISC::CMatrix &newTgSpace, 
		const NLMISC::CVector &bindedPos, const NLMISC::CVector &bindedTangent );

};

#endif // WE_EXPORT_H