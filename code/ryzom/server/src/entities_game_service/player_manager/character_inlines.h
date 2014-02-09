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


#ifndef CHARACTER_INLINES_H
#define CHARACTER_INLINES_H


//------------------------------------------------------------------------------

inline uint16 CCharacter::getCurrentVersion()
{
	return 69; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getSavedVersion()
{
	return _SavedVersion; 
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId& CCharacter::getCharId() const
{
	return CEntityBase::getId(); 
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId& CCharacter::getId() const
{
	return CEntityBase::getId(); 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getStartupInstance()				
{
	return _StartupInstance; 
}

//------------------------------------------------------------------------------

inline CHARACTER_TITLE::ECharacterTitle CCharacter::getTitle() const
{
	return _Title; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::getEnterFlag() const
{
	return _Enter; 
}

//------------------------------------------------------------------------------

//inline CEntityState& CCharacter::getState()
//{
//	return CEntityBase::getState(); 
//}
//
//inline const CEntityState& CCharacter::getState() const
//{
//	return CEntityBase::getState(); 
//}

//------------------------------------------------------------------------------

inline bool CCharacter::isDead() const
{
	return CEntityBase::isDead();
}	

//------------------------------------------------------------------------------

inline const std::set<NLMISC::CSheetId> &CCharacter::getKnownBricks() const
{
	return _KnownBricks; 
}

//------------------------------------------------------------------------------

inline CMirrorPropValueAlice< SPropVisualA, CPropLocationPacked<2> >& CCharacter::getVisualPropertyA()
{
	return _VisualPropertyA; 
}

//------------------------------------------------------------------------------

inline CMirrorPropValueAlice< SPropVisualB, CPropLocationPacked<2> >& CCharacter::getVisualPropertyB()
{
	return _VisualPropertyB; 
}

//------------------------------------------------------------------------------

inline CMirrorPropValueAlice< SPropVisualC, CPropLocationPacked<2> >& CCharacter::getVisualPropertyC()
{
	return _VisualPropertyC; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getTeamId() const
{
	return _TeamId;
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId & CCharacter::getTeamInvitor() const
{
	return _TeamInvitor; 
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId & CCharacter::getLeagueInvitor() const
{
	return _LeagueInvitor; 
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId &CCharacter::harvestedEntity() const
{
	return _MpSourceId; 
}

//------------------------------------------------------------------------------

inline const NLMISC::CSheetId &CCharacter::harvestedEntitySheetId() const
{
	return _MpSourceSheetId; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::harvestedMpIndex() const
{
	return _MpIndex; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::harvestedMpQuantity() const
{
	return _HarvestedQuantity; 
}

//------------------------------------------------------------------------------

inline HARVEST_INFOS::CHarvestInfos& CCharacter::getHarvestInfos()
{
	return  _DepositHarvestInformation;
}

//------------------------------------------------------------------------------

inline CForageProgress *CCharacter::forageProgress()
{ 
	return _ForageProgress;
}

//------------------------------------------------------------------------------

inline CSEffectPtr CCharacter::getProspectionLocateDepositEffect() const
{ 
	return _ProspectionLocateDepositEffect;
}

//------------------------------------------------------------------------------

inline NLMISC::TGameCycle	CCharacter::forageBonusExtractionTime() const 
{ 
	return _ForageBonusExtractionTime;
}

//------------------------------------------------------------------------------

inline NLMISC::TGameTime& CCharacter::getTimeOfDeath()
{
	return _TimeDeath;
}

//------------------------------------------------------------------------------

inline bool CCharacter::isNearPetTpIsAllowed() const
{
	return NearPetTpAllowed; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::getNbPact()
{
	return (uint8)_Pact.size(); 
}

//------------------------------------------------------------------------------

inline bool CCharacter::isExchanging() const
{
	return (_ExchangeView != NULL); 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::getBotChatType()const
{
	return _CurrentBotChatType; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::getCurrentBotChatListPage()
{
	return _CurrentBotChatListPage;
} 

//------------------------------------------------------------------------------

inline const SGameCoordinate& CCharacter::getTpCoordinate() const
{
	return _TpCoordinate; 
}

//------------------------------------------------------------------------------

inline const uint64 & CCharacter::getMoney()
{
	return _Money; 
}

//------------------------------------------------------------------------------

inline const NLMISC::CEntityId & CCharacter::getCurrentInterlocutor()
{
	return _CurrentInterlocutor; 
}

//------------------------------------------------------------------------------

inline CExchangeView * CCharacter::getExchangeView()
{
	return _ExchangeView; 
}

//------------------------------------------------------------------------------

inline const uint64 & CCharacter::getExchangeMoney() const
{
	return _ExchangeMoney; 
}

//------------------------------------------------------------------------------

inline CInventoryPtr CCharacter::getLootContainer()
{
	return _LootContainer;
}

//------------------------------------------------------------------------------

inline bool CCharacter::staticActionInProgress() const
{
	return _StaticActionInProgress;
}	

//------------------------------------------------------------------------------

inline uint32 CCharacter::getSaveDate()
{
	return _SaveDate; 
}

//------------------------------------------------------------------------------

inline const std::map< TAIAlias, TMissionHistory >& CCharacter::getMissionHistories()
{
	return _MissionHistories; 
}

//------------------------------------------------------------------------------

inline const std::vector<SBotChatMission> & CCharacter::getCurrentMissionList()
{
	return _CurrentMissionList; 
}

//------------------------------------------------------------------------------

inline std::vector<CTradePhrase> & CCharacter::currentPhrasesTradeList()
{
	return _CurrentPhrasesTradeList; 
}

//------------------------------------------------------------------------------

inline RM_FABER_TYPE::TRMFType CCharacter::getRawMaterialItemPartFilter() const
{
	return _RawMaterialItemPartFilter; 
}

//------------------------------------------------------------------------------

inline ITEM_TYPE::TItemType CCharacter::getItemTypeFilter() const
{
	return _ItemTypeFilter; 
}

//------------------------------------------------------------------------------

inline RM_CLASS_TYPE::TRMClassType CCharacter::getMinClassItemFilter() const
{
	return _MinClass; 
}

//------------------------------------------------------------------------------

inline RM_CLASS_TYPE::TRMClassType CCharacter::getMaxClassItemFilter() const
{
	return _MaxClass; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getMinQualityFilter() const
{
	return _MinQualityFilter; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getMaxQualityFilter() const
{
	return _MaxQualityFilter; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getMinPriceFilter() const
{
	return _MinPriceFilter; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getMaxPriceFilter() const
{
	return _MaxPriceFilter; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getCurrentTradeSession() const
{
	return _CurrentTradeSession;
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::trainMaxSize() const
{
	return _TrainMaxSize; 
}

//------------------------------------------------------------------------------

inline const std::vector<TDataSetRow> &CCharacter::beastTrain() const
{
	return _BeastTrain; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::actionCounter() const
{
	return _ActionCounter; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::interfaceCounter() const
{
	return _InterfaceCounter; 
}

//------------------------------------------------------------------------------

inline CONTINENT::TContinent CCharacter::getCurrentContinent()
{
	return _CurrentContinent; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getCurrentRegion()
{
	return _CurrentRegion; 
}

//------------------------------------------------------------------------------

inline const std::vector<uint16> & CCharacter::getPlaces()
{ 
	return _Places; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getCurrentStable()
{
	return _CurrentStable; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::meleeCombatIsValid() const
{
	return _MeleeCombatIsValid; 
}

//------------------------------------------------------------------------------

inline const std::vector<CFaberMsgItem> &CCharacter::getFaberRms()
{
	return _RmSelectedForFaber; 
}

//------------------------------------------------------------------------------

inline const std::vector<CFaberMsgItem> &CCharacter::getFaberRmsFormula()
{
	return _RmFormulaSelectedForFaber; 
}

//------------------------------------------------------------------------------

inline NLMISC::CSheetId CCharacter::getCraftPlan() const
{
	return _CraftPlan; 
}

//------------------------------------------------------------------------------

inline std::vector<CFaberMsgItem> &CCharacter::getFaberRmsNoConst()
{
	return _RmSelectedForFaber; 
}

//------------------------------------------------------------------------------

inline std::vector<CFaberMsgItem> &CCharacter::getFaberRmsFormulaNoConst()
{
	return _RmFormulaSelectedForFaber; 
}

//------------------------------------------------------------------------------

inline const std::vector<CKnownPhrase>& CCharacter::getKnownPhrases()
{
	return _KnownPhrases; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::cycleCounter() const
{
	return _CycleCounter; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::nextCounter() const
{
	return _NextCounter; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::dodgeAsDefense() const
{
	return _DodgeAsDefense; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::parrySuccessModifier() const
{ 
	return _ParrySuccessModifier;
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::dodgeSuccessModifier() const
{ 
	return _DodgeSuccessModifier;
}


//------------------------------------------------------------------------------

inline sint32 CCharacter::craftSuccessModifier() const
{
	return _CraftSuccessModifier;
}


//------------------------------------------------------------------------------

inline sint32 CCharacter::meleeSuccessModifier() const
{
	return _MeleeSuccessModifier;
}


//------------------------------------------------------------------------------

inline sint32 CCharacter::rangeSuccessModifier() const
{
	return _RangeSuccessModifier;
}


//------------------------------------------------------------------------------

inline sint32 CCharacter::magicSuccessModifier() const
{
	return _MagicSuccessModifier;
}


//------------------------------------------------------------------------------

inline sint32 CCharacter::forageSuccessModifier( ECOSYSTEM::EECosystem eco) const
{
	if ( eco != ECOSYSTEM::unknown )
	{
		if( _ForageSuccessModifiers[(uint8)eco] == 0 )
		{
			return _ForageSuccessModifiers[(uint8)ECOSYSTEM::common_ecosystem];
		}
		else
		{
			return _ForageSuccessModifiers[(uint8)eco];
		}
	}
	return 0;
}

//------------------------------------------------------------------------------

inline SLOT_EQUIPMENT::TSlotEquipment CCharacter::protectedSlot() const
{
	return _ProtectedSlot; 
}

//------------------------------------------------------------------------------

inline NLMISC::TGameCycle CCharacter::dateOfNextAllowedAction()
{
	return _DateOfNextAllowedAction; 
}

//------------------------------------------------------------------------------

inline NLMISC::TGameCycle CCharacter::getForbidAuraUseEndDate() const  
{ 
	return _ForbidAuraUseEndDate; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::nbAuras() const
{
	return _NbAuras; 
}

//------------------------------------------------------------------------------

//inline uint32 CCharacter::getCombatEventFlags() const
//{
//	return _ActiveCombatEventFlags;
//}

//------------------------------------------------------------------------------
inline bool CCharacter::isCombatEventFlagActive(BRICK_FLAGS::TBrickFlag flag) const
{
	if( flag < 32 )
	{
		return (_CombatEventFlagTicks[flag].EndTick != 0);
	}
	else
	{
		nlwarning("<CCharacter::isCombatEventFlagActive> flag %d is not a combat event, it should be <32",flag);
		return false;
	}
}

//------------------------------------------------------------------------------

inline float CCharacter::wearMalus()
{
	return _WearEquipmentMalus; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::adversaryDodgeModifier()
{
	return _AdversaryDodgeModifier; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::adversaryParryModifier()
{
	return _AdversaryParryModifier; 
}

//------------------------------------------------------------------------------

inline const std::vector< CPetAnimal >& CCharacter::getPlayerPets()
{
	return _PlayerPets; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getLastPosXInDB() const
{
	return _LastPosXInDB; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getLastPosYInDB() const
{
	return _LastPosYInDB; 
}

//------------------------------------------------------------------------------

inline CSkills& CCharacter::getSkills()
{
	return _Skills; 
}

//------------------------------------------------------------------------------

inline SKILLS::ESkills CCharacter::getBestSkill() const
{
	return _BestSkill; 
}

//------------------------------------------------------------------------------

inline SKILLS::ESkills CCharacter::getSkillUsedForDodge() const
{
	return _SkillUsedForDodge; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getCurrentDodgeLevel() const
{
	return _CurrentDodgeLevel; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getCurrentParryLevel() const
{
	return _CurrentParryLevel; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getBaseDodgeLevel() const
{
	return _BaseDodgeLevel; 
}

//------------------------------------------------------------------------------

inline sint32 CCharacter::getBaseParryLevel() const
{
	return _BaseParryLevel; 
}

//------------------------------------------------------------------------------

inline NLMISC::TGameCycle CCharacter::getIntangibleEndDate() const
{
	return _IntangibleEndDate; 
}

//------------------------------------------------------------------------------

inline const uint64 &CCharacter::whoSeesMeBeforeTP() const
{
	return _WhoSeesMeBeforeTP; 
}

//------------------------------------------------------------------------------

inline const TDataSetRow& CCharacter::getMonitoringCSR()
{
	return _MonitoringCSR; 
}

//------------------------------------------------------------------------------

inline float CCharacter::nextDeathPenaltyFactor() const
{
	return _NextDeathPenaltyFactor; 
}

//------------------------------------------------------------------------------

inline std::vector<NLMISC::CSheetId> & CCharacter::getPersistentItemServices()
{
	return _PersistentItemServices; 
}

//------------------------------------------------------------------------------

inline EGSPD::CFameContainerPD &CCharacter::getPlayerFamesContainer()
{
	return *_Fames; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::logXpGain() const
{
	return _LogXpGain; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::getAggroCount()
{
	return _AggroCount; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getGuildId() const
{
	return _GuildId;
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getTpTicketSlot() const
{
	return _TpTicketSlot; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getBuildingExitZone() const
{
	return _BuildingExitZone;
}

//------------------------------------------------------------------------------

inline bool CCharacter::getHairCutDiscount() const
{
	return _HairCuteDiscount; 
}

//------------------------------------------------------------------------------

inline const std::vector<uint32> &CCharacter::getMissionQueues() const
{
	return _MissionsQueues; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getEnterCriticalZoneProposalQueueId() const
{
	return _EnterCriticalZoneProposalQueueId; 
}

//------------------------------------------------------------------------------

inline uint8 CCharacter::getNbNonNullClassificationTypesSkillMod() const
{
	return _NbNonNullClassificationTypesSkillMod; 
}

//------------------------------------------------------------------------------

inline CPVPInterface &CCharacter::getPVPInterface()
{
	return *_PVPInterface; 
}

//------------------------------------------------------------------------------

inline const CPVPInterface & CCharacter::getPVPInterface() const
{
	return *_PVPInterface; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::priviledgePVP()
{
	return _PriviledgePvp; 
}

//------------------------------------------------------------------------------

inline TAIAlias CCharacter::getCurrentPVPZone() const
{
	return _CurrentPVPZone; 
}

//------------------------------------------------------------------------------

inline TAIAlias CCharacter::getCurrentOutpostZone() const
{
	return _CurrentOutpostZone; 
}

//------------------------------------------------------------------------------

inline uint16 CCharacter::getKilledPvPRegion()
{
	return _RegionKilledInPvp; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::getSafeInPvPSafeZone() const
{
	return _PvPSafeZoneActive; 
}

//------------------------------------------------------------------------------

inline CCharacter * CCharacter::getDuelOpponent() const
{
	return _DuelOpponent; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getFirstConnectedTime() const
{
	return _FirstConnectedTime; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getLastConnectedTime() const
{
	return _LastConnectedTime; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getLastConnectedDate() const
{
	return _LastConnectedDate; 
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getPlayedTime() const
{
	return _PlayedTime; 
}

//------------------------------------------------------------------------------
inline const std::string& CCharacter::getLangChannel() const

{
	return _LangChannel; 
}

//------------------------------------------------------------------------------
inline const std::string& CCharacter::getNewTitle() const

{
	return _NewTitle;
}

//------------------------------------------------------------------------------
inline std::string CCharacter::getTagA() const

{
	if (_TagA.empty())
		return "_";
	return _TagA;
}

//------------------------------------------------------------------------------
inline std::string CCharacter::getTagB() const

{
	if (_TagB.empty())
		return "_";
	return _TagB;
}


//------------------------------------------------------------------------------
inline std::string CCharacter::getTagPvPA() const

{
	if (_TagPvPA.empty())
		return "_";
	return _TagPvPA;
}

//------------------------------------------------------------------------------
inline std::string CCharacter::getTagPvPB() const

{
	if (_TagPvPB.empty())
		return "_";
	return _TagPvPB;
}


//------------------------------------------------------------------------------
inline std::string CCharacter::getFullTitle() const
{
	if (!_TagA.empty() || !_TagB.empty() || !_TagPvPA.empty() || !_TagPvPB.empty())
		return _NewTitle+"#"+getTagPvPA()+"#"+getTagPvPB()+"#"+getTagA()+"#"+getTagB();
	else
		return _NewTitle;
}

//------------------------------------------------------------------------------

inline uint32 CCharacter::getOrganization() const
{
	return _Organization; 
}


inline uint32 CCharacter::getOrganizationStatus() const
{
	return _OrganizationStatus; 
}


//------------------------------------------------------------------------------

inline const std::list<TCharacterLogTime>& CCharacter::getLastLogStats() const
{
	return _LastLogStats; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::isChannelAdded() const
{
	return _ChannelAdded; 
}

//------------------------------------------------------------------------------

inline bool CCharacter::showFactionChannelsMode(TChanID channel) const
{
	std::map<TChanID, bool>::const_iterator it = _FactionChannelsMode.find(channel);
	if (it != _FactionChannelsMode.end())
		return (*it).second;
	else
		return false;
}

//------------------------------------------------------------------------------

inline CFarPositionStack& CCharacter::getPositionStack()
{
	return PositionStack; 
}

//------------------------------------------------------------------------------

inline const CFarPositionStack& CCharacter::getPositionStack() const
{
	return PositionStack; 
}

//------------------------------------------------------------------------------

inline R2::TUserRole CCharacter::sessionUserRole() const
{
	return _SessionUserRole; 
}

//------------------------------------------------------------------------------

inline TSessionId CCharacter::sessionId() const
{
	return _SessionId; 
}

//------------------------------------------------------------------------------

inline TSessionId CCharacter::currentSessionId() const
{
	return _CurrentSessionId; 
}

// Store the current active animation session returned by SU after char synchronisation.
inline void CCharacter::setActiveAnimSessionId(TSessionId activeAnimSessionId)
{
	_ActiveAnimSessionId = activeAnimSessionId;
}
// read the current active animation session returned by SU after char synchronisation.
inline TSessionId CCharacter::getActiveAnimSessionId()
{
	return _ActiveAnimSessionId;
}

//------------------------------------------------------------------------------

inline TAIAlias CCharacter::getSelectedOutpost() const
{
	return _SelectedOutpost; 
}

//------------------------------------------------------------------------------

//inline CCharacter::CCharacterDbReminder* CCharacter::getDataIndexReminder()
//{
//	return _DataIndexReminder; 
//}

//------------------------------------------------------------------------------

#endif // CHARACTER_INLINES_H
