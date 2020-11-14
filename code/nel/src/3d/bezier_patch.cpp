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

#include "std3d.h"

#include "nel/3d/bezier_patch.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

// ***************************************************************************
void		CBezierPatch::make(CVector vertices[4], CVector	normals[4])
{
	sint	i;
	static	sint	starts[8]= {0,1, 1,2, 2,3, 3,0};
	static	sint	ends[8]= {1,0, 2,1, 3,2, 0,3};

	for(i=0;i<4;i++)
		Vertices[i]= vertices[i];

	// For all tangents.
	for(i=0;i<8;i++)
	{
		CVector		tgt= Vertices[ends[i]] - Vertices[starts[i]];
		CVector		I,J,K;
		J= normals[starts[i]];
		I= J^tgt;
		K= I^J;
		K.normalize();
		// Yes, we store tangents as position, not vectors...
		Tangents[i]= Vertices[starts[i]] + K*tgt.norm()/3;
	}

	makeInteriors();
}

// ***************************************************************************
void		CBezierPatch::makeInteriors()
{
	CVector		&a = Vertices[0];
	CVector		&b = Vertices[1];
	CVector		&c = Vertices[2];
	CVector		&d = Vertices[3];
	Interiors[0] = Tangents[7] + Tangents[0] - a;
	Interiors[1] = Tangents[1] + Tangents[2] - b;
	Interiors[2] = Tangents[3] + Tangents[4] - c;
	Interiors[3] = Tangents[5] + Tangents[6] - d;
}

// ***************************************************************************
void		CBezierPatch::applyMatrix(const CMatrix &m)
{
	sint	i;

	for(i=0;i<4;i++)
		Vertices[i]= m*Vertices[i];
	for(i=0;i<8;i++)
		Tangents[i]= m*Tangents[i];
	for(i=0;i<4;i++)
		Interiors[i]= m*Interiors[i];
}

// ***************************************************************************
static inline	void	mulAdd(CVector &tgt, const CVector &src, float f)
{
	tgt.x+= src.x*f;
	tgt.y+= src.y*f;
	tgt.z+= src.z*f;
}

// ***************************************************************************
static inline	void	mulAddD(CVectorD &tgt, const CVector &src, double f)
{
	tgt.x+= src.x*f;
	tgt.y+= src.y*f;
	tgt.z+= src.z*f;
}

// ***************************************************************************
CVector		CBezierPatch::eval(float ps, float pt) const
{
	CVector	p;

	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float s0 = ps12 * ps1;
	float s1 = 3.0f * ps * ps12;
	float s2 = 3.0f * ps2 * ps1;
	float s3 = ps2 * ps;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;
	float t0 = pt12 * pt1;
	float t1 = 3.0f * pt * pt12;
	float t2 = 3.0f * pt2 * pt1;
	float t3 = pt2 * pt;

	p.set(0,0,0);
	mulAdd(p, Vertices[0] , s0 * t0);
	mulAdd(p, Tangents[7] , s1 * t0);
	mulAdd(p, Tangents[6] , s2 * t0);
	mulAdd(p, Vertices[3] , s3 * t0);
	mulAdd(p, Tangents[0] , s0 * t1);
	mulAdd(p, Interiors[0], s1 * t1);
	mulAdd(p, Interiors[3], s2 * t1);
	mulAdd(p, Tangents[5] , s3 * t1);
	mulAdd(p, Tangents[1] , s0 * t2);
	mulAdd(p, Interiors[1], s1 * t2);
	mulAdd(p, Interiors[2], s2 * t2);
	mulAdd(p, Tangents[4] , s3 * t2);
	mulAdd(p, Vertices[1] , s0 * t3);
	mulAdd(p, Tangents[2] , s1 * t3);
	mulAdd(p, Tangents[3] , s2 * t3);
	mulAdd(p, Vertices[2] , s3 * t3);

	return p;
}

