// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2011  Robert TIMM (rti) <mail@rtti.de>
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

#include "nel/3d/ps_mesh.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/scene.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/ps_iterator.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/stream.h"
#include "nel/misc/path.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

////////////////////
// static members //
////////////////////




CPSConstraintMesh::CMeshDisplayShare		CPSConstraintMesh::_MeshDisplayShare(16);
CVertexBuffer								CPSConstraintMesh::_PreRotatedMeshVB;			  // mesh has no normals
CVertexBuffer								CPSConstraintMesh::_PreRotatedMeshVBWithNormal;  // mesh has normals
CPSConstraintMesh::TMeshName2RamVB			CPSConstraintMesh::_MeshRamVBs;



// this produce a random unit vector
static CVector MakeRandomUnitVect(void)
{
	NL_PS_FUNC(MakeRandomUnitVect)
	CVector v((float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  );
	v.normalize();
	return v;
}


////////////////////////////
// CPSMesh implementation //
////////////////////////////


//====================================================================================




const std::string DummyShapeName("dummy mesh shape");

/** a private function that create a dummy mesh :a cube with dummy textures
 */

static CMesh *CreateDummyMesh(void)
{
	NL_PS_FUNC(CreateDummyMesh)
	CMesh::CMeshBuild mb;
	CMeshBase::CMeshBaseBuild mbb;

	mb.VertexFlags = CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag;
	mb.Vertices.push_back(CVector(-.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, -.5f, .5f));

	mb.Vertices.push_back(CVector(-.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, .5f, .5f));

	// index for each face
	uint32 tab[] = { 4, 1, 0,
					 4, 5, 1,
					 5, 2, 1,
					 5, 6, 2,
					 6, 3, 2,
					 6, 7, 3,
					 7, 0, 3,
					 7, 4, 0,
					 7, 5, 4,
					 7, 6, 5,
					 2, 0, 1,
					 2, 3, 0
					};

	for (uint k = 0; k < 6; ++k)
	{
		CMesh::CFace f;
		f.Corner[0].Vertex = tab[6 * k];
		f.Corner[0].Uvws[0] = NLMISC::CUVW(0, 0, 0);

		f.Corner[1].Vertex = tab[6 * k + 1];
		f.Corner[1].Uvws[0] = NLMISC::CUVW(1, 1, 0);

		f.Corner[2].Vertex = tab[6 * k + 2];
		f.Corner[2].Uvws[0] = NLMISC::CUVW(0, 1, 0);

		f.MaterialId = 0;

		mb.Faces.push_back(f);

		f.Corner[0].Vertex = tab[6 * k + 3];
		f.Corner[0].Uvws[0] = NLMISC::CUVW(0, 0, 0);

		f.Corner[1].Vertex = tab[6 * k + 4];
		f.Corner[1].Uvws[0] = NLMISC::CUVW(1, 0, 0);

		f.Corner[2].Vertex = tab[6 * k + 5];
		f.Corner[2].Uvws[0] = NLMISC::CUVW(1, 1, 0);

		f.MaterialId = 0;
		mb.Faces.push_back(f);
	}

	CMaterial mat;
	CTextureMem *tex = new CTextureMem;
	tex->makeDummy();
	mat.setTexture(0, tex);
	mat.setLighting(false);
	mat.setColor(CRGBA::White);
	mbb.Materials.push_back(mat);
	CMesh *m = new CMesh;
	m->build(mbb, mb);
	return m;
}


//====================================================================================
void CPSMesh::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSMesh_IStream )
	(void)f.serialVersion(3);
	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial(_Shape);
	if (f.isReading())
	{

		uint maxSize = 0;
		if (_Owner)
		{
			maxSize = _Owner->getMaxSize();
			_Instances.resize(maxSize);
		}
		for(uint k = 0; k < maxSize; ++k)
		{
			_Instances.insert(NULL);
		}
	}
}

//====================================================================================
void CPSMesh::setShape(const std::string &shape)
{
	NL_PS_FUNC(CPSMesh_setShape)
	if (shape == _Shape) return;
	_Shape = shape;
	removeAllInstancesFromScene();
}

//====================================================================================
uint32 CPSMesh::getNumWantedTris() const
{
	NL_PS_FUNC(CPSMesh_getNumWantedTris)
	/// we don't draw any face ! (the meshs are drawn by the scene)
	return 0;
}

//====================================================================================
bool CPSMesh::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSMesh_hasTransparentFaces)
	/// we don't draw any tri ! (the meshs are drawn by the scene)
	return false;
}

//====================================================================================
bool CPSMesh::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSMesh_hasOpaqueFaces)
	/// We don't draw any tri !
	return false;
}

//====================================================================================
bool CPSMesh::hasLightableFaces()
{
	NL_PS_FUNC(CPSMesh_hasLightableFaces)
	/// we don't draw any tri ! (the meshs are drawn by the scene)
	return false;
}

//====================================================================================
void CPSMesh::releaseAllRef()
{
	NL_PS_FUNC(CPSMesh_releaseAllRef)
	CPSParticle::releaseAllRef();
	nlassert(_Owner && _Owner->getScene());
	removeAllInstancesFromScene();
}

//====================================================================================
void CPSMesh::removeAllInstancesFromScene()
{
	NL_PS_FUNC(CPSMesh_removeAllInstancesFromScene)
	for(uint k = 0; k < _Instances.getSize(); ++k)
	{
		if (_Instances[k])
		{
			if (_Owner) _Owner->getScene()->deleteInstance(_Instances[k]);
			_Instances[k] = NULL;
		}
	}
}

//====================================================================================
CTransformShape *CPSMesh::createInstance()
{
	NL_PS_FUNC(CPSMesh_createInstance)
	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called
	CTransformShape *instance = scene->createInstance(_Shape);
	if (!instance)
	{
		// mesh not found ...
		IShape *is = CreateDummyMesh();
		scene->getShapeBank()->add(DummyShapeName, is);
		instance = scene->createInstance(DummyShapeName);
		nlassert(instance);
	}
	instance->setTransformMode(CTransform::DirectMatrix);
	instance->hide(); // the object hasn't the right matrix yet so we hide it. It'll be shown once it is computed
	return instance;
}

//====================================================================================
void CPSMesh::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSMesh_newElement)
	newPlaneBasisElement(info);
	newAngle2DElement(info);
	newSizeElement(info);
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	CTransformShape *instance = createInstance();
	nlassert(instance);
	_Instances.insert(instance);
}

//====================================================================================
void CPSMesh::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSMesh_deleteElement)
	deleteSizeElement(index);
	deleteAngle2DElement(index);
	deletePlaneBasisElement(index);

	// check whether CTransformShape have been instanciated
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called
	if (_Instances[index])
	{
		scene->deleteInstance(_Instances[index]);
	}
	_Instances.remove(index);
}

//====================================================================================
void CPSMesh::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSMesh_step)
	if (pass == PSMotion)
	{
		updatePos();
	}
	else
	if (pass == PSToolRender) // edition mode only
	{
		showTool();
	}
}

//====================================================================================
void CPSMesh::updatePos()
{
	NL_PS_FUNC(CPSMesh_updatePos)
	const uint MeshBufSize = 512;
	PARTICLES_CHECK_MEM;
	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if (!size) return;


	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose


	if (!_Instances[0])
	{
		for (uint k = 0; k < size; ++k)
		{
			nlassert(!_Instances[k]);
			_Instances[k] = createInstance();
		}
	}

	float sizes[MeshBufSize];
	float angles[MeshBufSize];
	static CPlaneBasis planeBasis[MeshBufSize];

	uint32 leftToDo = size, toProcess;


	float *ptCurrSize;
	const uint  ptCurrSizeIncrement = _SizeScheme ? 1 : 0;

	float *ptCurrAngle;
	const uint  ptCurrAngleIncrement = _Angle2DScheme ? 1 : 0;

	CPlaneBasis *ptBasis;
	const uint  ptCurrPlaneBasisIncrement = _PlaneBasisScheme ? 1 : 0;

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt;


	TInstanceCont::iterator instanceIt = _Instances.begin();

	do
	{
		toProcess = leftToDo < MeshBufSize ? leftToDo : MeshBufSize;

		if (_SizeScheme)
		{
			ptCurrSize  = (float *) (_SizeScheme->make(_Owner, size - leftToDo, &sizes[0], sizeof(float), toProcess, true));
		}
		else
		{
			ptCurrSize =& _ParticleSize;
		}

		if (_Angle2DScheme)
		{
			ptCurrAngle  = (float *) (_Angle2DScheme->make(_Owner, size - leftToDo, &angles[0], sizeof(float), toProcess, true));
		}
		else
		{
			ptCurrAngle =& _Angle2D;
		}


		if (_PlaneBasisScheme)
		{
			ptBasis  = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size - leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true));
		}
		else
		{
			ptBasis = &_PlaneBasis;
		}

		endPosIt = posIt + toProcess;
		CMatrix mat, tmat;

		// the matrix used to get in the right basis
		const CMatrix &transfo = getLocalToWorldMatrix();
		do
		{

			tmat.identity();
			mat.identity();

			tmat.translate(*posIt);



			mat.setRot( ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle)
						, ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle + 64) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle + 64)
						, ptBasis->X ^ ptBasis->Y
					  );

			mat.scale(*ptCurrSize);

			(*instanceIt)->setMatrix(transfo * tmat * mat);
			if (CParticleSystem::OwnerModel)
			{
				// make sure the visibility is the same
				if (CParticleSystem::OwnerModel->isHrcVisible())
				{
					(*instanceIt)->show();
				}
				else
				{
					(*instanceIt)->hide();
				}
				(*instanceIt)->setClusterSystem(CParticleSystem::OwnerModel->getClusterSystem());
			}

			++instanceIt;
			++posIt;
			ptCurrSize += ptCurrSizeIncrement;
			ptCurrAngle += ptCurrAngleIncrement;
			ptBasis += ptCurrPlaneBasisIncrement;
		}
		while (posIt != endPosIt);
		leftToDo -= toProcess;
	}
	while (leftToDo);

	PARTICLES_CHECK_MEM;
}

