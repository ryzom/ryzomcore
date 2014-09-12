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



#ifndef NL_DISTANCE_PRIORITIZER_H
#define NL_DISTANCE_PRIORITIZER_H

#include "nel/misc/types_nl.h"
#include "fe_types.h"
#include "vision_array.h"
#include "processing_spreader.h"
#include "entity_container.h"
#include "history.h"
#include <vector>

extern TClientId verbosePropertiesSent;

#ifdef NL_RELEASE
#define LOG_WHAT_IS_SENT ;
#else
#define LOG_WHAT_IS_SENT if ( (verbosePropertiesSent!=0) && (verbosePropertiesSent!=TVPNodeServer::PrioContext.ClientId) ) {} else nldebug
#endif


/**
 * Comparison functor
 * Helps sorting in decreasing order the entities seen by a client, by priority
 */
struct TComparePairsByPriority
{
	//TComparePairsByPriority( CVisionArray *va, TClientId clientId ) : _VisionArray(va), _ClientId(clientId) {}	// CHANGED BEN
	TComparePairsByPriority( CVisionArray *va, TClientId clientId )
	{
		_ClientStateArray = va->getClientStateArray(clientId);
	}


	bool	operator() ( CLFECOMMON::TCLEntityId first, CLFECOMMON::TCLEntityId second )
	{
		//return ( _VisionArray->getPairState( _ClientId, first ).Priority > _VisionArray->getPairState( _ClientId, second ).Priority );	// CHANGED BEN
		return _ClientStateArray[first].getPrio() > _ClientStateArray[second].getPrio();
	}

	//CVisionArray	*_VisionArray;
	//TClientId		_ClientId;

	TPairState*		_ClientStateArray;
};


class CHistory;
class CClientHost;
class CVisionProvider;