// ***************************************************************************
CVectorD	CBezierPatch::evalDouble(double ps, double pt) const
{
	CVectorD	p;

	double ps2 = ps * ps;
	double ps1 = 1.0f - ps;
	double ps12 = ps1 * ps1;
	double s0 = ps12 * ps1;
	double s1 = 3.0f * ps * ps12;
	double s2 = 3.0f * ps2 * ps1;
	double s3 = ps2 * ps;
	double pt2 = pt * pt;
	double pt1 = 1.0f - pt;
	double pt12 = pt1 * pt1;
	double t0 = pt12 * pt1;
	double t1 = 3.0f * pt * pt12;
	double t2 = 3.0f * pt2 * pt1;
	double t3 = pt2 * pt;

	p.set(0,0,0);
	mulAddD(p, Vertices[0] , s0 * t0);
	mulAddD(p, Tangents[7] , s1 * t0);
	mulAddD(p, Tangents[6] , s2 * t0);
	mulAddD(p, Vertices[3] , s3 * t0);
	mulAddD(p, Tangents[0] , s0 * t1);
	mulAddD(p, Interiors[0], s1 * t1);
	mulAddD(p, Interiors[3], s2 * t1);
	mulAddD(p, Tangents[5] , s3 * t1);
	mulAddD(p, Tangents[1] , s0 * t2);
	mulAddD(p, Interiors[1], s1 * t2);
	mulAddD(p, Interiors[2], s2 * t2);
	mulAddD(p, Tangents[4] , s3 * t2);
	mulAddD(p, Vertices[1] , s0 * t3);
	mulAddD(p, Tangents[2] , s1 * t3);
	mulAddD(p, Tangents[3] , s2 * t3);
	mulAddD(p, Vertices[2] , s3 * t3);

	return p;
}

// ***************************************************************************
CVector		CBezierPatch::evalNormal(float ps, float pt) const
{
	CVector	tgtS, tgtT;

	float s0,s1,s2,s3;
	float t0,t1,t2,t3;
	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;

	// Compute tangentS
	//=================
	// s/ds.
	s0 = -3* ps12;
	s1 = 9*ps2 + 3 -12*ps;
	s2 =-9*ps2 + 6*ps ;
	s3 = 3* ps2;
	// t/dt.
	t0 = pt12 * pt1;
	t1 = 3.0f * pt * pt12;
	t2 = 3.0f * pt2 * pt1;
	t3 = pt2 * pt;

	tgtS.set(0,0,0);
	mulAdd(tgtS, Vertices[0] , s0 * t0);
	mulAdd(tgtS, Tangents[7] , s1 * t0);
	mulAdd(tgtS, Tangents[6] , s2 * t0);
	mulAdd(tgtS, Vertices[3] , s3 * t0);
	mulAdd(tgtS, Tangents[0] , s0 * t1);
	mulAdd(tgtS, Interiors[0], s1 * t1);
	mulAdd(tgtS, Interiors[3], s2 * t1);
	mulAdd(tgtS, Tangents[5] , s3 * t1);
	mulAdd(tgtS, Tangents[1] , s0 * t2);
	mulAdd(tgtS, Interiors[1], s1 * t2);
	mulAdd(tgtS, Interiors[2], s2 * t2);
	mulAdd(tgtS, Tangents[4] , s3 * t2);
	mulAdd(tgtS, Vertices[1] , s0 * t3);
	mulAdd(tgtS, Tangents[2] , s1 * t3);
	mulAdd(tgtS, Tangents[3] , s2 * t3);
	mulAdd(tgtS, Vertices[2] , s3 * t3);

	// Compute tangentT
	//=================
	// s/ds.
	s0 = ps12 * ps1;
	s1 = 3.0f * ps * ps12;
	s2 = 3.0f * ps2 * ps1;
	s3 = ps2 * ps;
	// t/dt.
	t0 = -3* pt12;
	t1 = 9*pt2 + 3 -12*pt;
	t2 =-9*pt2 + 6*pt ;
	t3 = 3* pt2;

	tgtT.set(0,0,0);
	mulAdd(tgtT, Vertices[0] , s0 * t0);
	mulAdd(tgtT, Tangents[7] , s1 * t0);
	mulAdd(tgtT, Tangents[6] , s2 * t0);
	mulAdd(tgtT, Vertices[3] , s3 * t0);
	mulAdd(tgtT, Tangents[0] , s0 * t1);
	mulAdd(tgtT, Interiors[0], s1 * t1);
	mulAdd(tgtT, Interiors[3], s2 * t1);
	mulAdd(tgtT, Tangents[5] , s3 * t1);
	mulAdd(tgtT, Tangents[1] , s0 * t2);
	mulAdd(tgtT, Interiors[1], s1 * t2);
	mulAdd(tgtT, Interiors[2], s2 * t2);
	mulAdd(tgtT, Tangents[4] , s3 * t2);
	mulAdd(tgtT, Vertices[1] , s0 * t3);
	mulAdd(tgtT, Tangents[2] , s1 * t3);
	mulAdd(tgtT, Tangents[3] , s2 * t3);
	mulAdd(tgtT, Vertices[2] , s3 * t3);

	// Return the normal.
	CVector	norm= tgtT^tgtS;
	norm.normalize();
	return norm;
}

