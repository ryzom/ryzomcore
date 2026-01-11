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


#include "std_afx.h"
#include "ps_initial_pos.h"
//
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_emitter.h"


// CPSInitialPos  implementation //
///////////////////////////////////
//******************************************************************************************************
void CPSInitialPos::reset()
{
	_InitInfoVect.clear();
	_RotScaleInfoVect.clear();
	_InitialSizeVect.clear();
}

//******************************************************************************************************
void CPSInitialPos::copySystemInitialPos(NL3D::CParticleSystem *ps)
{
	nlassert(!_PS);
	reset();
	uint32 nbLocated = ps->getNbProcess();

	_PS = ps; 
	for(uint32 k = 0; k < nbLocated; ++k)
	{

		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc)
		{

			_InitialSizeVect.push_back(std::make_pair(loc, loc->getSize()) );
			for (uint32 l = 0; l < loc->getSize(); ++l)
			{

				CInitPSInstanceInfo ii;
				ii.Index = l;
				ii.Loc = loc;
				ii.Pos = loc->getPos()[l];
				ii.Speed = loc->getSpeed()[l];
				_InitInfoVect.push_back(ii);
				
				for (uint32 m = 0; m < loc->getNbBoundObjects(); ++m)
				{

					if (dynamic_cast<NL3D::IPSMover *>(loc->getBoundObject(m)))
					{
						CRotScaleInfo rsi;
						rsi.Loc = loc;
						rsi.LB = loc->getBoundObject(m);
						rsi.Index = l;
						rsi.Psm = dynamic_cast<NL3D::IPSMover *>(loc->getBoundObject(m));
						rsi.Scale = rsi.Psm->getScale(l);											
						rsi.Rot = rsi.Psm->getMatrix(l);
						_RotScaleInfoVect.push_back(rsi);
					}
				}
			}
		}
	}	
}


// PRIVATE : a predicate used in CPSInitialPos::removeLocated
struct CRemoveLocatedPred
{
	NL3D::CPSLocated *Loc	;
};

// private : predicate to remove located from a CPSInitialPos::TInitialLocatedSizeVect vector
struct CRemoveLocatedFromLocatedSizePred : public CRemoveLocatedPred
{
	bool operator()(const std::pair<NL3D::CPSLocated *, uint32> &value) { return Loc == value.first; }
};

// private : predicate to remove located from a PSInitialPos::CInitPSInstanceInfo vector
struct CRemoveLocatedFromInitPSInstanceInfoVectPred : public CRemoveLocatedPred
{
	bool operator()(const CPSInitialPos::CInitPSInstanceInfo &value) { return value.Loc == Loc; }
};

// private : predicate to remove located from a PSInitialPos::CRotScaleInfo vector
struct CRemoveLocatedFromRotScaleInfoVectPred : public CRemoveLocatedPred
{
	bool operator()(const CPSInitialPos::CRotScaleInfo &value) { return value.Loc == Loc; }
};

// private : predicate to remove located bindable pointers in a TRotScaleInfoVect vect
struct CRemoveLocatedBindableFromRotScaleInfoVectPred
{
	// the located bindable taht has been removed
	NL3D::CPSLocatedBindable *LB;
	bool operator()(const CPSInitialPos::CRotScaleInfo &value) { return value.LB == LB; }
};

//******************************************************************************************************
void CPSInitialPos::removeLocated(NL3D::CPSLocated *loc)
{	
	// in each container, we delete every element that has a pointer over lthe located loc
	// , by using the dedicated predicate. 

	CRemoveLocatedFromLocatedSizePred p;
	p.Loc = loc;
	_InitialSizeVect.erase(std::remove_if(_InitialSizeVect.begin(), _InitialSizeVect.end(), p)
							, _InitialSizeVect.end() );

	CRemoveLocatedFromInitPSInstanceInfoVectPred p2;
	p2.Loc = loc;
	_InitInfoVect.erase(std::remove_if(_InitInfoVect.begin(), _InitInfoVect.end(), p2)
						, _InitInfoVect.end());

	CRemoveLocatedFromRotScaleInfoVectPred p3;
	p3.Loc = loc;
	_RotScaleInfoVect.erase(std::remove_if(_RotScaleInfoVect.begin(), _RotScaleInfoVect.end(), p3)
							 , _RotScaleInfoVect.end());

}

//******************************************************************************************************
void CPSInitialPos::removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
{	
	CRemoveLocatedBindableFromRotScaleInfoVectPred p;
	p.LB = lb;
	_RotScaleInfoVect.erase(std::remove_if(_RotScaleInfoVect.begin(), _RotScaleInfoVect.end(), p), _RotScaleInfoVect.end() );
}

//******************************************************************************************************
// reinitialize the system with its initial instances positions
void CPSInitialPos::restoreSystem()
{
	nlassert(_PS); // no system has been memorized yet
	_PS->stopSound();
	// delete all the instance of the system
	NL3D::CPSEmitter::setBypassEmitOnDeath(true);
	for (uint k = 0; k < _PS->getNbProcess(); ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(_PS->getProcess(k));
		if (loc)
		{
			while (loc->getSize())
			{
				loc->deleteElement(0);
			}

			nlassert(loc->getSize() == 0);
		}
	}	
	// recreate the initial number of instances
	for (TInitialLocatedSizeVect ::iterator itSize = _InitialSizeVect.begin(); itSize != _InitialSizeVect.end(); ++itSize)
	{
	//	nlassert(itSize->first->getSize() == 0)
		for (uint l = 0; l < itSize->second; ++l)
		{
			itSize->first->newElement(NLMISC::CVector::Null, NLMISC::CVector::Null, NULL, 0, itSize->first->getMatrixMode(), 0.f);
		}

		uint realSize = itSize->first->getSize();
		uint size = itSize->second;
		
	}
	NL3D::CPSEmitter::setBypassEmitOnDeath(false);
	for (TInitInfoVect::iterator it = _InitInfoVect.begin(); it != _InitInfoVect.end(); ++it)
	{
		if (it->Index < it->Loc->getSize())
		{
			it->Loc->getPos()[it->Index] = it->Pos;
			it->Loc->getSpeed()[it->Index] = it->Speed;
			// If the particle has parametric infos attach, restore them too
			if (it->Loc->isParametricMotionEnabled())
			{
				it->Loc->getParametricInfos()[it->Index].Pos = it->Pos;
				it->Loc->getParametricInfos()[it->Index].Speed = it->Speed;
				it->Loc->getParametricInfos()[it->Index].Date = 0.f;
			}			
		}
	}
	for (TRotScaleInfoVect::iterator it2 = _RotScaleInfoVect.begin(); it2 != _RotScaleInfoVect.end(); ++it2)
	{
		if (it2->Index < it2->Loc->getSize())
		{
			it2->Psm->setMatrix(it2->Index, it2->Rot);
			if (it2->Psm->supportNonUniformScaling())
			{
				it2->Psm->setScale(it2->Index, it2->Scale);
			}
			else if (it2->Psm->supportUniformScaling())
			{
				it2->Psm->setScale(it2->Index, it2->Scale.x);
			}
		}
	}
	_PS = NULL;
}