//====================================================================================
void CPSMesh::resize(uint32 size)
{
	NL_PS_FUNC(CPSMesh_resize)
	nlassert(size < (1 << 16));
	resizeSize(size);
	resizeAngle2D(size);
	resizePlaneBasis(size);
	if (size < _Instances.getSize())
	{
		for(uint k = size; k < _Instances.getSize(); ++k)
		{
			if (_Owner) _Owner->getScene()->deleteInstance(_Instances[k]);
		}
	}
	_Instances.resize(size);
}


//====================================================================================
CPSMesh::~CPSMesh()
{
	NL_PS_FUNC(CPSMesh_CPSMeshDtor)
	if (_Owner && _Owner->getOwner())
	{
		removeAllInstancesFromScene();
	}
	else
	{
		#ifdef NL_DEBUG
			for (TInstanceCont::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
			{
				nlassert(*it == NULL); // there's a leak..:(
			}
		#endif
	}
}

//////////////////////////////////////
// CPSConstraintMesh implementation //
//////////////////////////////////////

/// private : eval the number of triangles in a mesh
static uint getMeshNumTri(const CMesh &m)
{
	NL_PS_FUNC(getMeshNumTri)
	uint numFaces = 0;
	for (uint k = 0; k < m.getNbMatrixBlock(); ++k)
	{
		for (uint l = 0; l  < m.getNbRdrPass(k); ++l)
		{
			const CIndexBuffer pb = m.getRdrPassPrimitiveBlock(k, l);
			numFaces += pb.getNumIndexes()/3;

		}
	}
	return numFaces;
}


//====================================================================================
/// private use : check if there are transparent and / or opaque faces in a mesh
static void CheckForOpaqueAndTransparentFacesInMesh(const CMesh &m, bool &hasTransparentFaces, bool &hasOpaqueFaces)
{
	NL_PS_FUNC(CheckForOpaqueAndTransparentFacesInMesh)
	hasTransparentFaces = false;
	hasOpaqueFaces = false;

	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		const CMaterial &currMat = m.getMaterial(m.getRdrPassMaterial(0, k));
		if (!currMat.getZWrite())
		{
			hasTransparentFaces = true;
		}
		else // z-buffer write or no blending -> the face is opaque
		{
			hasOpaqueFaces = true;
		}
	}
}

//====================================================================================
/// private use : check if there are lightable faces in a mesh
static bool CheckForLightableFacesInMesh(const CMesh &m)
{
	NL_PS_FUNC(CheckForLightableFacesInMesh)
	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		const CMaterial &currMat = m.getMaterial(m.getRdrPassMaterial(0, k));
		if (currMat.isLighted()) return true;
	}
	return false;
}


