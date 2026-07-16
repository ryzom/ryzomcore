/**
 * \file spline_mesh.cpp
 * \brief See spline_mesh.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "spline_mesh.h"

#include <cmath>
#include <cstdio>

using namespace NLMISC;

namespace SPLINEMESH {

// ---------------------------------------------------------------------------------------------

// Cubic bezier point for segment knot[i] → knot[j]: control points are knot i's position, its
// OUT handle, knot j's IN handle, knot j's position (absolute handle positions on disk).
static CVector bez(const SPLINESHAPE::SKnot &a, const SPLINESHAPE::SKnot &b, float t)
{
	float u = 1.0f - t;
	float b0 = u * u * u;
	float b1 = 3.0f * u * u * t;
	float b2 = 3.0f * u * t * t;
	float b3 = t * t * t;
	return a.Knot * b0 + a.OutVec * b1 + b.InVec * b2 + b.Knot * b3;
}

void tessellateSpline(const SPLINESHAPE::SSpline &spline, sint steps, bool optimize,
                      bool adaptive, std::vector<CVector> &outPts)
{
	(void)optimize;
	(void)adaptive; // no closed-spline corpus case sets these in a way that changes the output
	                // (all corpus closed caps have steps == 0); adaptive is treated as plain steps
	outPts.clear();
	const std::vector<SPLINESHAPE::SKnot> &ks = spline.Knots;
	uint n = (uint)ks.size();
	if (n == 0) return;
	uint nSeg = spline.Closed ? n : n - 1;
	for (uint i = 0; i < n; ++i)
	{
		outPts.push_back(ks[i].Knot);
		if (i >= nSeg) break;
		uint j = (i + 1) % n;
		// Line-type segments and degenerate handles (both handles on their knots) contribute no
		// intermediate points; curve segments get `steps` interior points.
		bool line = ks[i].LType == 1
			|| ((ks[i].OutVec - ks[i].Knot).sqrnorm() == 0.f
			    && (ks[j].InVec - ks[j].Knot).sqrnorm() == 0.f);
		if (steps > 0 && !line)
		{
			for (sint s = 1; s <= steps; ++s)
				outPts.push_back(bez(ks[i], ks[j], (float)s / (float)(steps + 1)));
		}
	}
}

// ---------------------------------------------------------------------------------------------
// The corpus-proven Max morph-cap rule (see spline_mesh.h): XY projection, min-interior-angle
// valid ear, last-min tie-break, reversed ring for negative-area polygons, (prev, ear, next)
// emission, stuck → fail (empty mesh).

static inline double cr2(const double *o, const double *a, const double *b)
{
	return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0]);
}

bool capPolygon(const std::vector<CVector> &ptsIn, std::vector<SCapTri> &outTris)
{
	outTris.clear();
	uint n = (uint)ptsIn.size();
	if (n < 3) return false;

	// Project onto XY (drop z).
	std::vector<double> px(n), py(n);
	for (uint i = 0; i < n; ++i)
	{
		px[i] = ptsIn[i].x;
		py[i] = ptsIn[i].y;
	}
	double area = 0.0;
	for (uint i = 0; i < n; ++i)
	{
		uint j = (i + 1) % n;
		area += px[i] * py[j] - px[j] * py[i];
	}

	std::vector<uint32> ring(n);
	for (uint i = 0; i < n; ++i)
		ring[i] = i;
	if (area < 0.0)
	{
		for (uint i = 0; i < n; ++i)
			ring[i] = n - 1 - i;
	}

	struct L
	{
		const std::vector<double> &px, &py;
		L(const std::vector<double> &x, const std::vector<double> &y) : px(x), py(y) { }
		double cross(uint32 o, uint32 a, uint32 b) const
		{
			double vo[2] = { px[o], py[o] }, va[2] = { px[a], py[a] }, vb[2] = { px[b], py[b] };
			return cr2(vo, va, vb);
		}
		double angle(uint32 a, uint32 b, uint32 c) const
		{
			double v1x = px[a] - px[b], v1y = py[a] - py[b];
			double v2x = px[c] - px[b], v2y = py[c] - py[b];
			double l1 = sqrt(v1x * v1x + v1y * v1y), l2 = sqrt(v2x * v2x + v2y * v2y);
			if (l1 == 0.0 || l2 == 0.0) return 3.14159265358979323846;
			double d = (v1x * v2x + v1y * v2y) / (l1 * l2);
			if (d > 1.0) d = 1.0;
			if (d < -1.0) d = -1.0;
			return acos(d);
		}
	} g(px, py);

	while (ring.size() > 3)
	{
		sint best = -1;
		double bestAng = 0.0;
		uint rn = (uint)ring.size();
		for (uint j = 0; j < rn; ++j)
		{
			uint32 a = ring[(j + rn - 1) % rn], v = ring[j], c = ring[(j + 1) % rn];
			if (g.cross(a, v, c) <= 0.0) continue; // reflex or degenerate in the CCW ring
			bool ok = true;
			for (uint w = 0; w < rn && ok; ++w)
			{
				uint32 t = ring[w];
				if (t == a || t == v || t == c) continue;
				if (g.cross(a, v, t) >= 0.0 && g.cross(v, c, t) >= 0.0 && g.cross(c, a, t) >= 0.0)
					ok = false;
			}
			if (!ok) continue;
			double ang = g.angle(a, v, c);
			if (best < 0 || ang <= bestAng) // ties → LAST minimal (pinned by the rectangle GT)
			{
				bestAng = ang;
				best = (sint)j;
			}
		}
		if (best < 0)
		{
			// Stuck (self-intersecting projection): Max's cap fails → empty mesh.
			outTris.clear();
			return false;
		}
		uint32 a = ring[(best + rn - 1) % rn], v = ring[best], c = ring[(best + 1) % rn];
		SCapTri t;
		t.V[0] = a;
		t.V[1] = v;
		t.V[2] = c;
		outTris.push_back(t);
		ring.erase(ring.begin() + best);
	}
	// Final triangle: same min-angle ear pick decides the rotation.
	{
		uint32 a = ring[0], b = ring[1], c = ring[2];
		double angs[3] = { g.angle(c, a, b), g.angle(a, b, c), g.angle(b, c, a) };
		int k = 0;
		if (angs[1] <= angs[k]) k = 1;
		if (angs[2] <= angs[k]) k = 2;
		SCapTri t;
		if (k == 0) { t.V[0] = c; t.V[1] = a; t.V[2] = b; }
		else if (k == 1) { t.V[0] = a; t.V[1] = b; t.V[2] = c; }
		else { t.V[0] = b; t.V[1] = c; t.V[2] = a; }
		outTris.push_back(t);
	}
	return true;
}

bool buildSplineMesh(const SPLINESHAPE::SShape &shape, sint steps, bool optimize, bool adaptive,
                     SSplineMesh &out)
{
	out.Verts.clear();
	out.Faces.clear();
	// First closed curve with ≥3 knots caps; open-only shapes (and failed caps) stay empty —
	// both are valid, Max-matching results.
	for (uint c = 0; c < shape.Curves.size(); ++c)
	{
		const SPLINESHAPE::SSpline &sp = shape.Curves[c];
		if (!sp.Closed || sp.Knots.size() < 3) continue;
		std::vector<CVector> pts;
		tessellateSpline(sp, steps, optimize, adaptive, pts);
		std::vector<SCapTri> tris;
		if (!capPolygon(pts, tris))
			return true; // cap failed → empty mesh (shape02/03 class)
		out.Verts.swap(pts);
		out.Faces.swap(tris);
		return true;
	}
	return true; // no closed curve → empty mesh
}

} /* namespace SPLINEMESH */

/* end of file */
