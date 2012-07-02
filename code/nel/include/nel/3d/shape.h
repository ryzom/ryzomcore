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

#ifndef NL_SHAPE_H
#define NL_SHAPE_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/aabbox.h"


namespace NL3D
{


using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CTransformShape;
class	IDriver;
class	CScene;
class	IMeshGeom;
class	CRenderTrav;

// ***************************************************************************
/**
 * The basic interface for shapes. A shape is a kind of instanciable mesh.
 * For simplicity, render() and clip() virtual method are provided, so majority of shape could be implemented by just
 * define those methods, and let createInstance() as default. But other complex shapes may be defined, by implement
 * a compatible model which will comunicate with them.
 *
 * Serialisation of a shape MUST be done with ISTREAM::serialPolyPtr.
 *
 * \b DERIVER \b RULES:
 *		- simple: just implement clip() and render(). The shape will be movable via CTransform.
 *		- complex: if special interaction is needed between the instance and the shape:
 *			- implement a special Model, derived from CTransformShape, adding your instance behavior.
 *			- implement YourShape::createInstance(), so it create this good model.
 *			- implement your own communication system between the model and the shape.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IShape : public NLMISC::CRefCount, public NLMISC::IStreamable
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:

	/// Constructor
	IShape();
	/// Dtor.
	virtual ~IShape() {}

	/** create an instance of this shape. The instance may be a CTransformShape, or a specialized version of it.
	 * The default behavior is to createModel() a CTransformShape, and just assign to it the Shape.
	 * \param scene the scene used to createModel().
	 * \return the specialized instance for this shape.
	 */
	virtual	CTransformShape		*createInstance(CScene &scene);

	/** clip this shape with a pyramid.
	 * the pyramid is given in world space.The world matrix of the object is given.
	 * \param pyramid the clipping polytope, planes are normalized.
	 * \param worldMatrix the world matrix of the instance.
	 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
	 */
	virtual bool				clip(const std::vector<CPlane>& /* pyramid */, const CMatrix &/* worldMatrix */) {return true;}

	/** render() this shape in a driver, with the specified TransformShape information.
	 * CTransfromShape call this method in the render traversal.
	 * if opaquePass render the opaque materials else render the transparent materials.
	 */
	virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass)=0;

	/** flush textures used by this shape.
	 */
	virtual void				flushTextures (IDriver &driver, uint selectedTexture)=0;

	/** return the bounding box of the shape. Default is to return Null bbox.
	 */
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const;

	/** return the DistMax where the shape is no more displayed.
	 *	Default is to return -1, meaning DistMax = infinite.
	 */
	float						getDistMax() const {return _DistMax;}

	/** setup the DistMax where the shape is no more displayed.
	 *  Take effect only for the next created instances.
	 *	setting <0 means -1 and so means DistMax = infinite.
	 */
	void						setDistMax(float distMax);

	/** Profiling. Called in RenderPass if Current Frame profiled. No-Op by default
	 *	Information must be added in rdrTrav->Scene
	 */
	virtual void				profileSceneRender(CRenderTrav * /* rdrTrav */, CTransformShape * /* trans */, bool /* opaquePass */) { }


	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance will render for a fixed distance.
	 *	return 0 if do not support degradation.
	 */
	virtual float				getNumTriangles (float distance)=0;

	// @}


	/// \name Lighting method
	// @{

	/** tells if the shape wants LocalAttenuation for RealTime lighting.  Default is false
	 */
	virtual bool				useLightingLocalAttenuation () const {return false;}

	// @}


	/// \name Mesh Block Render Interface
	// @{

	/** return !NULL if this shape can support MeshBlock rendering for a special instance.
	 *	NB: Mesh Block render cannot occurs if the Mesh is Skinned/MeshMorphed.
	 *	NB: Mesh Block render can occurs only in Opaque pass
	 *	NB: Mesh block render can occurs only for CMeshBase meshes.
	 *	\param trans the instance to take into account (meshMultiLod may return NULL in blend transition).
	 *	\param polygonCount the number of polygons to render for the meshGeom returned
	 *	\return the meshgeom to render per block if OK, else NULL (default)
	 */
	virtual IMeshGeom			*supportMeshBlockRendering (CTransformShape * /* trans */, float &/* polygonCount */ ) const {return NULL;}

	// @}

	/** Build a system copy of geometry, typically for fast intersection computation
	 *	NB: typically, this must be called just after load, because the vertexbuffer must not be resident
	 *	Supported only on a subset of Shape, and specially not the skinned one (not useful since not relevant)
	 */
	virtual void				buildSystemGeometry() {}

protected:

	/// Default to -1
	float			_DistMax;

};

// ***************************************************************************
/**
 * This class is used to serialize a shape. In reading, just create a CShapeStream object
 * and serial your class with your input stream using "serial (IStream&)". Then take back the shape pointer.
 * It is yours. In writing, create a CShapeStream object with a pointer on the IShape
 * you want to serialize and serial it with "serial (IStream&)". You have to register all
 * the IShape derived classes you want to serial.
 \see IShape CClassRegistry
 */
class CShapeStream
{
public:
	/// Default constructor. Set the IShape pointer to NULL.
	CShapeStream ();

	/** Constructor. Get a IShape pointer. Used to output serialization.
	 *  \param shape the pointer on the IShape derived object you want to serialize.
	 */
	CShapeStream (IShape* shape);

	virtual ~CShapeStream() {}

	/** Set the pointer to the IShape object. Used to serial a shape in output.
	 *  \param shape the pointer on the IShape derived object you want to serialize.
	 */
	void						setShapePointer (IShape* shape);

	/** Get the pointer to the IShape object. Used to serial a shape in input.
	 *  \return shape the pointer on the IShape derived object serialized.
	 */
	IShape*						getShapePointer () const;

	/// serial the shape.
	virtual void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	IShape*			_Shape;
};

} // NL3D


#endif // NL_SHAPE_H

/* End of shape.h */