/** Well, we could have put a method template in CPSConstraintMesh, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSConstraintMeshHelper
{
public:
	template <class T>
	static void drawMeshs(T posIt, CPSConstraintMesh &m, uint size, uint32 srcStep, bool opaque)
	{
		NL_PS_FUNC(CPSConstraintMeshHelper_drawMeshs)
		const CVertexBuffer   &modelVb = m.getMeshVB(0);


		// size for model vertices
		const uint inVSize	  = modelVb.getVertexSize(); // vertex size

		// driver setup
		IDriver *driver = m.getDriver();
		m.setupDriverModelMatrix();

		// buffer to compute sizes
		float			sizes[ConstraintMeshBufSize];

		float *ptCurrSize;
		uint ptCurrSizeIncrement = m._SizeScheme ? 1 : 0;

		T endPosIt;
		uint leftToDo = size, toProcess;

		/// get a vb in which to write. It has the same format than the input mesh, but can also have a color flag added
		CPSConstraintMesh::CMeshDisplay  &md= m._MeshDisplayShare.getMeshDisplay(m._Meshes[0], modelVb, modelVb.getVertexFormat()
																| (m._ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));

		m.setupRenderPasses((float) m._Owner->getOwner()->getSystemDate() - m._GlobalAnimDate, md.RdrPasses, opaque);

		CVertexBuffer &outVb = md.VB;
		const uint outVSize = outVb.getVertexSize();



		// we don't have precomputed mesh there ... so each mesh must be transformed, which is the worst case
		CPlaneBasis planeBasis[ConstraintMeshBufSize];
		CPlaneBasis *ptBasis;
		uint ptBasisIncrement = m._PlaneBasisScheme ? 1 : 0;

		const uint nbVerticesInSource	= modelVb.getNumVertices();

		sint inNormalOff=0;
		sint outNormalOff=0;
		if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
		{
			inNormalOff  =  modelVb.getNormalOff();
			outNormalOff =  outVb.getNormalOff();
		}
		if (m._ColorScheme)
		{
			CVertexBuffer::TVertexColorType vtc = driver->getVertexColorFormat();
			m._ColorScheme->setColorType(vtc);
			if (modelVb.getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			{
				const_cast<CVertexBuffer &>(modelVb).setVertexColorFormat(vtc);
			}
		}
		CVertexBufferRead vbaRead;
		modelVb.lock (vbaRead);
		do
		{
			toProcess = std::min(leftToDo, ConstraintMeshBufSize);
			outVb.setNumVertices(toProcess * nbVerticesInSource);
			{
				CVertexBufferReadWrite vba;
				outVb.lock(vba);
				uint8 *outVertex = (uint8 *) vba.getVertexCoordPointer();
				if (m._SizeScheme)
				{
					ptCurrSize  = (float *) (m._SizeScheme->make(m._Owner, size -leftToDo, &sizes[0], sizeof(float), toProcess, true, srcStep));
				}
				else
				{
					ptCurrSize = &m._ParticleSize;
				}

				if (m._PlaneBasisScheme)
				{
					ptBasis = (CPlaneBasis *) (m._PlaneBasisScheme->make(m._Owner, size -leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true, srcStep));
				}
				else
				{
					ptBasis = &m._PlaneBasis;
				}


				endPosIt = posIt + toProcess;
				// transfo matrix & scaled transfo matrix;
				CMatrix  M, sM;


				if (m._Meshes.size() == 1)
				{
					/// unmorphed case
					do
					{
						const uint8 *inVertex = (const uint8 *) vbaRead.getVertexCoordPointer();
						uint k = nbVerticesInSource;

						// do we need a normal ?
						if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
						{
							M.identity();
							M.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
							sM = M;
							sM.scale(*ptCurrSize);

							// offset of normals in the prerotated mesh
							do
							{
								CHECK_VERTEX_BUFFER(modelVb, inVertex);
								CHECK_VERTEX_BUFFER(outVb,	  outVertex);
								CHECK_VERTEX_BUFFER(modelVb, inVertex + inNormalOff);
								CHECK_VERTEX_BUFFER(outVb,	  outVertex + outNormalOff);

								// translate and resize the vertex (relatively to the mesh origin)
								*(CVector *) outVertex = *posIt + sM * *(CVector *) inVertex;
								// copy the normal
								*(CVector *) (outVertex + outNormalOff) = M * *(CVector *) (inVertex + inNormalOff);


								inVertex  += inVSize;
								outVertex += outVSize;
							}
							while (--k);
						}
						else
						{
							// no normal to transform
							sM.identity();
							sM.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
							sM.scale(*ptCurrSize);

							do
							{
								CHECK_VERTEX_BUFFER(modelVb, inVertex);
								CHECK_VERTEX_BUFFER(outVb, outVertex);

								// translate and resize the vertex (relatively to the mesh origin)
								*(CVector *) outVertex = *posIt + sM * *(CVector *) inVertex;

								inVertex  += inVSize;
								outVertex += outVSize;
							}
							while (--k);
						}


						++posIt;
						ptCurrSize += ptCurrSizeIncrement;
						ptBasis += ptBasisIncrement;
					}
					while (posIt != endPosIt);
				}
				else
				{
					// morphed case

					// first, compute the morph value for each mesh
					float	morphValues[ConstraintMeshBufSize];
					float	*currMorphValue;
					uint	morphValueIncr;

					if (m._MorphScheme) // variable case
					{
						currMorphValue = (float *) m._MorphScheme->make(m._Owner, size - leftToDo, &morphValues[0], sizeof(float), toProcess, true, srcStep);
						morphValueIncr  = 1;
					}
					else /// constant case
					{
						currMorphValue = &m._MorphValue;
						morphValueIncr  = 0;
					}

					do
					{
						const uint numShapes = (uint)m._Meshes.size();
						const uint8 *m0, *m1;
						float lambda;
						float opLambda;
						const CVertexBuffer *inVB0, *inVB1;
						if (*currMorphValue >= numShapes - 1)
						{
							lambda = 0.f;
							opLambda = 1.f;
							inVB0 = inVB1 = &(m.getMeshVB(numShapes - 1));
						}
						else if (*currMorphValue <= 0)
						{
							lambda = 0.f;
							opLambda = 1.f;
							inVB0 = inVB1 = &(m.getMeshVB(0));
						}
						else
						{
							uint iMeshIndex = (uint) *currMorphValue;
							lambda = *currMorphValue - iMeshIndex;
							opLambda = 1.f - lambda;
							inVB0 = &(m.getMeshVB(iMeshIndex));
							inVB1 = &(m.getMeshVB(iMeshIndex + 1));
						}
						CVertexBufferRead vba0;
						inVB0->lock (vba0);
						CVertexBufferRead vba1;
						inVB1->lock (vba1);

						m0 = (uint8 *) vba0.getVertexCoordPointer();
						m1 = (uint8 *) vba1.getVertexCoordPointer();


						uint k = nbVerticesInSource;
						// do we need a normal ?
						if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
						{
							M.identity();
							M.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
							sM = M;
							sM.scale(*ptCurrSize);

							// offset of normals in the prerotated mesh
							do
							{
								CHECK_VERTEX_BUFFER((*inVB0),	  m0);
								CHECK_VERTEX_BUFFER((*inVB1),	  m1);
								CHECK_VERTEX_BUFFER((*inVB0),	  m0 + inNormalOff);
								CHECK_VERTEX_BUFFER((*inVB1),	  m1 + inNormalOff);
								CHECK_VERTEX_BUFFER(outVb,	  outVertex);
								CHECK_VERTEX_BUFFER(outVb,	  outVertex + outNormalOff);

								// morph, and transform the vertex
								*(CVector *) outVertex = *posIt + sM * (opLambda * *(CVector *) m0 + lambda * *(CVector *) m1);
								// morph, and transform the normal
								*(CVector *) (outVertex + outNormalOff) = M * (opLambda * *(CVector *) (m0 + inNormalOff)
																			  + lambda * *(CVector *) (m1 + inNormalOff)).normed();


								m0  += inVSize;
								m1  += inVSize;
								outVertex += outVSize;
							}
							while (--k);
						}
						else
						{
							// no normal to transform
							sM.identity();
							sM.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
							sM.scale(*ptCurrSize);

							do
							{
								CHECK_VERTEX_BUFFER((*inVB0),	  m0);
								CHECK_VERTEX_BUFFER((*inVB1),	  m1);
								CHECK_VERTEX_BUFFER(outVb, outVertex);
								// morph, and transform the vertex
								*(CVector *) outVertex = *posIt + sM * (opLambda * *(CVector *) m0 + opLambda * *(CVector *) m1);

								m0  += inVSize;
								m1  += inVSize;
								outVertex += outVSize;
							}
							while (--k);
						}


						++posIt;
						ptCurrSize += ptCurrSizeIncrement;
						ptBasis += ptBasisIncrement;
						currMorphValue += morphValueIncr;
					}
					while (posIt != endPosIt);
				}

				// compute colors if needed
				if (m._ColorScheme)
				{
					m.computeColors(outVb, modelVb, size - leftToDo, toProcess, srcStep, *driver, vba, vbaRead);
				}
			}

			// render meshs
			driver->activeVertexBuffer(outVb);
			m.doRenderPasses(driver, toProcess, md.RdrPasses, opaque);
			leftToDo -= toProcess;

		}
		while (leftToDo);
	}


	template <class T, class U>
	static void drawPrerotatedMeshs(T posIt,
								    U indexIt,
									CPSConstraintMesh &m,
									uint size,
									uint32 srcStep,
									bool opaque)
	{
		// get the vb from the original mesh
		const CVertexBuffer	  &modelVb = m.getMeshVB(0);

		/// precompute rotation in a VB from the src mesh
		CVertexBuffer &prerotVb  = m.makePrerotatedVb(modelVb);

		// driver setup
		IDriver *driver = m.getDriver();
		m.setupDriverModelMatrix();

		// renderPasses setup
		nlassert(m._Owner);

		// storage for sizes of meshs
		float sizes[ConstraintMeshBufSize];

		// point the size for the current mesh
		float *ptCurrSize;
		uint ptCurrSizeIncrement = m._SizeScheme ? 1 : 0;

		T endPosIt;
		uint leftToDo = size, toProcess;
		const uint nbVerticesInSource = modelVb.getNumVertices();



		// size of a complete prerotated model
		const uint prerotatedModelSize = prerotVb.getVertexSize() * modelVb.getNumVertices();

		/// get a mesh display struct on this shape, with eventually a primary color added.
		CPSConstraintMesh::CMeshDisplay  &md    = m._MeshDisplayShare.getMeshDisplay(m._Meshes[0], modelVb, modelVb.getVertexFormat()
																| (m._ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));


		m.setupRenderPasses((float) m._Owner->getOwner()->getSystemDate() - m._GlobalAnimDate, md.RdrPasses, opaque);

		CVertexBuffer &outVb = md.VB;



		// size of vertices in prerotated model
		const uint inVSize = prerotVb.getVertexSize();

		// size ofr vertices in dest vb
		const uint outVSize = outVb.getVertexSize();

		// offset of normals in vertices of the prerotated model, and source model
		uint normalOff=0;
		uint pNormalOff=0;
		if (prerotVb.getVertexFormat() & CVertexBuffer::NormalFlag)
		{
			normalOff  =  outVb.getNormalOff();
			pNormalOff =  prerotVb.getNormalOff();
		}

		if (m._ColorScheme)
		{
			CVertexBuffer::TVertexColorType vtc = driver->getVertexColorFormat();
			m._ColorScheme->setColorType(vtc);
			if (modelVb.getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			{
				const_cast<CVertexBuffer &>(modelVb).setVertexColorFormat(vtc);
			}
		}

		CVertexBufferRead PrerotVba;
		prerotVb.lock(PrerotVba);
		do
		{
			toProcess = std::min(leftToDo, ConstraintMeshBufSize);
			outVb.setNumVertices(toProcess * nbVerticesInSource);
			{
				CVertexBufferReadWrite vba;
				outVb.lock(vba);


				if (m._SizeScheme)
				{
					// compute size
					ptCurrSize = (float *) (m._SizeScheme->make(m._Owner, size - leftToDo, &sizes[0], sizeof(float), toProcess, true, srcStep));
				}
				else
				{
					// pointer on constant size
					ptCurrSize = &m._ParticleSize;
				}

				endPosIt = posIt + toProcess;
				uint8 *outVertex  = (uint8 *) vba.getVertexCoordPointer();
				/// copy datas for several mesh
				do
				{
					uint8 *inVertex = (uint8 *) PrerotVba.getVertexCoordPointer() + prerotatedModelSize * *indexIt; // prerotated vertex
					uint k = nbVerticesInSource;

					if (prerotVb.getVertexFormat() & CVertexBuffer::NormalFlag) // has it a normal ?
					{
						do
						{
							CHECK_VERTEX_BUFFER(outVb, outVertex);
							CHECK_VERTEX_BUFFER(prerotVb, inVertex);
							CHECK_VERTEX_BUFFER(outVb, outVertex + normalOff);
							CHECK_VERTEX_BUFFER(prerotVb, inVertex + pNormalOff);


							// translate and resize the vertex (relatively to the mesh origin)
							*(CVector *)  outVertex						 = *posIt + *ptCurrSize * *(CVector *) inVertex;
							// copy the normal
							*(CVector *)  (outVertex + normalOff ) = *(CVector *) (inVertex + pNormalOff);
							inVertex  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}
					else
					{
						do
						{
							// translate and resize the vertex (relatively to the mesh origin)
							CHECK_VERTEX_BUFFER(outVb, outVertex);
							CHECK_VERTEX_BUFFER(prerotVb, inVertex);
							*(CVector *)  outVertex = *posIt + *ptCurrSize * *(CVector *) inVertex;
							inVertex  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}

					++indexIt;
					++posIt;
					ptCurrSize += ptCurrSizeIncrement;
				}
				while (posIt != endPosIt);

				// compute colors if needed
				if (m._ColorScheme)
				{
					m.computeColors(outVb, modelVb, size - leftToDo, toProcess, srcStep, *driver, vba, PrerotVba);
				}
			}

			/// render the result
			driver->activeVertexBuffer(outVb);
			m.doRenderPasses(driver, toProcess, md.RdrPasses, opaque);
			leftToDo -= toProcess;

		}
		while (leftToDo);
		PARTICLES_CHECK_MEM
	}
};

CPSConstraintMesh::CPSConstraintMesh() : _NumFaces(0),
										 _ModelBank(NULL),
										 _ModulatedStages(0),
										 _Touched(1),
										 _HasOpaqueFaces(0),
										 _VertexColorLightingForced(false),
										 _GlobalAnimationEnabled(0),
										 _ReinitGlobalAnimTimeOnNewElement(0),
										 _HasLightableFaces(0),
										 _ValidBuild(0),
										 _MorphValue(0),
										 _MorphScheme(NULL)
{
	NL_PS_FUNC(CPSConstraintMesh_CPSConstraintMesh)
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("ConstraintMesh");
}

//====================================================================================
uint32 CPSConstraintMesh::getNumWantedTris() const
{
	NL_PS_FUNC(CPSConstraintMesh_getNumWantedTris)
//	nlassert(_ModelVb);
	//return _NumFaces * _Owner->getMaxSize();
	return _NumFaces * _Owner->getSize();
}


//====================================================================================
bool CPSConstraintMesh::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSConstraintMesh_hasTransparentFaces)
	if (!_Touched) return _HasTransparentFaces != 0;
	/// we must update the mesh to know whether it has transparent faces
	update();
	return _HasTransparentFaces != 0;
}

//====================================================================================
bool CPSConstraintMesh::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSConstraintMesh_hasOpaqueFaces)
	if (!_Touched) return _HasOpaqueFaces != 0;
	update();
	return _HasOpaqueFaces != 0;
}

//====================================================================================
bool CPSConstraintMesh::hasLightableFaces()
{
	NL_PS_FUNC(CPSConstraintMesh_hasLightableFaces)
	if (!_Touched) return _HasLightableFaces != 0;
	update();
	return _HasLightableFaces != 0;
}


//====================================================================================
void CPSConstraintMesh::setShape(const std::string &meshFileName)
{
	NL_PS_FUNC(CPSConstraintMesh_setShape)
	_MeshShapeFileName.resize(1);
	_MeshShapeFileName[0] = meshFileName;
	_Touched = 1;
	_ValidBuild = 0;
}


//===========================================================================
std::string			CPSConstraintMesh::getShape(void) const
{
	NL_PS_FUNC(CPSConstraintMesh_getShape)
	if (_Touched)
	{
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	nlassert(_MeshShapeFileName.size() == 1);
	return _MeshShapeFileName[0];
}

//====================================================================================
bool CPSConstraintMesh::isValidBuild() const
{
	NL_PS_FUNC(CPSConstraintMesh_isValidBuild)
	if (_Touched)
	{
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	return _ValidBuild != 0;
}

//====================================================================================
void		CPSConstraintMesh::setShapes(const std::string *shapesNames, uint numShapes)
{
	NL_PS_FUNC(CPSConstraintMesh_setShapes)
	_MeshShapeFileName.resize(numShapes);
	std::copy(shapesNames, shapesNames + numShapes, _MeshShapeFileName.begin());
	_Touched = 1;
	_ValidBuild = 0;
}

//====================================================================================
uint	    CPSConstraintMesh::getNumShapes() const
{
	NL_PS_FUNC(CPSConstraintMesh_getNumShapes)
	if (_Touched)
	{
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	return (uint)_MeshShapeFileName.size();
}

//====================================================================================
void	CPSConstraintMesh::getShapesNames(std::string *shapesNames) const
{
	NL_PS_FUNC(CPSConstraintMesh_getShapesNames)
	if (_Touched)
	{
		const_cast<CPSConstraintMesh *>(this)->update();
	}
#ifdef NL_COMP_VC14
	std::copy(_MeshShapeFileName.begin(), _MeshShapeFileName.end(), stdext::make_unchecked_array_iterator(shapesNames));
#else
	std::copy(_MeshShapeFileName.begin(), _MeshShapeFileName.end(), shapesNames);
#endif
}

//====================================================================================
void		CPSConstraintMesh::setShape(uint index, const std::string &shapeName)
{
	NL_PS_FUNC(CPSConstraintMesh_setShape)
	nlassert(index < _MeshShapeFileName.size());
	_MeshShapeFileName[index] = shapeName;
	_Touched = 1;
	_ValidBuild = 0;
}

//====================================================================================
const std::string          &CPSConstraintMesh::getShape(uint index) const
{
	NL_PS_FUNC(CPSConstraintMesh_getShape)
	if (_Touched)
	{
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	nlassert(index < _MeshShapeFileName.size());
	return _MeshShapeFileName[index];
}



//====================================================================================
void	CPSConstraintMesh::setMorphValue(float value)
{
	NL_PS_FUNC(CPSConstraintMesh_setMorphValue)
	delete _MorphScheme;
	_MorphScheme = NULL;
	_MorphValue = value;
}


//====================================================================================
float	CPSConstraintMesh::getMorphValue() const
{
	NL_PS_FUNC(CPSConstraintMesh_getMorphValue)
	return _MorphValue;
}

//====================================================================================
void	CPSConstraintMesh::setMorphScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSConstraintMesh_setMorphScheme)
	delete _MorphScheme;
	_MorphScheme = scheme;
	if (_MorphScheme->hasMemory()) _MorphScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
}

//====================================================================================
CPSAttribMaker<float>		*CPSConstraintMesh::getMorphScheme()
{
	NL_PS_FUNC(CPSConstraintMesh_getMorphScheme)
	return _MorphScheme;
}

//====================================================================================
const CPSAttribMaker<float>	*CPSConstraintMesh::getMorphScheme() const
{
	NL_PS_FUNC(CPSConstraintMesh_getMorphScheme)
	return _MorphScheme;
}


//====================================================================================
static CMesh *GetDummyMeshFromBank(CShapeBank &sb)
{
	NL_PS_FUNC(GetDummyMeshFromBank)
	static const std::string dummyMeshName("dummy constraint mesh shape");
	if (sb.getPresentState(dummyMeshName) == CShapeBank::Present)
	{
		return NLMISC::safe_cast<CMesh *>(sb.addRef(dummyMeshName));
	}
	else
	{
		// no dummy shape created -> add one to the bank
		CMesh *m = CreateDummyMesh();
		sb.add(std::string("dummy constraint mesh shape"), m);
		return m;
	}
}

//====================================================================================
void CPSConstraintMesh::getShapeNumVerts(std::vector<sint> &numVerts)
{
	NL_PS_FUNC(CPSConstraintMesh_getShapeNumVerts)
	_Touched = 1; // force reload
	update(&numVerts);
}

//====================================================================================
bool CPSConstraintMesh::update(std::vector<sint> *numVertsVect /*= NULL*/)
{
	NL_PS_FUNC(CPSConstraintMesh_update)
	bool ok = true;
	if (!_Touched) return ok;

	clean();

	nlassert(_Owner->getScene());

	CScene *scene = _Owner->getScene();
	_ModelBank = scene->getShapeBank();
	IShape *is = 0;


	uint32 vFormat = 0;
	uint   numVerts = 0;
	uint8  uvRouting[CVertexBuffer::MaxStage];

	if (_MeshShapeFileName.empty())
	{
		_MeshShapeFileName.resize(1);
		_MeshShapeFileName[0] = DummyShapeName;
	}


	_Meshes.resize(_MeshShapeFileName.size());
	_MeshVertexBuffers.resize(_MeshShapeFileName.size());
	std::fill(_MeshVertexBuffers.begin(), _MeshVertexBuffers.end(), (CVertexBuffer *) NULL);
	if (numVertsVect) numVertsVect->resize(_MeshShapeFileName.size());
	for (uint k = 0; k < _MeshShapeFileName.size(); ++k)
	{
		if (_ModelBank->getPresentState(_MeshShapeFileName[k]) == CShapeBank::Present)
		{
			CMesh *mesh = dynamic_cast<CMesh *>( _ModelBank->addRef(_MeshShapeFileName[k]));
			if (!mesh)
			{
				nlwarning("Tried to bind a shape that is not a mesh to a mesh particle : %s", _MeshShapeFileName[k].c_str());
				_ModelBank->release(is);
				ok = false;
				if (numVertsVect) (*numVertsVect)[k] = ShapeFileIsNotAMesh;
			}
			else
			{
				_Meshes[k] = mesh;
				/// get  the mesh format, or check that is was the same that previous shapes ' one
				if (k == 0)
				{
					vFormat = mesh->getVertexBuffer().getVertexFormat();
					numVerts =  mesh->getVertexBuffer().getNumVertices();
					std::copy(mesh->getVertexBuffer().getUVRouting(), mesh->getVertexBuffer().getUVRouting() + CVertexBuffer::MaxStage, uvRouting);
					if (numVertsVect) (*numVertsVect)[k] = (sint) numVerts;
				}
				else
				{
					if (vFormat != mesh->getVertexBuffer().getVertexFormat())
					{
						nlwarning("Vertex format differs between meshs");
						ok = false;
					}
					if (numVerts != mesh->getVertexBuffer().getNumVertices())
					{
						nlwarning("Num vertices differs between meshs");
						ok = false;
					}
					if (!std::equal(mesh->getVertexBuffer().getUVRouting(), mesh->getVertexBuffer().getUVRouting() + CVertexBuffer::MaxStage, uvRouting))
					{
						nlwarning("UV routing differs between meshs");
						ok = false;
					}
					if (numVertsVect) (*numVertsVect)[k] = (sint) mesh->getVertexBuffer().getNumVertices();
				}
			}
		}
		else
		{
			try
			{
				_ModelBank->load(_MeshShapeFileName[k]);
			}
			catch (const NLMISC::EPathNotFound &)
			{
				nlwarning("mesh not found : %s; used as a constraint mesh particle", _MeshShapeFileName[k].c_str());
				// shape not found, so not present in the shape bank -> we create a dummy shape
			}

			if (_ModelBank->getPresentState(_MeshShapeFileName[k]) != CShapeBank::Present)
			{
				ok = false;
				if (numVertsVect) (*numVertsVect)[k] = ShapeFileNotLoaded;
			}
			else
			{
				is = _ModelBank->addRef(_MeshShapeFileName[k]);
				if (!dynamic_cast<CMesh *>(is)) // is it a mesh
				{
					nlwarning("Tried to bind a shape that is not a mesh to a mesh particle : %s", _MeshShapeFileName[k].c_str());
					_ModelBank->release(is);
					ok = false;
					if (numVertsVect) (*numVertsVect)[k] = ShapeFileIsNotAMesh;
				}
				else
				{
					CMesh &m  = * NLMISC::safe_cast<CMesh *>(is);
					/// make sure there are not too many vertices
					if (m.getVertexBuffer().getNumVertices() > ConstraintMeshMaxNumVerts)
					{
						nlwarning("Tried to bind a mesh that has more than %d vertices to a particle mesh: %s", (int) ConstraintMeshMaxNumVerts, _MeshShapeFileName[k].c_str());
						_ModelBank->release(is);
						ok = false;
						if (numVertsVect) (*numVertsVect)[k] = ShapeHasTooMuchVertices;
					}
					else
					{
						_Meshes[k] = &m;
						if (k == 0)
						{
							vFormat = m.getVertexBuffer().getVertexFormat();
							numVerts =  m.getVertexBuffer().getNumVertices();
							std::copy(m.getVertexBuffer().getUVRouting(), m.getVertexBuffer().getUVRouting() + CVertexBuffer::MaxStage, uvRouting);
							if (numVertsVect) (*numVertsVect)[k] = numVerts;
						}
						else
						{
							uint32 otherVFormat = m.getVertexBuffer().getVertexFormat();
							uint   otherNumVerts = m.getVertexBuffer().getNumVertices();
							if (otherVFormat != vFormat ||
								otherNumVerts != numVerts ||
							    !(std::equal(m.getVertexBuffer().getUVRouting(), m.getVertexBuffer().getUVRouting() + CVertexBuffer::MaxStage, uvRouting)))
							{
								ok = false;
							}
							if (numVertsVect) (*numVertsVect)[k] = otherNumVerts;
						}
					}
				}
			}
		}

		if (!ok && !numVertsVect) break;
	}

	if (!ok)
	{
		releaseShapes();
		_Meshes.resize(1);
		_MeshVertexBuffers.resize(1);
		_Meshes[0] = GetDummyMeshFromBank(*_ModelBank);
		_MeshVertexBuffers[0] = &_Meshes[0]->getVertexBuffer();
	}

	const CMesh &m  = *_Meshes[0];

	/// update the number of faces
	_NumFaces = getMeshNumTri(m);
	/*
	notifyOwnerMaxNumFacesChanged();
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->notifyMaxNumFacesChanged();
	}*/

	/// update opacity / transparency state
	bool hasTransparentFaces, hasOpaqueFaces;
	CheckForOpaqueAndTransparentFacesInMesh(m, hasTransparentFaces, hasOpaqueFaces);
	_HasTransparentFaces = hasTransparentFaces;
	_HasOpaqueFaces = hasOpaqueFaces;
	_HasLightableFaces = CheckForLightableFacesInMesh(m);
	_GlobalAnimDate = _Owner->getOwner()->getSystemDate();
	_Touched = 0;
	_ValidBuild = ok ? 1 : 0;
	nlassert(!_Meshes.empty());

	return ok;

}



