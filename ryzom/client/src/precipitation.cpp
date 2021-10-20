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



#include "stdpch.h"
#include "precipitation.h"
#include "time_client.h"
//
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"
#include "nel/3d/u_particle_system_instance.h"

extern NL3D::UScene *Scene;


H_AUTO_DECL(RZ_Precipitation)

CPrecipitation::TClipGridMap CPrecipitation::_PrecipitationClipGripMap;

static const float DEFAUT_PRECIPITATION_TIMEOUT = 10.f; // at most 10 second before precipitations are removed


//============================================================
CPrecipitation::CPrecipitation() : _ClipGrid(NULL), _Strenght(0), _TimeOut(0.f), _XSize(0), _YSize(0), _OldX(0), _OldY(0), _Touched(false)
{
	H_AUTO_USE(RZ_Precipitation)
}

//============================================================
void CPrecipitation::init(const CPrecipitationDesc &desc)
{
	H_AUTO_USE(RZ_Precipitation)
	nlassert(desc.GridSize > 0);
	release();
	_Strenght = 0;
	_Desc = desc;
	if (!_Desc.ReleasableModel)
	{
		allocateFXs(); // keep fxs allocated;
		forceSetStrenght(0); // no precipitation is the default
	}
	_ClipGrid = NULL;
	_Touched = true;
}

//============================================================
void CPrecipitation::allocateFXs()
{
	H_AUTO_USE(RZ_Precipitation)
	if (!Scene) return;
	nlassert(_Blocks.empty()); // should be called only once
	_Blocks.resize(_Desc.GridSize * _Desc.GridSize);
	std::fill(_Blocks.begin(), _Blocks.end(), NL3D::UParticleSystemInstance ());
	for(uint k = 0; k < _Desc.GridSize * _Desc.GridSize; ++k)
	{
		_Blocks[k].cast (Scene->createInstance(_Desc.FxName.c_str()));
		if (_Blocks[k].empty())
		{
			nlwarning("can't find %s", _Desc.FxName.c_str());
			release();
			return;
		}
		_Blocks[k].forceInstanciate();
	}
	if (_Desc.UseBBoxSize)
	{
		NLMISC::CAABBox bbox;
		getBlock(0, 0).getShapeAABBox(bbox);
		_XSize = 2.f * bbox.getHalfSize().x;
		_YSize = 2.f * bbox.getHalfSize().y;
	}
	else
	{
		_XSize = _YSize = _Desc.Size;
	}

	// get the associated precipitation clipGrid if it hasn't yet
	/*
	if (!_ClipGrid)
	{
		NLMISC::CVector2f gridSize(_XSize, _YSize);
		TClipGridMap::iterator it = _PrecipitationClipGripMap.find(gridSize);
		if (it != _PrecipitationClipGripMap.end())
		{
			_ClipGrid = &it->second;
		}
		else
		{
			_ClipGrid = &(_PrecipitationClipGripMap[gridSize]);
			_ClipGrid->initGrid(_Desc.GridSize, _XSize, _YSize);
		}
	}
	*/
}

//============================================================
void CPrecipitation::deallocateFXs()
{
	H_AUTO_USE(RZ_Precipitation)
	if (Scene)
	{
		for(uint k = 0; k < _Blocks.size(); ++k)
		{
			Scene->deleteInstance(_Blocks[k]);
		}
		NLMISC::contReset(_Blocks);
	}
	_TimeOut = 0.f;
	_Touched = true;
}


//============================================================
void CPrecipitation::setStrenght(float strenght)
{
	H_AUTO_USE(RZ_Precipitation)
	if (strenght == _Strenght) return;
	forceSetStrenght(strenght);
}

//============================================================
void CPrecipitation::forceSetStrenght(float strenght)
{
	H_AUTO_USE(RZ_Precipitation)
	if (_Desc.ReleasableModel)
	{
		if (strenght > 0)
		{
			if (!areFXsAllocated())
			{
				allocateFXs();
			}
		}
	}
	 NLMISC::clamp(strenght, 0, 1);
	if (_Desc.GridSize == 0) return;
	if (!areFXsAllocated()) return;
	if (strenght <= 0.f)
	{
		if (_Strenght != 0.f)
		{
			_TimeOut = DEFAUT_PRECIPITATION_TIMEOUT;
		}
		for(uint k = 0; k < _Blocks.size(); ++k)
		{
			_Blocks[k].activateEmitters(false);
			_Blocks[k].setUserParam(0, strenght);
		}
	}
	else
	{
		_TimeOut = 0.f;
		for(uint k = 0; k < _Blocks.size(); ++k)
		{
			if (_Strenght == 0.f)
			{
				_Blocks[k].activateEmitters(true);
			}
			_Blocks[k].setUserParam(0, strenght);

		}
	}
	_Strenght = strenght;
}

