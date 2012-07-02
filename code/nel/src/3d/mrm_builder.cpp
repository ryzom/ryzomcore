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

#include "nel/3d/mrm_builder.h"
#include "nel/3d/mrm_parameters.h"
using namespace NLMISC;
using namespace std;


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// Tools Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static bool	findElement(vector<sint>	&array, sint elt)
{
	return find(array.begin(), array.end(), elt) != array.end();
}
// ***************************************************************************
static bool	deleteElement(vector<sint>	&array, sint elt)
{
	bool	found=false;
	vector<sint>::iterator	it=array.begin();

	while(  (it=find(array.begin(), array.end(), elt)) != array.end() )
		found=true, array.erase(it);

	return found;
	// Do not use remove<> since it desn't modify size ... (???)
	// This doesn't seem to work.
	//return remove(array.begin(), array.end(), elt)!=array.end();
}


// ***************************************************************************
// ***************************************************************************
// Edge Cost methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::vertexHasOneWedge(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		sint	numwedge=-1;
		for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
		{
			sint	w= TmpFaces[vert.SharedFaces[i]].getAssociatedWedge(attId, numvertex);
			if(numwedge>=0 && numwedge!=w)	return false;
			else	numwedge=w;
		}
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexHasOneMaterial(sint numvertex)
{
	sint	matId=-1;
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		sint	m= TmpFaces[vert.SharedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexContinue(sint numvertex)
{
	return vertexHasOneWedge(numvertex) && vertexHasOneMaterial(numvertex);
}
// ***************************************************************************
bool	CMRMBuilder::vertexClosed(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	map<CMRMEdge, sint>		EdgeShare;
	// Init to 0.
	sint i;
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]= 0;
		EdgeShare[f.getEdge(1)]= 0;
		EdgeShare[f.getEdge(2)]= 0;
	}
	// Inc count.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]++;
		EdgeShare[f.getEdge(1)]++;
		EdgeShare[f.getEdge(2)]++;
	}
	// Test open edges.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		sint	v0= f.Corner[0].Vertex;
		sint	v1= f.Corner[1].Vertex;
		sint	v2= f.Corner[2].Vertex;
		if(EdgeShare[f.getEdge(0)]<2 && (v0==numvertex || v1==numvertex)) return false;
		if(EdgeShare[f.getEdge(1)]<2 && (v1==numvertex || v2==numvertex)) return false;
		if(EdgeShare[f.getEdge(2)]<2 && (v0==numvertex || v2==numvertex)) return false;
	}
	return true;
}
// ***************************************************************************
float	CMRMBuilder::getDeltaFaceNormals(sint numvertex)
{
	// return a positive value of Somme(|DeltaNormals|) / NNormals.
	CMRMVertex	&vert= TmpVertices[numvertex];
	float	delta=0;
	CVector	refNormal(0.f, 0.f, 0.f);
	sint	nfaces=(sint)vert.SharedFaces.size();
	for(sint i=0;i<nfaces;i++)
	{
		CVector	normal;
		CVector	&v0= TmpVertices[TmpFaces[i].Corner[0].Vertex].Current;
		CVector	&v1= TmpVertices[TmpFaces[i].Corner[1].Vertex].Current;
		CVector	&v2= TmpVertices[TmpFaces[i].Corner[2].Vertex].Current;
		normal= (v1-v0)^(v2-v0);
		normal.normalize();
		if(i==0)
			refNormal=normal;
		else
			delta+=(1-refNormal*normal);
	}
	if(nfaces<2)
		return 0;
	else
		return delta/(nfaces-1);
}
// ***************************************************************************
bool	CMRMBuilder::edgeContinue(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	sint i;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	sint	matId=-1;
	// test if faces have same material.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		sint	m;
		m= TmpFaces[deletedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
	}

	// test if faces have same wedge (for all att).
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		sint	numwedge1=-1,numwedge2=-1;
		for(i=0;i<(sint)deletedFaces.size();i++)
		{
			sint	w;
			w= TmpFaces[deletedFaces[i]].getAssociatedWedge(attId, v0);
			if(numwedge1>=0 && numwedge1!=w)	return false;
			else	numwedge1=w;
			w= TmpFaces[deletedFaces[i]].getAssociatedWedge(attId, v1);
			if(numwedge2>=0 && numwedge2!=w)	return false;
			else	numwedge2=w;
		}
	}

	return true;
}
// ***************************************************************************
bool	CMRMBuilder::edgeNearUniqueMatFace(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	sint i;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	// test if faces are not isolated OneMaterial faces.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		CMRMFaceBuild	&f=TmpFaces[deletedFaces[i]];
		if( !edgeContinue(f.getEdge(0)) &&
			!edgeContinue(f.getEdge(1)) &&
			!edgeContinue(f.getEdge(2)))
			return true;
	}

	return false;
}

// ***************************************************************************
float	CMRMBuilder::computeEdgeCost(const CMRMEdge &edge)
{
	sint	v1= edge.v0;
	sint	v2= edge.v1;
	// more expensive is the edge, later it will collapse.


	// **** standard cost

	// compute size of the edge.
	float	cost=(TmpVertices[v1].Current-TmpVertices[v2].Current).norm();

	// compute "curvature" of the edge.
	float	faceCost= (getDeltaFaceNormals(v1)+getDeltaFaceNormals(v2));
	// totally plane faces (faceCost==0) must be collapsed with respect to size (and not random if cost==0).
	// else we may have Plane Mesh (like flags) that will collapse in a very ugly way.
	faceCost= max(faceCost, 0.01f);

	// modulate size with curvature.
	cost*= faceCost;

	// Like H.Hope, add a weight on discontinuities..
	if( !vertexContinue(v1) && !vertexContinue(v2) )
	{
		// Nb: don't do this on discontinuities edges, unless the unique material face will collapse (pffiou!!).
		if( edgeContinue(edge) || edgeNearUniqueMatFace(edge) )
			cost*=4;
	}

	// **** Interface Sewing cost
	if(_HasMeshInterfaces)
	{
		// if the 2 vertices come from a Sewing Interface mesh (must be a real interface id)
		sint	meshSewingId= TmpVertices[v1].InterfaceLink.InterfaceId;
		if( meshSewingId>=0 && TmpVertices[v2].InterfaceLink.InterfaceId>=0 )
		{
			// if the 2 vertices come from the same Sewing Interface mesh
			if( meshSewingId == TmpVertices[v2].InterfaceLink.InterfaceId )
			{
				// Then the edge is one of the sewing interface mesh. must do special things for it
				CMRMSewingMesh	&sewingMesh= _SewingMeshes[meshSewingId];
				uint	dummy;

				// get the sewing edge id
				CMRMEdge	sewingEdge;
				sewingEdge.v0= TmpVertices[v1].InterfaceLink.InterfaceVertexId;
				sewingEdge.v1= TmpVertices[v2].InterfaceLink.InterfaceVertexId;
				// if the current sewing lod want to collapse this edge
				sint collapseId= sewingMesh.mustCollapseEdge(_CurrentLodComputed, sewingEdge, dummy);
				if(collapseId>=0)
				{
					// Then set a negative priority (ie will collapse as soon as possible). from -N to -1.
					// NB: sort them according to collapseId
					cost= (float)(-sewingMesh.getNumCollapseEdge(_CurrentLodComputed) + collapseId);
				}
				else
				{
					// This edge must not collapse at this Lod, set an infinite priority (hope will never collapse).
					cost= FLT_MAX;
				}
			}
			else
			{
				/* The edge is between 2 interfaces but not the same. If we collide it we'll have holes!
					This problem arise if space beetween interfaces is small. eg: if we setup an interface beetween
					hair and head, and another one beetween head and torso, then we'll have this problem in the
					back of the neck.
					The solution is to make a big big cost to hope we'll never collide them (else Holes...)!!
					Don't use FLT_MAX to still have a correct order if we don't have choice...
				*/
				cost*= 10000;
			}
		}
	}

	return cost;
}




// ***************************************************************************
// ***************************************************************************
// Collapse Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::faceShareWedges(CMRMFaceBuild *face, sint attribId, sint numVertex1, sint numVertex2)
{
	sint	numWedge1= face->getAssociatedWedge(attribId, numVertex1);
	sint	numWedge2= face->getAssociatedWedge(attribId, numVertex2);
	if(numWedge1<0) return false;
	if(numWedge2<0) return false;

	CMRMAttribute	&w1= TmpAttributes[attribId][numWedge1];
	CMRMAttribute	&w2= TmpAttributes[attribId][numWedge2];
	return w1.Shared && w2.Shared && w1.NbSharedFaces>0 && w2.NbSharedFaces>0;
}


// ***************************************************************************
void	CMRMBuilder::insertFaceIntoEdgeList(CMRMFaceBuild &f)
{
	float	len;
	if(f.ValidIt0)
	{
		len= computeEdgeCost(f.getEdge(0));
		f. It0= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(0),&f) ) );
	}
	if(f.ValidIt1)
	{
		len= computeEdgeCost(f.getEdge(1));
		f. It1= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(1),&f) ) );
	}
	if(f.ValidIt2)
	{
		len= computeEdgeCost(f.getEdge(2));
		f. It2= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(2),&f) ) );
	}
}
// ***************************************************************************
void	CMRMBuilder::removeFaceFromEdgeList(CMRMFaceBuild &f)
{
	if(f.ValidIt0)
		EdgeCollapses.erase(f.It0);
	if(f.ValidIt1)
		EdgeCollapses.erase(f.It1);
	if(f.ValidIt2)
		EdgeCollapses.erase(f.It2);
}



// ***************************************************************************
struct		CTmpVertexWeight
{
	uint32		MatrixId;
	float		Weight;
	// For find().
	bool	operator==(const CTmpVertexWeight &o) const
	{
		return MatrixId==o.MatrixId;
	}
	// For sort().
	bool	operator<(const CTmpVertexWeight &o) const
	{
		return Weight>o.Weight;
	}

};


