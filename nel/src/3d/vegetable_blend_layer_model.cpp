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

#include "nel/3d/vegetable_blend_layer_model.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/3d/vegetable_sort_block.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/misc/debug.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene.h"


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
void	CVegetableBlendLayerModel::registerBasic()
{
	CScene::registerModel(VegetableBlendLayerModelId, TransformId, CVegetableBlendLayerModel::creator);
}


// ***************************************************************************
CVegetableBlendLayerModel::CVegetableBlendLayerModel()
{
	VegetableManager= NULL;

	// The model must always be renderer in transparency pass only.
	setTransparency(true);
	setOpacity(false);

	// The model is of course renderable
	CTransform::setIsRenderable(true);
}


// ***************************************************************************
void	CVegetableBlendLayerModel::setWorldPos(const CVector &pos)
{
	// setup directly the local matrix.
	_LocalMatrix.setPos(pos);

	// setup directly the world matrix.
	_WorldMatrix.setPos(pos);

}


// ***************************************************************************
void	CVegetableBlendLayerModel::render(IDriver *driver)
{
	H_AUTO( NL3D_Vegetable_Render );

	nlassert(VegetableManager);

	if(SortBlocks.empty())
		return;

	// Setup VegetableManager renderState (like pre-setuped material)
	//==================
	VegetableManager->setupRenderStateForBlendLayerModel(driver);


	// Render SortBlocks of this layer
	//==================
	uint	rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT;

	// first time, activate the hard VB.
	bool	precVBHardMode= true;
	CVegetableVBAllocator	*vbAllocator= &VegetableManager->getVBAllocatorForRdrPassAndVBHardMode(rdrPass, 1);
	vbAllocator->activate();

	// profile
	CPrimitiveProfile	ppIn, ppOut;
	driver->profileRenderedPrimitives(ppIn, ppOut);
	uint	precNTriRdr= ppOut.NTriangles;

	// render from back to front the list setuped in CVegetableManager::render()
	for(uint i=0; i<SortBlocks.size();i++)
	{
		CVegetableSortBlock	*ptrSortBlock= SortBlocks[i];

		// change of VertexBuffer (soft / hard) if needed.
		if(ptrSortBlock->ZSortHardMode != precVBHardMode)
		{
			// setup new VB for hardMode.
			CVegetableVBAllocator	*vbAllocator= &VegetableManager->getVBAllocatorForRdrPassAndVBHardMode(rdrPass, ptrSortBlock->ZSortHardMode);
			vbAllocator->activate();
			// prec.
			precVBHardMode= ptrSortBlock->ZSortHardMode;
		}

		// render him. we are sure that size > 0, because tested before.
		driver->activeIndexBuffer(ptrSortBlock->_SortedTriangleArray);
		#ifdef NL_DEBUG
			if (ptrSortBlock->ZSortHardMode)
			{
				nlassert(ptrSortBlock->_SortedTriangleArray.getFormat() == CIndexBuffer::Indices16);
			}
			else
			{
				nlassert(ptrSortBlock->_SortedTriangleArray.getFormat() == CIndexBuffer::Indices32);
			}
		#endif
		driver->renderSimpleTriangles(
			ptrSortBlock->_SortedTriangleIndices[ptrSortBlock->_QuadrantId],
			ptrSortBlock->_NTriangles);
	}

	// add number of triangles rendered with vegetable manager.
	driver->profileRenderedPrimitives(ppIn, ppOut);
	VegetableManager->_NumVegetableFaceRendered+= ppOut.NTriangles-precNTriRdr;


	// refresh list now!
	// We must do it here, because if CVegetableManager::render() is no more called (eg: disabled),
	// then the blend layers models must do nothing.
	SortBlocks.clear();


	// Reset RenderState.
	//==================
	VegetableManager->exitRenderStateForBlendLayerModel(driver);

}


// ***************************************************************************
void	CVegetableBlendLayerModel::traverseRender()
{
	CRenderTrav		&rTrav= getOwnerScene()->getRenderTrav();
	render(rTrav.getDriver());
}


} // NL3D