//====================================================================================
void CPSConstraintMesh::hintRotateTheSame(uint32 nbConfiguration,
										  float minAngularVelocity,
										  float maxAngularVelocity
										)
{
	NL_PS_FUNC(CPSConstraintMesh_hintRotateTheSame)
	nlassert(nbConfiguration <= ConstraintMeshMaxNumPrerotatedModels);

	// TODO : avoid code duplication with CPSFace ...
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;



	_PrecompBasis.resize(nbConfiguration);

	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}

		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}
}


//====================================================================================
void CPSConstraintMesh::fillIndexesInPrecompBasis(void)
{
	NL_PS_FUNC(CPSConstraintMesh_fillIndexesInPrecompBasis)
	// TODO : avoid code duplication with CPSFace ...
	const uint32 nbConf = (uint32)_PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}
	for (CPSVector<uint32>::V::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}

//====================================================================================
/// serialisation. Derivers must override this, and call their parent version
void CPSConstraintMesh::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSConstraintMesh_IStream )

	sint ver = f.serialVersion(4);
	if (f.isReading())
	{
		clean();
	}

	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	// prerotations ...

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);
	}
	else
	{
		uint32 nbConfigurations = (uint32)_PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);
		}
	}

	// saves the model file name, or an empty string if nothing has been set
	static std::string emptyStr;

	if (ver < 4) // early version : no morphing support
	{
		if (!f.isReading())
		{
			if (!_MeshShapeFileName.empty())
			{
				f.serial(_MeshShapeFileName[0]);
			}
			else
			{
				f.serial(emptyStr);
			}
		}
		else
		{
			_MeshShapeFileName.resize(1);
			f.serial(_MeshShapeFileName[0]);
			_Touched = true;
			_ValidBuild = 0;
		}
	}

	if (ver > 1)
	{
		CPSColoredParticle::serialColorScheme(f);
		f.serial(_ModulatedStages);
		if (f.isReading())
		{
			bool vcEnabled;
			f.serial(vcEnabled);
			_VertexColorLightingForced = vcEnabled;
		}
		else
		{
			bool vcEnabled = (_VertexColorLightingForced != 0);
			f.serial(vcEnabled);
		}
	}

	if (ver > 2) // texture animation
	{
		if (f.isReading())
		{
			bool gaEnabled;
			f.serial(gaEnabled);
			_GlobalAnimationEnabled = gaEnabled;
			if (gaEnabled)
			{
				PGlobalTexAnims newPtr(new CGlobalTexAnims); // create new
				//std::swap(_GlobalTexAnims, newPtr);			 // replace old
				_GlobalTexAnims = CUniquePtrMove(newPtr);
				f.serial(*_GlobalTexAnims);
			}

			bool rgt;
			f.serial(rgt);
			_ReinitGlobalAnimTimeOnNewElement = rgt;
		}
		else
		{
			bool gaEnabled = (_GlobalAnimationEnabled != 0);
			f.serial(gaEnabled);
			if (gaEnabled)
			{
				f.serial(*_GlobalTexAnims);
			}

			bool rgt = _ReinitGlobalAnimTimeOnNewElement != 0;
			f.serial(rgt);
		}
	}

	if (ver > 3) // mesh morphing
	{
		if (!f.isReading())
		{
			// remove path
			TMeshNameVect meshNamesWithoutPath = _MeshShapeFileName;
			std::transform(meshNamesWithoutPath.begin(), meshNamesWithoutPath.end(), meshNamesWithoutPath.begin(), std::ptr_fun(NLMISC::CFile::getFilename));
			f.serialCont(meshNamesWithoutPath);
		}
		else
		{
			f.serialCont(_MeshShapeFileName);
		}
		bool useScheme;
		if (f.isReading())
		{
			delete _MorphScheme;
		}
		else
		{
			useScheme = _MorphScheme != NULL;
		}
		f.serial(useScheme);
		if (useScheme)
		{
			f.serialPolyPtr(_MorphScheme);
		}
		else
		{
			f.serial(_MorphValue);
		}
	}
}