// ***************************************************************************
CMesh::CSkinWeight	CMRMBuilder::collapseSkinWeight(const CMesh::CSkinWeight &sw1, const CMesh::CSkinWeight &sw2, float interValue) const
{
	// If fast interpolation.
	if(interValue==0)
		return sw1;
	if(interValue==1)
		return sw2;

	// else, must blend a skinWeight: must identify matrix which exist in the 2 sws, and add new ones.
	uint	nbMats1=0;
	uint	nbMats2=0;
	static vector<CTmpVertexWeight>		sws;
	sws.reserve(NL3D_MESH_SKINNING_MAX_MATRIX * 2);
	sws.clear();

	// For all weights of sw1.
	uint i;
	for(i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
	{
		CTmpVertexWeight	vw;
		vw.MatrixId= sw1.MatrixId[i];
		vw.Weight= sw1.Weights[i]*(1-interValue);
		// if this weight is not null.
		if(vw.Weight>0)
		{
			// add it to the list.
			sws.push_back(vw);
		}
		// For skinning reduction.
		if(sw1.Weights[i]>0)
			nbMats1++;
	}


	// For all weights of sw1.
	for(i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
	{
		CTmpVertexWeight	vw;
		vw.MatrixId= sw2.MatrixId[i];
		vw.Weight= sw2.Weights[i]*(interValue);
		// if this weight is not null.
		if(vw.Weight>0)
		{
			// add it or add influence to the matrix.
			vector<CTmpVertexWeight>::iterator		it= find(sws.begin(), sws.end(), vw);
			if(it== sws.end())
				sws.push_back(vw);
			else
				it->Weight+= vw.Weight;
		}
		// For skinning reduction.
		if(sw2.Weights[i]>0)
			nbMats2++;
	}


	// Then keep just the best.
	// sort by Weight decreasing order.
	sort(sws.begin(), sws.end());

	// clamp the result to the wanted max matrix.
	uint	nbMatsOut = 0;
	switch(_SkinReduction)
	{
	case CMRMParameters::SkinReductionMin:
		nbMatsOut= min(nbMats1, nbMats2);
		break;
	case CMRMParameters::SkinReductionMax:
		nbMatsOut= max(nbMats1, nbMats2);
		break;
	case CMRMParameters::SkinReductionBest:
		nbMatsOut= min( (uint)sws.size(), (uint)NL3D_MESH_SKINNING_MAX_MATRIX );
		break;
	default:
		nlstop;
	};
	// For security.
	nbMatsOut= min(nbMatsOut, (uint)sws.size());
	nlassert(nbMatsOut<=NL3D_MESH_SKINNING_MAX_MATRIX);


	// Then output the result to the skinWeight, normalizing.
	float	sumWeight=0;
	for(i= 0; i<nbMatsOut; i++)
	{
		sumWeight+= sws[i].Weight;
	}

	CMesh::CSkinWeight	ret;
	// Fill only needed matrix (other are rested in CMesh::CSkinWeight ctor).
	for(i= 0; i<nbMatsOut; i++)
	{
		ret.MatrixId[i]= sws[i].MatrixId;
		ret.Weights[i]= sws[i].Weight / sumWeight;
	}

	return ret;
}

// ***************************************************************************
sint	CMRMBuilder::collapseEdge(const CMRMEdge &edge)
{
	sint	i,j;
	float	InterValue;
	sint	edgeV1=edge.v0;
	sint	edgeV2=edge.v1;


	// 0. collapse the vertices.
	//==========================

	// edge.Vertex1 kept, but morphed.
	// edge.Vertex2 deleted, and must know on which vertex it collapse.
	CMRMVertex	&Vertex1=TmpVertices[edgeV1], &Vertex2=TmpVertices[edgeV2];

	// Interpolation choice.
	// Default is to interpolate vertex 0 to the middle of the edge.
	InterValue=0.5;
	//InterValue=1;
	// **** If at least one vertex of the edge is on a mesh sewing interface, must change InterValue
	if( _HasMeshInterfaces && (Vertex1.InterfaceLink.InterfaceId>=0 || Vertex2.InterfaceLink.InterfaceId>=0) )
	{
		// If this is an edge of a mesh sewing interface
		if(Vertex1.InterfaceLink.InterfaceId==Vertex2.InterfaceLink.InterfaceId)
		{
			// Then the edge is one of the sewing interface mesh. must do special things for it
			CMRMSewingMesh	&sewingMesh= _SewingMeshes[Vertex1.InterfaceLink.InterfaceId];

			// get the sewing edge id
			CMRMEdge	sewingEdge;
			sewingEdge.v0= Vertex1.InterfaceLink.InterfaceVertexId;
			sewingEdge.v1= Vertex2.InterfaceLink.InterfaceVertexId;

			// Get the edge in the sewing mesh which is said to be collapsed
			uint	vertToCollapse;
			sint collapseId= sewingMesh.mustCollapseEdge(_CurrentLodComputed, sewingEdge, vertToCollapse);
			// if exist
			if(collapseId>=0)
			{
				// if it is v0 which must collapse, then InterValue=1
				if(vertToCollapse==(uint)sewingEdge.v0)
					InterValue= 1;
				else
					InterValue= 0;

			}
			else
			{
				// This should not happens. But it is still possible if this edge don't want to collapse but if their
				// is no more choice. Take a default value
				InterValue= 0;
			}
		}
		else
		{
			// must collapse to the vertex on the sewing interface (as if it was open)
			if(Vertex1.InterfaceLink.InterfaceId>=0)
			{
				// NB: it is possible that both vertices are on a different sewing interface... still collapse (must have to)
				InterValue= 0;
			}
			else
			{
				// Then Vertex2 is on a sewing interface, collapse to it
				InterValue= 1;
			}
		}
	}
	// **** Else, on special cases, it is much more efficient to interpolate at start or at end of edge.
	else
	{
		// If one vertex is "open", ie his shared faces do not represent a closed Fan, then interpolate to this one,
		// so the mesh has the same silhouette.
		bool	vc1= vertexClosed(edgeV1);
		bool	vc2= vertexClosed(edgeV2);
		if(!vc1 && vc2) InterValue=0;
		else if(vc1 && !vc2) InterValue=1;
		else
		{
			// Do the same test but with vertex continue: it is preferable to not move the boundaries
			// of a material, or a mapping.
			bool	vc1= vertexContinue(edgeV1);
			bool	vc2= vertexContinue(edgeV2);
			if(!vc1 && vc2) InterValue=0;
			if(vc1 && !vc2) InterValue=1;
		}
	}
	/*BENCH_TotalCollapses++;
	if(InterValue==0.5)
		BENCH_MiddleCollapses++;*/

	// Collapse the Vertex.
	//========================
	Vertex1.Current= Vertex1.Current*(1-InterValue) + Vertex2.Current*InterValue;
	for (i = 0; i < (sint)Vertex1.BSCurrent.size(); ++i)
		Vertex1.BSCurrent[i] = Vertex1.BSCurrent[i]*(1-InterValue) + Vertex2.BSCurrent[i]*InterValue;
	Vertex2.CollapsedTo= edgeV1;
	if(_Skinned)
		Vertex1.CurrentSW= collapseSkinWeight(Vertex1.CurrentSW, Vertex2.CurrentSW, InterValue);
	if( _HasMeshInterfaces )
		Vertex1.InterfaceLink= InterValue<0.5f? Vertex1.InterfaceLink : Vertex2.InterfaceLink;

	// \todo yoyo: TODO_BUG: Don't know why, but vertices may point on deleted faces.
	// Temp: we destroy here thoses face from SharedFaces...
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].Deleted)
			deleteElement(Vertex1.SharedFaces, numFace), i--;
	}
	for(i=0;i<(sint)Vertex2.SharedFaces.size();i++)
	{
		sint	numFace= Vertex2.SharedFaces[i];
		if(TmpFaces[numFace].Deleted)
			deleteElement(Vertex2.SharedFaces, numFace), i--;
	}


	// Build Neighbor faces.
	vector<sint>	neighboorFaces;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(!findElement(neighboorFaces, numFace))
			neighboorFaces.push_back(numFace);
	}
	for(i=0;i<(sint)Vertex2.SharedFaces.size();i++)
	{
		sint	numFace= Vertex2.SharedFaces[i];
		if(!findElement(neighboorFaces, numFace))
			neighboorFaces.push_back(numFace);
	}

	// Build faces which will be destroyed (may 1 or 2, maybe more for non conventionnal meshes).
	vector<sint>	deletedFaces;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		nlassert(!TmpFaces[numFace].Deleted);
		if(TmpFaces[numFace].hasVertex(edgeV2))
			deletedFaces.push_back(numFace);
	}


	// 1. Collapse the wedges.
	//========================

	// For ALL Attributes.
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		// a/ Stock the wedge interpolation in each destroyed face.
		//------------------------------------------------------
		for(i=0;i<(sint)deletedFaces.size();i++)
		{
			CMRMFaceBuild		&face= TmpFaces[deletedFaces[i]];

			CVectorH	&w0= TmpAttributes[attId][ face.getAssociatedWedge(attId, edgeV1) ].Current;
			CVectorH	&w1= TmpAttributes[attId][ face.getAssociatedWedge(attId, edgeV2) ].Current;

			CVectorH	&itp= face.InterpolatedAttribute;
			itp.x= w0.x*(1-InterValue) + w1.x*InterValue;
			itp.y= w0.y*(1-InterValue) + w1.y*InterValue;
			itp.z= w0.z*(1-InterValue) + w1.z*InterValue;
			itp.w= w0.w*(1-InterValue) + w1.w*InterValue;

			for (j = 0; j < (sint)face.BSInterpolated.size(); ++j)
			{
				CVectorH &w0 = TmpAttributes[attId][face.getAssociatedWedge(attId, edgeV1)].BSCurrent[j];
				CVectorH &w1 = TmpAttributes[attId][face.getAssociatedWedge(attId, edgeV2)].BSCurrent[j];
				CVectorH &itb = face.BSInterpolated[j];
				itb.x = w0.x*(1-InterValue) + w1.x*InterValue;
				itb.y = w0.y*(1-InterValue) + w1.y*InterValue;
				itb.z = w0.z*(1-InterValue) + w1.z*InterValue;
				itb.w = w0.w*(1-InterValue) + w1.w*InterValue;
			}
		}


		// b/ Build wedge list to be modify.
		//----------------------------------
		vector<sint>	wedges;

		for(i=0;i<(sint)neighboorFaces.size();i++)
		{
			CMRMFaceBuild	&face= TmpFaces[neighboorFaces[i]];
			sint	numWedge;

			numWedge= face.getAssociatedWedge(attId, edgeV1);
			if(numWedge>=0 && !findElement(wedges, numWedge))
				wedges.push_back(numWedge);

			numWedge= face.getAssociatedWedge(attId, edgeV2);
			if(numWedge>=0 && !findElement(wedges, numWedge))
				wedges.push_back(numWedge);
		}


		// c/ Count numFaces which point on those wedges. (- deleted faces).
		//------------------------------------------------------------------

		for(i=0;i<(sint)wedges.size();i++)
		{
			sint			numWedge= wedges[i];
			CMRMAttribute	&wedge= TmpAttributes[attId][numWedge];

			wedge.NbSharedFaces=0;
			wedge.Shared=false;

			// Count total ref count.
			for(j=0;j<(sint)neighboorFaces.size();j++)
			{
				if(TmpFaces[neighboorFaces[j]].hasWedge(attId, numWedge))
					wedge.NbSharedFaces++;
			}

			// Minus deleted faces.
			for(j=0;j<(sint)deletedFaces.size();j++)
			{
				if(TmpFaces[deletedFaces[j]].hasWedge(attId, numWedge))
				{
					wedge.NbSharedFaces--;
					wedge.Shared=true;
					wedge.InterpolatedFace=deletedFaces[j];
				}
			}
		}


		// d/ Collapse wedge following 3 possibles cases.
		//-----------------------------------------------


		for(i=0;i<(sint)wedges.size();i++)
		{
			sint			numWedge= wedges[i];
			CMRMAttribute	&wedge= TmpAttributes[attId][numWedge];

			// if wedge not shared...
			if(!wedge.Shared)
			{
				// We've got an "exterior wedge" which lost no corner => do not merge it nor delete it.
				// Leave it as the same value (extrapolate it may not be a good solution).
			}
			else
			{
				// if wedge dissapears, notify.
				if(wedge.NbSharedFaces==0)
				{
					wedge.CollapsedTo=-2;
					// Do not change his value. (as specified in Hope article).
				}
				else
				{
					CMRMFaceBuild	&face= TmpFaces[wedge.InterpolatedFace];

					// Must interpolate it.
					wedge.Current= face.InterpolatedAttribute;
					wedge.BSCurrent = face.BSInterpolated;

					// Must merge the wedge of the second vertex on first
					// ONLY IF 2 interpolated wedges are shared and NbSharedFaces!=0.
					if(	numWedge==face.getAssociatedWedge(attId, edgeV2) &&
						faceShareWedges(&face, attId, edgeV1, edgeV2) )
					{
						wedge.CollapsedTo= face.getAssociatedWedge(attId, edgeV1);
					}
				}
			}
		}

	}

	// 3. collapse faces.
	//===================

	// delete face shared by edge.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		sint	numFace= deletedFaces[i];
		TmpFaces[numFace].Deleted=true;

		// release edges from list.
		removeFaceFromEdgeList(TmpFaces[numFace]);
		// invalid all it!!
		TmpFaces[numFace].invalidAllIts(EdgeCollapses);


		// delete from vertex1 and 2 the deleted faces.
		deleteElement( Vertex1.SharedFaces, numFace);
		deleteElement( Vertex2.SharedFaces, numFace);
	}


	// must ref correctly the faces.
	for(i=0;i<(sint)neighboorFaces.size();i++)
	{
		CMRMFaceBuild		&face=TmpFaces[neighboorFaces[i]];

		// good vertices
		if(face.Corner[0].Vertex ==edgeV2)	face.Corner[0].Vertex=edgeV1;
		if(face.Corner[1].Vertex ==edgeV2)	face.Corner[1].Vertex=edgeV1;
		if(face.Corner[2].Vertex ==edgeV2)	face.Corner[2].Vertex=edgeV1;
		// nb: doesn't matter if deletedFaces are modified...

		// good wedges
		for(sint attId=0;attId<NumAttributes;attId++)
		{
			sint	newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[0].Attributes[attId] ].CollapsedTo;
			if(newWedge>=0)	face.Corner[0].Attributes[attId]= newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[1].Attributes[attId] ].CollapsedTo;
			if(newWedge>=0)	face.Corner[1].Attributes[attId]= newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[2].Attributes[attId] ].CollapsedTo;
			if(newWedge>=0)	face.Corner[2].Attributes[attId]= newWedge;
		}

		// good edges.
		/* Those ones are updated in collapseEdges(): they are removed from the edgeCollapseList,
			then they are re-inserted with good Vertex indices.
		*/
	}


	// The vertex1 has now the shared env of vertex2.
	Vertex1.SharedFaces.insert(Vertex1.SharedFaces.end(), Vertex2.SharedFaces.begin(),
		Vertex2.SharedFaces.end());


	return (sint)deletedFaces.size();
}


// ***************************************************************************
sint	CMRMBuilder::followVertex(sint i)
{
	CMRMVertex	&vert=TmpVertices[i];
	if(vert.CollapsedTo>=0)
		return followVertex(vert.CollapsedTo);
	else
		return i;
}
// ***************************************************************************
sint	CMRMBuilder::followWedge(sint attribId, sint i)
{
	CMRMAttribute	&wedge= TmpAttributes[attribId][i];
	if(wedge.CollapsedTo>=0)
		return followWedge(attribId, wedge.CollapsedTo);
	else
		return i;
}


// ***************************************************************************
// ***************************************************************************
// Mesh Level method.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMRMBuilder::CMRMBuilder()
{
	NumAttributes= 0;
	_Skinned= false;
	_HasMeshInterfaces= false;
}

