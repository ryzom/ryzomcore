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

#ifndef RYAI_NF_HELPERS_H
#define RYAI_NF_HELPERS_H

extern AITYPES::CPropertySet readSet(std::string strings, std::string separator = "|");

extern AIVM::IScriptContext* spawnNewGroup(CStateInstance* entity, AIVM::CScriptStack& stack, CAIInstance* aiInstance, CAIVector const& spawnPosition, sint32 baseLevel, double dispersionRadius);
extern void getZoneWithFlags_helper(CStateInstance* entity, AIVM::CScriptStack& stack, CAIInstance* const aiInstance, CZoneScorer const& scorer);

//----------------------------------------------------------------------------
class CZoneScorerMandatoryAndOneOfPlusExcept
: public CZoneScorer
{
public:
	CZoneScorerMandatoryAndOneOfPlusExcept(AITYPES::CPropertySet const& oneOfSet, AITYPES::CPropertySet const& mandatorySet, AITYPES::CPropertySet const& exceptSet, CNpcZone const* zone)
		: CZoneScorer()
		, _oneOfSet(oneOfSet)
		, _mandatorySet(mandatorySet)
		, _exceptSet(exceptSet)
		, _zone(zone)
	{
	}
	
	float getScore(CNpcZone const& zone) const
	{
		if (&zone==_zone)
			return -1.f;
		if (!zone.properties().containsPartOfNotStrict(_oneOfSet))
			return -1.f;
		if (!zone.properties().containsAllOf(_mandatorySet))
			return -1.f;
		if (zone.properties().containsPartOfStrict(_exceptSet))
			return -1.f;
		return zone.getFreeAreaScore();
	}
private:
	AITYPES::CPropertySet	_oneOfSet;
	AITYPES::CPropertySet	_mandatorySet;
	AITYPES::CPropertySet	_exceptSet;
	CNpcZone const*			_zone;
};

class CZoneScorerMandatoryAndOneOfAndDist
: public CZoneScorer
{
public:
	CZoneScorerMandatoryAndOneOfAndDist(AITYPES::CPropertySet const& oneOfSet, AITYPES::CPropertySet const& mandatorySet, AITYPES::CPropertySet const& exceptSet, CAIVector const& pos)
		: CZoneScorer()
		, _oneOfSet(oneOfSet)
		, _mandatorySet(mandatorySet)
		, _exceptSet(exceptSet)
		, _Pos(pos)
	{
	}
	
	float getScore(CNpcZone const& zone) const
	{
		if (!zone.properties().containsPartOfNotStrict(_oneOfSet))
			return	-1.f;
		if (!zone.properties().containsAllOf(_mandatorySet))
			return	-1.f;
		if (zone.properties().containsPartOfStrict(_exceptSet))
			return	-1.f;
		return 1.f/getDist(zone);
	}
	virtual float getParam(CNpcZone	const& zone) const { return getDist(zone); };
private:
	float getDist(CNpcZone const& zone) const
	{
		return (float)zone.midPos().quickDistTo(_Pos)+0.0001f;
	}
	AITYPES::CPropertySet	_oneOfSet;
	AITYPES::CPropertySet	_mandatorySet;
	AITYPES::CPropertySet	_exceptSet;
	CAIVector				_Pos;
};

class CZoneScorerMandatoryAndOneOfAndDistAndSpace
: public CZoneScorerMandatoryAndOneOfAndDist
{
public:
	CZoneScorerMandatoryAndOneOfAndDistAndSpace(AITYPES::CPropertySet const& oneOfSet, AITYPES::CPropertySet const& mandatorySet, AITYPES::CPropertySet const& exceptSet, CAIVector const& pos)
	: CZoneScorerMandatoryAndOneOfAndDist(oneOfSet, mandatorySet, exceptSet, pos)
	{
	}
	
	float getScore(CNpcZone const& zone) const
	{
		return (float)(zone.getFreeAreaScore()*CZoneScorerMandatoryAndOneOfAndDist::getScore(zone));
	}
};

//----------------------------------------------------------------------------
// Ugly but no time, to refactor ..

class CDoOnFamily
{
public:
	CDoOnFamily() { }
	virtual ~CDoOnFamily() { }
	virtual void doOnFamily(CFamilyBehavior *fb) const = 0;
	virtual void doOnCellZone(CCellZone *cz) const = 0;
};

extern bool doOnFamily(std::vector<std::string> const& args, CDoOnFamily* fam);

//----------------------------------------------------------------------------
class CDoOnFamilyCopyDynEnergy
:public	CDoOnFamily
{
public:
	CDoOnFamilyCopyDynEnergy(size_t indexSrc, size_t indexDest)
	{
		_IndexSrc = indexSrc;
		_IndexDest = indexDest;
		if (_IndexSrc>3 || _IndexDest>3)
			nlwarning("CDoOnFamilyCopyDynEnergy: index out of bounds (0-3)");
		nlassert(_IndexSrc<4 && _IndexDest<4);
	}
	
	void doOnFamily(CFamilyBehavior* fb) const
	{
		float value = fb->getModifier((uint32)_IndexSrc);
		fb->setModifier(value, (uint32)_IndexDest);
	}
	void doOnCellZone(CCellZone* cz) const { }
	
private:
	size_t _IndexSrc;
	size_t _IndexDest;
};

//----------------------------------------------------------------------------
class CDoOnFamilySetDynEnergy
:public	CDoOnFamily
{
public:
	CDoOnFamilySetDynEnergy	(size_t index, float value)
	{
		_index=index;
		_value=value;
	}
	
	void	doOnFamily(CFamilyBehavior	*fb)	const
	{
		if	(_value==-1)	//	not for affectation.
			return;
		
		if	(_index==~0)	//	all indexs ?
		{
			for	(uint32 nrjIndex=0;nrjIndex<4;nrjIndex++)
				fb->setModifier	(_value, nrjIndex);
			return;
		}
		fb->setModifier	(_value, (uint32)_index);
	}
	void doOnCellZone(CCellZone *cz) const { }
	
private:
	size_t _index;
	float _value;
};

#endif