void		fillSHEET( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillBEHAVIOUR( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillNAME_STRING_ID( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillTARGET_LIST( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillVISUAL_FX( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillBARS( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillVisualPropertyABC( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillCONTEXTUAL( TOutBox& outbox, CLFECOMMON::TPropIndex propIndex );
void		fillMODE( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillGUILD_SYMBOL( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillGUILD_NAME_ID( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillEVENT_FACTION_ID( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillPVP_MODE( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillPVP_CLAN( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillOWNER_PEOPLE( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillOUTPOST_INFOS( TOutBox& outbox, CLFECOMMON::TPropIndex );
void		fillRowProperty( TOutBox& outbox, CLFECOMMON::TPropIndex propIndex );


class CDistancePrioritizer;

typedef void (*TFillFunc) (TOutBox&, CLFECOMMON::TPropIndex);

/*
 * Visual property tree node
 */
class TVPNodeServer : public CLFECOMMON::TVPNodeBase
{
public:
	struct TPrioContext
	{
		CDistancePrioritizer					*Prioritizer;
		CClientHost								*ClientHost;
		TClientId								ClientId;
		CLFECOMMON::TCLEntityId					Slot;
		CEntity									*Sentity;
		TEntityIndex							EntityIndex;
		//TPairState								*PairState;
		CLFECOMMON::TCoord						DistanceCE;
		NLMISC::TGameCycle						Timestamp;
		bool									IsTarget;
		bool									PositionAlreadySent;
		sint32									ZCache;
	};

	// Static data
	static TPrioContext							PrioContext;

	TFillFunc									FillFunc;

	/// Flattened Node Tree (speed up server VP tree accesses)
	static TVPNodeServer*						FlatVPTree[CLFECOMMON::NB_VISUAL_PROPERTIES];

	/*
	 * propagateBackBranchHasPayload used to walk through VPNode tree build.
	 * Yet, this tree is invariable, so scanning processing is invariable too.
	 * So I build a simple array of all nodes to be examined (all nodes except leaves.)
	 */

	static const bool							FalseBoolPayLoad;

	struct CSortedFlatVPTreeItem
	{
		CSortedFlatVPTreeItem() : Node(NULL), APayLoad(&FalseBoolPayLoad), BPayLoad(&FalseBoolPayLoad)		{ }

		TVPNodeServer*		Node;
		const bool*			APayLoad;
		const bool*			BPayLoad;

	};

	struct CSortedFlatVPTreeFillItem
	{
		CSortedFlatVPTreeFillItem() : Node(NULL), NextIfNoPayload(0)	{ }

		TVPNodeServer*		Node;
		uint				NextIfNoPayload;
	};

	/// Reordered Node tree (used by fastPropagateBackBranchHasPayload())
	static std::vector<CSortedFlatVPTreeItem>	SortedFlatVPTree;

	static void		fastPropagateBackBranchHasPayload()
	{
		std::vector<CSortedFlatVPTreeItem>::iterator	it;
		for (it=SortedFlatVPTree.begin(); it!=SortedFlatVPTree.end(); ++it)
			(*it).Node->BranchHasPayload = ( *((*it).APayLoad) || *((*it).BPayLoad) );
	}

	/// Reordered Node tree (used by fastFillDiscreetProperties)
	static std::vector<CSortedFlatVPTreeFillItem>	SortedFlatVPTreeFill;

	static void		fastFillDiscreetProperties(TOutBox& outbox)
	{
		uint	i = 0, size = (uint)SortedFlatVPTreeFill.size();
		while (i < size)
		{
			CSortedFlatVPTreeFillItem&	item = SortedFlatVPTreeFill[i++];
			TVPNodeServer*				node = item.Node;

			//nlinfo("examine node %p:%d", node, node->PropIndex);

			if (!node->isLeaf())
			{
				outbox.serialBitAndLog( node->BranchHasPayload );
				if (!node->BranchHasPayload)
					i = item.NextIfNoPayload;
			}
			else
			{
				// The payload bit depends on the success of fillDiscreetProperty, so it is serialized inside
				node->FillFunc( outbox, node->PropIndex );
			}
		}
	}


	/// Constructor
	TVPNodeServer() : TVPNodeBase(), FillFunc(NULL) {}

	//bool			isLeaf() const { return FillFunc != NULL; } // for discreet ones
	
	TVPNodeServer	*a() { return (TVPNodeServer*)VPA; }
	TVPNodeServer	*b() { return (TVPNodeServer*)VPB; }
	TVPNodeServer	*parent() { return (TVPNodeServer*)VPParent; }

	/// Set the flags in the entire tree using the flags in the leaves
	bool			propagateBackBranchHasPayload()
	{
		//nldebug( "Level: %u - Node=%p A=%p B=%p", getLevel(), this, VPA, VPB );
		if ( isLeaf() )
		{
			return BranchHasPayload;
		}
		else
		{
			bool leftBranch = (a() && a()->propagateBackBranchHasPayload());
			bool rightBranch = (b() && b()->propagateBackBranchHasPayload());
			BranchHasPayload = (leftBranch || rightBranch);
			return BranchHasPayload;
		}
	}

	/// Fill the values of the discreet properties the PropIndexes of which are in the leaves, using the BranchHasPayload flags
	void			fillDiscreetProperties( TOutBox& outbox )
	{
		//nlinfo("examine node %p:%d", this, PropIndex);

		if ( BranchHasPayload )
		{
			//nldebug( "Level: %u - Node=%p A=%p B=%p", getLevel(), this, VPA, VPB );
			if ( isLeaf() )
			{
				// The payload bit depends on the success of fillDiscreetProperty, so it is serialized inside
				(*FillFunc)( outbox, PropIndex );
			}
			else
			{
				outbox.serialBitAndLog( BranchHasPayload );
				if ( a() ) a()->fillDiscreetProperties( outbox );
				if ( b() ) b()->fillDiscreetProperties( outbox );
			}
		}
		else
		{
			outbox.serialBitAndLog( BranchHasPayload );
		}
	}

#define INIT_FLAT_VP_TREE( vpname )				FlatVPTree[ CLFECOMMON::PROPERTY_##vpname ] = (TVPNodeServer*)(discreetRoot->get##vpname##node())
#define SET_FILLFUNC( name )					INIT_FLAT_VP_TREE(name);		((TVPNodeServer*)(discreetRoot->get##name##node()))->FillFunc = fill##name
#define SET_FILLFUNC2( nameProp, nameFunc )		INIT_FLAT_VP_TREE(nameProp);	((TVPNodeServer*)(discreetRoot->get##nameProp##node()))->FillFunc = fill##nameFunc

	sint		buildTree()
	{
		sint res = TVPNodeBase::buildTree();

		TVPNodeBase *discreetRoot = VPB->VPB;

		// setup position a orientation manually
		FlatVPTree[ CLFECOMMON::PROPERTY_POSITION ] = (TVPNodeServer*)VPA;
		FlatVPTree[ CLFECOMMON::PROPERTY_POSX ] = FlatVPTree[ CLFECOMMON::PROPERTY_POSITION ];
		FlatVPTree[ CLFECOMMON::PROPERTY_POSY ] = FlatVPTree[ CLFECOMMON::PROPERTY_POSITION ];
		FlatVPTree[ CLFECOMMON::PROPERTY_POSZ ] = FlatVPTree[ CLFECOMMON::PROPERTY_POSITION ];
		FlatVPTree[ CLFECOMMON::PROPERTY_ORIENTATION ] = (TVPNodeServer*)(VPB->VPA);

		SET_FILLFUNC( SHEET );
		SET_FILLFUNC( BEHAVIOUR );
		SET_FILLFUNC( NAME_STRING_ID );
		SET_FILLFUNC( TARGET_LIST );
		FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST_0 ] = FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST ];
		FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST_1 ] = FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST ];
		FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST_2 ] = FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST ];
		FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST_3 ] = FlatVPTree[ CLFECOMMON::PROPERTY_TARGET_LIST ];
		SET_FILLFUNC( VISUAL_FX );
		SET_FILLFUNC( CONTEXTUAL );
		SET_FILLFUNC( MODE );
		SET_FILLFUNC( BARS );
		SET_FILLFUNC2( VPA, VisualPropertyABC );
		SET_FILLFUNC2( VPB, VisualPropertyABC );
		SET_FILLFUNC2( VPC, VisualPropertyABC );
		SET_FILLFUNC2( TARGET_ID, RowProperty );
		SET_FILLFUNC2( ENTITY_MOUNTED_ID, RowProperty );
		SET_FILLFUNC2( RIDER_ENTITY_ID, RowProperty );
		SET_FILLFUNC( GUILD_SYMBOL );
		SET_FILLFUNC( GUILD_NAME_ID );
		SET_FILLFUNC( EVENT_FACTION_ID );
		SET_FILLFUNC( PVP_MODE );
		SET_FILLFUNC( PVP_CLAN );
		SET_FILLFUNC( OWNER_PEOPLE );
		SET_FILLFUNC( OUTPOST_INFOS );

		for (uint i=0; i<CLFECOMMON::NB_VISUAL_PROPERTIES; ++i)
			nlassert(getServerVPNode(i) != NULL);

		//nlassert( DSPropertyVPC == (DSPropertyVPB+1) == (DSPropertyVPA+2) ); // not set at this time
		nlassert( (sizeof(TYPE_VPA) == sizeof(TYPE_VPB)) && (sizeof(TYPE_VPB) == sizeof(TYPE_VPC)) );

		initSortedFlatVPTree();
		((TVPNodeServer*)discreetRoot)->initSortedFlatVPTreeFill();

		return res;
	}

	void		initSortedFlatVPTree();
	void		initSortedFlatVPTreeFill();

#define GET_VP_NODE(name)	TVPNodeServer::getServerVPNode(PROPERTY_##name)
	static TVPNodeServer*	getServerVPNode(CLFECOMMON::TPropIndex prop)
	{
		return FlatVPTree[prop];
	}

	void		deleteBranches()
	{
		if ( a() )
		{	a()->deleteBranches();
			delete a();
		}
		if ( b() )
		{	b()->deleteBranches();
			delete b();
		}
	}

	void		deleteA()
	{
		if ( a() )
		{	a()->deleteBranches();
			delete a();
		}
	}

	void		deleteB()
	{
		if ( b() )
		{	b()->deleteBranches();
			delete b();
		}
	}

	void		displayStatesOfTreeLeaves()
	{
		std::string /*s1,*/ s2;
		displayStatesOfTreeLeaves( /*s1,*/ s2 );
		if ( ! s2.empty() /*s1 != "000000000"*/ )
		{
			//nldebug( "%u: Filled the properties for C%hu S%hu: %s", CTickEventHandler::getGameCycle(), PrioContext.ClientId, (uint16)PrioContext.Slot, s1.c_str() );
			LOG_WHAT_IS_SENT( "%u: Filled props of C%hu S%hu: %s", CTickEventHandler::getGameCycle(), PrioContext.ClientId, (uint16)PrioContext.Slot, s2.c_str() );
		}
	}

	void		displayStatesOfTreeLeaves( /*std::string& s1,*/ std::string& s2 )
	{
		if ( isLeaf() )
		{
			//s1 += (BranchHasPayload?"1":"0");
			if ( BranchHasPayload )
				s2 += std::string(CLFECOMMON::getPropText( PropIndex )) + std::string(" ");
		}
		else
		{
			if ( a() ) a()->displayStatesOfTreeLeaves( /*s1,*/ s2 );
			if ( b() ) b()->displayStatesOfTreeLeaves( /*s1,*/ s2 );
		}
	}

	/// From the root, get the node mode (using hardcoded mapping as in CDistancePrioritizer::init())
	TVPNodeServer *getModeNodeFromRoot()
	{
		//nlassert( b()->b()->b()->a()->a()->PropIndex == CLFECOMMON::PROPERTY_MODE );
		return b()->b()->b()->a()->a();
	}

};



/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CDistancePrioritizer
{
public:

	/// Constructor
	CDistancePrioritizer() : _VisionArray(NULL), _VisionProvider(NULL), _DistanceDeltaRatio(10)
	{
		for ( uint c=0; c!=MAX_NB_CLIENTS; ++c )
		{
			_CurrentEntitiesToStudy[c] = 0;
		}
	}

	/// Destructor
	~CDistancePrioritizer()
	{
		_VisualPropertyTreeRoot->deleteBranches();
	}

	/// Init
	void		init( CVisionArray *va, CVisionProvider *vp, CHistory *hy );

	/// Calculate the priorities
	void		calculatePriorities();

	/// Called when processing the vision received from the GPMS (and when a client connects, for slot 0)
	void		addEntitySeenByClient( TClientId clientId, CLFECOMMON::TCLEntityId slot )
	{
		_PrioritizedEntitiesByClient[clientId].push_back( slot );
		if ( slot == 0 )
		{
			_VisionArray->getPairState( clientId, slot ).setSteadyPrio( 100 );
		}
	}

	/// Called when processing the vision received from the GPMS
	void		removeEntitySeenByClient( TClientId clientId, CLFECOMMON::TCLEntityId slot )
	{
		std::vector<CLFECOMMON::TCLEntityId>& vec = _PrioritizedEntitiesByClient[clientId];
		std::vector<CLFECOMMON::TCLEntityId>::iterator is = std::find( vec.begin(), vec.end(), slot );
		if ( is != vec.end() )
		{
			(*is) = vec.back(); // it breaks the sorting, nevermind
			vec.pop_back();
		}
	}

	/// Called when a client leaves
	void		removeAllEntitiesSeenByClient( TClientId clientId )
	{
		_PrioritizedEntitiesByClient[clientId].clear();
	}

	///
	void		fillOutBox( CClientHost& client, TOutBox& outbox );

	/// Set/change the distance/delta ratio that triggers the sending of a position
	void		setDistanceDeltaRatioForPos( uint32 ddratio ) { _DistanceDeltaRatio = ddratio; }

	CProcessingSpreader			SortSpreader;

protected:

	/// Begin a browsing cycle
	void		initDispatchingCycle( TClientId clientId )
	{
		_CurrentEntitiesToStudy[clientId] = 0;
	}

	/// Browse the entities seen in order of priority; returns INVALID_SLOT if no more
	CLFECOMMON::TCLEntityId	getNextEntityToStudy( TClientId clientId )
	{
		if ( ! _DissassociationsToResend[clientId].empty() )
		{
			CLFECOMMON::TCLEntityId slot = _DissassociationsToResend[clientId].front();
			_DissassociationsToResend[clientId].pop_front();
#ifdef NL_DEBUG
			nldebug( "Returning dissassociation to resend for C%hu S%hu", clientId, (uint16)slot );
#endif
			return slot;
		}
		else
		{
			uint& current = _CurrentEntitiesToStudy[clientId];
			if ( current < _PrioritizedEntitiesByClient[clientId].size() )
				return _PrioritizedEntitiesByClient[clientId][current++];
			else
				return CLFECOMMON::INVALID_SLOT;
		}
	}

	///
	void		serialSlotHeader( CClientHost& client, CEntity *sentity, TPairState& pairState, CLFECOMMON::TCLEntityId slot, TOutBox& outbox );
	
	/// Called by calculatePriorities()
	void		updatePriorityOfEntitiesSeenByClient( TClientId clientId )
	{
		CLFECOMMON::TCLEntityId slot;
		TPairState*		states = _VisionArray->getClientStateArray(clientId) + 1;
		for ( slot=1; slot!=MAX_SEEN_ENTITIES_PER_CLIENT; ++slot )
		{
			states->updatePrio();
			++states;
			//_VisionArray->getPairState( clientId, slot ).updatePrio();	// CHANGED BEN
		}
	}

	/// Called by calculatePriorities()
	void		sortEntitiesOfClient( TClientId clientId )
	{	
		std::sort( _PrioritizedEntitiesByClient[clientId].begin(), _PrioritizedEntitiesByClient[clientId].end(), TComparePairsByPriority(_VisionArray, clientId) );

/*#ifdef NL_DEBUG
		std::string s1 = "Sorted slots:", s2 = "Distances(m):", s3 = "Priorities:";
		std::vector<CLFECOMMON::TCLEntityId>::iterator iv;
		for ( iv=_PrioritizedEntitiesByClient[clientId].begin(); iv!=_PrioritizedEntitiesByClient[clientId].end(); ++iv )
		{
			s1 += NLMISC::toString( " %hu", (uint16)(*iv) );
			//s2 += NLMISC::toString( " %d", _VisionArray->getPairState( clientId, *iv ).DistanceCE/1000 );
			//s3 += NLMISC::toString( " %.1f", _VisionArray->getPairState( clientId, *iv ).Priority );
		}
		nldebug( "%s", s1.c_str() );
#endif*/

	}

	/// Test the criterion for the position of the entity 'slot' seen by 'clientId'
	bool		positionHasChangedEnough();

	/// Test the criterion for the orientation of the entity 'slot' seen by 'clientId' + initialized
	bool		orientationHasChangedEnough(const CPropertyHistory::CPropertyEntry& entry, float angleRatio );

	// Test the criterion for thetaIntMode
	bool		thetaIntModehasChanged(const CPropertyHistory::CPropertyEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateAllDiscreetProperties(const CPropertyHistory::CEntityEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateNPCDiscreetProperties(const CPropertyHistory::CEntityEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateCreatureDiscreetProperties(const CPropertyHistory::CEntityEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateForageSourceDiscreetProperties(const CPropertyHistory::CEntityEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateCommonPosAndMode(const CPropertyHistory::CEntityEntry& entry);

	/// Fill the BranchHasPayload flags of the tree, using the rules deciding if a discreet property need to be sent
	void		arbitrateSlot0PosAndMode(const CPropertyHistory::CEntityEntry& entry);

	// Get distance threshold
	CLFECOMMON::TCoord			getDistThreshold( CLFECOMMON::TPropIndex propIndex )
	{
		//nlassert( propertyid < CLFECOMMON::NB_PROPERTIES );
		return _DistThresholdTable[propIndex];
	}


	void		arbitrateDiscreetBehaviourProperty(const CPropertyHistory::CEntityEntry& entry, CEntity *sentity);


public:




#ifdef STORE_MIRROR_VP_IN_CLASS

	/*
	 * Test the criterion for discreet properties + initialized
	 */
	template <class T>
	bool		discreetPropertyHasChanged(const CPropertyHistory::CPropertyEntry& entry, const CMirrorPropValueRO<T>& currentValue, CLFECOMMON::TPropIndex propIndex, T* ) const
	{
		// Although the client should already know the sheet id of the controlled player, let's send it
		//if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) && (TVPNodeServer::PrioContext.Slot == 0) )
		//	return false;

		if (entry.HasValue)
		{
#ifdef NL_DEBUG
			if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) && (currentValue() != *((T*)&(entry.LastSent))) )
				LOG_WHAT_IS_SENT( "C%hu S%hu: sheet changes from %u to %u", TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot, (uint32)entry.LastSent, asUInt32<T>(currentValue()) );
#endif
			return (currentValue() != *((T*)&(entry.LastSent)));
		}
		else
		{
			TPropertyIndex dsPropertyIndex = CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex );
#ifdef NL_DEBUG
			bool	res = TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (T*)NULL );
			if (res)
			{
				// Not sent yet
				//nldebug( "No history yet for C%hu - slot %hu - prop %hu", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex );
				if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) )
				{
					CMirrorPropValueRO<T> currentValue( TheDataset, TVPNodeServer::PrioContext.EntityIndex, dsPropertyIndex );
					LOG_WHAT_IS_SENT( "C%hu S%hu: sheet initializes to %u", TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot,	asUInt32<T>(currentValue()) );
				}
			}
			return res;
#else
			return TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (T*)NULL );
#endif
		}
	}

	/*
	 * Test the criterion + initialized: overload for row properties (specialization syntax is too much different among compilers)
	 */
	bool		discreetPropertyHasChanged(const CPropertyHistory::CPropertyEntry& entry, const CMirrorPropValueRO<TEntityIndex>& currentValue, CLFECOMMON::TPropIndex propIndex, TDataSetRow* ) const
	{
		if (entry.HasValue)
		{
			//nldebug( "History for C%hu - slot %hu - prop %hu: %"NL_I64"u", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex, lastsent_value );
			return (currentValue().getIndex() != *((TDataSetIndex*)&(entry.LastSent)));
		}
		else
		{
			TPropertyIndex dsPropertyIndex = CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex );
			return TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (TDataSetRow*)NULL );
		}
	}




#else // STORE_MIRROR_VP_IN_CLASS




	/*
	 * Test the criterion for discreet properties + initialized
	 */
	template <class T>
	bool		discreetPropertyHasChanged(const CPropertyHistory::CPropertyEntry& entry, TPropIndex propIndex, T* ) const
	{
		// Although the client should already know the sheet id of the controlled player, let's send it
		//if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) && (TVPNodeServer::PrioContext.Slot == 0) )
		//	return false;

		TPropertyIndex dsPropertyIndex = CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex );

		if (entry.HasValue)
		{
			//nldebug( "History for C%hu - slot %hu - prop %hu: %"NL_I64"u", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex, lastsent_value );
			CMirrorPropValueRO<T> currentValue( TheDataset, TVPNodeServer::PrioContext.EntityIndex, dsPropertyIndex );

#ifdef NL_DEBUG
			if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) && (currentValue() != *((T*)&(entry.LastSent))) )
				LOG_WHAT_IS_SENT( "C%hu S%hu: sheet changes from %u to %u", TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot, (uint32)entry.LastSent, asUInt32<T>(currentValue()) );
#endif

			return (currentValue() != *((T*)&(entry.LastSent)));
		}
		else
		{
#ifdef NL_DEBUG
			bool	res = TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (T*)NULL );
			if (res)
			{
				// Not sent yet
				//nldebug( "No history yet for C%hu - slot %hu - prop %hu", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex );
				if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) )
				{
					CMirrorPropValueRO<T> currentValue( TheDataset, TVPNodeServer::PrioContext.EntityIndex, dsPropertyIndex );
					LOG_WHAT_IS_SENT( "C%hu S%hu: sheet initializes to %u", TVPNodeServer::PrioContext.ClientId, TVPNodeServer::PrioContext.Slot,	asUInt32<T>(currentValue()) );
				}
			}
			return res;