// ***************************************************************************
void	CMRMBuilder::init(const CMRMMesh &baseMesh)
{
	sint	i, attId;


	// First clear ALL.
	TmpVertices.clear();
	for(attId=0;attId<NL3D_MRM_MAX_ATTRIB;attId++)
	{
		TmpAttributes[attId].clear();
	}
	TmpFaces.clear();
	EdgeCollapses.clear();


	// resize.
	NumAttributes= baseMesh.NumAttributes;
	TmpVertices.resize(baseMesh.Vertices.size());
	for(attId=0;attId<NumAttributes;attId++)
	{
		TmpAttributes[attId].resize(baseMesh.Attributes[attId].size());
	}
	TmpFaces.resize(baseMesh.Faces.size());


	// Then copy.
	for(i=0;i<(sint)baseMesh.Vertices.size();i++)
	{
		TmpVertices[i].Current= TmpVertices[i].Original= baseMesh.Vertices[i];
		TmpVertices[i].BSCurrent.resize(baseMesh.BlendShapes.size());
		for(uint32 j = 0; j <baseMesh.BlendShapes.size() ;++j)
			TmpVertices[i].BSCurrent[j]= baseMesh.BlendShapes[j].Vertices[i];
		if(_Skinned)
			TmpVertices[i].CurrentSW= TmpVertices[i].OriginalSW= baseMesh.SkinWeights[i];
		if(_HasMeshInterfaces)
			TmpVertices[i].InterfaceLink= baseMesh.InterfaceLinks[i];
	}
	for(attId=0;attId<NumAttributes;attId++)
	{
		for(i=0;i<(sint)baseMesh.Attributes[attId].size();i++)
		{
			TmpAttributes[attId][i].Current= TmpAttributes[attId][i].Original=
			baseMesh.Attributes[attId][i];
			TmpAttributes[attId][i].BSCurrent.resize(baseMesh.BlendShapes.size());
			for(uint32 j = 0; j <baseMesh.BlendShapes.size() ;++j)
				TmpAttributes[attId][i].BSCurrent[j]= baseMesh.BlendShapes[j].Attributes[attId][i];
		}
	}
	for(i=0;i<(sint)baseMesh.Faces.size();i++)
	{
		TmpFaces[i]= baseMesh.Faces[i];
		TmpFaces[i].BSInterpolated.resize(baseMesh.BlendShapes.size());
	}


	// Create vertices sharedFaces.
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild		&face= TmpFaces[i];

		TmpVertices[face.Corner[0].Vertex].SharedFaces.push_back(i);
		TmpVertices[face.Corner[1].Vertex].SharedFaces.push_back(i);
		TmpVertices[face.Corner[2].Vertex].SharedFaces.push_back(i);
	}


	// Compute EdgeCost.
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild		&f= TmpFaces[i];
		// At start, valid all edges.
		f. ValidIt0= true;
		f. ValidIt1= true;
		f. ValidIt2= true;
		insertFaceIntoEdgeList(f);
	}
}
// ***************************************************************************
void	CMRMBuilder::collapseEdges(sint nWantedFaces)
{
	ItEdgeMap		EdgeIt;

	sint	nCurrentFaces=(sint)TmpFaces.size();
	sint	bug0=0,bug2=0,bug3=0;

	while(nCurrentFaces>nWantedFaces)
	{
		bug0++;
		EdgeIt= EdgeCollapses.begin();

		if(EdgeIt== EdgeCollapses.end())
			break;

		// 0. Look if edge already deleted
		//================================
		CMRMEdge	edge=(*EdgeIt).second;

		// Is it valid?? (ie his vertices exist yet??).
		if(TmpVertices[ edge.v0 ].CollapsedTo>=0
			|| TmpVertices[ edge.v1 ].CollapsedTo>=0)
		{
			// \todo yoyo: TODO_BUG: potential bug here...
			CMRMFaceBuild		&f= *(EdgeIt->second.Face);
			nlassert(f.validEdgeIt(EdgeIt->second));
			f.invalidEdgeIt(EdgeIt->second, EdgeCollapses);
			EdgeCollapses.erase(EdgeIt);
			bug2++;
			continue;
		}
		// \todo yoyo: TODO_BUG: potential bug here...
		// If a mesh is "open" it will crash if a "hole collapse"...
		if(edge.v0==edge.v1)
		{
			CMRMFaceBuild		&f= *(EdgeIt->second.Face);
			nlassert(f.validEdgeIt(EdgeIt->second));
			f.invalidEdgeIt(EdgeIt->second, EdgeCollapses);
			EdgeCollapses.erase(EdgeIt);
			bug3++;
			continue;
		}


		// 1. else, OK, collapse it!!
		//===========================
		sint	vertexCollapsed= edge.v0;
		nCurrentFaces-= collapseEdge(edge);


		// 2. Must reorder all his neighborhood.
		//======================================
		CMRMVertex	&vert=TmpVertices[vertexCollapsed];
		sint	i;
		// we delete from list modified edges, and we re-add them with their new value.
		for(i=0;i<(sint)vert.SharedFaces.size();i++)
		{
			CMRMFaceBuild		&f= TmpFaces[vert.SharedFaces[i]];
			removeFaceFromEdgeList(f);
			insertFaceIntoEdgeList(f);
		}

	}
}
// ***************************************************************************
void	CMRMBuilder::saveCoarserMesh(CMRMMesh &coarserMesh)
{
	sint	i,attId,index;
	// First clear ALL.
	coarserMesh.Vertices.clear();
	coarserMesh.SkinWeights.clear();
	coarserMesh.InterfaceLinks.clear();
	for(attId=0;attId<NL3D_MRM_MAX_ATTRIB;attId++)
	{
		coarserMesh.Attributes[attId].clear();
	}
	coarserMesh.Faces.clear();
	coarserMesh.NumAttributes= NumAttributes;

	// Vertices.
	//==========
	index=0;
	for(i=0;i<(sint)TmpVertices.size();i++)
	{
		CMRMVertex	&vert=TmpVertices[i];
		if(vert.CollapsedTo==-1)	// if exist yet.
		{
			vert.CoarserIndex=index;
			coarserMesh.Vertices.push_back(vert.Current);
			if(_Skinned)
				coarserMesh.SkinWeights.push_back(vert.CurrentSW);
			if(_HasMeshInterfaces)
				coarserMesh.InterfaceLinks.push_back(vert.InterfaceLink);

			index++;
		}
		else
			vert.CoarserIndex=-1;	// indicate that this vertex no more exist and is to be geomorphed to another.
	}


	// Attributes.
	//============
	for(attId=0;attId<NumAttributes;attId++)
	{
		index=0;
		for(i=0;i<(sint)TmpAttributes[attId].size();i++)
		{
			CMRMAttribute	&wedge= TmpAttributes[attId][i];
			if(wedge.CollapsedTo==-1)	// if exist yet.
			{
				wedge.CoarserIndex=index;
				coarserMesh.Attributes[attId].push_back(wedge.Current);
				index++;
			}
			else if(wedge.CollapsedTo==-2)	// else if totaly destroyed.
			{
				// Insert this wedge in the coarser mesh.
				// NB: the coarser mesh faces do not use it anymore, but FinerMesh use it
				// for geomorph (LODMesh.CoarserFaces may point to it).
				// NB: look at buildFinalMRM(), it works fine for all cases.
				wedge.CoarserIndex=index;
				coarserMesh.Attributes[attId].push_back(wedge.Current);
				index++;
			}
			else
				wedge.CoarserIndex=-1;	// indicate that this wedge no more exist and is to be geomorphed to another.
		}
	}

	// Faces.
	//=======
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild	&face=TmpFaces[i];
		if(!face.Deleted)
		{
			CMRMFace	newFace;
			// Material.
			newFace.MaterialId= face.MaterialId;
			for(sint j=0;j<3;j++)
			{
				// Vertex.
				newFace.Corner[j].Vertex= TmpVertices[face.Corner[j].Vertex].CoarserIndex;
				nlassert(newFace.Corner[j].Vertex>=0);
				// Attributes.
				for(attId=0;attId<NumAttributes;attId++)
				{
					sint	oldidx= face.Corner[j].Attributes[attId];
					newFace.Corner[j].Attributes[attId]= TmpAttributes[attId][oldidx].CoarserIndex;
					nlassert(newFace.Corner[j].Attributes[attId]>=0);
				}

			}

			coarserMesh.Faces.push_back(newFace);
		}
	}

}


// ***************************************************************************
void	CMRMBuilder::makeLODMesh(CMRMMeshGeom &lodMesh)
{
	sint	i,j,attId,index,coidx;

	// for all faces of this mesh, find target in the coarser mesh.
	for(i=0;i<(sint)lodMesh.CoarserFaces.size();i++)
	{
		CMRMFace	&face= lodMesh.CoarserFaces[i];

		// For 3 corners.
		for(j=0;j<3;j++)
		{
			// Vertex.
			// The index is yet the index in the finer mesh.
			index= face.Corner[j].Vertex;
			// the index in the coarser mesh is vert.CoarserIndex.
			coidx= TmpVertices[index].CoarserIndex;
			// but if this vertex is collapsed, must find the good index (yet in the finer mesh)
			if(coidx==-1)
			{
				// find to which we must collapse.
				index= followVertex(index);
				// and so we have the coarser index. this one must be valid.
				coidx= TmpVertices[index].CoarserIndex;
				nlassert(coidx>=0);
			}
			// update corner of CoarserFace.
			face.Corner[j].Vertex= coidx;


			// Do exactly same thing for all attributes.
			for(attId=0;attId<NumAttributes;attId++)
			{
				index= face.Corner[j].Attributes[attId];
				coidx= TmpAttributes[attId][index].CoarserIndex;
				if(coidx==-1)
				{
					index= followWedge(attId, index);
					coidx= TmpAttributes[attId][index].CoarserIndex;
					nlassert(coidx>=0);
				}
				face.Corner[j].Attributes[attId]= coidx;
			}
		}
	}

}

// ***************************************************************************
// Transform source blend shapes to source blend shapes modified (just calculate new vertex/attr position)
/*void	CMRMBuilder::computeBsVerticesAttributes(vector<CMRMMesh> &srcBsMeshs, vector<CMRMMesh> &bsMeshsMod)
{
	sint	i, j, k, attId;

	bsMeshsMod.resize (srcBsMeshs.size());
	for (k = 0; k < (sint)srcBsMeshs.size(); ++k)
	{
		CMRMMesh &rBsMesh = srcBsMeshs[k];
		CMRMMesh &rBsMeshMod = bsMeshsMod[k];

		// Calculate modified vertices with the linear equation back tracking help
		rBsMeshMod.Vertices.resize (rBsMesh.Vertices.size());
		for (i = 0; i < (sint)rBsMesh.Vertices.size(); ++i)
		{
			CLinearEquation &LinEq = TmpVertices[i].CurrentLinEq;
			rBsMeshMod.Vertices[i] = CVector(0.0f, 0.0f, 0.0f);
			for (j = 0; j < (sint)LinEq.Elts.size(); ++j)
			{
				rBsMeshMod.Vertices[i] += LinEq.Elts[j].factor * rBsMesh.Vertices[LinEq.Elts[j].index];
			}
		}

		// All attributes
		rBsMeshMod.NumAttributes = NumAttributes;
		for (attId = 0; attId < NumAttributes; attId++)
		{
			rBsMeshMod.Attributes[attId].resize (rBsMesh.Attributes[attId].size());
			for (i = 0; i < (sint)rBsMesh.Attributes[attId].size(); ++i)
			{
				CLinearEquation &LinEq = TmpAttributes[attId][i].CurrentLinEq;
				rBsMeshMod.Attributes[attId][i] = CVectorH(0.0f, 0.0f, 0.0f, 0.0f);
				for (j = 0; j < (sint)LinEq.Elts.size(); ++j)
				{
					rBsMeshMod.Attributes[attId][i].x += LinEq.Elts[j].factor * rBsMesh.Attributes[attId][LinEq.Elts[j].index].x;
					rBsMeshMod.Attributes[attId][i].y += LinEq.Elts[j].factor * rBsMesh.Attributes[attId][LinEq.Elts[j].index].y;
					rBsMeshMod.Attributes[attId][i].z += LinEq.Elts[j].factor * rBsMesh.Attributes[attId][LinEq.Elts[j].index].z;
					rBsMeshMod.Attributes[attId][i].w += LinEq.Elts[j].factor * rBsMesh.Attributes[attId][LinEq.Elts[j].index].w;
				}
			}
		}
	}
}*/

// ***************************************************************************
// Transform source Blend Shape Meshes Modified into coarser blend shape mesh (compact vertices)
void	CMRMBuilder::makeCoarserBS (vector<CMRMBlendShape> &csBsMeshs)
{
	uint32 i, k;
	sint32 nSizeVert, nSizeAttr, attId;

	// Calculate size of vertices array
	nSizeVert = 0;
	for (i = 0; i < TmpVertices.size(); ++i)
		if(TmpVertices[i].CoarserIndex > nSizeVert)
			nSizeVert = TmpVertices[i].CoarserIndex;
	++nSizeVert;

	for (k = 0; k < csBsMeshs.size(); ++k)
	{
		CMRMBlendShape &rBsCoarserMesh = csBsMeshs[k];

		rBsCoarserMesh.Vertices.resize (nSizeVert);
		rBsCoarserMesh.NumAttributes = NumAttributes;

		// Vertices
		for(i = 0; i < TmpVertices.size(); ++i)
		{
			CMRMVertex &vert = TmpVertices[i];
			if (vert.CoarserIndex != -1)
			{
				rBsCoarserMesh.Vertices[vert.CoarserIndex] = vert.BSCurrent[k];
			}
		}

		for (attId = 0; attId < NumAttributes; attId++)
		{
			// Calculate size of attribute attId array
			nSizeAttr = 0;
			for(i = 0; i < TmpAttributes[attId].size(); i++)
				if (TmpAttributes[attId][i].CoarserIndex > nSizeAttr)
					nSizeAttr = TmpAttributes[attId][i].CoarserIndex;
			++nSizeAttr;

			rBsCoarserMesh.Attributes[attId].resize (nSizeAttr);

			for (i = 0; i < TmpAttributes[attId].size(); i++)
			{
				CMRMAttribute &wedge = TmpAttributes[attId][i];
				if (wedge.CoarserIndex != -1)
				{
					rBsCoarserMesh.Attributes[attId][wedge.CoarserIndex] = wedge.BSCurrent[k];
				}
			}
		}
	}
}