//====================================================================================
CPSConstraintMesh::~CPSConstraintMesh()
{
	NL_PS_FUNC(CPSConstraintMesh_CPSConstraintMeshDtor)
	clean();
	delete _MorphScheme;
}



//====================================================================================
void CPSConstraintMesh::releaseShapes()
{
	NL_PS_FUNC(CPSConstraintMesh_releaseShapes)
	for (TMeshVect::iterator it = _Meshes.begin(); it != _Meshes.end(); ++it)
	{
		if (*it)
		{
			if (_ModelBank) _ModelBank->release(*it);
		}
	}
	_Meshes.clear();
	_MeshVertexBuffers.clear();
}

//====================================================================================
void CPSConstraintMesh::clean(void)
{
	NL_PS_FUNC(CPSConstraintMesh_clean)
	if (_ModelBank)
	{
		releaseShapes();
	}
}


//====================================================================================
CVertexBuffer &CPSConstraintMesh::makePrerotatedVb(const CVertexBuffer &inVb)
{
	NL_PS_FUNC(CPSConstraintMesh_makePrerotatedVb)
	// get a VB that has positions and eventually normals
	CVertexBuffer &prerotatedVb = inVb.getVertexFormat() & CVertexBuffer::NormalFlag ? _PreRotatedMeshVBWithNormal : _PreRotatedMeshVB;
	CVertexBufferReadWrite vba;
	prerotatedVb.lock (vba);
	CVertexBufferRead vbaIn;
	inVb.lock (vbaIn);

	// size of vertices for source VB
	const uint vSize = inVb.getVertexSize();

	// size for vertices in prerotated model
	const uint vpSize = prerotatedVb.getVertexSize();


	// offset of normals in vertices of the prerotated model, and source model
	uint normalOff=0;
	uint pNormalOff=0;
	if (prerotatedVb.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		normalOff  =  inVb.getNormalOff();
		pNormalOff =  prerotatedVb.getNormalOff();
	}

	const uint nbVerticesInSource	= inVb.getNumVertices();


	// rotate basis
	// and compute the set of prerotated meshs that will then duplicated (with scale and translation) to create the Vb of what must be drawn
	uint8 *outVertex = (uint8 *) vba.getVertexCoordPointer();
	for (CPSVector<CPlaneBasisPair>::V::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
	{
		// not optimized at all, but this will apply to very few elements anyway...
		CMatrix mat;
		mat.rotate(CQuat(it->Axis, CParticleSystem::EllapsedTime * it->AngularVelocity));
		CVector n = mat * it->Basis.getNormal();
		it->Basis = CPlaneBasis(n);

		mat.identity();
		mat.setRot(it->Basis.X, it->Basis.Y, it->Basis.X ^ it->Basis.Y);

		uint8 *inVertex = (uint8 *) vbaIn.getVertexCoordPointer();

		uint k = nbVerticesInSource;

		// check whether we need to rotate normals as well...
		if (inVb.getVertexFormat() & CVertexBuffer::NormalFlag)
		{

			do
			{
				CHECK_VERTEX_BUFFER(inVb, inVertex);
				CHECK_VERTEX_BUFFER(inVb, inVertex + normalOff);
				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex);
				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex + pNormalOff);

				* (CVector *) outVertex =  mat.mulVector(* (CVector *) inVertex);
				* (CVector *) (outVertex + normalOff) =  mat.mulVector(* (CVector *) (inVertex + pNormalOff) );
				outVertex += vpSize;
				inVertex  += vSize;

			}
			while (--k);
		}
		else
		{
			// no normal included
			do
			{

				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex);
				CHECK_VERTEX_BUFFER(inVb, inVertex);

				* (CVector *) outVertex =  mat.mulVector(* (CVector *) inVertex);
				outVertex += vpSize;
				inVertex += vSize;
			}
			while (--k);

		}
	}
	return prerotatedVb;
}


