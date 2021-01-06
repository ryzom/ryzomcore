// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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



#ifndef CDB_LEAF_H
#define CDB_LEAF_H

#include "cdb.h"
#include "cdb_branch.h"
#include "time_nl.h"
#include "rgba.h"

namespace NLMISC{

/**
 * Database node which contains a unique property
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CCDBNodeLeaf : public ICDBNode
{
public:
	// flush all observers calls for modified nodes
	static void flushObserversCalls();


	/// Return the value of the property.
	inline sint64 getValue64() const { return _Property; }

	/// Set the value of the property (set '_Changed' flag with 'true').
	void setValue64 (sint64 prop);

	inline sint32 getValue32() const { return (sint32)(_Property & 0xffffffff); }
	void setValue32 (sint32 prop);
	inline sint16 getValue16() const { return (sint16)(_Property & 0xffff); }
	void setValue16 (sint16 prop);
	inline sint8 getValue8() const { return (sint8)(_Property & 0xff); }
	void setValue8 (sint8 prop);
	inline bool getValueBool() const { return (_Property!=(sint64)0 ); }
	void setValueBool (bool prop);
	inline CRGBA getValueRGBA() const
	{
		CRGBA col;
		col.R = (uint8)(_Property&0xff);
		col.G = (uint8)((_Property>>8)&0xff);
		col.B = (uint8)((_Property>>16)&0xff);
		col.A = (uint8)((_Property>>24)&0xff);
		return col;
	}
	void setValueRGBA (const CRGBA &color);

	/// Return the value of the property before the database change
	inline sint64 getOldValue64() const { return _oldProperty; }
	inline sint32 getOldValue32() const { return (sint32)(_oldProperty & 0xffffffff); }
	inline sint16 getOldValue16() const { return (sint16)(_oldProperty & 0xffff); }
	inline sint8 getOldValue8() const { return (sint8)(_oldProperty & 0xff); }
	inline bool getOldValueBool() const { return (_oldProperty!=(sint64)0 ); }


	/// Return the type of the property.
	inline const EPropType &type() const {return _Type;}

	/// Set the property Type.
	inline void type(const EPropType &t) {_Type = t;}

	/// Return 'true' if the property changed since last call.
	inline const bool &changed() const {return _Changed;}

	/// Set the property flag to known if the property changed since last call.
	inline void changed(const bool &c) {_Changed = c;}

	/**
	 * Default constructor
	 */
	CCDBNodeLeaf(const std::string &name) : ICDBNode(name)
	{
		_Parent=0;
		_Property = 0;
		_oldProperty = 0;
		_Type = UNKNOWN;
		_Changed = false;
		_LastChangeGC = 0;
	}

	/**
	 *	Build the structure of the database from a file
	 * \param f is the stream
	 */
	void init( xmlNodePtr node, IProgressCallback &progressCallBack, bool mapBanks=false, CCDBBankHandler *bankHandler = NULL );

	/**
	 * Get a node
	 * \param idx is the node index
	 */
	ICDBNode * getNode( uint16 idx );

	/**
	 * Get a node . Create it if it does not exist yet
	 * \param id : the CTextId identifying the node
	 */
	ICDBNode * getNode (const CTextId& id, bool bCreate);

	/**
	 * Get a node index
	 * \param node is a pointer to the node
	 */
	virtual bool getNodeIndex( ICDBNode* /* node */, uint& /* index */)
	{
		return false;
	}

	/**
	 * Save a backup of the database
	 * \param id is the text id of the property/grp
	 * \param f is the stream
	 */
	void write( CTextId& id, FILE * f);

	/**
	 * Update the database from a stream coming from the FE
	 * \param f : the stream.
	 */
	void readDelta(TGameCycle gc, CBitMemStream & f );

	/**
	 * Return the value of a property (the update flag is set to false)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \return the structure of the property
	 */
	sint64 getProp( CTextId& id );

	/**
	 * Set the value of a property (the update flag is set to true)
	 * \param id is the text id of the property/grp
	 * \param name is the name of the property
	 * \param value is the value of the property
	 * \return bool : 'false' if id is too long.
	 */
	bool setProp( CTextId& id, sint64 value );

	/**
	 * Set the value of a property, only if gc>=_LastChangeGC
	 */
	bool setPropCheckGC(TGameCycle gc, sint64 value);

	/// Reset all leaf data from this point
	void resetData(TGameCycle gc, bool forceReset=false);

	/**
	 * Clear the node and his children
	 */
	void clear();


	// the parent node for a branch (NULL by default)
	virtual void setParent(CCDBNodeBranch* parent) { _Parent=parent; }

	//get the node parent
	virtual CCDBNodeBranch	*getParent()
	{
		return _Parent;
	}

	/// Count the leaves
	virtual uint			countLeaves() const
	{
		return 1;
	}

	/// Find the leaf which count is specified (if found, the returned value is non-null and count is 0)
	virtual CCDBNodeLeaf	*findLeafAtCount( uint& count )
	{
		if ( count == 0 )
			return this;
		else
		{
			--count;
			return NULL;
		}
	}

	/// Debug purpose
	virtual void display(const std::string &prefix);


	virtual bool isLeaf() const { return true; }

	/**
	* add an observer to a property
	* \param observer : pointer to an observer
	* \param id text id identifying the property
	* \return false if the node doen t exist
	*/
	virtual bool addObserver(IPropertyObserver* observer, CTextId& id);

	/** remove an obsever
	 * \param observer : pointer to an observer
	 * \param id text id identifying the property
	 * \return false if the node or observer doesn t exist
	 */
	virtual bool removeObserver(IPropertyObserver* observer, CTextId& id);


	/// get the last change GameCycle (server tick) for this value
	TGameCycle	getLastChangeGC() const {return _LastChangeGC;}


private:

	CCDBNodeBranch *	_Parent;

	/// property value
	sint64				_Property;
	sint64				_oldProperty;

	/// property type
	EPropType			_Type;

	/// true if this value has changed
	bool				_Changed;

	/// gamecycle (servertick) of the last change for this value.
	/// change are made in readDelta only for change >= _LastChangeGC
	TGameCycle	_LastChangeGC;

	/// observers to call when the value really change
	std::vector<IPropertyObserver*> _Observers;

private:
	void notifyObservers();

};

////////////////////
// INLINE MEMBERS //
////////////////////


}


#endif // CDB_LEAF_H






