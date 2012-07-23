#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000
#define SUBDIV_EDGES 0
#define SUBDIV_PATCHES 1

// ------------------------------------------------------------------------------------------------------------------------------------------------------

class NewEdge 
{
public:
	int oldEdge;
	int v1;
	int vec12;
	int vec21;
	int v2;
	int vec23;
	int vec32;
	int v3;
	NewEdge() { oldEdge = v1 = v2 = v3 = vec12 = vec21 = vec23 = vec32 = -1; }
};

class PatchDivInfo 
{
public:
	BOOL div02;
	BOOL div13;
	PatchDivInfo() { div02 = div13 = FALSE; }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void DeletePatchParts(PatchMesh *patch, RPatchMesh *rpatch, BitArray &delVerts, BitArray &delPatches);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------

// Handy fractional constants
#define _1_16 0.0625f
#define _1_8 0.125f
#define _3_16 0.1875f
#define _1_4 0.25f

static Point3 GetOuterInside(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f) 
{
	return a * _1_8 + b * _1_8 + c * _1_4 + d * _1_8 + e * _1_4 + f * _1_8;
}

// --------------------------------------------------------------------------------

static Point3 GetNewEdgeVec(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h) 
{
	return a * _1_16 + b * _1_16 + c * _3_16 + d * _3_16 + e * _1_16 + f * _1_16 + g * _3_16 + h * _3_16;
}

// --------------------------------------------------------------------------------

static Point3 GetCentralInterior(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h, Point3 i, Point3 j) 
{
	return a * _1_16 + b * _1_8 + c * _1_16 + d * _1_16 + e * _1_16 + f * _1_16 + g * _1_16 + h * _3_16 + i * _3_16 + j * _1_8;
}

// --------------------------------------------------------------------------------

static Point3 GetNewEdgeCenter(Point3 a, Point3 b, Point3 c, Point3 d, Point3 e, Point3 f, Point3 g, Point3 h, Point3 i) 
{
	return a * _1_16 + b * _1_8 + c * _1_16 + d * _1_8 + e * _1_16 + f * _1_16 + g * _1_4 + h * _1_8 + i * _1_8;
}

// --------------------------------------------------------------------------------

static Point3 GetOuterOutside(Point3 a, Point3 b, Point3 c, Point3 d) 
{
	return a * _1_4 + b * _1_4 + c * _1_4 + d * _1_4;
}

// --------------------------------------------------------------------------------

static Point3 InterpCenter(PatchMesh *patch, int index, int e1, int i1, int i2, int e2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 e1i1 =(v[p.vec[e1]].p + v[p.interior[i1]].p) / 2.0f;
	Point3 i1i2 =(v[p.interior[i1]].p + v[p.interior[i2]].p) / 2.0f;
	Point3 i2e2 =(v[p.interior[i2]].p + v[p.vec[e2]].p) / 2.0f;
	Point3 a =(e1i1 + i1i2) / 2.0f;
	Point3 b =(i1i2 + i2e2) / 2.0f;
	if (v1)
		*v1 = e1i1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = i2e2;
	return (a + b) / 2.0f;
}

// --------------------------------------------------------------------------------

static Point3 InterpCenter(PatchMesh *patch, int index, int e1, int i1, int e2, Point3 *v1 = NULL, Point3 *v2 = NULL) 
{
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 a =(p.aux[e1] + v[p.interior[i1]].p) / 2.0f;
	Point3 b =(v[p.interior[i1]].p + p.aux[e2]) / 2.0f;
	if (v1)
		*v1 = a;
	if (v2)
		*v2 = b;
	return (a + b) / 2.0f;
}

// --------------------------------------------------------------------------------

static Point3 InterpCenter(Point3 e1, Point3 i1, Point3 i2, Point3 e2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	Point3 e1i1 =(e1 + i1) / 2.0f;
	Point3 i1i2 =(i1 + i2) / 2.0f;
	Point3 i2e2 =(i2 + e2) / 2.0f;
	Point3 a =(e1i1 + i1i2) / 2.0f;
	Point3 b =(i1i2 + i2e2) / 2.0f;
	if (v1)
		*v1 = e1i1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = i2e2;
	return (a + b) / 2.0f;
}

// --------------------------------------------------------------------------------

static Point3 InterpCenter(Point3 e1, Point3 i1, Point3 e2, Point3 *v1 = NULL, Point3 *v2 = NULL) 
{
	Point3 a =(e1 + i1) / 2.0f;
	Point3 b =(i1 + e2) / 2.0f;
	if (v1)
		*v1 = a;
	if (v2)
		*v2 = b;
	return (a + b) / 2.0f;
}

// --------------------------------------------------------------------------------

static Point3 InterpEdge(PatchMesh *patch, int index, float pct, int c1, int e1, int e2, int c2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	PatchVert *vert = patch->verts;
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 pv1 = vert[p.v[c1]].p;
	Point3 pv2 = vert[p.v[c2]].p;
	Point3 pe1 = v[p.vec[e1]].p;
	Point3 pe2 = v[p.vec[e2]].p;
	Point3 v1e1 = pv1 +(pe1 - pv1) * pct;
	Point3 e1e2 = pe1 +(pe2 - pe1) * pct;
	Point3 e2v2 = pe2 +(pv2 - pe2) * pct;
	Point3 a = v1e1 +(e1e2 - v1e1) * pct;
	Point3 b = e1e2 +(e2v2 - e1e2) * pct;
	if (v1)
		*v1 = v1e1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = e2v2;
	return a +(b - a) * pct;
}

// --------------------------------------------------------------------------------

static void FindNewTriEdge(PatchMesh *patch, Patch &p, int vert, Point3 &e1, Point3 &e2, Point3 &e3) 
{
	int a = vert;
	int b = vert * 3;
	int c = b + 1;
	int d =(b + 8) % 9;
	int e =(b + 7) % 9;
	int f =(b + 4) % 9;
	int g = vert;
	int h =(g + 1) % 3;
	int i =(g + 2) % 3;
	int j =(b + 6) % 9;
	int k =(b + 5) % 9;
	int l = b + 2;
	int m =(b + 3) % 9;
	Point3 pa = patch->verts[p.v[a]].p;
	Point3 pb = p.aux[b];
	Point3 pc = p.aux[c];
	Point3 pd = p.aux[d];
	Point3 pe = p.aux[e];
	Point3 pf = p.aux[f];
	Point3 pg = patch->vecs[p.interior[g]].p;
	Point3 ph = patch->vecs[p.interior[h]].p;
	Point3 pi = patch->vecs[p.interior[i]].p;
	Point3 pj = p.aux[j];
	Point3 pk = p.aux[k];
	Point3 pl = p.aux[l];
	Point3 pm = p.aux[m];
	e1 = GetNewEdgeVec(pa, pb, pd, pe, pj, pk, pg, pi);
	e2 = GetNewEdgeCenter(pa, pb, pc, pd, pe, pf, pg, ph, pi);
	e3 = GetNewEdgeVec(pa, pd, pb, pc, pl, pm, pg, ph);
}

// --------------------------------------------------------------------------------

static void FindNewOuterTriInteriors(PatchMesh *patch, Patch &p, int vert, Point3 &i1, Point3 &i2, Point3 &i3) 
{
	int a = vert;
	int b = vert * 3;
	int c =(b + 8) % 9;
	int d =(b + 7) % 9;
	int e = vert;
	int f =(e + 2) % 3;
	int g = b + 1;
	int h =(e + 1) % 3;
	Point3 pa = patch->verts[p.v[a]].p;
	Point3 pb = p.aux[b];
	Point3 pc = p.aux[c];
	Point3 pd = p.aux[d];
	Point3 pe = patch->vecs[p.interior[e]].p;
	Point3 pf = patch->vecs[p.interior[f]].p;
	Point3 pg = p.aux[g];
	Point3 ph = patch->vecs[p.interior[h]].p;
	i1 = GetOuterOutside(pa, pb, pc, pe);
	i2 = GetOuterInside(pa, pc, pb, pg, pe, ph);
	i3 = GetOuterInside(pa, pb, pc, pd, pe, pf);
}

// --------------------------------------------------------------------------------

static void FindNewInnerTriInteriors(PatchMesh *patch, Patch &p, Point3 &i1, Point3 &i2, Point3 &i3) 
{
	Point3 pa = p.aux[0];
	Point3 pb = p.aux[1];
	Point3 pc = p.aux[2];
	Point3 pd = p.aux[3];
	Point3 pe = p.aux[4];
	Point3 pf = p.aux[5];
	Point3 pg = p.aux[6];
	Point3 ph = p.aux[7];
	Point3 pi = p.aux[8];
	Point3 pj = patch->vecs[p.interior[0]].p;
	Point3 pk = patch->vecs[p.interior[1]].p;
	Point3 pl = patch->vecs[p.interior[2]].p;
	i1 = GetCentralInterior(pa, pb, pc, pi, ph, pe, pd, pj, pk, pl);
	i2 = GetCentralInterior(pd, pe, pf, pc, pb, ph, pg, pk, pl, pj);
	i3 = GetCentralInterior(pg, ph, pi, pf, pe, pb, pa, pl, pj, pk);
}

// This is a first shot at a degree reducer which turns a degree-4 curve into a degree-3 curve,
// it probably won't give very good results unless the curve was converted from degree 3 to degree 4
// returns just the vector points
static void CubicFromQuartic(Point3 q1, Point3 q2, Point3 q3, Point3 q4, Point3 q5, Point3 &c2, Point3 &c3) 
{
	c2 = q1 +(q2 - q1) * 1.33333f;
	c3 = q5 +(q4 - q5) * 1.33333f;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void SubdividePatch(int type, BOOL propagate, PatchMesh *patch, RPatchMesh *rpatch)
{
	int i;

	int verts = patch->getNumVerts();
	int vecs = patch->getNumVecs();
	int edges = patch->getNumEdges();
	int patches = patch->getNumPatches();

	// Make an edge flags array to note which edges must be processed
	BitArray eDiv(edges);
	// Make a patch flags array to note which patches must be processed
	BitArray pDiv(patches);
	// Make an edge flags array to note which edges have been done
	BitArray eDone(edges);
	eDone.ClearAll();
	// Make a patch flags array to note which patches have been done
	BitArray pDone(patches);
	pDone.ClearAll();

	switch (type)
	{
	case SUBDIV_EDGES:
		if (!patch->edgeSel.NumberSet())
			return;		// Nothing to do!
		eDiv = patch->edgeSel;
		pDiv.ClearAll();
		break;
	case SUBDIV_PATCHES:
		if (!patch->patchSel.NumberSet())
			return;		// Nothing to do!
		eDiv.ClearAll();
		pDiv = patch->patchSel;
		for (i = 0; i < patches; ++i)
		{
			if (pDiv[i])
			{
				Patch &p = patch->patches[i];
				// Mark all edges for division
				eDiv.Set(p.edge[0]);
				eDiv.Set(p.edge[1]);
				eDiv.Set(p.edge[2]);
				if (p.type == PATCH_QUAD)
					eDiv.Set(p.edge[3]);
			}
		}
		// If not propagating, mark the edges as done
		if (!propagate)
			eDone = eDiv;
		break;
	}

	BOOL more = TRUE;
	while (more) 
	{
		BOOL altered = FALSE;
		for (i = 0; i < edges; ++i)
		{
			if (eDiv[i] && !eDone[i])
			{
				PatchEdge &e = patch->edges[i];
#if (MAX_RELEASE < 4000)
				pDiv.Set(e.patch1);
				if (e.patch2 >= 0)
					pDiv.Set(e.patch2);
#else // (MAX_RELEASE < 4000)
				pDiv.Set(e.patches[0]);
				if (e.patches.Count() > 1)
					pDiv.Set(e.patches[1]);
#endif // (MAX_RELEASE < 4000)
				eDone.Set(i);
				altered = TRUE;
			}
		}
		if (altered && propagate)
		{
			for (i = 0; i < patches; ++i)
			{
				if (pDiv[i] && !pDone[i])
				{
					Patch &p = patch->patches[i];
					if (p.type == PATCH_TRI)
					{
						// Triangle -- tag all edges for division
						eDiv.Set(p.edge[0]);
						eDiv.Set(p.edge[1]);
						eDiv.Set(p.edge[2]);
					}
					else 
					{		// Quad -- Tag edges opposite tagged edges
						if (eDiv[p.edge[0]])
							eDiv.Set(p.edge[2]);
						if (eDiv[p.edge[1]])
							eDiv.Set(p.edge[3]);
						if (eDiv[p.edge[2]])
							eDiv.Set(p.edge[0]);
						if (eDiv[p.edge[3]])
							eDiv.Set(p.edge[1]);
					}
					pDone.Set(i);
				}
			}
		}
		else
			more = FALSE;
	}

	// Keep a count of the new interior vectors
	int newInteriors = 0;

	// Also keep a count of the new vertices inside double-divided quads
	int newCenters = 0;

	// And a count of new texture vertices
	Tab < int> newTVerts;
	newTVerts.SetCount(patch->getNumMaps());
	int chan;
	for (chan = 0; chan < patch->getNumMaps(); ++chan)
		newTVerts[chan] = 0;

	// And a count of new patches
	int newPatches = 0;

	int divPatches = pDiv.NumberSet();
	PatchDivInfo *pInfo = new PatchDivInfo[divPatches];
	int pDivIx;

	// Tag the edges that are on tagged patches but aren't tagged (only happens in propagate=0)
	// And set up a table with useful division info
	for (i = 0, pDivIx = 0; i < patches; ++i)
	{
		if (pDiv[i])
		{
			PatchDivInfo &pi = pInfo[pDivIx];
			Patch &p = patch->patches[i];
			if (p.type == PATCH_TRI)
			{
				// Triangle -- tag all edges for division
				eDiv.Set(p.edge[0]);
				eDiv.Set(p.edge[1]);
				eDiv.Set(p.edge[2]);
				newInteriors +=(6 + 12);
				newPatches += 4;
				for (chan = 0; chan < patch->getNumMaps(); ++chan)
				{
					if (patch->tvPatches[chan])
						newTVerts[chan] += 3;
				}
			}
			else 
			{		// Quad -- Tag edges opposite tagged edges
				int divs = 0;
				pi.div02 = pi.div13 = FALSE;
				if (eDiv[p.edge[0]])
				{
					eDiv.Set(p.edge[2]);
					divs++;
					pi.div02 = TRUE;
				}
				else
					if (eDiv[p.edge[2]])
					{
						eDiv.Set(p.edge[0]);
						divs++;
						pi.div02 = TRUE;
					}
					if (eDiv[p.edge[1]])
					{
						eDiv.Set(p.edge[3]);
						divs++;
						pi.div13 = TRUE;
					}
					else
						if (eDiv[p.edge[3]])
						{
							eDiv.Set(p.edge[1]);
							divs++;
							pi.div13 = TRUE;
						}
						newPatches +=(divs == 1) ? 2 : 4;
						newInteriors +=(divs == 1) ?(2 + 8) :(8 + 16);
						for (chan = 0; chan < patch->getNumMaps(); ++chan)
						{
							if (patch->tvPatches[chan])
							{
								if (divs == 2)
									newTVerts[chan] += 5;
								else
									newTVerts[chan] += 2;
							}
						}
						if (divs == 2)
							newCenters++;
			}
			pDivIx++;
		}
	}

	// Figure out how many new verts and vecs we'll need...
	int divEdges = eDiv.NumberSet();
	int newVerts = divEdges + newCenters;		// 1 new vert per edge
	int newVecs = divEdges * 4 + newInteriors;	// 4 new vectors per edge + new interior verts

	int vert = verts;
	Tab < int> tvert;
	tvert.SetCount(patch->getNumMaps());
	Tab < int> tverts;
	tverts.SetCount(patch->getNumMaps());
	Tab < int> tpat;
	tpat.SetCount(patch->getNumMaps());
	for (chan = 0; chan < patch->getNumMaps(); ++chan)
	{
		tverts[chan] = tvert[chan] = patch->getNumMapVerts(chan);
		tpat[chan] = patches;
	}
	int vec = vecs;
	int pat = patches;

	// Add the new vertices
	patch->setNumVerts(verts + newVerts, TRUE);
	rpatch->SetNumVerts(verts + newVerts);

	// Add the new texture vertices
	for (chan = 0; chan < patch->getNumMaps(); ++chan)
		patch->setNumMapVerts(chan, tverts[chan] + newTVerts[chan], TRUE);

	// Add the new vectors
	patch->setNumVecs(vecs + newVecs, TRUE);

	// Add the new patches
	patch->setNumPatches(patches + newPatches, TRUE);
	rpatch->SetNumPatches(patches + newPatches);

	// Create a new edge map
	NewEdge *eMap = new NewEdge[edges];
	for (i = 0; i < edges; ++i)
	{
		if (eDiv[i])
		{
			PatchEdge &edge = patch->edges[i];
			NewEdge &map = eMap[i];
			map.oldEdge = i;
			map.v1 = edge.v1;
			map.vec12 = vec++;
			map.vec21 = vec++;
			map.v2 = vert++;
			map.vec23 = vec++;
			map.vec32 = vec++;
			map.v3 = edge.v2;
			
			// Compute the new edge vertex and vectors
			Point3 v00 = patch->verts[edge.v1].p;
			Point3 v10 = patch->vecs[edge.vec12].p;
			Point3 v20 = patch->vecs[edge.vec21].p;
			Point3 v30 = patch->verts[edge.v2].p;
			Point3 v01 =(v10 + v00) / 2.0f;
			Point3 v21 =(v30 + v20) / 2.0f;
			Point3 v11 =(v20 + v10) / 2.0f;
			Point3 v02 =(v11 + v01) / 2.0f;
			Point3 v12 =(v21 + v11) / 2.0f;
			Point3 v03 =(v12 + v02) / 2.0f;
			
			patch->verts[map.v2].p = v03;
			patch->vecs[map.vec12].p = v01;
			patch->vecs[map.vec21].p = v02;
			patch->vecs[map.vec23].p = v12;
			patch->vecs[map.vec32].p = v21;
		}
	}

#ifdef DUMPING
	// Dump edge map
	DebugPrint("Edge map:\n");
	for (i = 0; i < edges; ++i)
	{
		NewEdge &e = eMap[i];
		DebugPrint("Old edge: %d  New edge: %d (%d %d) %d (%d %d) %d\n", e.oldEdge, e.v1, e.vec12, e.vec21, e.v2, e.vec23, e.vec32, e.v3);
	}
#endif

	// Now go and subdivide them!

	for (i = 0, pDivIx = 0; i < patches; ++i)
	{
		if (pDiv[i])
		{
			PatchDivInfo &pi = pInfo[pDivIx];
			Patch &p = patch->patches[i];
			if (p.type == PATCH_TRI)
			{
				// Need to create four new patches
				int newev1 = vec++;	// edge 0 -> edge 1
				int newev2 = vec++;	// edge 1 -> edge 0
				int newev3 = vec++;	// edge 1 -> edge 2
				int newev4 = vec++;	// edge 2 -> edge 1
				int newev5 = vec++;	// edge 2 -> edge 0
				int newev6 = vec++;	// edge 0 -> edge 2
				
				// Get pointers to new edges
				NewEdge &e0 = eMap[p.edge[0]];
				NewEdge &e1 = eMap[p.edge[1]];
				NewEdge &e2 = eMap[p.edge[2]];
				
				// See if edges need to be flopped
				BOOL flop0 =(e0.v1 == p.v[0]) ? FALSE : TRUE;
				BOOL flop1 =(e1.v1 == p.v[1]) ? FALSE : TRUE;
				BOOL flop2 =(e2.v1 == p.v[2]) ? FALSE : TRUE;
				
				// Create the four new patches
				Patch &p1 = patch->patches[pat++];
				Patch &p2 = patch->patches[pat++];
				Patch &p3 = patch->patches[pat++];
				Patch &p4 = patch->patches[pat++];
				
				p1.SetType(PATCH_TRI);
				p1.v[0] = e0.v2;
				p1.v[1] = flop1 ? e1.v3 : e1.v1;
				p1.v[2] = e1.v2;
				p1.vec[0] = flop0 ? e0.vec21 : e0.vec23;
				p1.vec[1] = flop0 ? e0.vec12 : e0.vec32;
				p1.vec[2] = flop1 ? e1.vec32 : e1.vec12;
				p1.vec[3] = flop1 ? e1.vec23 : e1.vec21;
				p1.vec[4] = newev2;
				p1.vec[5] = newev1;
				p1.interior[0] = vec++;
				p1.interior[1] = vec++;
				p1.interior[2] = vec++;
				
				p2.SetType(PATCH_TRI);
				p2.v[0] = e1.v2;
				p2.v[1] = flop2 ? e2.v3 : e2.v1;
				p2.v[2] = e2.v2;
				p2.vec[0] = flop1 ? e1.vec21 : e1.vec23;
				p2.vec[1] = flop1 ? e1.vec12 : e1.vec32;
				p2.vec[2] = flop2 ? e2.vec32 : e2.vec12;
				p2.vec[3] = flop2 ? e2.vec23 : e2.vec21;
				p2.vec[4] = newev4;
				p2.vec[5] = newev3;
				p2.interior[0] = vec++;
				p2.interior[1] = vec++;
				p2.interior[2] = vec++;
				
				p3.SetType(PATCH_TRI);
				p3.v[0] = e0.v2;
				p3.v[1] = e1.v2;
				p3.v[2] = e2.v2;
				p3.vec[0] = newev1;
				p3.vec[1] = newev2;
				p3.vec[2] = newev3;
				p3.vec[3] = newev4;
				p3.vec[4] = newev5;
				p3.vec[5] = newev6;
				p3.interior[0] = vec++;
				p3.interior[1] = vec++;
				p3.interior[2] = vec++;
				
				p4.SetType(PATCH_TRI);
				p4.v[0] = flop0 ? e0.v3 : e0.v1;
				p4.v[1] = e0.v2;
				p4.v[2] = e2.v2;
				p4.vec[0] = flop0 ? e0.vec32 : e0.vec12;
				p4.vec[1] = flop0 ? e0.vec23 : e0.vec21;
				p4.vec[2] = newev6;
				p4.vec[3] = newev5;
				p4.vec[4] = flop2 ? e2.vec21 : e2.vec23;
				p4.vec[5] = flop2 ? e2.vec12 : e2.vec32;
				p4.interior[0] = vec++;
				p4.interior[1] = vec++;
				p4.interior[2] = vec++;
				
				// If this patch is textured, create three new texture verts for it
				for (chan = 0; chan < patch->getNumMaps(); ++chan)
				{
					if (patch->tvPatches[chan])
					{
						int tva = tvert[chan]++;
						int tvb = tvert[chan]++;
						int tvc = tvert[chan]++;
						TVPatch &tp = patch->tvPatches[chan][i];
						TVPatch &tp1 = patch->tvPatches[chan][tpat[chan]++];
						TVPatch &tp2 = patch->tvPatches[chan][tpat[chan]++];
						TVPatch &tp3 = patch->tvPatches[chan][tpat[chan]++];
						TVPatch &tp4 = patch->tvPatches[chan][tpat[chan]++];
						tp1.tv[0] = tva;
						tp1.tv[1] = tp.tv[1];
						tp1.tv[2] = tvb;
						tp2.tv[0] = tvb;
						tp2.tv[1] = tp.tv[2];
						tp2.tv[2] = tvc;
						tp3.tv[0] = tva;
						tp3.tv[1] = tvb;
						tp3.tv[2] = tvc;
						tp4.tv[0] = tp.tv[0];
						tp4.tv[1] = tva;
						tp4.tv[2] = tvc;
#if MAX_RELEASE <= 3100
						patch->tVerts[chan][tva] =(patch->tVerts[chan][tp.tv[0]] + patch->tVerts[chan][tp.tv[1]]) / 2.0f;
						patch->tVerts[chan][tvb] =(patch->tVerts[chan][tp.tv[1]] + patch->tVerts[chan][tp.tv[2]]) / 2.0f;
						patch->tVerts[chan][tvc] =(patch->tVerts[chan][tp.tv[2]] + patch->tVerts[chan][tp.tv[0]]) / 2.0f;
#else
						patch->tVerts[chan][tva] =((UVVert&)patch->tVerts[chan][tp.tv[0]] + (UVVert&)patch->tVerts[chan][tp.tv[1]]) / 2.0f;
						patch->tVerts[chan][tvb] =((UVVert&)patch->tVerts[chan][tp.tv[1]] + (UVVert&)patch->tVerts[chan][tp.tv[2]]) / 2.0f;
						patch->tVerts[chan][tvc] =((UVVert&)patch->tVerts[chan][tp.tv[2]] + (UVVert&)patch->tVerts[chan][tp.tv[0]]) / 2.0f;
#endif
					}
				}
				
				// Now we'll compute the vectors for the three new edges being created inside this patch
				// These come back as degree 4's, and we need to reduce them to degree 3 for use in our
				// edges -- This is a bit risky because we aren't guaranteed a perfect fit.
				Point3 i1, i2, i3, i4, i5, i6, i7, i8, i9;
				FindNewTriEdge(patch, p, 0, i1, i2, i3);
				FindNewTriEdge(patch, p, 1, i4, i5, i6);
				FindNewTriEdge(patch, p, 2, i7, i8, i9);
				Point3 v1, v2, v3, v4, v5, v6;
				CubicFromQuartic(patch->verts[e2.v2].p, i1, i2, i3, patch->verts[e0.v2].p, v1, v2);
				CubicFromQuartic(patch->verts[e0.v2].p, i4, i5, i6, patch->verts[e1.v2].p, v3, v4);
				CubicFromQuartic(patch->verts[e1.v2].p, i7, i8, i9, patch->verts[e2.v2].p, v5, v6);
				patch->vecs[newev1].p = v3;
				patch->vecs[newev2].p = v4;
				patch->vecs[newev3].p = v5;
				patch->vecs[newev4].p = v6;
				patch->vecs[newev5].p = v1;
				patch->vecs[newev6].p = v2;
				// Now compute the interior vectors for the new patches if the one we're dividing isn't automatic
				// Must compute vectors for this patch's divided edges
				if (!(p.flags & PATCH_AUTO))
				{
					p1.flags &= ~PATCH_AUTO;
					p2.flags &= ~PATCH_AUTO;
					p3.flags &= ~PATCH_AUTO;
					p4.flags &= ~PATCH_AUTO;
					
					FindNewOuterTriInteriors(patch, p, 1, patch->vecs[p1.interior[1]].p, patch->vecs[p1.interior[2]].p, patch->vecs[p1.interior[0]].p);
					FindNewOuterTriInteriors(patch, p, 2, patch->vecs[p2.interior[1]].p, patch->vecs[p2.interior[2]].p, patch->vecs[p2.interior[0]].p);
					FindNewInnerTriInteriors(patch, p, patch->vecs[p3.interior[0]].p, patch->vecs[p3.interior[1]].p, patch->vecs[p3.interior[2]].p);
					FindNewOuterTriInteriors(patch, p, 0, patch->vecs[p4.interior[0]].p, patch->vecs[p4.interior[1]].p, patch->vecs[p4.interior[2]].p);
				}
				}
				else 
				{		// Quad patch
					// Check division flags to see how many patches we'll need
					if (pi.div02 && pi.div13)
					{
						// Divide both ways
						// Need a new central vertex
						Point3 newc = p.interp(patch, 0.5f, 0.5f);
						patch->verts[vert].p = newc;
						int center = vert++;
						
						// Need to create four new patches
						int newev1 = vec++;	// edge 0 -> center
						int newev2 = vec++;	// center -> edge 0
						int newev3 = vec++;	// edge 1 -> center
						int newev4 = vec++;	// center -> edge 1
						int newev5 = vec++;	// edge 2 -> center
						int newev6 = vec++;	// center -> edge 2
						int newev7 = vec++;	// edge 3 -> center
						int newev8 = vec++;	// center -> edge 3
						
						// Get pointers to new edges
						NewEdge &e0 = eMap[p.edge[0]];
						NewEdge &e1 = eMap[p.edge[1]];
						NewEdge &e2 = eMap[p.edge[2]];
						NewEdge &e3 = eMap[p.edge[3]];
						
						// See if edges need to be flopped
						BOOL flop0 =(e0.v1 == p.v[0]) ? FALSE : TRUE;
						BOOL flop1 =(e1.v1 == p.v[1]) ? FALSE : TRUE;
						BOOL flop2 =(e2.v1 == p.v[2]) ? FALSE : TRUE;
						BOOL flop3 =(e3.v1 == p.v[3]) ? FALSE : TRUE;
						
						// Compute the new vectors for the dividing line
						Point3 w1, w2, w3, w4;
						w1 = InterpCenter(patch, i, 7, 0, 1, 2);
						w2 = InterpCenter(patch, i, 6, 3, 2, 3);
						w3 = InterpCenter(patch, i, 1, 1, 2, 4);
						w4 = InterpCenter(patch, i, 0, 0, 3, 5);
						Point3 new0 = patch->verts[e0.v2].p;
						Point3 new1 = patch->verts[e1.v2].p;
						Point3 new2 = patch->verts[e2.v2].p;
						Point3 new3 = patch->verts[e3.v2].p;
						InterpCenter(new0, w1, w2, new2, &patch->vecs[newev1].p, &patch->vecs[newev2].p, &patch->vecs[newev6].p, &patch->vecs[newev5].p);
						InterpCenter(new1, w3, w4, new3, &patch->vecs[newev3].p, &patch->vecs[newev4].p, &patch->vecs[newev8].p, &patch->vecs[newev7].p);
						
						// Create the four new patches
						Patch &p1 = patch->patches[pat++];
						Patch &p2 = patch->patches[pat++];
						Patch &p3 = patch->patches[pat++];
						Patch &p4 = patch->patches[pat++];

						p1.SetType(PATCH_QUAD);
						p1.v[0] = p.v[0];
						p1.v[1] = e0.v2;
						p1.v[2] = center;
						p1.v[3] = e3.v2;
						p1.vec[0] = flop0 ? e0.vec32 : e0.vec12;
						p1.vec[1] = flop0 ? e0.vec23 : e0.vec21;
						p1.vec[2] = newev1;
						p1.vec[3] = newev2;
						p1.vec[4] = newev8;
						p1.vec[5] = newev7;
						p1.vec[6] = flop3 ? e3.vec21 : e3.vec23;
						p1.vec[7] = flop3 ? e3.vec12 : e3.vec32;
						p1.interior[0] = vec++;
						p1.interior[1] = vec++;
						p1.interior[2] = vec++;
						p1.interior[3] = vec++;

#define RPO_REMAR_V2(a,b,c) (a)
						p2.SetType(PATCH_QUAD);
						p2.v[RPO_REMAR_V2(0,1,4)] = p.v[1];
						p2.v[RPO_REMAR_V2(1,1,4)] = e1.v2;
						p2.v[RPO_REMAR_V2(2,1,4)] = center;
						p2.v[RPO_REMAR_V2(3,1,4)] = e0.v2;
						p2.vec[RPO_REMAR_V2(0,1,8)] = flop1 ? e1.vec32 : e1.vec12;
						p2.vec[RPO_REMAR_V2(1,1,8)] = flop1 ? e1.vec23 : e1.vec21;
						p2.vec[RPO_REMAR_V2(2,1,8)] = newev3;
						p2.vec[RPO_REMAR_V2(3,1,8)] = newev4;
						p2.vec[RPO_REMAR_V2(4,1,8)] = newev2;
						p2.vec[RPO_REMAR_V2(5,1,8)] = newev1;
						p2.vec[RPO_REMAR_V2(6,1,8)] = flop0 ? e0.vec21 : e0.vec23;
						p2.vec[RPO_REMAR_V2(7,1,8)] = flop0 ? e0.vec12 : e0.vec32;
						p2.interior[RPO_REMAR_V2(0,1,4)] = vec++;
						p2.interior[RPO_REMAR_V2(1,1,4)] = vec++;
						p2.interior[RPO_REMAR_V2(2,1,4)] = vec++;
						p2.interior[RPO_REMAR_V2(3,1,4)] = vec++;

						p3.SetType(PATCH_QUAD);
						p3.v[RPO_REMAR_V2(0,2,4)] = p.v[2];
						p3.v[RPO_REMAR_V2(1,2,4)] = e2.v2;
						p3.v[RPO_REMAR_V2(2,2,4)] = center;
						p3.v[RPO_REMAR_V2(3,2,4)] = e1.v2;
						p3.vec[RPO_REMAR_V2(0,2,8)] = flop2 ? e2.vec32 : e2.vec12;
						p3.vec[RPO_REMAR_V2(1,2,8)] = flop2 ? e2.vec23 : e2.vec21;
						p3.vec[RPO_REMAR_V2(2,2,8)] = newev5;
						p3.vec[RPO_REMAR_V2(3,2,8)] = newev6;
						p3.vec[RPO_REMAR_V2(4,2,8)] = newev4;
						p3.vec[RPO_REMAR_V2(5,2,8)] = newev3;
						p3.vec[RPO_REMAR_V2(6,2,8)] = flop1 ? e1.vec21 : e1.vec23;
						p3.vec[RPO_REMAR_V2(7,2,8)] = flop1 ? e1.vec12 : e1.vec32;
						p3.interior[RPO_REMAR_V2(0,2,4)] = vec++;
						p3.interior[RPO_REMAR_V2(1,2,4)] = vec++;
						p3.interior[RPO_REMAR_V2(2,2,4)] = vec++;
						p3.interior[RPO_REMAR_V2(3,2,4)] = vec++;

						p4.SetType(PATCH_QUAD);
						p4.v[RPO_REMAR_V2(0,3,4)] = p.v[3];
						p4.v[RPO_REMAR_V2(1,3,4)] = e3.v2;
						p4.v[RPO_REMAR_V2(2,3,4)] = center;
						p4.v[RPO_REMAR_V2(3,3,4)] = e2.v2;
						p4.vec[RPO_REMAR_V2(0,3,8)] = flop3 ? e3.vec32 : e3.vec12;
						p4.vec[RPO_REMAR_V2(1,3,8)] = flop3 ? e3.vec23 : e3.vec21;
						p4.vec[RPO_REMAR_V2(2,3,8)] = newev7;
						p4.vec[RPO_REMAR_V2(3,3,8)] = newev8;
						p4.vec[RPO_REMAR_V2(4,3,8)] = newev6;
						p4.vec[RPO_REMAR_V2(5,3,8)] = newev5;
						p4.vec[RPO_REMAR_V2(6,3,8)] = flop2 ? e2.vec21 : e2.vec23;
						p4.vec[RPO_REMAR_V2(7,3,8)] = flop2 ? e2.vec12 : e2.vec32;
						p4.interior[RPO_REMAR_V2(0,3,4)] = vec++;
						p4.interior[RPO_REMAR_V2(1,3,4)] = vec++;
						p4.interior[RPO_REMAR_V2(2,3,4)] = vec++;
						p4.interior[RPO_REMAR_V2(3,3,4)] = vec++;

						// If this patch is textured, create five new texture verts for it
						for (chan = 0; chan < patch->getNumMaps(); ++chan)
						{
							if (patch->tvPatches[chan])
							{
								int tva = tvert[chan]++;
								int tvb = tvert[chan]++;
								int tvc = tvert[chan]++;
								int tvd = tvert[chan]++;
								int tve = tvert[chan]++;
								TVPatch &tp = patch->tvPatches[chan][i];
								TVPatch &tp1 = patch->tvPatches[chan][tpat[chan]++];
								TVPatch &tp2 = patch->tvPatches[chan][tpat[chan]++];
								TVPatch &tp3 = patch->tvPatches[chan][tpat[chan]++];
								TVPatch &tp4 = patch->tvPatches[chan][tpat[chan]++];
								tp1.tv[0] = tp.tv[0];
								tp1.tv[1] = tva;
								tp1.tv[2] = tve;
								tp1.tv[3] = tvd;
								tp2.tv[RPO_REMAR_V2(0,1,4)] = tp.tv[1];
								tp2.tv[RPO_REMAR_V2(1,1,4)] = tvb;
								tp2.tv[RPO_REMAR_V2(2,1,4)] = tve;
								tp2.tv[RPO_REMAR_V2(3,1,4)] = tva;
								tp3.tv[RPO_REMAR_V2(0,2,4)] = tp.tv[2];
								tp3.tv[RPO_REMAR_V2(1,2,4)] = tvc;
								tp3.tv[RPO_REMAR_V2(2,2,4)] = tve;
								tp3.tv[RPO_REMAR_V2(3,2,4)] = tvb;
								tp4.tv[RPO_REMAR_V2(0,3,4)] = tp.tv[3];
								tp4.tv[RPO_REMAR_V2(1,3,4)] = tvd;
								tp4.tv[RPO_REMAR_V2(2,3,4)] = tve;
								tp4.tv[RPO_REMAR_V2(3,3,4)] = tvc;
#if MAX_RELEASE <= 3100
								patch->tVerts[chan][tva] =(patch->tVerts[chan][tp.tv[0]] + patch->tVerts[chan][tp.tv[1]]) / 2.0f;
								patch->tVerts[chan][tvb] =(patch->tVerts[chan][tp.tv[1]] + patch->tVerts[chan][tp.tv[2]]) / 2.0f;
								patch->tVerts[chan][tvc] =(patch->tVerts[chan][tp.tv[3]] + patch->tVerts[chan][tp.tv[0]]) / 2.0f;
								patch->tVerts[chan][tve] =(patch->tVerts[chan][tp.tv[0]] + patch->tVerts[chan][tp.tv[1]] + patch->tVerts[chan][tp.tv[2]] + patch->tVerts[chan][tp.tv[3]]) / 4.0f;
#else
								patch->tVerts[chan][tva] =((UVVert&)patch->tVerts[chan][tp.tv[0]] + (UVVert&)patch->tVerts[chan][tp.tv[1]]) / 2.0f;
								patch->tVerts[chan][tvb] =((UVVert&)patch->tVerts[chan][tp.tv[1]] + (UVVert&)patch->tVerts[chan][tp.tv[2]]) / 2.0f;
								patch->tVerts[chan][tvc] =((UVVert&)patch->tVerts[chan][tp.tv[3]] + (UVVert&)patch->tVerts[chan][tp.tv[0]]) / 2.0f;
								patch->tVerts[chan][tve] =((UVVert&)patch->tVerts[chan][tp.tv[0]] + (UVVert&)patch->tVerts[chan][tp.tv[1]] + patch->tVerts[chan][tp.tv[2]] + patch->tVerts[chan][tp.tv[3]]) / 4.0f;
#endif
							}
						}
						
						// If it's not an auto patch, compute the new interior points
						if (!(p.flags & PATCH_AUTO))
						{
							p1.flags &= ~PATCH_AUTO;
							p2.flags &= ~PATCH_AUTO;
							p3.flags &= ~PATCH_AUTO;
							p4.flags &= ~PATCH_AUTO;
							
							Point3 a, b, c, d;
							Point3 a1, b1, c1, d1;
							Point3 a2, b2, c2, d2;
							Point3 a3, b3, c3, d3;
							Point3 a4, b4, c4, d4;
							InterpEdge(patch, i, 0.5f, 0, 0, 1, 1, &a1, &b1, &c1, &d1);
							InterpCenter(patch, i, 7, 0, 1, 2, &a2, &b2, &c2, &d2);
							InterpCenter(patch, i, 6, 3, 2, 3, &a3, &b3, &c3, &d3);
							InterpEdge(patch, i, 0.5f, 3, 5, 4, 2, &a4, &b4, &c4, &d4);
							
							InterpCenter(a1, a2, a3, a4, &a, &b, &c, &d);
							patch->vecs[p1.interior[0]].p = a;
							patch->vecs[p1.interior[3]].p = b;
							patch->vecs[p4.interior[1]].p = c;
							patch->vecs[p4.interior[0]].p = d;
							InterpCenter(b1, b2, b3, b4, &a, &b, &c, &d);
							patch->vecs[p1.interior[1]].p = a;
							patch->vecs[p1.interior[2]].p = b;
							patch->vecs[p4.interior[2]].p = c;
							patch->vecs[p4.interior[3]].p = d;
							InterpCenter(c1, c2, c3, c4, &a, &b, &c, &d);
							patch->vecs[p2.interior[3]].p = a;
							patch->vecs[p2.interior[2]].p = b;
							patch->vecs[p3.interior[2]].p = c;
							patch->vecs[p3.interior[1]].p = d;
							InterpCenter(d1, d2, d3, d4, &a, &b, &c, &d);
							patch->vecs[p2.interior[0]].p = a;
							patch->vecs[p2.interior[1]].p = b;
							patch->vecs[p3.interior[3]].p = c;
							patch->vecs[p3.interior[0]].p = d;
						}
						
						// Subdivide both ways the rpatch
						rpatch->Subdivide (i, e0.v2, e1.v2, e2.v2, e3.v2, center, pat-4, *patch);
					}
					else
						if (pi.div02)
						{
							// Divide edges 0 & 2
							// Need to create two new patches
							// Compute new edge vectors between new edge verts
							int newev1 = vec++;	// edge 0 -> edge 2
							int newev2 = vec++;	// edge 2 -> edge 0
							
							// Get pointers to new edges
							NewEdge &e0 = eMap[p.edge[0]];
							NewEdge &e2 = eMap[p.edge[2]];
							
							// See if edges need to be flopped
							BOOL flop0 =(e0.v1 == p.v[0]) ? FALSE : TRUE;
							BOOL flop2 =(e2.v1 == p.v[2]) ? FALSE : TRUE;
							
							// Compute the new vectors for the dividing line
							
							patch->vecs[newev1].p = InterpCenter(patch, i, 7, 0, 1, 2);
							patch->vecs[newev2].p = InterpCenter(patch, i, 6, 3, 2, 3);
							
							// Create the two new patches
							Patch &p1 = patch->patches[pat++];
							Patch &p2 = patch->patches[pat++];
							
							p1.SetType(PATCH_QUAD);
							p1.v[0] = flop0 ? e0.v3 : e0.v1;
							p1.v[1] = e0.v2;
							p1.v[2] = e2.v2;
							p1.v[3] = flop2 ? e2.v1 : e2.v3;
							p1.vec[0] = flop0 ? e0.vec32 : e0.vec12;
							p1.vec[1] = flop0 ? e0.vec23 : e0.vec21;
							p1.vec[2] = newev1;
							p1.vec[3] = newev2;
							p1.vec[4] = flop2 ? e2.vec21 : e2.vec23;
							p1.vec[5] = flop2 ? e2.vec12 : e2.vec32;
							p1.vec[6] = p.vec[6];
							p1.vec[7] = p.vec[7];
							p1.interior[0] = vec++;
							p1.interior[1] = vec++;
							p1.interior[2] = vec++;
							p1.interior[3] = vec++;
							
							p2.SetType(PATCH_QUAD);
							p2.v[0] = e0.v2;
							p2.v[1] = flop0 ? e0.v1 : e0.v3;
							p2.v[2] = flop2 ? e2.v3 : e2.v1;
							p2.v[3] = e2.v2;
							p2.vec[0] = flop0 ? e0.vec21 : e0.vec23;
							p2.vec[1] = flop0 ? e0.vec12 : e0.vec32;
							p2.vec[2] = p.vec[2];
							p2.vec[3] = p.vec[3];
							p2.vec[4] = flop2 ? e2.vec32 : e2.vec12;
							p2.vec[5] = flop2 ? e2.vec23 : e2.vec21;
							p2.vec[6] = newev2;
							p2.vec[7] = newev1;
							p2.interior[0] = vec++;
							p2.interior[1] = vec++;
							p2.interior[2] = vec++;
							p2.interior[3] = vec++;
							
							// If this patch is textured, create two new texture verts for it
							for (chan = 0; chan < patch->getNumMaps(); ++chan)
							{
								if (patch->tvPatches[chan])
								{
									int tva = tvert[chan]++;
									int tvb = tvert[chan]++;
									TVPatch &tp = patch->tvPatches[chan][i];
									TVPatch &tp1 = patch->tvPatches[chan][tpat[chan]++];
									TVPatch &tp2 = patch->tvPatches[chan][tpat[chan]++];
									tp1.tv[0] = tp.tv[0];
									tp1.tv[1] = tva;
									tp1.tv[2] = tvb;
									tp1.tv[3] = tp.tv[3];
									tp2.tv[0] = tva;
									tp2.tv[1] = tp.tv[1];
									tp2.tv[2] = tp.tv[2];
									tp2.tv[3] = tvb;
#if MAX_RELEASE <= 3100
									patch->tVerts[chan][tva] =(patch->tVerts[chan][tp.tv[0]] + patch->tVerts[chan][tp.tv[1]]) / 2.0f;
									patch->tVerts[chan][tvb] =(patch->tVerts[chan][tp.tv[2]] + patch->tVerts[chan][tp.tv[3]]) / 2.0f;
#else
									patch->tVerts[chan][tva] =((UVVert&)patch->tVerts[chan][tp.tv[0]] + (UVVert&)patch->tVerts[chan][tp.tv[1]]) / 2.0f;
									patch->tVerts[chan][tvb] =((UVVert&)patch->tVerts[chan][tp.tv[2]] + (UVVert&)patch->tVerts[chan][tp.tv[3]]) / 2.0f;
#endif
								}
							}
							
							// If it's not an auto patch, compute the new interior points
							if (!(p.flags & PATCH_AUTO))
							{
								p1.flags &= ~PATCH_AUTO;
								p2.flags &= ~PATCH_AUTO;
								
								Point3 a, b, c, d;
								InterpCenter(patch, i, 7, 0, 1, 2, &a, &b, &c, &d);
								patch->vecs[p1.interior[0]].p = a;
								patch->vecs[p1.interior[1]].p = b;
								patch->vecs[p2.interior[0]].p = c;
								patch->vecs[p2.interior[1]].p = d;
								InterpCenter(patch, i, 6, 3, 2, 3, &a, &b, &c, &d);
								patch->vecs[p1.interior[3]].p = a;
								patch->vecs[p1.interior[2]].p = b;
								patch->vecs[p2.interior[3]].p = c;
								patch->vecs[p2.interior[2]].p = d;
							}
						
							// Subdivide edge 0 and 2
							rpatch->SubdivideV (i, p1.v[1], p2.v[0], pat-2, *patch);
					}
					else 
					{							// Divide edges 1 & 3
						// Need to create two new patches
						// Compute new edge vectors between new edge verts
						int newev1 = vec++;	// edge 1 -> edge 3
						int newev2 = vec++;	// edge 3 -> edge 1
						
						// Get pointers to new edges
						NewEdge &e1 = eMap[p.edge[1]];
						NewEdge &e3 = eMap[p.edge[3]];
						
						// See if edges need to be flopped
						BOOL flop1 =(e1.v1 == p.v[1]) ? FALSE : TRUE;
						BOOL flop3 =(e3.v1 == p.v[3]) ? FALSE : TRUE;
						
						// Compute the new vectors for the dividing line
						patch->vecs[newev1].p = InterpCenter(patch, i, 1, 1, 2, 4);
						patch->vecs[newev2].p = InterpCenter(patch, i, 0, 0, 3, 5);
						
						// Create the two new patches
						Patch &p1 = patch->patches[pat++];
						Patch &p2 = patch->patches[pat++];

#define RPO_REMAP_V(a) (a)
#define RPO_REMAP_VEC(a) (a)
						p1.SetType(PATCH_QUAD);
						p1.v[RPO_REMAP_V(0)] = p.v[1];
						p1.v[RPO_REMAP_V(1)] = e1.v2;
						p1.v[RPO_REMAP_V(2)] = e3.v2;
						p1.v[RPO_REMAP_V(3)] = p.v[0];
						p1.vec[RPO_REMAP_VEC(0)] = flop1 ? e1.vec32 : e1.vec12;
						p1.vec[RPO_REMAP_VEC(1)] = flop1 ? e1.vec23 : e1.vec21;
						p1.vec[RPO_REMAP_VEC(2)] = newev1;
						p1.vec[RPO_REMAP_VEC(3)] = newev2;
						p1.vec[RPO_REMAP_VEC(4)] = flop3 ? e3.vec21 : e3.vec23;
						p1.vec[RPO_REMAP_VEC(5)] = flop3 ? e3.vec12 : e3.vec32;
						p1.vec[RPO_REMAP_VEC(6)] = p.vec[0];
						p1.vec[RPO_REMAP_VEC(7)] = p.vec[1];
						p1.interior[RPO_REMAP_V(0)] = vec++;
						p1.interior[RPO_REMAP_V(1)] = vec++;
						p1.interior[RPO_REMAP_V(2)] = vec++;
						p1.interior[RPO_REMAP_V(3)] = vec++;
						
						p2.SetType(PATCH_QUAD);
						p2.v[RPO_REMAP_V(0)] = e1.v2;
						p2.v[RPO_REMAP_V(1)] = p.v[2];
						p2.v[RPO_REMAP_V(2)] = p.v[3];
						p2.v[RPO_REMAP_V(3)] = e3.v2;
						p2.vec[RPO_REMAP_VEC(0)] = flop1 ? e1.vec21 : e1.vec23;
						p2.vec[RPO_REMAP_VEC(1)] = flop1 ? e1.vec12 : e1.vec32;
						p2.vec[RPO_REMAP_VEC(2)] = p.vec[4];
						p2.vec[RPO_REMAP_VEC(3)] = p.vec[5];
						p2.vec[RPO_REMAP_VEC(4)] = flop3 ? e3.vec32 : e3.vec12;
						p2.vec[RPO_REMAP_VEC(5)] = flop3 ? e3.vec23 : e3.vec21;
						p2.vec[RPO_REMAP_VEC(6)] = newev2;
						p2.vec[RPO_REMAP_VEC(7)] = newev1;
						p2.interior[RPO_REMAP_V(0)] = vec++;
						p2.interior[RPO_REMAP_V(1)] = vec++;
						p2.interior[RPO_REMAP_V(2)] = vec++;
						p2.interior[RPO_REMAP_V(3)] = vec++;
						
						// If this patch is textured, create two new texture verts for it
						for (chan = 0; chan < patch->getNumMaps(); ++chan)
						{
							if (patch->tvPatches[chan])
							{
								int tva = tvert[chan]++;
								int tvb = tvert[chan]++;
								TVPatch &tp = patch->tvPatches[chan][i];
								TVPatch &tp1 = patch->tvPatches[chan][tpat[chan]++];
								TVPatch &tp2 = patch->tvPatches[chan][tpat[chan]++];
								tp1.tv[RPO_REMAP_V(0)] = tp.tv[1];
								tp1.tv[RPO_REMAP_V(1)] = tva;
								tp1.tv[RPO_REMAP_V(2)] = tvb;
								tp1.tv[RPO_REMAP_V(3)] = tp.tv[0];
								tp2.tv[RPO_REMAP_V(0)] = tva;
								tp2.tv[RPO_REMAP_V(1)] = tp.tv[2];
								tp2.tv[RPO_REMAP_V(2)] = tp.tv[3];
								tp2.tv[RPO_REMAP_V(3)] = tvb;
#if MAX_RELEASE <= 3100
								patch->tVerts[chan][tva] =(patch->tVerts[chan][tp.tv[1]] + patch->tVerts[chan][tp.tv[2]]) / 2.0f;
								patch->tVerts[chan][tvb] =(patch->tVerts[chan][tp.tv[0]] + patch->tVerts[chan][tp.tv[3]]) / 2.0f;
#else
								patch->tVerts[chan][tva] =((UVVert&)patch->tVerts[chan][tp.tv[1]] + (UVVert&)patch->tVerts[chan][tp.tv[2]]) / 2.0f;
								patch->tVerts[chan][tvb] =((UVVert&)patch->tVerts[chan][tp.tv[0]] + (UVVert&)patch->tVerts[chan][tp.tv[3]]) / 2.0f;
#endif
							}
						}
						
						// If it's not an auto patch, compute the new interior points
						if (!(p.flags & PATCH_AUTO))
						{
							p1.flags &= ~PATCH_AUTO;
							p2.flags &= ~PATCH_AUTO;
							
							Point3 a, b, c, d;
							InterpCenter(patch, i, 1, 1, 2, 4, &a, &b, &c, &d);
							patch->vecs[p1.interior[0]].p = a;
							patch->vecs[p1.interior[1]].p = b;
							patch->vecs[p2.interior[0]].p = c;
							patch->vecs[p2.interior[1]].p = d;
							InterpCenter(patch, i, 0, 0, 3, 5, &a, &b, &c, &d);
							patch->vecs[p1.interior[3]].p = a;
							patch->vecs[p1.interior[2]].p = b;
							patch->vecs[p2.interior[3]].p = c;
							patch->vecs[p2.interior[2]].p = d;
						}
						
						// Subdivide edge 1 and 3
						rpatch->SubdivideU (i, p1.v[1], p2.v[0], pat-2, *patch);
					}	
				}
				pDivIx++;
			}
		}
		
		delete[] pInfo;
		delete[] eMap;
		
		// Now call the DeletePatchParts function to clean it all up
		BitArray dumVerts(patch->getNumVerts());
		dumVerts.ClearAll();
		BitArray dumPatches(patch->getNumPatches());
		dumPatches.ClearAll();
		// Mark the subdivided patches as deleted
		for (i = 0; i < patches; ++i)
			dumPatches.Set(i, pDiv[i]);
		
#ifdef DUMPING
		DebugPrint("Before:\n");
		patch->Dump();
#endif
		
		DeletePatchParts(patch, rpatch, dumVerts, dumPatches);
		
#ifdef DUMPING
		DebugPrint("After:\n");
		patch->Dump();
#endif
		
		patch->computeInteriors();
		patch->buildLinkages();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoSubdivide(int type) 
{
	switch (type)
	{
		case EP_EDGE:
			DoEdgeSubdivide();
			break;
		case EP_PATCH:
			DoPatchSubdivide();
			break;
		}
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoEdgeSubdivide() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		patchData->RecordTopologyTags(patch);
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->edgeSel.NumberSet())
		{
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoEdgeSubdivide"));
			// Call the patch add function
			SubdividePatch(SUBDIV_EDGES, propagate, patch, rpatch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_EDGESUBDIVIDE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDEDGESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoPatchSubdivide() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		patchData->RecordTopologyTags(patch);
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->patchSel.NumberSet())
		{
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoPatchSubdivide"));
			// Call the patch add function
			SubdividePatch(SUBDIV_PATCHES, propagate, patch, rpatch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHSUBDIVIDE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void TurnPatch(PatchMesh *patch, RPatchMesh *rpatch, bool ccw)
{
	// 
	if (ccw)
	{
		// For each patches
		for (int p=0; p<patch->numPatches; p++)
		{
			// Selected and quad ?
			if ((patch->patchSel[p])&&(patch->patches[p].type==PATCH_QUAD))
			{
				// Turn it!
				
				// Turn the vertices
				int tmp=patch->patches[p].v[3];
				patch->patches[p].v[3]=patch->patches[p].v[0];
				patch->patches[p].v[0]=patch->patches[p].v[1];
				patch->patches[p].v[1]=patch->patches[p].v[2];
				patch->patches[p].v[2]=tmp;

				// Turn the vectors
				tmp=patch->patches[p].vec[6];
				int tmp2=patch->patches[p].vec[7];
				patch->patches[p].vec[6]=patch->patches[p].vec[0];
				patch->patches[p].vec[7]=patch->patches[p].vec[1];
				patch->patches[p].vec[0]=patch->patches[p].vec[2];
				patch->patches[p].vec[1]=patch->patches[p].vec[3];
				patch->patches[p].vec[2]=patch->patches[p].vec[4];
				patch->patches[p].vec[3]=patch->patches[p].vec[5];
				patch->patches[p].vec[4]=tmp;
				patch->patches[p].vec[5]=tmp2;

				// Turn the interiors
				tmp=patch->patches[p].interior[3];
				patch->patches[p].interior[3]=patch->patches[p].interior[0];
				patch->patches[p].interior[0]=patch->patches[p].interior[1];
				patch->patches[p].interior[1]=patch->patches[p].interior[2];
				patch->patches[p].interior[2]=tmp;

#if MAX_RELEASE <= 3100
				// Turn the adjacents
				tmp=patch->patches[p].adjacent[3];
				patch->patches[p].adjacent[3]=patch->patches[p].adjacent[0];
				patch->patches[p].adjacent[0]=patch->patches[p].adjacent[1];
				patch->patches[p].adjacent[1]=patch->patches[p].adjacent[2];
				patch->patches[p].adjacent[2]=tmp;
#else
// todo, there s no more adj. what to do?
#endif
				// Turn the edges
				tmp=patch->patches[p].edge[3];
				patch->patches[p].edge[3]=patch->patches[p].edge[0];
				patch->patches[p].edge[0]=patch->patches[p].edge[1];
				patch->patches[p].edge[1]=patch->patches[p].edge[2];
				patch->patches[p].edge[2]=tmp;
			}
		}

		// Turn the rpatch
		rpatch->TurnPatch(patch);
	}
	else
	{
		// Turn three times the other way
		TurnPatch(patch, rpatch, true);
		TurnPatch(patch, rpatch, true);
		TurnPatch(patch, rpatch, true);
	}
}

void EditPatchMod::DoPatchTurn(bool ccw)
{
	ModContextList mcList;
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		patchData->RecordTopologyTags(patch);
					
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->patchSel.NumberSet())
		{
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoTurnPatch"));
			
			// Call the patch add function
			TurnPatch (patch, rpatch, ccw);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHSUBDIVIDE));
	}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