//====================================================================================
void CPSConstraintMesh::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSConstraintMesh_step)
		if (
			(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
			)
		{
			draw(pass == PSSolidRender);
		}
		else
		if (pass == PSToolRender) // edition mode only
		{
			showTool();
		}
}

//====================================================================================
void CPSConstraintMesh::draw(bool opaque)
{
//	if (!FilterPS[4]) return;
	NL_PS_FUNC(CPSConstraintMesh_draw)
	PARTICLES_CHECK_MEM;
	nlassert(_Owner);

	update(); // update mesh datas if needed
	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);
	if (!numToProcess) return;
	_Owner->incrementNbDrawnParticles(numToProcess); // for benchmark purpose


	if (_PrecompBasis.empty()) /// do we deal with prerotated meshs ?
	{
		if (step == (1 << 16))
		{
			CPSConstraintMeshHelper::drawMeshs(_Owner->getPos().begin(),
											  *this,
						                      numToProcess,
											  step,
											  opaque
											 );
		}
		else
		{
			CPSConstraintMeshHelper::drawMeshs(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
											  *this,
						                      numToProcess,
											  step,
											  opaque
											 );
		}
	}
	else
	{
		if (step == (1 << 16))
		{
			CPSConstraintMeshHelper::drawPrerotatedMeshs(_Owner->getPos().begin(),
													     _IndexInPrecompBasis.begin(),
														 *this,
														 numToProcess,
														 step,
														 opaque
													    );
		}
		else
		{
			typedef CAdvance1616Iterator<CPSVector<uint32>::V::const_iterator, uint32> TIndexIterator;
			CPSConstraintMeshHelper::drawPrerotatedMeshs(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
														 TIndexIterator(_IndexInPrecompBasis.begin(), 0, step),
														 *this,
														 numToProcess,
														 step,
														 opaque
													    );
		}
	}


}

//====================================================================================
void CPSConstraintMesh::setupMaterialColor(CMaterial &destMat, CMaterial &srcMat)
{
	NL_PS_FUNC(CPSConstraintMesh_setupMaterialColor)
	if (destMat.getShader() != CMaterial::Normal) return;
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_ModulatedStages & (1 << k))
		{
			destMat.texEnvArg0RGB(k, CMaterial::Texture, CMaterial::SrcColor);
			destMat.texEnvArg0Alpha(k, CMaterial::Texture, CMaterial::SrcAlpha);
			destMat.texEnvArg1RGB(k, CMaterial::Diffuse, CMaterial::SrcColor);
			destMat.texEnvArg1Alpha(k, CMaterial::Diffuse, CMaterial::SrcAlpha);
			destMat.texEnvOpRGB(k, CMaterial::Modulate);
			destMat.texEnvOpAlpha(k, CMaterial::Modulate);
		}
		else // restore from source material
		{
			destMat.setTexEnvMode(k, srcMat.getTexEnvMode(k));
		}
	}
	if (_ColorScheme == NULL) // per mesh color ?
	{
		destMat.setColor(_Color);
		if (destMat.isLighted())
		{
			destMat.setDiffuse(_Color);
		}
	}
}




//====================================================================================
void	CPSConstraintMesh::setupRenderPasses(float date, TRdrPassSet &rdrPasses, bool opaque)
{
	NL_PS_FUNC(CPSConstraintMesh_setupRenderPasses)
	// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin();
	     rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{

		CMaterial &Mat = rdrPassIt->Mat;
		CMaterial &SourceMat = rdrPassIt->SourceMat;


		/// check whether this material has to be rendered
		if ((opaque && Mat.getZWrite()) || (!opaque && ! Mat.getZWrite()))
		{


			// has to setup material constant color ?
			// global color not supported for mesh
		/*	CParticleSystem &ps = *(_Owner->getOwner());
			if (!_ColorScheme)
			{
				NLMISC::CRGBA col;
				col.modulateFromColor(SourceMat.getColor(), _Color);
				if (ps.getColorAttenuationScheme() == NULL || ps.isUserColorUsed())
				{
					col.modulateFromColor(col, ps.getGlobalColor());
				}
				Mat.setColor(col);
			}
			else
			{
				Mat.setColor(ps.getGlobalColor());
			}*/

			/** Force modulation for some stages & setup global color
			  */
			setupMaterialColor(Mat, SourceMat);

			/// force vertex lighting
			bool forceVertexcolorLighting;
			if (_ColorScheme != NULL)
			{
				forceVertexcolorLighting = _VertexColorLightingForced != 0 ? true : SourceMat.getLightedVertexColor();
			}
			else
			{
				forceVertexcolorLighting = false;
			}
			if (forceVertexcolorLighting != Mat.getLightedVertexColor()) // avoid to touch mat if not needed
			{
				Mat.setLightedVertexColor(forceVertexcolorLighting);
			}

			///global texture animation
			if (_GlobalAnimationEnabled != 0)
			{
				for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
				{
					if (Mat.getTexture(k) != NULL)
					{
						Mat.enableUserTexMat(k, true);
						CMatrix mat;
						_GlobalTexAnims->Anims[k].buildMatrix(date, mat);
						Mat.setUserTexMat(k ,mat);
					}
				}
			}
		}
	}

}

//====================================================================================
void	CPSConstraintMesh::doRenderPasses(IDriver *driver, uint numObj, TRdrPassSet &rdrPasses, bool opaque)
{
	NL_PS_FUNC(CPSConstraintMesh_doRenderPasses)
	// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin(); rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{
		CMaterial &Mat = rdrPassIt->Mat;
		if ((opaque && Mat.getZWrite()) || (!opaque && ! Mat.getZWrite()))
		{
			/// setup number of primitives to be rendered
			rdrPassIt->PbTri.setNumIndexes(((rdrPassIt->PbTri.capacity()/3)   * numObj / ConstraintMeshBufSize) * 3);
			rdrPassIt->PbLine.setNumIndexes(((rdrPassIt->PbLine.capacity()/2) * numObj / ConstraintMeshBufSize) * 2);

			/// render the primitives
			driver->activeIndexBuffer (rdrPassIt->PbTri);
			driver->renderTriangles(rdrPassIt->Mat, 0, rdrPassIt->PbTri.getNumIndexes()/3);
			if (rdrPassIt->PbLine.getNumIndexes() != 0)
			{
				driver->activeIndexBuffer (rdrPassIt->PbLine);
				driver->renderLines(rdrPassIt->Mat, 0, rdrPassIt->PbLine.getNumIndexes()/2);
			}
		}
	}

}