//============================================================
void CPrecipitation::release()
{
	H_AUTO_USE(RZ_Precipitation)
	deallocateFXs();
}

//============================================================
bool CPrecipitation::isRunning() const
{
	H_AUTO_USE(RZ_Precipitation)
	if (!areFXsAllocated()) return false;
	// if the last particles have been removed, we can remove the models
	for(uint k = 0; k < _Blocks.size(); ++k)
	{
		if (_Blocks[k].hasParticles()) return true;
	}
	return false;
}

//============================================================
// snap a float by the given factor
static inline float fsnapf(float v, float mod)
{
	H_AUTO_USE(RZ_Precipitation)
	return mod * floorf(v / mod);
}

//============================================================
void CPrecipitation::update(const NLMISC::CMatrix &camMat, NLPACS::UGlobalRetriever * /* retriever */)
{
	H_AUTO_USE(RZ_Precipitation)
	if (_TimeOut != 0.f)
	{
		nlassert(_Strenght == 0.f);
		_TimeOut -= DT;
		if (_TimeOut <= 0.f)
		{
			_TimeOut = 0.f;
			deallocateFXs();
			return;
		}
	}
	if (_Desc.ReleasableModel)
	{
		if (_Strenght == 0.f && areFXsAllocated()) // no more precipitations & FXs are still allocated ? This means that there are particles left
		{
			if (!isRunning())
			{
				deallocateFXs();
				return;
			}
		}
	}
	if (!areFXsAllocated()) return;
	NLMISC::CVector userPos = camMat.getPos();
	//
	if (_XSize == 0 || _YSize == 0) return;

	// get world position of the user in grid units
	sint worldX = (sint) floorf(userPos.x / _XSize);
	sint worldY = (sint) floorf(userPos.y / _YSize);

	// See if we need to update pos
	if (_Touched || worldX != _OldX || worldY != _OldY)
	{
		_Touched = false;
		_OldX = worldX;
		_OldY = worldY;

		/*
		if (_ClipGrid)
		{
			_ClipGrid->updateGrid(userPos, retriever);
		}
		*/

		//
		// snap the user pos
		//
		NLMISC::CVector snappedPos(fsnapf(userPos.x, _XSize), fsnapf(userPos.y, _YSize), userPos.z);
		snappedPos -= NLMISC::CVector((float(_Desc.GridSize >> 1) - 0.5f) * _XSize, (float(_Desc.GridSize >> 1) - 0.5f) * _YSize, 0);
		// get world position of grid corner
//		sint gridCornerX = worldX - (_Desc.GridSize >> 1);
//		sint gridCornerY = worldY - (_Desc.GridSize >> 1);
		// move / show / hide each block
		for(uint k = 0; k < _Desc.GridSize; ++k)
		{
			NLMISC::CVector hPos = snappedPos;
			for(uint l = 0; l < _Desc.GridSize; ++l)
			{
				nlassert(!getBlock(l, k).empty());
				/*if (!_ClipGrid)
				{*/
					getBlock(l, k).setPos(hPos);
					hPos.x += _XSize;
				/*
				}
				else // use the clip grid to see if that block should be activated, and at what height it should be put
				{
					typedef CPrecipitationClipGrid::CGridPoint CGridPoint; // alias CGridPoint
					const CGridPoint *p0 = _ClipGrid->get(l + gridCornerX, k + gridCornerY);
					const CGridPoint *p1 = _ClipGrid->get(l + gridCornerX + 1, k + gridCornerY);
					const CGridPoint *p2 = _ClipGrid->get(l + gridCornerX + 1, k + gridCornerY + 1);
					const CGridPoint *p3 = _ClipGrid->get(l + gridCornerX, k + gridCornerY + 1);
					if (p0 && p1 && p2 && p3)
					{
						// if one of the 4 corners is in an interior, don't display the precipiations
						if (p0->Clipped || p1->Clipped || p2->Clipped || p3->Clipped)
						{
							getBlock(l, k).hide();
						}
						else
						{
							NL3D::UParticleSystemInstance psi = getBlock(l, k);
							psi.show();
							// take the mean height
							hPos.z = 0.25f * (p0->MeanHeight + p1->MeanHeight + p2->MeanHeight + p3->MeanHeight);
							psi.setPos(hPos);
						}
					}
					else
					{
						getBlock(l, k).setPos(hPos);
					}
					hPos.x += _XSize;
				}
				*/
			}
			snappedPos.y += _YSize;
		}
	}
}

//============================================================
void CPrecipitation::drawClipGrid(NL3D::UDriver &drv) const
{
	if (!_ClipGrid) return;
	_ClipGrid->display(drv);
}