// ***************************************************************************
CVector		CBezierPatch::evalTangentS(float ps, float pt) const
{
	CVector	tgtS;

	float s0,s1,s2,s3;
	float t0,t1,t2,t3;
	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;

	// Compute tangentS
	//=================
	// s/ds.
	s0 = -3* ps12;
	s1 = 9*ps2 + 3 -12*ps;
	s2 =-9*ps2 + 6*ps ;
	s3 = 3* ps2;
	// t/dt.
	t0 = pt12 * pt1;
	t1 = 3.0f * pt * pt12;
	t2 = 3.0f * pt2 * pt1;
	t3 = pt2 * pt;

	tgtS.set(0,0,0);
	mulAdd(tgtS, Vertices[0] , s0 * t0);
	mulAdd(tgtS, Tangents[7] , s1 * t0);
	mulAdd(tgtS, Tangents[6] , s2 * t0);
	mulAdd(tgtS, Vertices[3] , s3 * t0);
	mulAdd(tgtS, Tangents[0] , s0 * t1);
	mulAdd(tgtS, Interiors[0], s1 * t1);
	mulAdd(tgtS, Interiors[3], s2 * t1);
	mulAdd(tgtS, Tangents[5] , s3 * t1);
	mulAdd(tgtS, Tangents[1] , s0 * t2);
	mulAdd(tgtS, Interiors[1], s1 * t2);
	mulAdd(tgtS, Interiors[2], s2 * t2);
	mulAdd(tgtS, Tangents[4] , s3 * t2);
	mulAdd(tgtS, Vertices[1] , s0 * t3);
	mulAdd(tgtS, Tangents[2] , s1 * t3);
	mulAdd(tgtS, Tangents[3] , s2 * t3);
	mulAdd(tgtS, Vertices[2] , s3 * t3);

	// Return the tgt normalized
	return tgtS.normed();
}

// ***************************************************************************
CVector		CBezierPatch::evalTangentT(float ps, float pt) const
{
	CVector	tgtT;

	float s0,s1,s2,s3;
	float t0,t1,t2,t3;
	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;

	// Compute tangentT
	//=================
	// s/ds.
	s0 = ps12 * ps1;
	s1 = 3.0f * ps * ps12;
	s2 = 3.0f * ps2 * ps1;
	s3 = ps2 * ps;
	// t/dt.
	t0 = -3* pt12;
	t1 = 9*pt2 + 3 -12*pt;
	t2 =-9*pt2 + 6*pt ;
	t3 = 3* pt2;

	tgtT.set(0,0,0);
	mulAdd(tgtT, Vertices[0] , s0 * t0);
	mulAdd(tgtT, Tangents[7] , s1 * t0);
	mulAdd(tgtT, Tangents[6] , s2 * t0);
	mulAdd(tgtT, Vertices[3] , s3 * t0);
	mulAdd(tgtT, Tangents[0] , s0 * t1);
	mulAdd(tgtT, Interiors[0], s1 * t1);
	mulAdd(tgtT, Interiors[3], s2 * t1);
	mulAdd(tgtT, Tangents[5] , s3 * t1);
	mulAdd(tgtT, Tangents[1] , s0 * t2);
	mulAdd(tgtT, Interiors[1], s1 * t2);
	mulAdd(tgtT, Interiors[2], s2 * t2);
	mulAdd(tgtT, Tangents[4] , s3 * t2);
	mulAdd(tgtT, Vertices[1] , s0 * t3);
	mulAdd(tgtT, Tangents[2] , s1 * t3);
	mulAdd(tgtT, Tangents[3] , s2 * t3);
	mulAdd(tgtT, Vertices[2] , s3 * t3);

	// Return the tgt normalized
	return tgtT.normed();
}