// ***************************************************************************
void	CMRMBuilder::makeFromMesh(const CMRMMesh &baseMesh, CMRMMeshGeom &lodMesh, CMRMMesh &coarserMesh, sint nWantedFaces)
{
	// Init Tmp values in MRM builder.
	init(baseMesh);

	// compute MRM too next tgt face.
	collapseEdges(nWantedFaces);

	// save the coarser mesh.
	saveCoarserMesh(coarserMesh);
	// Build coarser BlendShapes.
	coarserMesh.BlendShapes.resize(baseMesh.BlendShapes.size());
	makeCoarserBS(coarserMesh.BlendShapes);

	// build the lodMesh (baseMesh, with vertex/Attributes collapse infos).
	lodMesh= baseMesh;
	makeLODMesh(lodMesh);

	// end for this level.
}



// ***************************************************************************
// ***************************************************************************
// Global MRM Level method.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMRMBuilder::buildAllLods(const CMRMMesh &baseMesh, std::vector<CMRMMeshGeom> &lodMeshs,
								  uint nWantedLods, uint divisor)
{
	sint	nFaces= (sint)baseMesh.Faces.size();
	sint	nBaseFaces;
	sint	i;
	CMRMMesh srcMesh = baseMesh;

	// coarsest LOD will have those number of faces.
	nBaseFaces=nFaces/divisor;
	nBaseFaces=max(nBaseFaces,4);

	// must have at least 2 LOD to be really intersting. But the rest of the process work too with only one Lod!!
	nlassert(nWantedLods>=1);
	lodMeshs.resize(nWantedLods);

	// If only one lod asked, must init some Tmp Global values (like NumAttributes)
	if(nWantedLods==1)
	{
		_CurrentLodComputed= 0;
		init(baseMesh);
	}

	// must fill all LODs, from end to start. do not proces last lod since it will be the coarsest mesh.
	for(i=nWantedLods-1;i>0;i--)
	{
		sint	nbWantedFaces;

		// for sewing computing
		_CurrentLodComputed= i;

		// Linear.
		nbWantedFaces= nBaseFaces + (nFaces-nBaseFaces) * (i-1)/(nWantedLods-1);
		nbWantedFaces=max(nbWantedFaces,4);

		// Build this LOD.
		CMRMMesh	csMesh;
		// The mesh
		makeFromMesh(srcMesh, lodMeshs[i], csMesh, nbWantedFaces);

		// next mesh to process is csMesh.
		srcMesh = csMesh;
	}
	// the first lodMedsh gets the coarsest mesh.
	lodMeshs[0]= srcMesh;
}


// ***************************************************************************
void	CMRMBuilder::buildFinalMRM(std::vector<CMRMMeshGeom> &lodMeshs, CMRMMeshFinal &finalMRM)
{
	sint	i,j;
	sint	lodId, attId;
	sint	nLods= (sint)lodMeshs.size();

	// Init.
	// ===============
	finalMRM.reset();
	finalMRM.NumAttributes= NumAttributes;
	finalMRM.Skinned= _Skinned;
	CMRMMeshFinal::CWedge::NumAttributesToCompare= NumAttributes;
	CMRMMeshFinal::CWedge::CompareSkinning= _Skinned;
	finalMRM.Lods.resize(nLods);


	// Build Wedges, and faces index.
	// ===============
	// for all lods.
	for(lodId=0; lodId<nLods; lodId++)
	{
		CMRMMeshGeom	&lodMesh= lodMeshs[lodId];
		CMRMMeshGeom	&lodMeshPrec= lodMeshs[lodId==0?0:lodId-1];
		// for all face corner.
		for(i=0; i<(sint)lodMesh.Faces.size();i++)
		{
			// The current face.
			CMRMFace	&face= lodMesh.Faces[i];
			// the current face, but which points to the prec LOD vertices/attributes.
			CMRMFace	&faceCoarser= lodMesh.CoarserFaces[i];
			// for 3 corners.
			for(j=0;j<3;j++)
			{
				CMRMCorner	&corner= face.Corner[j];
				CMRMCorner	&cornerCoarser= faceCoarser.Corner[j];
				// start and end wedge (geomorph), maybe same.
				CMRMMeshFinal::CWedge		wedgeStart;
				CMRMMeshFinal::CWedge		wedgeEnd;

				// fill wedgeStart with values from lodMesh.
				wedgeStart.Vertex= lodMesh.Vertices[corner.Vertex];
				if(_Skinned)
					wedgeStart.VertexSkin= lodMesh.SkinWeights[corner.Vertex];
				for(attId=0; attId<NumAttributes; attId++)
				{
					wedgeStart.Attributes[attId]= lodMesh.Attributes[attId][corner.Attributes[attId]];
				}

				// if geomorph possible (ie not lod 0).
				if(lodId>0)
				{
					// fill wedgeEnd with values from coarser lodMesh.
					wedgeEnd.Vertex= lodMeshPrec.Vertices[cornerCoarser.Vertex];
					if(_Skinned)
						wedgeEnd.VertexSkin= lodMeshPrec.SkinWeights[cornerCoarser.Vertex];
					for(attId=0; attId<NumAttributes; attId++)
					{
						wedgeEnd.Attributes[attId]= lodMeshPrec.Attributes[attId][cornerCoarser.Attributes[attId]];
					}
				}
				else
				{
					// no geomorph.
					wedgeEnd= wedgeStart;
				}

				// find/insert wedge, and get Ids. NB: if start/end same, same indices.
				sint	wedgeStartId= finalMRM.findInsertWedge(wedgeStart);
				sint	wedgeEndId= finalMRM.findInsertWedge(wedgeEnd);

				// store in TmpCorner.
				corner.WedgeStartId= wedgeStartId;
				corner.WedgeEndId= wedgeEndId;
			}
		}

		// Here, the number of wedge indicate the max number of wedge this LOD needs.
		finalMRM.Lods[lodId].NWedges= (sint)finalMRM.Wedges.size();
	}


	// Count NBWedges necessary for geomorph, and compute Dest geomorph wedges ids.
	// ===============
	// the number of geomorph required for one LOD.
	sint	sglmGeom;
	// the number of geomorph required for all LOD (max of sglmGeom).
	sint	sglmGeomMax= 0;

	// Do not process lod 0, since no geomorph.
	for(lodId=1; lodId<nLods; lodId++)
	{
		CMRMMeshGeom	&lodMesh= lodMeshs[lodId];

		// reset the GeomMap, the one which indicate if we have already inserted a geomorph.
		_GeomMap.clear();
		sglmGeom= 0;

		// for all face corner.
		for(i=0; i<(sint)lodMesh.Faces.size();i++)
		{
			// The current face.
			CMRMFace	&face= lodMesh.Faces[i];
			// for 3 corners.
			for(j=0;j<3;j++)
			{
				CMRMCorner	&corner= face.Corner[j];

				// if not same wedge Ids, this is a geomorphed wedge.
				if(corner.WedgeStartId != corner.WedgeEndId)
				{
					// search if it exist yet in the set.
					CMRMWedgeGeom	geom;
					geom.Start= corner.WedgeStartId;
					geom.End= corner.WedgeEndId;
					sint	geomDest= sglmGeom;
					// if don't find this geom in the set, then it is a new one.
					TGeomMap::const_iterator	it= _GeomMap.find(geom);
					if(it == _GeomMap.end())
					{
						_GeomMap.insert( make_pair(geom, geomDest) );
						sglmGeom++;
					}
					else
						geomDest= it->second;

					// store this Geom Id in the corner.
					corner.WedgeGeomId= geomDest;
				}
			}
		}

		// take the max.
		sglmGeomMax= max(sglmGeomMax, sglmGeom);
	}


	// inform the finalMRM.
	finalMRM.NGeomSpace= sglmGeomMax;


	// decal all wedges/ face index.
	// ===============
	// insert an empty space for dest geomorph.
	finalMRM.Wedges.insert(finalMRM.Wedges.begin(), sglmGeomMax, CMRMMeshFinal::CWedge());

	// Parse all faces corner of All lods, and decal Start/End Wedge index.
	for(lodId=0; lodId<nLods; lodId++)
	{
		CMRMMeshGeom	&lodMesh= lodMeshs[lodId];

		// for all face corner.
		for(i=0; i<(sint)lodMesh.Faces.size();i++)
		{
			// The current face.
			CMRMFace	&face= lodMesh.Faces[i];
			// for 3 corners.
			for(j=0;j<3;j++)
			{
				CMRMCorner	&corner= face.Corner[j];

				// decal indices.
				corner.WedgeStartId+= sglmGeomMax;
				corner.WedgeEndId+= sglmGeomMax;
			}
		}

		// increment too the number of wedge required for this Lod.
		finalMRM.Lods[lodId].NWedges+= sglmGeomMax;
	}


	// fill faces.
	// ===============
	// Parse all faces corner of All lods, and build Faces/Geomorphs..
	for(lodId=0; lodId<nLods; lodId++)
	{
		CMRMMeshGeom			&lodMesh= lodMeshs[lodId];
		CMRMMeshFinal::CLod		&lodDest= finalMRM.Lods[lodId];

		// alloc final faces of this LOD.
		lodDest.Faces.resize(lodMesh.Faces.size());

		// reset the GeomMap, the one which indicate if we have already inserted a geomorph.
		_GeomMap.clear();

		// for all face corner.
		for(i=0; i<(sint)lodMesh.Faces.size();i++)
		{
			// The current face.
			CMRMFace	&face= lodMesh.Faces[i];
			// The dest face.
			CMRMMeshFinal::CFace		&faceDest= lodDest.Faces[i];
			// fill good material.
			faceDest.MaterialId= face.MaterialId;

			// for 3 corners.
			for(j=0;j<3;j++)
			{
				CMRMCorner	&corner= face.Corner[j];

				// if not same wedge Ids, this is a geomorphed wedge.
				if(corner.WedgeStartId != corner.WedgeEndId)
				{
					// geomorph, so point to geomorphed wedge.
					faceDest.WedgeId[j]= corner.WedgeGeomId;

					// Build the geomorph, add it to the list (if not yet inserted).
					CMRMWedgeGeom	geom;
					geom.Start= corner.WedgeStartId;
					geom.End=	corner.WedgeEndId;
					// if don't find this geom in the set, then it is a new one.
					TGeomMap::const_iterator	it= _GeomMap.find(geom);
					if(it == _GeomMap.end())
					{
						// mark it as inserted.
						_GeomMap.insert( make_pair(geom, corner.WedgeGeomId) );
						// and we must insert this geom in the array.
						nlassert( corner.WedgeGeomId==(sint)lodDest.Geomorphs.size() );
						lodDest.Geomorphs.push_back(geom);
					}
				}
				else
				{
					// no geomorph, so just point to good wedge.
					faceDest.WedgeId[j]= corner.WedgeStartId;
				}
			}
		}
	}


	// process all wedges, and compute NSkinMatUsed, skipping geomorphs.
	// ===============
	// NB: this works because weights are sorted from biggest to lowest.
	if(_Skinned)
	{
		for(i=finalMRM.NGeomSpace; i<(sint)finalMRM.Wedges.size();i++)
		{
			CMRMMeshFinal::CWedge	&wedge= finalMRM.Wedges[i];
			for(j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
			{
				if(wedge.VertexSkin.Weights[j]==0)
					break;
			}
			nlassert(j>0);
			wedge.NSkinMatUsed= j;
		}
	}

	// Blend Shape Stuff
	finalMRM.MRMBlendShapesFinals.resize (lodMeshs[0].BlendShapes.size());
	for (lodId = 0; lodId < nLods; ++lodId)
	{
		CMRMMeshGeom &lodMesh= lodMeshs[lodId];
		CMRMMeshGeom &lodMeshPrec= lodMeshs[lodId==0?0:lodId-1];

		// for all face corner.
		for (i = 0; i < (sint)lodMesh.Faces.size(); ++i)
		{
			// The current face.
			CMRMFace &face = lodMesh.Faces[i];
			// the current face, but which points to the prec LOD vertices/attributes.
			CMRMFace &faceCoarser = lodMesh.CoarserFaces[i];
			// for 3 corners.
			for (j = 0; j < 3; ++j)
			{
				CMRMCorner &corner = face.Corner[j];
				CMRMCorner &cornerCoarser = faceCoarser.Corner[j];

				sint startDestIndex = corner.WedgeStartId;

				for (sint k = 0; k < (sint)finalMRM.MRMBlendShapesFinals.size(); ++k)
				{
					CMRMMeshFinal::CMRMBlendShapeFinal &rBSFinal = finalMRM.MRMBlendShapesFinals[k];

					rBSFinal.Wedges.resize (finalMRM.Wedges.size());
					// Fill WedgeStart used by this corner.
					rBSFinal.Wedges[startDestIndex].Vertex = lodMesh.BlendShapes[k].Vertices[corner.Vertex];
					for (attId = 0; attId < NumAttributes; ++attId)
					{
						rBSFinal.Wedges[startDestIndex].Attributes[attId] = lodMesh.BlendShapes[k].Attributes[attId][corner.Attributes[attId]];
					}

					// If geomorph, must fill the end too
					if(lodId>0 && corner.WedgeStartId != corner.WedgeEndId)
					{
						sint endDestIndex = corner.WedgeEndId;

						rBSFinal.Wedges[endDestIndex].Vertex = lodMeshPrec.BlendShapes[k].Vertices[cornerCoarser.Vertex];
						for (attId = 0; attId < NumAttributes; ++attId)
						{
							rBSFinal.Wedges[endDestIndex].Attributes[attId] = lodMeshPrec.BlendShapes[k].Attributes[attId][cornerCoarser.Attributes[attId]];
						}
					}
				}

			}
		}
	}
}



// ***************************************************************************
// ***************************************************************************
// Interface to MeshBuild Part.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
sint			CMRMBuilder::findInsertAttributeInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const CVectorH &att)
{
	// find this attribute in the map.
	CAttributeKey	key;
	key.VertexId= vertexId;
	key.Attribute= att;
	TAttributeMap::iterator		it= _AttributeMap[attId].find(key);

	// if attribute not found in the map, then insert a new one.
	if(it==_AttributeMap[attId].end())
	{
		sint	idx= (sint)baseMesh.Attributes[attId].size();
		// insert into the array.
		baseMesh.Attributes[attId].push_back(att);
		// insert into the map.
		_AttributeMap[attId].insert(make_pair(key, idx));
		return idx;
	}
	else
	{
		// return the one found.
		return it->second;
	}
}