//====================================================================================
void	CPSConstraintMesh::computeColors(CVertexBuffer &outVB, const CVertexBuffer &inVB, uint startIndex, uint toProcess, uint32 srcStep, IDriver &drv,
										 CVertexBufferReadWrite &vba,
										 CVertexBufferRead &vbaIn
										)
{
	NL_PS_FUNC(CPSConstraintMesh_computeColors)
	nlassert(_ColorScheme);
	// there are 2 case : 1 - the source mesh has colors, which are modulated with the current color
	//					  2 - the source mesh has no colors : colors are directly copied into the dest vb

	if (inVB.getVertexFormat() & CVertexBuffer::PrimaryColorFlag) // case 1
	{
		// TODO: optimisation : avoid to duplicate colors...
		_ColorScheme->makeN(_Owner, startIndex, vba.getColorPointer(), outVB.getVertexSize(), toProcess, inVB.getNumVertices(), srcStep);
		// modulate from the source mesh
		// todo hulud d3d vertex color RGBA / BGRA
		uint8 *vDest  = (uint8 *) vba.getColorPointer();
		uint8 *vSrc   = (uint8 *) vbaIn.getColorPointer();
		const uint vSize = outVB.getVertexSize();
		const uint numVerts = inVB.getNumVertices();
		uint  meshSize = vSize * numVerts;
		for (uint k = 0; k < toProcess; ++k)
		{
			NLMISC::CRGBA::modulateColors((CRGBA *) vDest, (CRGBA *) vSrc, (CRGBA *) vDest, numVerts, vSize, vSize);
			vDest += meshSize;
		}
	}
	else // case 2
	{
		_ColorScheme->makeN(_Owner, startIndex, vba.getColorPointer(), outVB.getVertexSize(), toProcess, inVB.getNumVertices(), srcStep);
	}
}


//====================================================================================
void CPSConstraintMesh::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSConstraintMesh_newElement)
	newSizeElement(info);
	newPlaneBasisElement(info);
	// TODO : avoid code duplication with CPSFace ...
	const uint32 nbConf = (uint32)_PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}
	newColorElement(info);
	if (_GlobalAnimationEnabled && _ReinitGlobalAnimTimeOnNewElement)
	{
		_GlobalAnimDate = _Owner->getOwner()->getSystemDate();
	}
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->newElement(info);
}

//====================================================================================
void CPSConstraintMesh::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSConstraintMesh_deleteElement)
	deleteSizeElement(index);
	deletePlaneBasisElement(index);
	// TODO : avoid code cuplication with CPSFace ...
	if (!_PrecompBasis.empty()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}
	deleteColorElement(index);
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->deleteElement(index);
}

//====================================================================================
void CPSConstraintMesh::resize(uint32 size)
{
	NL_PS_FUNC(CPSConstraintMesh_resize)
	nlassert(size < (1 << 16));
	resizeSize(size);
	resizePlaneBasis(size);
	// TODO : avoid code cuplication with CPSFace ...
	if (!_PrecompBasis.empty()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
	resizeColor(size);
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->resize(size, _Owner->getSize());
}

//====================================================================================
void CPSConstraintMesh::updateMatAndVbForColor(void)
{
	NL_PS_FUNC(CPSConstraintMesh_updateMatAndVbForColor)
	// nothing to do for us...
}

//====================================================================================
void	CPSConstraintMesh::forceStageModulationByColor(uint stage, bool force)
{
	NL_PS_FUNC(CPSConstraintMesh_forceStageModulationByColor)
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (force)
	{
		_ModulatedStages |= 1 << stage;
	}
	else
	{
		_ModulatedStages &= ~(1 << stage);
	}
}

//====================================================================================
bool	CPSConstraintMesh::isStageModulationForced(uint stage) const
{
	NL_PS_FUNC(CPSConstraintMesh_isStageModulationForced)
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return (_ModulatedStages & (1 << stage)) != 0;
}

//====================================================================================

/** This duplicate a primitive block n time in the destination primitive block
 *  This is used to draw several mesh at once
 *  For each duplication, vertices indices are shifted from the given offset (number of vertices in the mesh)
 */

static void DuplicatePrimitiveBlock(const CIndexBuffer &srcBlock, CIndexBuffer &destBlock, uint nbReplicate, uint vertOffset)
{
	NL_PS_FUNC(DuplicatePrimitiveBlock)
	PARTICLES_CHECK_MEM;

	// this must be update each time a new primitive is added

	// loop counters, and index of the current primitive in the dest pb
	uint k, l, index;

	// the current vertex offset.
	uint currVertOffset;


	// duplicate triangles
	uint numTri = srcBlock.getNumIndexes()/3;
	destBlock.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	destBlock.setNumIndexes(3 * numTri * nbReplicate);

	index = 0;
	currVertOffset = 0;

	CIndexBufferRead ibaRead;
	srcBlock.lock (ibaRead);
	CIndexBufferReadWrite ibaWrite;
	destBlock.lock (ibaWrite);

#ifdef NL_FORCE_INDEX_BUFFER_16
	nlassert(destBlock.getFormat() == CIndexBuffer::Indices16);
#endif

	// TMP TMP TMP
	if (ibaRead.getFormat() == CIndexBuffer::Indices16)
	{
		const TIndexType *triPtr = (TIndexType *) ibaRead.getPtr();
		const TIndexType *currTriPtr; // current Tri
		for (k = 0; k < nbReplicate; ++k)
		{
			currTriPtr = triPtr;
			for (l = 0; l < numTri; ++l)
			{
				ibaWrite.setTri(3*index, currTriPtr[0] + currVertOffset, currTriPtr[1] + currVertOffset, currTriPtr[2] + currVertOffset);
				currTriPtr += 3;
				++ index;
			}
			currVertOffset += vertOffset;
		}
	}
	else
	{
		const uint32 *triPtr = (uint32 *) ibaRead.getPtr();
		const uint32 *currTriPtr; // current Tri
		for (k = 0; k < nbReplicate; ++k)
		{
			currTriPtr = triPtr;
			for (l = 0; l < numTri; ++l)
			{
				nlassert(currTriPtr[0] + currVertOffset <= 0xffff);
				nlassert(currTriPtr[1] + currVertOffset <= 0xffff);
				nlassert(currTriPtr[2] + currVertOffset <= 0xffff);
				//
				ibaWrite.setTri(3*index, (uint16) (currTriPtr[0] + currVertOffset), (uint16) (currTriPtr[1] + currVertOffset), (uint16) (currTriPtr[2] + currVertOffset));
				currTriPtr += 3;
				++ index;
			}
			currVertOffset += vertOffset;
		}
	}



	// TODO quad / strips duplication : (unimplemented in primitive blocks for now)

	PARTICLES_CHECK_MEM;
}

//====================================================================================
void CPSConstraintMesh::initPrerotVB()
{
	NL_PS_FUNC(CPSConstraintMesh_initPrerotVB)
	// position, no normals
	_PreRotatedMeshVB.setVertexFormat(CVertexBuffer::PositionFlag);
	_PreRotatedMeshVB.setNumVertices(ConstraintMeshMaxNumPrerotatedModels * ConstraintMeshMaxNumVerts);
	_PreRotatedMeshVB.setName("CPSConstraintMesh::_PreRotatedMeshVB");

	// position & normals
	_PreRotatedMeshVBWithNormal.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag);
	_PreRotatedMeshVBWithNormal.setNumVertices(ConstraintMeshMaxNumPrerotatedModels * ConstraintMeshMaxNumVerts);
	_PreRotatedMeshVB.setName("CPSConstraintMesh::_PreRotatedMeshVBWithNormal");
}

//====================================================================================
CPSConstraintMesh::CMeshDisplay &CPSConstraintMesh::CMeshDisplayShare::getMeshDisplay(CMesh *mesh, const CVertexBuffer &meshVB, uint32 format)
{
	NL_PS_FUNC(CMeshDisplayShare_getMeshDisplay)
	nlassert(mesh);
	// linear search is ok because of small size
	for(std::list<CMDEntry>::iterator it = _Cache.begin(); it != _Cache.end(); ++it)
	{
		if (it->Format == format && it->Mesh == mesh)
		{
			// relink at start (most recent use)
			_Cache.splice(_Cache.begin(), _Cache, it);
			return it->MD;
		}
	}
	if (_NumMD == _MaxNumMD)
	{
		_Cache.pop_back(); // remove least recently used mesh
		-- _NumMD;
	}
	//NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
	_Cache.push_front(CMDEntry());
	_Cache.front().Mesh = mesh;
	_Cache.front().Format = format;
	buildRdrPassSet(_Cache.front().MD.RdrPasses, *mesh);
	_Cache.front().MD.VB.setName("CPSConstraintMesh::CMeshDisplay");
	buildVB(_Cache.front().MD.VB, meshVB, format);
	++ _NumMD;
	/*NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
	nlinfo("mesh setup time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));	*/
	return _Cache.front().MD;
}

//====================================================================================
void CPSConstraintMesh::CMeshDisplayShare::buildRdrPassSet(TRdrPassSet &dest,  const CMesh &m)
{
	NL_PS_FUNC(CMeshDisplayShare_buildRdrPassSet)
	// we don't support skinning for mesh particles, so there must be only one matrix block
	nlassert(m.getNbMatrixBlock() == 1);  // SKINNING UNSUPPORTED

	dest.resize(m.getNbRdrPass(0));
	const CVertexBuffer &srcVb = m.getVertexBuffer();

	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		dest[k].Mat = m.getMaterial(m.getRdrPassMaterial(0, k));
		dest[k].SourceMat = dest[k].Mat;
		DuplicatePrimitiveBlock(m.getRdrPassPrimitiveBlock(0, k), dest[k].PbTri, ConstraintMeshBufSize, srcVb.getNumVertices() );
	}
}