// ***************************************************************************
void		CBezierPatch::CBezierCurve::subdivide(CBezierCurve &left, CBezierCurve &right, float t)
{
	float	t1= 1-t;

	// Subdivide the 2 curves.
	left.P0= P0;
	right.P3= P3;

	left.P1= t1*P0 + t*P1;
	right.P2= t1*P2 + t*P3;
	CVector		middle12= t1*P1 + t*P2;

	left.P2= t1*left.P1 + t*middle12;
	right.P1= t1*middle12 + t*right.P2;

	left.P3= right.P0= t1*left.P2 + t*right.P1;
}

// ***************************************************************************
void		CBezierPatch::subdivideS(CBezierPatch &left, CBezierPatch &right, float s) const
{
	CBezierCurve	curveT[4];
	CBezierCurve	curveTLeft[4];
	CBezierCurve	curveTRight[4];

	// Setup horizontal curves.
	curveT[0].set(Vertices[0], Tangents[7] , Tangents[6] , Vertices[3]);
	curveT[1].set(Tangents[0], Interiors[0], Interiors[3], Tangents[5]);
	curveT[2].set(Tangents[1], Interiors[1], Interiors[2], Tangents[4]);
	curveT[3].set(Vertices[1], Tangents[2] , Tangents[3] , Vertices[2]);

	// Subdivide curves.
	for(sint i=0;i<4;i++)
		curveT[i].subdivide(curveTLeft[i], curveTRight[i], s);

	// Setup bezier patchs.
	// left.
	curveTLeft[0].get(left.Vertices[0], left.Tangents[7] , left.Tangents[6] , left.Vertices[3]);
	curveTLeft[1].get(left.Tangents[0], left.Interiors[0], left.Interiors[3], left.Tangents[5]);
	curveTLeft[2].get(left.Tangents[1], left.Interiors[1], left.Interiors[2], left.Tangents[4]);
	curveTLeft[3].get(left.Vertices[1], left.Tangents[2] , left.Tangents[3] , left.Vertices[2]);
	// right.
	curveTRight[0].get(right.Vertices[0], right.Tangents[7] , right.Tangents[6] , right.Vertices[3]);
	curveTRight[1].get(right.Tangents[0], right.Interiors[0], right.Interiors[3], right.Tangents[5]);
	curveTRight[2].get(right.Tangents[1], right.Interiors[1], right.Interiors[2], right.Tangents[4]);
	curveTRight[3].get(right.Vertices[1], right.Tangents[2] , right.Tangents[3] , right.Vertices[2]);
}

// ***************************************************************************
void		CBezierPatch::subdivideT(CBezierPatch &top, CBezierPatch &bottom, float t) const
{
	CBezierCurve	curveS[4];
	CBezierCurve	curveSTop[4];
	CBezierCurve	curveSBottom[4];

	// Setup vertical curves.
	curveS[0].set(Vertices[0], Tangents[0] , Tangents[1] , Vertices[1]);
	curveS[1].set(Tangents[7], Interiors[0], Interiors[1], Tangents[2]);
	curveS[2].set(Tangents[6], Interiors[3], Interiors[2], Tangents[3]);
	curveS[3].set(Vertices[3], Tangents[5] , Tangents[4] , Vertices[2]);

	// Subdivide curves.
	for(sint i=0;i<4;i++)
		curveS[i].subdivide(curveSTop[i], curveSBottom[i], t);

	// Setup bezier patchs.
	// top.
	curveSTop[0].get(top.Vertices[0], top.Tangents[0] , top.Tangents[1] , top.Vertices[1]);
	curveSTop[1].get(top.Tangents[7], top.Interiors[0], top.Interiors[1], top.Tangents[2]);
	curveSTop[2].get(top.Tangents[6], top.Interiors[3], top.Interiors[2], top.Tangents[3]);
	curveSTop[3].get(top.Vertices[3], top.Tangents[5] , top.Tangents[4] , top.Vertices[2]);
	// bottom.
	curveSBottom[0].get(bottom.Vertices[0], bottom.Tangents[0] , bottom.Tangents[1] , bottom.Vertices[1]);
	curveSBottom[1].get(bottom.Tangents[7], bottom.Interiors[0], bottom.Interiors[1], bottom.Tangents[2]);
	curveSBottom[2].get(bottom.Tangents[6], bottom.Interiors[3], bottom.Interiors[2], bottom.Tangents[3]);
	curveSBottom[3].get(bottom.Vertices[3], bottom.Tangents[5] , bottom.Tangents[4] , bottom.Vertices[2]);
}

} // NL3D