// ***************************************************************************
sint			CMRMBuilder::findInsertNormalInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const CVector &normal)
{
	CVectorH	att;
	att= normal;
	att.w= 0;
	return findInsertAttributeInBaseMesh(baseMesh, attId, vertexId, att);
}


// ***************************************************************************
sint			CMRMBuilder::findInsertColorInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, CRGBA col)
{
	CVectorH	att;
	att.x= col.R;
	att.y= col.G;
	att.z= col.B;
	att.w= col.A;
	return findInsertAttributeInBaseMesh(baseMesh, attId, vertexId, att);
}


// ***************************************************************************
sint			CMRMBuilder::findInsertUvwInBaseMesh(CMRMMesh &baseMesh, sint attId, sint vertexId, const NLMISC::CUVW &uvw)
{
	CVectorH	att;
	att.x= uvw.U;
	att.y= uvw.V;
	att.z= uvw.W;
	att.w= 0;
	return findInsertAttributeInBaseMesh(baseMesh, attId, vertexId, att);
}


// ***************************************************************************
CRGBA			CMRMBuilder::attToColor(const CVectorH &att) const
{
	CRGBA	ret;
	float	tmp;
	tmp= att.x; clamp(tmp, 0, 255);
	ret.R= (uint8)(uint)tmp;
	tmp= att.y; clamp(tmp, 0, 255);
	ret.G= (uint8)(uint)tmp;
	tmp= att.z; clamp(tmp, 0, 255);
	ret.B= (uint8)(uint)tmp;
	tmp= att.w; clamp(tmp, 0, 255);
	ret.A= (uint8)(uint)tmp;

	return ret;
}


// ***************************************************************************
NLMISC::CUVW			CMRMBuilder::attToUvw(const CVectorH &att) const
{
	return CUVW(att.x, att.y, att.z);
}


// ***************************************************************************
uint32			CMRMBuilder::buildMrmBaseMesh(const CMesh::CMeshBuild &mbuild, CMRMMesh &baseMesh)
{
	sint		i,j,k;
	sint		nFaces;
	sint		attId;
	// build the supported VertexFormat.
	uint32		retVbFlags= CVertexBuffer::PositionFlag;


	// reset the baseMesh.
	baseMesh= CMRMMesh();
	// reset Tmp.
	for(attId=0; attId<NL3D_MRM_MAX_ATTRIB;attId++)
		_AttributeMap[attId].clear();


	// Compute number of attributes used by the MeshBuild.
	// ========================
	// Compute too
	if(mbuild.VertexFlags & CVertexBuffer::NormalFlag)
	{
		baseMesh.NumAttributes++;
		retVbFlags|= CVertexBuffer::NormalFlag;
	}
	if(mbuild.VertexFlags & CVertexBuffer::PrimaryColorFlag)
	{
		baseMesh.NumAttributes++;
		retVbFlags|= CVertexBuffer::PrimaryColorFlag;
	}
	if(mbuild.VertexFlags & CVertexBuffer::SecondaryColorFlag)
	{
		baseMesh.NumAttributes++;
		retVbFlags|= CVertexBuffer::SecondaryColorFlag;
	}
	for(k=0; k<CVertexBuffer::MaxStage;k++)
	{
		uint flag=CVertexBuffer::TexCoord0Flag<<k;
		if(mbuild.VertexFlags & flag)
		{
			baseMesh.NumAttributes++;
			retVbFlags|=flag;
		}
	}
	nlassert(baseMesh.NumAttributes<=NL3D_MRM_MAX_ATTRIB);


	// Fill basics: Vertices and Faces materials / index to vertices.
	// ========================
	// Just copy vertices.
	baseMesh.Vertices= mbuild.Vertices;
	// Just copy SkinWeights.
	if(_Skinned)
		baseMesh.SkinWeights= mbuild.SkinWeights;
	// Just copy InterfaceLinks
	if(_HasMeshInterfaces)
		baseMesh.InterfaceLinks= mbuild.InterfaceLinks;
	// Resize faces.
	nFaces= (sint)mbuild.Faces.size();
	baseMesh.Faces.resize(nFaces);
	for(i=0; i<nFaces; i++)
	{
		// copy material Id.
		baseMesh.Faces[i].MaterialId= mbuild.Faces[i].MaterialId;
		// Copy Vertex index.
		for(j=0; j<3; j++)
		{
			baseMesh.Faces[i].Corner[j].Vertex= mbuild.Faces[i].Corner[j].Vertex;
		}
	}

	// Resolve attributes discontinuities and Fill attributes of the baseMesh.
	// ========================
	// For all corners.
	for(i=0; i<nFaces; i++)
	{
		for(j=0; j<3; j++)
		{
			const CMesh::CCorner	&srcCorner= mbuild.Faces[i].Corner[j];
			CMRMCorner				&destCorner= baseMesh.Faces[i].Corner[j];
			attId= 0;

			// For all activated attributes in mbuild, find/insert the attribute in the baseMesh.
			// NB: 2 attributes are said to be different if they have not the same value OR if they don't lie
			// on the same vertex. This is very important for MRM computing.
			if(mbuild.VertexFlags & CVertexBuffer::NormalFlag)
			{
				destCorner.Attributes[attId]= findInsertNormalInBaseMesh(baseMesh, attId, destCorner.Vertex, srcCorner.Normal);
				attId++;
			}
			if(mbuild.VertexFlags & CVertexBuffer::PrimaryColorFlag)
			{
				destCorner.Attributes[attId]= findInsertColorInBaseMesh(baseMesh, attId, destCorner.Vertex, srcCorner.Color);
				attId++;
			}
			if(mbuild.VertexFlags & CVertexBuffer::SecondaryColorFlag)
			{
				destCorner.Attributes[attId]= findInsertColorInBaseMesh(baseMesh, attId, destCorner.Vertex, srcCorner.Specular);
				attId++;
			}
			for(k=0; k<CVertexBuffer::MaxStage;k++)
			{
				if(mbuild.VertexFlags & (CVertexBuffer::TexCoord0Flag<<k))
				{
					destCorner.Attributes[attId]= findInsertUvwInBaseMesh(baseMesh, attId, destCorner.Vertex, srcCorner.Uvws[k]);
					attId++;
				}
			}
		}
	}



	// End. clear Tmp infos.
	// ========================
	// reset Tmp.
	for(attId=0; attId<NL3D_MRM_MAX_ATTRIB;attId++)
		_AttributeMap[attId].clear();

	return	retVbFlags;
}