//====================================================================================
void CPSConstraintMesh::CMeshDisplayShare::buildVB(CVertexBuffer &dest, const CVertexBuffer &meshVb, uint32 destFormat)
{
	NL_PS_FUNC(CMeshDisplayShare_buildVB)
	/// we duplicate the original mesh data's 'ConstraintMeshBufSize' times, eventually adding a color
	nlassert(destFormat == meshVb.getVertexFormat() || destFormat == (meshVb.getVertexFormat() | (uint32) CVertexBuffer::PrimaryColorFlag) );
	dest.setVertexFormat(destFormat);
	dest.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
	dest.setNumVertices(ConstraintMeshBufSize * meshVb.getNumVertices());
	for(uint k = 0; k < CVertexBuffer::MaxStage; ++k)
	{
		dest.setUVRouting((uint8) k, meshVb.getUVRouting()[k]);
	}

	CVertexBufferReadWrite vba;
	dest.lock (vba);
	CVertexBufferRead vbaIn;
	meshVb.lock (vbaIn);

	uint8 *outPtr = (uint8 *) vba.getVertexCoordPointer();
	uint8 *inPtr = (uint8 *)  vbaIn.getVertexCoordPointer();
	uint  meshSize  = dest.getVertexSize() * meshVb.getNumVertices();

	if (destFormat == meshVb.getVertexFormat()) // no color added
	{
		for (uint k = 0; k < ConstraintMeshBufSize; ++k)
		{
			::memcpy((void *) (outPtr + k * meshSize), (void *) inPtr, meshSize);
		}
	}
	else // color added, but not available in src
	{
		sint colorOff = dest.getColorOff();
		uint inVSize    = meshVb.getVertexSize();
		uint outVSize   = dest.getVertexSize();
		for (uint k = 0; k < ConstraintMeshBufSize; ++k)
		{
			for (uint v = 0; v < meshVb.getNumVertices(); ++v)
			{
				// copy until color
				::memcpy((void *) (outPtr + k * meshSize + v * outVSize), (void *) (inPtr + v * inVSize), colorOff);
				// copy datas after color
				::memcpy((void *) (outPtr + k * meshSize + v * outVSize + colorOff + sizeof(uint8[4])), (void *) (inPtr + v * inVSize + colorOff), inVSize - colorOff);
			}
		}
	}
}


//=====================================================================================
CPSConstraintMesh::CGlobalTexAnim::CGlobalTexAnim() : TransOffset(NLMISC::CVector2f::Null),
													  TransSpeed(NLMISC::CVector2f::Null),
													  TransAccel(NLMISC::CVector2f::Null),
													  ScaleStart(1 ,1),
													  ScaleSpeed(NLMISC::CVector2f::Null),
													  ScaleAccel(NLMISC::CVector2f::Null),
													  WRotSpeed(0),
													  WRotAccel(0)
{
	NL_PS_FUNC(CGlobalTexAnim_CGlobalTexAnim)
}

//=====================================================================================
void	CPSConstraintMesh::CGlobalTexAnim::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CGlobalTexAnim_IStream )
	// version 1 : added offset
	sint ver = f.serialVersion(1);
	if (ver >= 1)
	{
		f.serial(TransOffset);
	}
	f.serial(TransSpeed, TransAccel, ScaleStart, ScaleSpeed, ScaleAccel);
	f.serial(WRotSpeed, WRotAccel);
}

//=====================================================================================
void CPSConstraintMesh::CGlobalTexAnim::buildMatrix(float date, NLMISC::CMatrix &dest)
{
	NL_PS_FUNC(CGlobalTexAnim_buildMatrix)
	float fDate = (float) date;
	float halfDateSquared   = 0.5f * fDate * fDate;
	NLMISC::CVector2f pos   = fDate * TransSpeed + halfDateSquared * fDate * TransAccel + TransOffset;
	NLMISC::CVector2f scale = ScaleStart + fDate * ScaleSpeed + halfDateSquared * fDate * ScaleAccel;
	float rot = fDate * WRotSpeed + halfDateSquared * WRotAccel;


	float fCos, fSin;
	if (rot != 0.f)
	{
		fCos = ::cosf(- rot);
		fSin = ::sinf(- rot);
	}
	else
	{
		fCos = 1.f;
		fSin = 0.f;
	}

	NLMISC::CVector I(fCos, fSin, 0);
	NLMISC::CVector J(-fSin, fCos, 0);
	dest.setRot(scale.x * I, scale.y * J, NLMISC::CVector::K);
	NLMISC::CVector center(-0.5f, -0.5f, 0.f);
	NLMISC::CVector t(pos.x, pos.y, 0);
	dest.setPos(t + dest.mulVector(center) - center);
}

//=====================================================================================
void	CPSConstraintMesh::setGlobalTexAnim(uint stage, const CGlobalTexAnim &properties)
{
	NL_PS_FUNC(CPSConstraintMesh_setGlobalTexAnim)
	nlassert(_GlobalAnimationEnabled != 0);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(_GlobalTexAnims.get());
	_GlobalTexAnims->Anims[stage] = properties;
}

//=====================================================================================
const CPSConstraintMesh::CGlobalTexAnim &CPSConstraintMesh::getGlobalTexAnim(uint stage) const
{
	NL_PS_FUNC(CPSConstraintMesh_getGlobalTexAnim)
	nlassert(_GlobalAnimationEnabled != 0);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(_GlobalTexAnims.get());
	return _GlobalTexAnims->Anims[stage];
}


//=====================================================================================
CPSConstraintMesh::TTexAnimType CPSConstraintMesh::getTexAnimType() const
{
	NL_PS_FUNC(CPSConstraintMesh_getTexAnimType)
	return (TTexAnimType) (_GlobalAnimationEnabled != 0 ? GlobalAnim : NoAnim);
}

//=====================================================================================
void  CPSConstraintMesh::setTexAnimType(TTexAnimType type)
{
	NL_PS_FUNC(CPSConstraintMesh_setTexAnimType)
	nlassert(type < Last);
	if (type == getTexAnimType()) return; // does the type of animation change ?
	switch (type)
	{
		case NoAnim:
			_GlobalTexAnims.reset();
			restoreMaterials();
			_GlobalAnimationEnabled = 0;
		break;
		case GlobalAnim:
		{
			PGlobalTexAnims newPtr(new CGlobalTexAnims);
			//std::swap(_GlobalTexAnims, newPtr);
			_GlobalTexAnims = CUniquePtrMove(newPtr);
			_GlobalAnimationEnabled = 1;
		}
		break;
		default: break;
	}
}

//=====================================================================================
void	CPSConstraintMesh::CGlobalTexAnims::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CGlobalTexAnims_IStream )
	f.serialVersion(0);
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		f.serial(Anims[k]);
	}
}

//=====================================================================================
void CPSConstraintMesh::restoreMaterials()
{
	NL_PS_FUNC(CPSConstraintMesh_restoreMaterials)
	update();
	CMeshDisplay  &md= _MeshDisplayShare.getMeshDisplay(_Meshes[0], getMeshVB(0), _Meshes[0]->getVertexBuffer().getVertexFormat() | (_ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));
	TRdrPassSet rdrPasses = md.RdrPasses;
	// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin(); rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{
		rdrPassIt->Mat = rdrPassIt->SourceMat;
	}
}

//=====================================================================================
const CVertexBuffer &CPSConstraintMesh::getMeshVB(uint index)
{
	nlassert(!_Touched);
	nlassert(index < _Meshes.size());
	nlassert(_MeshVertexBuffers.size() == _Meshes.size());
	const CVertexBuffer *vb = _MeshVertexBuffers[index];
	if (!vb )
	{
		CMesh &mesh	= * NLMISC::safe_cast<CMesh *>((IShape *) _Meshes[index]);
		vb = _MeshVertexBuffers[index] = &mesh.getVertexBuffer();
	}
	if (vb->getLocation() != CVertexBuffer::NotResident)
	{
		TMeshName2RamVB::iterator it = _MeshRamVBs.find(_MeshShapeFileName[index]);
		if (it == _MeshRamVBs.end())
		{
			CVertexBuffer &destVb = _MeshRamVBs[_MeshShapeFileName[index]];
			CMesh &mesh	= * NLMISC::safe_cast<CMesh *>((IShape *) _Meshes[index]);
			mesh.getVertexBuffer().copyVertices(destVb);
			_MeshVertexBuffers[index] = vb = &destVb;
		}
		else
		{
			_MeshVertexBuffers[index] = vb = &it->second;
		}
	}
	nlassert(vb->getLocation() == CVertexBuffer::NotResident);
	return *vb;
}

//=====================================================================================
void CPSMesh::onShow(bool shown)
{
	for(uint k = 0; k < _Instances.getSize(); ++k)
	{
		if (_Instances[k])
		{
			if (shown)
			{
				_Instances[k]->show();
			}
			else
			{
				_Instances[k]->hide();
			}
		}
	}
}

} // NL3D
