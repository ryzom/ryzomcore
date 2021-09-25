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

#ifndef NL_LANDSCAPE_MODEL_H
#define NL_LANDSCAPE_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transform.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/landscape.h"


namespace NL3D
{

class	CLandscape;

// ***************************************************************************
const NLMISC::CClassId		LandscapeModelId=NLMISC::CClassId(0x5a573b55, 0x6b395829);


// ***************************************************************************
/**
 * The model for MOT. A landscape is not designed to move, but easier here.
 * If you translate/rotate this model, nothing happens. Landscape cannot move.
 * See CLandscape for more information on Landscape.
 * \see CLandscape.
 */
class	CLandscapeModel : public CTransform
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

public:
	CLandscape		Landscape;

	/** Set additive value
	  *
	  * \param additive new additive value. [0, 1]
	  */
	void			setAdditive (float additive)
	{
		_Additive=additive;
	}

	/** Get additive value
	  *
	  * \return the additive value. [0, 1]
	  */
	float			getAdditive () const
	{
		return _Additive;
	}

	/** Set additive set
	  *
	  * \param enable is true to activbe additive, false to disactive it.
	  */
	void			enableAdditive (bool enable)
	{
		_ActiveAdditive=enable;
	}

	/** Get additive set
	  *
	  * \return true to if additive is actived, else false.
	  */
	bool			isAdditive () const
	{
		return _ActiveAdditive;
	}

	/** if true, the refine Center is auto computed each frame from Camera Position. Else must be given by
	 *	setRefineCenterUser()
	 *	Default to true.
	 */
	void			setRefineCenterAuto(bool mode) {_RefineCenterAuto= mode;}
	bool			getRefineCenterAuto() const {return _RefineCenterAuto;}
	void			setRefineCenterUser(const CVector &refineCenter) {_RefineCenterUser= refineCenter;}
	const CVector	&getRefineCenterUser() const {return _RefineCenterUser;}


	/** Override CTransform::initModel(), to create CLandscape's VegetableManager's BlendLayer models in the scene.
	 */
	virtual void	initModel();
	/// special traverseHRC.
	virtual void	traverseHrc();
	/// special clip(). NB: the real landscape clip is done in traverseRender()
	virtual void	traverseClip();
	virtual bool	clip() {return true;}
	/// traverseRender()
	virtual void	traverseRender();
	/// Some prof infos
	virtual	void	profileRender();

	/// Actual Clip and Render!! See Implementation for Why this scheme
	void			clipAndRenderLandscape();


	/// \name ShadowMap Behavior. Receive only
	// @{
	virtual void		getReceiverBBox(CAABBox &bbox);
	virtual void		receiveShadowMap(CShadowMap *shadowMap, const CVector &casterPos, const CMaterial &shadowMat);
	virtual const CMatrix	&getReceiverRenderWorldMatrix() const {return _RenderWorldMatrix;}
	// @}

protected:
	CLandscapeModel();
	virtual ~CLandscapeModel() {}

private:
	static CTransform	*creator() {return new CLandscapeModel;}

	bool	_ActiveAdditive;
	float	_Additive;

	// The current small pyramid, for faster clip.
	CPlane					CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];

	// The current clustered pyramid. computed in clip(), and parsed in traverseRender()
	std::vector<CPlane>		ClusteredPyramid;

	// If ClusteredPyramid is already the WorldFrustumPyramid. NB: if false, it may still be actually
	bool					ClusteredPyramidIsFrustum;

	// This is The Last WorldMatrix used to render (not identity for ZBuffer considerations).
	CMatrix					_RenderWorldMatrix;

	//
	CVector					_RefineCenterUser;
	bool					_RefineCenterAuto;
};


} // NL3D


#endif // NL_LANDSCAPE_MODEL_H

/* End of landscape_model.h */