#else
			return TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (T*)NULL );
#endif
		}
	}

	/*
	 * Test the criterion + initialized: overload for row properties (specialization syntax is too much different among compilers)
	 */
	bool		discreetPropertyHasChanged(const CPropertyHistory::CPropertyEntry& entry, TPropIndex propIndex, TDataSetRow* ) const
	{
		TPropertyIndex dsPropertyIndex = CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex );

		if (entry.HasValue)
		{
			//nldebug( "History for C%hu - slot %hu - prop %hu: %"NL_I64"u", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex, lastsent_value );
			CMirrorPropValueRO<TEntityIndex> currentValue( TheDataset, TVPNodeServer::PrioContext.EntityIndex, dsPropertyIndex );

			return (currentValue().getIndex() != *((TDataSetIndex*)&(entry.LastSent)));
		}
		else
		{
			return TVPNodeServer::PrioContext.Sentity->propertyIsInitialized( propIndex, dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex, (TDataSetRow*)NULL );
		}
	}

#endif // STORE_MIRROR_VP_IN_CLASS

	/*
	 * Test the criterion for target list + initialized
	 */
	template <class T>
	bool		targetListHasChanged(const CPropertyHistory::CPropertyEntry& entry, CLFECOMMON::TPropIndex propIndex, T* ) const
	{
		// Although the client should already know the sheet id of the controlled player, let's send it
		//if ( (propIndex==CLFECOMMON::PROPERTY_SHEET) && (TVPNodeServer::PrioContext.Slot == 0) )
		//	return false;

		TPropertyIndex dsPropertyIndex = CEntityContainer::propertyIndexInDataSetToVisualPropIndex( propIndex );

		if (entry.HasValue)
		{
			//nldebug( "History for C%hu - slot %hu - prop %hu: %"NL_I64"u", TVPNodeServer::PrioContext.ClientHost->clientId(), (uint16)TVPNodeServer::PrioContext.Slot, propIndex, lastsent_value );
			NLMISC::TGameCycle	mirrorCycle = TheDataset.getChangeTimestamp( dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex );

			return (mirrorCycle != entry.LastSent);
		}
		else
		{
			return TheDataset.getChangeTimestamp( dsPropertyIndex, TVPNodeServer::PrioContext.EntityIndex ) != 0;
		}
	}


	/// Test the criterion for the specified property of the entity 'slot' seen by 'clientId'
	bool		entityIsWithinDistanceThreshold( CLFECOMMON::TPropIndex propIndex );

	/// Fill the specified dicreet property
	void		fillDiscreetProperty( TOutBox& outbox, CLFECOMMON::TPropIndex propIndex );

	///
	void		pushDissociationToResend( TClientId clientId, CLFECOMMON::TCLEntityId slot )
	{
		_DissassociationsToResend[clientId].push_back( slot );
	}

	CVisionArray*	getVisionArray() 		{ return _VisionArray; }

private:

	std::vector<CLFECOMMON::TCLEntityId>	_PrioritizedEntitiesByClient [MAX_NB_CLIENTS];

	std::list<CLFECOMMON::TCLEntityId>		_DissassociationsToResend [MAX_NB_CLIENTS];

	uint						_CurrentEntitiesToStudy [MAX_NB_CLIENTS];

	TVPNodeServer				*_VisualPropertyTreeRoot;

	CVisionArray				*_VisionArray;

	CVisionProvider				*_VisionProvider;

	CHistory					*_History;

	/// Distance threshold
	CLFECOMMON::TCoord			_DistThresholdTable [CLFECOMMON::NB_VISUAL_PROPERTIES];

	uint32						_DistanceDeltaRatio;
};


#define A a()
#define B b()
#define Parent parent()


#endif // NL_DISTANCE_PRIORITIZER_H

/* End of distance_prioritizer.h */