// ***************************************************************************
CMesh::CSkinWeight	CMRMBuilder::normalizeSkinWeight(const CMesh::CSkinWeight &sw) const
{
	uint	nbMats= 0;
	static vector<CTmpVertexWeight>		sws;
	sws.reserve(NL3D_MESH_SKINNING_MAX_MATRIX);
	sws.clear();

	// For all weights of sw1.
	uint i;
	for(i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
	{
		CTmpVertexWeight	vw;
		vw.MatrixId= sw.MatrixId[i];
		vw.Weight= sw.Weights[i];
		// if this weight is not null.
		if(vw.Weight>0)
		{
			// add it to the list.
			sws.push_back(vw);
			nbMats++;
		}
	}

	// sort by Weight decreasing order.
	sort(sws.begin(), sws.end());


	// Then output the result to the skinWeight, normalizing.
	float	sumWeight=0;
	for(i= 0; i<nbMats; i++)
	{
		sumWeight+= sws[i].Weight;
	}

	CMesh::CSkinWeight	ret;
	// Fill only needed matrix (other are rested in CMesh::CSkinWeight ctor).
	for(i= 0; i<nbMats; i++)
	{
		ret.MatrixId[i]= sws[i].MatrixId;
		ret.Weights[i]= sws[i].Weight / sumWeight;
	}

	return ret;
}


// ***************************************************************************
void			CMRMBuilder::normalizeBaseMeshSkin(CMRMMesh &baseMesh) const
{
	nlassert(_Skinned);

	for(uint i=0; i<baseMesh.SkinWeights.size(); i++)
	{
		baseMesh.SkinWeights[i]= normalizeSkinWeight(baseMesh.SkinWeights[i]);
	}
}



// ***************************************************************************
void			CMRMBuilder::buildMeshBuildMrm(const CMRMMeshFinal &finalMRM, CMeshMRMGeom::CMeshBuildMRM &mbuild, uint32 vbFlags, uint32 nbMats, const CMesh::CMeshBuild &mb)
{
	sint	i,j,k;
	sint	attId;

	// reset the mbuild.
	mbuild= CMeshMRMGeom::CMeshBuildMRM();
	// Setup VB.

	bool useFormatExt = false;
	// Check whether there are texture coordinates with more than 2 compnents, which force us to use an extended vertex format
	for (k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		if (
			(vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			&& mb.NumCoords[k] != 2)
		{
			useFormatExt = true;
			break;
		}
	}

	uint numTexCoordUsed = 0;


	for (k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		if (vbFlags & (CVertexBuffer::TexCoord0Flag << k))
		{
			numTexCoordUsed = k;
		}
	}

	if (!useFormatExt)
	{
		// setup standard format
		mbuild.VBuffer.setVertexFormat(vbFlags);
	}
	else // setup extended format
	{
		mbuild.VBuffer.clearValueEx();
		if (vbFlags & CVertexBuffer::PositionFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Position, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::NormalFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Normal, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::PrimaryColorFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::PrimaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::SecondaryColorFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::SecondaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::WeightFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Weight, CVertexBuffer::Float4);
		if (vbFlags & CVertexBuffer::PaletteSkinFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::PaletteSkin, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::FogFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Fog, CVertexBuffer::Float1);

		for (k = 0; k < CVertexBuffer::MaxStage; ++k)
		{
			if (vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			{
				switch(mb.NumCoords[k])
				{
					case 2:
						mbuild.VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float2);
					break;
					case 3:
						mbuild.VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float3);
					break;
					default:
						nlassert(0);
					break;
				}
			}
		}
		mbuild.VBuffer.initEx();
	}

	// Copy the UVRouting
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		mbuild.VBuffer.setUVRouting (i, mb.UVRouting[i]);
	}

	// Setup the VertexBuffer.
	// ========================
	// resize the VB.
	mbuild.VBuffer.setNumVertices((uint32)finalMRM.Wedges.size());
	// Setup SkinWeights.
	if(_Skinned)
		mbuild.SkinWeights.resize(finalMRM.Wedges.size());

	CVertexBufferReadWrite vba;
	mbuild.VBuffer.lock (vba);

	// fill the VB.
	for(i=0; i<(sint)finalMRM.Wedges.size(); i++)
	{
		const CMRMMeshFinal::CWedge	&wedge= finalMRM.Wedges[i];

		// setup Vertex.
		vba.setVertexCoord(i, wedge.Vertex);

		// seutp attributes.
		attId= 0;

		// For all activated attributes in mbuild, retriev the attribute from the finalMRM.
		if(vbFlags & CVertexBuffer::NormalFlag)
		{
			vba.setNormalCoord(i, wedge.Attributes[attId] );
			attId++;
		}
		if(vbFlags & CVertexBuffer::PrimaryColorFlag)
		{
			vba.setColor(i, attToColor(wedge.Attributes[attId]) );
			attId++;
		}
		if(vbFlags & CVertexBuffer::SecondaryColorFlag)
		{
			vba.setSpecular(i, attToColor(wedge.Attributes[attId]) );
			attId++;
		}
		for(k=0; k<CVertexBuffer::MaxStage;k++)
		{
			if(vbFlags & (CVertexBuffer::TexCoord0Flag<<k))
			{
				switch(mb.NumCoords[k])
				{
					case 2:
						vba.setTexCoord(i, k, (CUV) attToUvw(wedge.Attributes[attId]) );
					break;
					case 3:
					{
						CUVW uvw = attToUvw(wedge.Attributes[attId]);
						vba.setValueFloat3Ex((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), i, uvw.U, uvw.V, uvw.W);
					}
					break;
					default:
						nlassert(0);
					break;
				}
				attId++;
			}
		}

		// Setup SkinWeights.
		if(_Skinned)
		{
			mbuild.SkinWeights[i]= wedge.VertexSkin;
		}
	}


	// Build Lods.
	// ========================
	// resize
	mbuild.Lods.resize(finalMRM.Lods.size());
	// fill.
	for(i=0; i<(sint)finalMRM.Lods.size(); i++)
	{
		const CMRMMeshFinal::CLod	&srcLod= finalMRM.Lods[i];
		CMeshMRMGeom::CLod			&destLod= mbuild.Lods[i];

		// Basic.
		//---------

		// Copy NWedges infos.
		destLod.NWedges= srcLod.NWedges;
		// Copy Geomorphs infos.
		destLod.Geomorphs= srcLod.Geomorphs;


		// Reorder faces by rdrpass.
		//---------

		// First count the number of faces used by this LOD for each material
		vector<sint>	matCount;
		// resize, and reset to 0.
		matCount.clear();
		matCount.resize(nbMats, 0);
		// For each face of this Lods, incr the mat face counter.
		for(j= 0; j<(sint)srcLod.Faces.size(); j++)
		{
			sint	matId= srcLod.Faces[j].MaterialId;
			nlassert(matId>=0);
			nlassert(matId<(sint)nbMats);
			// increment the refcount of this material by this LOD.
			matCount[matId]++;
		}

		// Then for each material not empty, create a rdrPass, and ref it for this material.
		vector<sint>	rdrPassIndex;	// material to rdrPass map.
		rdrPassIndex.resize(nbMats);
		for(j=0; j<(sint)nbMats; j++)
		{
			if(matCount[j]==0)
				rdrPassIndex[j]= -1;
			else
			{
				// map material to rdrPass.
				sint	idRdrPass= (sint)destLod.RdrPass.size();
				rdrPassIndex[j]= idRdrPass;
				// create a rdrPass.
				destLod.RdrPass.push_back(CMeshMRMGeom::CRdrPass());
				// assign the good materialId to this rdrPass.
				destLod.RdrPass[idRdrPass].MaterialId= j;
				// reserve the array of faces of this rdrPass.
				destLod.RdrPass[idRdrPass].PBlock.reserve(3*matCount[j]);
			}
		}

		// Then for each face, add it to the good rdrPass of this Lod.
		for(j= 0; j<(sint)srcLod.Faces.size(); j++)
		{
			sint	matId= srcLod.Faces[j].MaterialId;
			sint	idRdrPass= rdrPassIndex[matId];
			// add this face to the good rdrPass.
			sint	w0= srcLod.Faces[j].WedgeId[0];
			sint	w1= srcLod.Faces[j].WedgeId[1];
			sint	w2= srcLod.Faces[j].WedgeId[2];
			CIndexBuffer &ib = destLod.RdrPass[idRdrPass].PBlock;
			uint index = ib.getNumIndexes();
			ib.setNumIndexes(index+3);
			CIndexBufferReadWrite ibaWrite;
			ib.lock (ibaWrite);
			ibaWrite.setTri(index, w0, w1, w2);
		}


		// Build skin info for this Lod.
		//---------
		for(j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
		{
			destLod.InfluencedVertices[j].clear();
		}
		destLod.MatrixInfluences.clear();
		if(_Skinned)
		{
			// This is the set which tell what wedge has already been inserted.
			set<uint>	wedgeInfSet;

			// First, build the list of vertices influenced by this Lod.
			for(j= 0; j<(sint)srcLod.Faces.size(); j++)
			{
				for(k=0; k<3; k++)
				{
					sint	wedgeId= srcLod.Faces[j].WedgeId[k];
					// If it is a geomorph
					if(wedgeId<finalMRM.NGeomSpace)
					{
						// add the start and end to the list (if not here). NB: wedgeId is both the id
						// of the dest wedge, and the id of the geomorph.
						sint	wedgeStartId= destLod.Geomorphs[wedgeId].Start;
						sint	wedgeEndId= destLod.Geomorphs[wedgeId].End;
						uint	nMatUsedStart= finalMRM.Wedges[wedgeStartId].NSkinMatUsed;
						uint	nMatUsedEnd= finalMRM.Wedges[wedgeEndId].NSkinMatUsed;

						// if insertion in the set work, add to the good array.
						if( wedgeInfSet.insert(wedgeStartId).second )
							destLod.InfluencedVertices[nMatUsedStart-1].push_back(wedgeStartId);
						if( wedgeInfSet.insert(wedgeEndId).second )
							destLod.InfluencedVertices[nMatUsedEnd-1].push_back(wedgeEndId);
					}
					else
					{
						uint	nMatUsed= finalMRM.Wedges[wedgeId].NSkinMatUsed;

						// just add this wedge to the list (if not here).
						// if insertion in the set work, add to the array.
						if( wedgeInfSet.insert(wedgeId).second )
							destLod.InfluencedVertices[nMatUsed-1].push_back(wedgeId);
					}
				}
			}

			// Optimisation: for better cache, sort the destLod.InfluencedVertices in increasing order.
			for(j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
			{
				sort(destLod.InfluencedVertices[j].begin(), destLod.InfluencedVertices[j].end());
			}


			// Then Build the MatrixInfluences array, for all thoses Influenced Vertices only.
			// This is the map MatrixId -> MatrixInfId.
			map<uint, uint>		matrixInfMap;

			// For all influenced vertices, flags matrix they use.
			uint	iSkinMat;
			for(iSkinMat= 0; iSkinMat<NL3D_MESH_SKINNING_MAX_MATRIX; iSkinMat++)
			{
				for(j= 0; j<(sint)destLod.InfluencedVertices[iSkinMat].size(); j++)
				{
					uint	wedgeId= destLod.InfluencedVertices[iSkinMat][j];

					// take the original wedge.
					const CMRMMeshFinal::CWedge	&wedge= finalMRM.Wedges[wedgeId];
					// For all matrix with not null influence...
					for(k= 0; k<NL3D_MESH_SKINNING_MAX_MATRIX; k++)
					{
						float	matWeight= wedge.VertexSkin.Weights[k];

						// This check the validity of skin weights sort. If false, problem before in the algo.
						if((uint)k<iSkinMat+1)
						{
							nlassert( matWeight>0 );
						}
						else
						{
							nlassert( matWeight==0 );
						}
						// if not null influence.
						if(matWeight>0)
						{
							uint	matId= wedge.VertexSkin.MatrixId[k];

							// search/insert the matrixInfId.
							map<uint, uint>::iterator	it= matrixInfMap.find(matId);
							if( it==matrixInfMap.end() )
							{
								uint matInfId= (uint)destLod.MatrixInfluences.size();
								matrixInfMap.insert( make_pair(matId, matInfId) );
								// create the new MatrixInfluence.
								destLod.MatrixInfluences.push_back(matId);
							}
						}
					}
				}
			}

		}

	}

	// Indicate Skinning.
	mbuild.Skinned= _Skinned;



	bool useTgSpace = mb.MeshVertexProgram != NULL ? mb.MeshVertexProgram->needTangentSpace() : false;

	// Construct Blend Shapes
	//// mbuild <- finalMRM
	mbuild.BlendShapes.resize (finalMRM.MRMBlendShapesFinals.size());
	for (k = 0; k < (sint)mbuild.BlendShapes.size(); ++k)
	{
		CBlendShape &rBS = mbuild.BlendShapes[k];
		sint32 nNbVertVB = (sint32)finalMRM.Wedges.size();
		bool bIsDeltaPos = false;
		rBS.deltaPos.resize (nNbVertVB, CVector(0.0f,0.0f,0.0f));
		bool bIsDeltaNorm = false;
		rBS.deltaNorm.resize (nNbVertVB, CVector(0.0f,0.0f,0.0f));
		bool bIsDeltaUV = false;
		rBS.deltaUV.resize (nNbVertVB, CUV(0.0f,0.0f));
		bool bIsDeltaCol = false;
		rBS.deltaCol.resize (nNbVertVB, CRGBAF(0.0f,0.0f,0.0f,0.0f));
		bool bIsDeltaTgSpace = false;
		if (useTgSpace)
		{
			rBS.deltaTgSpace.resize(nNbVertVB, CVector::Null);
		}

		rBS.VertRefs.resize (nNbVertVB, 0xffffffff);

		for (i = 0; i < nNbVertVB; i++)
		{
			const CMRMMeshFinal::CWedge	&rWedgeRef = finalMRM.Wedges[i];
			const CMRMMeshFinal::CWedge	&rWedgeTar = finalMRM.MRMBlendShapesFinals[k].Wedges[i];

			CVector delta = rWedgeTar.Vertex - rWedgeRef.Vertex;
			CVectorH attr;

			if (delta.norm() > 0.001f)
			{
				rBS.deltaPos[i] = delta;
				rBS.VertRefs[i] = i;
				bIsDeltaPos = true;
			}

			attId = 0;
			if (vbFlags & CVertexBuffer::NormalFlag)
			{
				attr = rWedgeRef.Attributes[attId];
				CVector NormRef = CVector(attr.x, attr.y, attr.z);
				attr = rWedgeTar.Attributes[attId];
				CVector NormTar = CVector(attr.x, attr.y, attr.z);
				delta = NormTar - NormRef;
				if (delta.norm() > 0.001f)
				{
					rBS.deltaNorm[i] = delta;
					rBS.VertRefs[i] = i;
					bIsDeltaNorm = true;
				}
				attId++;
			}

			if (vbFlags & CVertexBuffer::PrimaryColorFlag)
			{
				attr = rWedgeRef.Attributes[attId];
				CRGBAF RGBARef = CRGBAF(attr.x/255.0f, attr.y/255.0f, attr.z/255.0f, attr.w/255.0f);
				attr = rWedgeTar.Attributes[attId];
				CRGBAF RGBATar = CRGBAF(attr.x/255.0f, attr.y/255.0f, attr.z/255.0f, attr.w/255.0f);
				CRGBAF deltaRGBA = RGBATar - RGBARef;
				if ((deltaRGBA.R*deltaRGBA.R + deltaRGBA.G*deltaRGBA.G +
					deltaRGBA.B*deltaRGBA.B + deltaRGBA.A*deltaRGBA.A) > 0.0001f)
				{
					rBS.deltaCol[i] = deltaRGBA;
					rBS.VertRefs[i] = i;
					bIsDeltaCol = true;
				}
				attId++;
			}

			if (vbFlags & CVertexBuffer::SecondaryColorFlag)
			{	// Nothing to do !
				attId++;
			}

			// Do that only for the UV0
			if (vbFlags & CVertexBuffer::TexCoord0Flag)
			{
				attr = rWedgeRef.Attributes[attId];
				CUV UVRef = CUV(attr.x, attr.y);
				attr = rWedgeTar.Attributes[attId];
				CUV UVTar = CUV(attr.x, attr.y);
				CUV deltaUV = UVTar - UVRef;
				if ((deltaUV.U*deltaUV.U + deltaUV.V*deltaUV.V) > 0.0001f)
				{
					rBS.deltaUV[i] = deltaUV;
					rBS.VertRefs[i] = i;
					bIsDeltaUV = true;
				}
				attId++;
			}

			if (useTgSpace)
			{
				attr = rWedgeRef.Attributes[attId];
				CVector TgSpaceRef = CVector(attr.x, attr.y, attr.z);
				attr = rWedgeTar.Attributes[attId];
				CVector TgSpaceTar = CVector(attr.x, attr.y, attr.z);
				delta = TgSpaceTar - TgSpaceRef;
				if (delta.norm() > 0.001f)
				{
					rBS.deltaTgSpace[i] = delta;
					rBS.VertRefs[i] = i;
					bIsDeltaTgSpace = true;
				}
				attId++;
			}

		} // End of all vertices added in blend shape

		// Delete unused items and calculate the number of vertex used (blended)

		sint32 nNbVertUsed = nNbVertVB;
		sint32 nDstPos = 0;
		for (j = 0; j < nNbVertVB; ++j)
		{
			if (rBS.VertRefs[j] == 0xffffffff) // Is vertex UNused
			{
				--nNbVertUsed;
			}
			else // Vertex used
			{
				if (nDstPos != j)
				{
					rBS.VertRefs[nDstPos]	= rBS.VertRefs[j];
					rBS.deltaPos[nDstPos]	= rBS.deltaPos[j];
					rBS.deltaNorm[nDstPos]	= rBS.deltaNorm[j];
					rBS.deltaUV[nDstPos]	= rBS.deltaUV[j];
					rBS.deltaCol[nDstPos]	= rBS.deltaCol[j];
					if (useTgSpace)
					{
						rBS.deltaTgSpace[nDstPos]	= rBS.deltaTgSpace[j];
					}
				}
				++nDstPos;
			}
		}

		if (bIsDeltaPos)
			rBS.deltaPos.resize (nNbVertUsed);
		else
			rBS.deltaPos.resize (0);

		if (bIsDeltaNorm)
			rBS.deltaNorm.resize (nNbVertUsed);
		else
			rBS.deltaNorm.resize (0);

		if (bIsDeltaUV)
			rBS.deltaUV.resize (nNbVertUsed);
		else
			rBS.deltaUV.resize (0);

		if (bIsDeltaCol)
			rBS.deltaCol.resize (nNbVertUsed);
		else
			rBS.deltaCol.resize (0);

		if (bIsDeltaTgSpace)
			rBS.deltaTgSpace.resize (nNbVertUsed);
		else
			rBS.deltaTgSpace.resize (0);


		rBS.VertRefs.resize (nNbVertUsed);

	}
}

// ***************************************************************************
void			CMRMBuilder::buildMeshBuildMrm(const CMRMMeshFinal &finalMRM, CMeshMRMSkinnedGeom::CMeshBuildMRM &mbuild, uint32 vbFlags, uint32 nbMats, const CMesh::CMeshBuild &mb)
{
	sint	i,j,k;
	sint	attId;

	// reset the mbuild.
	mbuild= CMeshMRMSkinnedGeom::CMeshBuildMRM();
	// Setup VB.

	bool useFormatExt = false;
	// Check whether there are texture coordinates with more than 2 compnents, which force us to use an extended vertex format
	for (k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		if (
			(vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			&& mb.NumCoords[k] != 2)
		{
			useFormatExt = true;
			break;
		}
	}

	uint numTexCoordUsed = 0;


	for (k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		if (vbFlags & (CVertexBuffer::TexCoord0Flag << k))
		{
			numTexCoordUsed = k;
		}
	}

	if (!useFormatExt)
	{
		// setup standard format
		mbuild.VBuffer.setVertexFormat(vbFlags);
	}
	else // setup extended format
	{
		mbuild.VBuffer.clearValueEx();
		if (vbFlags & CVertexBuffer::PositionFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Position, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::NormalFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Normal, CVertexBuffer::Float3);
		if (vbFlags & CVertexBuffer::PrimaryColorFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::PrimaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::SecondaryColorFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::SecondaryColor, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::WeightFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Weight, CVertexBuffer::Float4);
		if (vbFlags & CVertexBuffer::PaletteSkinFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::PaletteSkin, CVertexBuffer::UChar4);
		if (vbFlags & CVertexBuffer::FogFlag) mbuild.VBuffer.addValueEx(CVertexBuffer::Fog, CVertexBuffer::Float1);

		for (k = 0; k < CVertexBuffer::MaxStage; ++k)
		{
			if (vbFlags & (CVertexBuffer::TexCoord0Flag << k))
			{
				switch(mb.NumCoords[k])
				{
					case 2:
						mbuild.VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float2);
					break;
					case 3:
						mbuild.VBuffer.addValueEx((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), CVertexBuffer::Float3);
					break;
					default:
						nlassert(0);
					break;
				}
			}
		}
		mbuild.VBuffer.initEx();
	}

	// Copy the UVRouting
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		mbuild.VBuffer.setUVRouting (i, mb.UVRouting[i]);
	}

	// Setup the VertexBuffer.
	// ========================
	// resize the VB.
	mbuild.VBuffer.setNumVertices((uint32)finalMRM.Wedges.size());

	CVertexBufferReadWrite vba;
	mbuild.VBuffer.lock (vba);

	// Setup SkinWeights.
	if(_Skinned)
		mbuild.SkinWeights.resize(finalMRM.Wedges.size());

	// fill the VB.
	for(i=0; i<(sint)finalMRM.Wedges.size(); i++)
	{
		const CMRMMeshFinal::CWedge	&wedge= finalMRM.Wedges[i];

		// setup Vertex.
		vba.setVertexCoord(i, wedge.Vertex);

		// seutp attributes.
		attId= 0;

		// For all activated attributes in mbuild, retrieve the attribute from the finalMRM.
		if(vbFlags & CVertexBuffer::NormalFlag)
		{
			vba.setNormalCoord(i, wedge.Attributes[attId] );
			attId++;
		}
		if(vbFlags & CVertexBuffer::PrimaryColorFlag)
		{
			vba.setColor(i, attToColor(wedge.Attributes[attId]) );
			attId++;
		}
		if(vbFlags & CVertexBuffer::SecondaryColorFlag)
		{
			vba.setSpecular(i, attToColor(wedge.Attributes[attId]) );
			attId++;
		}
		for(k=0; k<CVertexBuffer::MaxStage;k++)
		{
			if(vbFlags & (CVertexBuffer::TexCoord0Flag<<k))
			{
				switch(mb.NumCoords[k])
				{
					case 2:
						vba.setTexCoord(i, k, (CUV) attToUvw(wedge.Attributes[attId]) );
					break;
					case 3:
					{
						CUVW uvw = attToUvw(wedge.Attributes[attId]);
						vba.setValueFloat3Ex((CVertexBuffer::TValue) (CVertexBuffer::TexCoord0 + k), i, uvw.U, uvw.V, uvw.W);
					}
					break;
					default:
						nlassert(0);
					break;
				}
				attId++;
			}
		}

		// Setup SkinWeights.
		if(_Skinned)
		{
			mbuild.SkinWeights[i]= wedge.VertexSkin;
		}
	}


	// Build Lods.
	// ========================
	// resize
	mbuild.Lods.resize(finalMRM.Lods.size());
	// fill.
	for(i=0; i<(sint)finalMRM.Lods.size(); i++)
	{
		const CMRMMeshFinal::CLod	&srcLod= finalMRM.Lods[i];
		CMeshMRMSkinnedGeom::CLod			&destLod= mbuild.Lods[i];

		// Basic.
		//---------

		// Copy NWedges infos.
		destLod.NWedges= srcLod.NWedges;
		// Copy Geomorphs infos.
		destLod.Geomorphs= srcLod.Geomorphs;


		// Reorder faces by rdrpass.
		//---------

		// First count the number of faces used by this LOD for each material
		vector<sint>	matCount;
		// resize, and reset to 0.
		matCount.clear();
		matCount.resize(nbMats, 0);
		// For each face of this Lods, incr the mat face counter.
		for(j= 0; j<(sint)srcLod.Faces.size(); j++)
		{
			sint	matId= srcLod.Faces[j].MaterialId;
			nlassert(matId>=0);
			nlassert(matId<(sint)nbMats);
			// increment the refcount of this material by this LOD.
			matCount[matId]++;
		}

		// Then for each material not empty, create a rdrPass, and ref it for this material.
		vector<sint>	rdrPassIndex;	// material to rdrPass map.
		rdrPassIndex.resize(nbMats);
		for(j=0; j<(sint)nbMats; j++)
		{
			if(matCount[j]==0)
				rdrPassIndex[j]= -1;
			else
			{
				// map material to rdrPass.
				sint	idRdrPass= (sint)destLod.RdrPass.size();
				rdrPassIndex[j]= idRdrPass;
				// create a rdrPass.
				destLod.RdrPass.push_back(CMeshMRMSkinnedGeom::CRdrPass());
				// assign the good materialId to this rdrPass.
				destLod.RdrPass[idRdrPass].MaterialId= j;
				// reserve the array of faces of this rdrPass.
				destLod.RdrPass[idRdrPass].PBlock.reserve(3*matCount[j]);
			}
		}

		// Then for each face, add it to the good rdrPass of this Lod.
		for(j= 0; j<(sint)srcLod.Faces.size(); j++)
		{
			sint	matId= srcLod.Faces[j].MaterialId;
			sint	idRdrPass= rdrPassIndex[matId];
			// add this face to the good rdrPass.
			sint	w0= srcLod.Faces[j].WedgeId[0];
			sint	w1= srcLod.Faces[j].WedgeId[1];
			sint	w2= srcLod.Faces[j].WedgeId[2];
			destLod.RdrPass[idRdrPass].PBlock.push_back (w0);
			destLod.RdrPass[idRdrPass].PBlock.push_back (w1);
			destLod.RdrPass[idRdrPass].PBlock.push_back (w2);
		}


		// Build skin info for this Lod.
		//---------
		for(j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
		{
			destLod.InfluencedVertices[j].clear();
		}
		destLod.MatrixInfluences.clear();
		if(_Skinned)
		{
			// This is the set which tell what wedge has already been inserted.
			set<uint>	wedgeInfSet;

			// First, build the list of vertices influenced by this Lod.
			for(j= 0; j<(sint)srcLod.Faces.size(); j++)
			{
				for(k=0; k<3; k++)
				{
					sint	wedgeId= srcLod.Faces[j].WedgeId[k];
					// If it is a geomorph
					if(wedgeId<finalMRM.NGeomSpace)
					{
						// add the start and end to the list (if not here). NB: wedgeId is both the id
						// of the dest wedge, and the id of the geomorph.
						sint	wedgeStartId= destLod.Geomorphs[wedgeId].Start;
						sint	wedgeEndId= destLod.Geomorphs[wedgeId].End;
						uint	nMatUsedStart= finalMRM.Wedges[wedgeStartId].NSkinMatUsed;
						uint	nMatUsedEnd= finalMRM.Wedges[wedgeEndId].NSkinMatUsed;

						// if insertion in the set work, add to the good array.
						if( wedgeInfSet.insert(wedgeStartId).second )
							destLod.InfluencedVertices[nMatUsedStart-1].push_back(wedgeStartId);
						if( wedgeInfSet.insert(wedgeEndId).second )
							destLod.InfluencedVertices[nMatUsedEnd-1].push_back(wedgeEndId);
					}
					else
					{
						uint	nMatUsed= finalMRM.Wedges[wedgeId].NSkinMatUsed;

						// just add this wedge to the list (if not here).
						// if insertion in the set work, add to the array.
						if( wedgeInfSet.insert(wedgeId).second )
							destLod.InfluencedVertices[nMatUsed-1].push_back(wedgeId);
					}
				}
			}

			// Optimisation: for better cache, sort the destLod.InfluencedVertices in increasing order.
			for(j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
			{
				sort(destLod.InfluencedVertices[j].begin(), destLod.InfluencedVertices[j].end());
			}


			// Then Build the MatrixInfluences array, for all thoses Influenced Vertices only.
			// This is the map MatrixId -> MatrixInfId.
			map<uint, uint>		matrixInfMap;

			// For all influenced vertices, flags matrix they use.
			uint	iSkinMat;
			for(iSkinMat= 0; iSkinMat<NL3D_MESH_SKINNING_MAX_MATRIX; iSkinMat++)
			{
				for(j= 0; j<(sint)destLod.InfluencedVertices[iSkinMat].size(); j++)
				{
					uint	wedgeId= destLod.InfluencedVertices[iSkinMat][j];

					// take the original wedge.
					const CMRMMeshFinal::CWedge	&wedge= finalMRM.Wedges[wedgeId];
					// For all matrix with not null influence...
					for(k= 0; k<NL3D_MESH_SKINNING_MAX_MATRIX; k++)
					{
						float	matWeight= wedge.VertexSkin.Weights[k];

						// This check the validity of skin weights sort. If false, problem before in the algo.
						if((uint)k<iSkinMat+1)
						{
							nlassert( matWeight>0 );
						}
						else
						{
							nlassert( matWeight==0 );
						}
						// if not null influence.
						if(matWeight>0)
						{
							uint	matId= wedge.VertexSkin.MatrixId[k];

							// search/insert the matrixInfId.
							map<uint, uint>::iterator	it= matrixInfMap.find(matId);
							if( it==matrixInfMap.end() )
							{
								uint matInfId= (uint)destLod.MatrixInfluences.size();
								matrixInfMap.insert( make_pair(matId, matInfId) );
								// create the new MatrixInfluence.
								destLod.MatrixInfluences.push_back(matId);
							}
						}
					}
				}
			}

		}

	}

	// Indicate Skinning.
	mbuild.Skinned= _Skinned;



	bool useTgSpace = mb.MeshVertexProgram != NULL ? mb.MeshVertexProgram->needTangentSpace() : false;

	// Construct Blend Shapes
	//// mbuild <- finalMRM
	mbuild.BlendShapes.resize (finalMRM.MRMBlendShapesFinals.size());
	for (k = 0; k < (sint)mbuild.BlendShapes.size(); ++k)
	{
		CBlendShape &rBS = mbuild.BlendShapes[k];
		sint32 nNbVertVB = (sint32)finalMRM.Wedges.size();
		bool bIsDeltaPos = false;
		rBS.deltaPos.resize (nNbVertVB, CVector(0.0f,0.0f,0.0f));
		bool bIsDeltaNorm = false;
		rBS.deltaNorm.resize (nNbVertVB, CVector(0.0f,0.0f,0.0f));
		bool bIsDeltaUV = false;
		rBS.deltaUV.resize (nNbVertVB, CUV(0.0f,0.0f));
		bool bIsDeltaCol = false;
		rBS.deltaCol.resize (nNbVertVB, CRGBAF(0.0f,0.0f,0.0f,0.0f));
		bool bIsDeltaTgSpace = false;
		if (useTgSpace)
		{
			rBS.deltaTgSpace.resize(nNbVertVB, CVector::Null);
		}

		rBS.VertRefs.resize (nNbVertVB, 0xffffffff);

		for (i = 0; i < nNbVertVB; i++)
		{
			const CMRMMeshFinal::CWedge	&rWedgeRef = finalMRM.Wedges[i];
			const CMRMMeshFinal::CWedge	&rWedgeTar = finalMRM.MRMBlendShapesFinals[k].Wedges[i];

			CVector delta = rWedgeTar.Vertex - rWedgeRef.Vertex;
			CVectorH attr;

			if (delta.norm() > 0.001f)
			{
				rBS.deltaPos[i] = delta;
				rBS.VertRefs[i] = i;
				bIsDeltaPos = true;
			}

			attId = 0;
			if (vbFlags & CVertexBuffer::NormalFlag)
			{
				attr = rWedgeRef.Attributes[attId];
				CVector NormRef = CVector(attr.x, attr.y, attr.z);
				attr = rWedgeTar.Attributes[attId];
				CVector NormTar = CVector(attr.x, attr.y, attr.z);
				delta = NormTar - NormRef;
				if (delta.norm() > 0.001f)
				{
					rBS.deltaNorm[i] = delta;
					rBS.VertRefs[i] = i;
					bIsDeltaNorm = true;
				}
				attId++;
			}

			if (vbFlags & CVertexBuffer::PrimaryColorFlag)
			{
				attr = rWedgeRef.Attributes[attId];
				CRGBAF RGBARef = CRGBAF(attr.x/255.0f, attr.y/255.0f, attr.z/255.0f, attr.w/255.0f);
				attr = rWedgeTar.Attributes[attId];
				CRGBAF RGBATar = CRGBAF(attr.x/255.0f, attr.y/255.0f, attr.z/255.0f, attr.w/255.0f);
				CRGBAF deltaRGBA = RGBATar - RGBARef;
				if ((deltaRGBA.R*deltaRGBA.R + deltaRGBA.G*deltaRGBA.G +
					deltaRGBA.B*deltaRGBA.B + deltaRGBA.A*deltaRGBA.A) > 0.0001f)
				{
					rBS.deltaCol[i] = deltaRGBA;
					rBS.VertRefs[i] = i;
					bIsDeltaCol = true;
				}
				attId++;
			}

			if (vbFlags & CVertexBuffer::SecondaryColorFlag)
			{	// Nothing to do !
				attId++;
			}

			// Do that only for the UV0
			if (vbFlags & CVertexBuffer::TexCoord0Flag)
			{
				attr = rWedgeRef.Attributes[attId];
				CUV UVRef = CUV(attr.x, attr.y);
				attr = rWedgeTar.Attributes[attId];
				CUV UVTar = CUV(attr.x, attr.y);
				CUV deltaUV = UVTar - UVRef;
				if ((deltaUV.U*deltaUV.U + deltaUV.V*deltaUV.V) > 0.0001f)
				{
					rBS.deltaUV[i] = deltaUV;
					rBS.VertRefs[i] = i;
					bIsDeltaUV = true;
				}
				attId++;
			}

			if (useTgSpace)
			{
				attr = rWedgeRef.Attributes[attId];
				CVector TgSpaceRef = CVector(attr.x, attr.y, attr.z);
				attr = rWedgeTar.Attributes[attId];
				CVector TgSpaceTar = CVector(attr.x, attr.y, attr.z);
				delta = TgSpaceTar - TgSpaceRef;
				if (delta.norm() > 0.001f)
				{
					rBS.deltaTgSpace[i] = delta;
					rBS.VertRefs[i] = i;
					bIsDeltaTgSpace = true;
				}
				attId++;
			}

		} // End of all vertices added in blend shape

		// Delete unused items and calculate the number of vertex used (blended)

		sint32 nNbVertUsed = nNbVertVB;
		sint32 nDstPos = 0;
		for (j = 0; j < nNbVertVB; ++j)
		{
			if (rBS.VertRefs[j] == 0xffffffff) // Is vertex UNused
			{
				--nNbVertUsed;
			}
			else // Vertex used
			{
				if (nDstPos != j)
				{
					rBS.VertRefs[nDstPos]	= rBS.VertRefs[j];
					rBS.deltaPos[nDstPos]	= rBS.deltaPos[j];
					rBS.deltaNorm[nDstPos]	= rBS.deltaNorm[j];
					rBS.deltaUV[nDstPos]	= rBS.deltaUV[j];
					rBS.deltaCol[nDstPos]	= rBS.deltaCol[j];
					if (useTgSpace)
					{
						rBS.deltaTgSpace[nDstPos]	= rBS.deltaTgSpace[j];
					}
				}
				++nDstPos;
			}
		}

		if (bIsDeltaPos)
			rBS.deltaPos.resize (nNbVertUsed);
		else
			rBS.deltaPos.resize (0);

		if (bIsDeltaNorm)
			rBS.deltaNorm.resize (nNbVertUsed);
		else
			rBS.deltaNorm.resize (0);

		if (bIsDeltaUV)
			rBS.deltaUV.resize (nNbVertUsed);
		else
			rBS.deltaUV.resize (0);

		if (bIsDeltaCol)
			rBS.deltaCol.resize (nNbVertUsed);
		else
			rBS.deltaCol.resize (0);

		if (bIsDeltaTgSpace)
			rBS.deltaTgSpace.resize (nNbVertUsed);
		else
			rBS.deltaTgSpace.resize (0);


		rBS.VertRefs.resize (nNbVertUsed);

	}
}

// ***************************************************************************
void CMRMBuilder::buildBlendShapes (CMRMMesh& baseMesh,
									std::vector<CMesh::CMeshBuild*> &bsList, uint32 VertexFlags)
{
	uint32 i, j, k, m, destIndex;
	uint32 attId;
	CVectorH vh;
	vector<CMRMBlendShape>	&bsMeshes= baseMesh.BlendShapes;

	bsMeshes.resize (bsList.size());

	for (i = 0; i < bsList.size(); ++i)
	{
		// Construct a blend shape like a mrm mesh
		nlassert (baseMesh.Vertices.size() == bsList[i]->Vertices.size());
		bsMeshes[i].Vertices.resize (baseMesh.Vertices.size());
		bsMeshes[i].Vertices = bsList[i]->Vertices;

		bsMeshes[i].NumAttributes = baseMesh.NumAttributes;
		for (j = 0; j < (uint32)bsMeshes[i].NumAttributes; ++j)
			bsMeshes[i].Attributes[j].resize(baseMesh.Attributes[j].size());

		// For all corners parse the faces (given by the baseMesh) and construct blend shape mrm meshes
		for (j = 0; j < baseMesh.Faces.size(); ++j)
		for (k = 0; k < 3; ++k)
		{
			const CMesh::CCorner &srcCorner = bsList[i]->Faces[j].Corner[k];
			CMRMCorner	&neutralCorner = baseMesh.Faces[j].Corner[k];

			attId= 0;

			if (VertexFlags & CVertexBuffer::NormalFlag)
			{
				destIndex = neutralCorner.Attributes[attId];
				vh.x = srcCorner.Normal.x;
				vh.y = srcCorner.Normal.y;
				vh.z = srcCorner.Normal.z;
				vh.w = 0.0f;
				bsMeshes[i].Attributes[attId].operator[](destIndex) = vh;
				attId++;
			}
			if (VertexFlags & CVertexBuffer::PrimaryColorFlag)
			{
				destIndex = neutralCorner.Attributes[attId];
				vh.x = srcCorner.Color.R;
				vh.y = srcCorner.Color.G;
				vh.z = srcCorner.Color.B;
				vh.w = srcCorner.Color.A;
				bsMeshes[i].Attributes[attId].operator[](destIndex) = vh;
				attId++;
			}
			if (VertexFlags & CVertexBuffer::SecondaryColorFlag)
			{
				destIndex = neutralCorner.Attributes[attId];
				vh.x = srcCorner.Specular.R;
				vh.y = srcCorner.Specular.G;
				vh.z = srcCorner.Specular.B;
				vh.w = srcCorner.Specular.A;
				bsMeshes[i].Attributes[attId].operator[](destIndex) = vh;
				attId++;
			}
			for (m = 0; m < CVertexBuffer::MaxStage; ++m)
			{
				if (VertexFlags & (CVertexBuffer::TexCoord0Flag<<m))
				{
					destIndex = neutralCorner.Attributes[attId];
					vh.x = srcCorner.Uvws[m].U;
					vh.y = srcCorner.Uvws[m].V;
					vh.z = srcCorner.Uvws[m].W;
					vh.w = 0.0f;
					bsMeshes[i].Attributes[attId].operator[](destIndex) = vh;
					attId++;
				}
			}
		}
	}
}


// ***************************************************************************
void	CMRMBuilder::compileMRM(const CMesh::CMeshBuild &mbuild, std::vector<CMesh::CMeshBuild*> &bsList,
								const CMRMParameters &params, CMeshMRMGeom::CMeshBuildMRM &mrmMesh,
								uint numMaxMaterial)
{
	// Temp data.
	CMRMMesh						baseMesh;
	vector<CMRMMeshGeom>			lodMeshs;
	CMRMMeshFinal					finalMRM;
	vector<CMRMMeshFinal>			finalBsMRM;
	uint32	vbFlags;


	nlassert(params.DistanceFinest>=0);
	nlassert(params.DistanceMiddle > params.DistanceFinest);
	nlassert(params.DistanceCoarsest > params.DistanceMiddle);


	// Copy some parameters.
	_SkinReduction= params.SkinReduction;

	// Skinning??
	_Skinned= ((mbuild.VertexFlags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag);
	// Skinning is OK only if SkinWeights are of same size as vertices.
	_Skinned= _Skinned && ( mbuild.Vertices.size()==mbuild.SkinWeights.size() );

	// MeshInterface setuped ?
	_HasMeshInterfaces= buildMRMSewingMeshes(mbuild, params.NLods, params.Divisor);

	// from mbuild, build an internal MRM mesh representation.
	// vbFlags returned is the VBuffer format supported by CMRMBuilder.
	// NB: skinning is removed because skinning is made in software in CMeshMRMGeom.
	vbFlags= buildMrmBaseMesh(mbuild, baseMesh);

	// Construct all blend shapes in the same way we have constructed the basemesh mrm
	buildBlendShapes (baseMesh, bsList, vbFlags);

	// If skinned, must ensure that skin weights have weights in ascending order.
	if(_Skinned)
	{
		normalizeBaseMeshSkin(baseMesh);
	}

	// from this baseMesh, builds all LODs of the MRM, with geomorph info. NB: vertices/wedges are duplicated.
	buildAllLods (	baseMesh, lodMeshs, params.NLods, params.Divisor );

	// From this array of LOD, build a finalMRM, by regrouping identical vertices/wedges, and compute index geomorphs.
	buildFinalMRM(lodMeshs, finalMRM);

	// From this finalMRM, build output: a CMeshBuildMRM.
	buildMeshBuildMrm(finalMRM, mrmMesh, vbFlags, numMaxMaterial, mbuild);

	// Copy degradation control params.
	mrmMesh.DistanceFinest= params.DistanceFinest;
	mrmMesh.DistanceMiddle= params.DistanceMiddle;
	mrmMesh.DistanceCoarsest= params.DistanceCoarsest;
}


// ***************************************************************************
void	CMRMBuilder::compileMRM(const CMesh::CMeshBuild &mbuild, std::vector<CMesh::CMeshBuild*> &bsList,
								const CMRMParameters &params, CMeshMRMSkinnedGeom::CMeshBuildMRM &mrmMesh,
								uint numMaxMaterial)
{
	// Temp data.
	CMRMMesh						baseMesh;
	vector<CMRMMeshGeom>			lodMeshs;
	CMRMMeshFinal					finalMRM;
	vector<CMRMMeshFinal>			finalBsMRM;
	uint32	vbFlags;


	nlassert(params.DistanceFinest>=0);
	nlassert(params.DistanceMiddle > params.DistanceFinest);
	nlassert(params.DistanceCoarsest > params.DistanceMiddle);


	// Copy some parameters.
	_SkinReduction= params.SkinReduction;

	// Skinning??
	_Skinned= ((mbuild.VertexFlags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag);
	// Skinning is OK only if SkinWeights are of same size as vertices.
	_Skinned= _Skinned && ( mbuild.Vertices.size()==mbuild.SkinWeights.size() );

	// MeshInterface setuped ?
	_HasMeshInterfaces= buildMRMSewingMeshes(mbuild, params.NLods, params.Divisor);

	// from mbuild, build an internal MRM mesh representation.
	// vbFlags returned is the VBuffer format supported by CMRMBuilder.
	// NB: skinning is removed because skinning is made in software in CMeshMRMGeom.
	vbFlags= buildMrmBaseMesh(mbuild, baseMesh);

	// Construct all blend shapes in the same way we have constructed the basemesh mrm
	buildBlendShapes (baseMesh, bsList, vbFlags);

	// If skinned, must ensure that skin weights have weights in ascending order.
	if(_Skinned)
	{
		normalizeBaseMeshSkin(baseMesh);
	}

	// from this baseMesh, builds all LODs of the MRM, with geomorph info. NB: vertices/wedges are duplicated.
	buildAllLods (	baseMesh, lodMeshs, params.NLods, params.Divisor );

	// From this array of LOD, build a finalMRM, by regrouping identical vertices/wedges, and compute index geomorphs.
	buildFinalMRM(lodMeshs, finalMRM);

	// From this finalMRM, build output: a CMeshBuildMRM.
	buildMeshBuildMrm(finalMRM, mrmMesh, vbFlags, numMaxMaterial, mbuild);

	// Copy degradation control params.
	mrmMesh.DistanceFinest= params.DistanceFinest;
	mrmMesh.DistanceMiddle= params.DistanceMiddle;
	mrmMesh.DistanceCoarsest= params.DistanceCoarsest;
}


// ***************************************************************************
// ***************************************************************************
// MRM Interface system
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
bool	CMRMBuilder::buildMRMSewingMeshes(const CMesh::CMeshBuild &mbuild, uint nWantedLods, uint divisor)
{
	nlassert(nWantedLods>=1);
	nlassert(divisor>=1);
	if(mbuild.Interfaces.size()==0)
		return false;
	// must have same size
	if(mbuild.InterfaceLinks.size()!=mbuild.Vertices.size())
		return false;

	// **** For each interface, MRM-ize it and store.
	_SewingMeshes.resize(mbuild.Interfaces.size());
	for(uint i=0;i<mbuild.Interfaces.size();i++)
	{
		_SewingMeshes[i].build(mbuild.Interfaces[i], nWantedLods, divisor);
	}


	return true;
}


} // NL3D
