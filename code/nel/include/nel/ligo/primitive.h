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

#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"

// Include from libxml2
#include <libxml/parser.h>

#include <vector>

namespace NLLIGO
{

#ifdef NL_DEBUG
#	define NLLIGO_DEBUG
#endif


/**
  * Ligo primitives are used to add logical geometrical gameplay information.
  * Ligo primitives are NODES, POINTS, PATHES or ZONES.
  * Ligo primitives have a CLASS.
  *
  * The primitive class defines the properties attached to the primitive
  * The primitive class are defined in the XML file specified in the LigoClass field of the CLigoConfig class.
  */

class CPrimitives;
class CLigoConfig;

// ***************************************************************************

void Register ();

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 */
class IProperty : public NLMISC::IStreamable
{
public:
	IProperty ()
	{
		Default = false;
	}

	// This property is set to default
	bool Default;
	// Force class to be polymorphic
	virtual void foo () const = 0;
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a simple string
 */
class CPropertyString : public IProperty
{
public:
	CPropertyString () {}
	CPropertyString (const char *str);
	CPropertyString (const std::string &str);
	CPropertyString (const char *str, bool _default);
	virtual ~CPropertyString () {}
	std::string			String;

	NLMISC_DECLARE_CLASS (CPropertyString)

	virtual void serial(NLMISC::IStream &f)
	{
		f.serial(Default);
		f.serial(String);
	}

	// Force class to be polymorphic
	virtual void foo () const {}
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a string array
 */
class CPropertyStringArray : public IProperty
{
public:
	CPropertyStringArray () {}
	virtual ~CPropertyStringArray () {}
	CPropertyStringArray (const std::vector<std::string> &stringArray);
	CPropertyStringArray (const std::vector<std::string> &stringArray, bool _default);
	std::vector<std::string>	StringArray;

	NLMISC_DECLARE_CLASS (CPropertyStringArray)

	virtual void serial(NLMISC::IStream &f)
	{
		f.serial(Default);
		f.serialCont(StringArray);
	}
	// Force class to be polymorphic
	virtual void foo () const {}
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a string array
 */
class CPropertyColor : public IProperty
{
public:
	NLMISC::CRGBA		Color;

	NLMISC_DECLARE_CLASS (CPropertyColor)

	virtual void serial(NLMISC::IStream &f)
	{
		f.serial(Default);
		f.serial(Color);
	}
	// Force class to be polymorphic
	virtual void foo () const {}

	// ctors
	CPropertyColor() {}
	CPropertyColor(NLMISC::CRGBA col) : Color(col) {}
};

// ***************************************************************************

class CPrimVector : public NLMISC::CVector
{
public:
	CPrimVector ()
	{
		Selected = false;
	}
	CPrimVector (const NLMISC::CVector &v)
	{
		CVector::operator= (v);
		Selected = false;
	}

	void serial(NLMISC::IStream &f)
	{
		CVector::serial(f);
		f.serial(Selected);
	}

	bool	Selected;
};

// ***************************************************************************

/*
 * This class is the base class for ligo primitive.
 *
 * Provide access to common properties.
 * Provide access to the primitive hierachy
 */
class IPrimitive : public NLMISC::IStreamable
{
	friend class CPrimitives;
public:

	// Deprecated
//	std::string						Layer;
	// Deprecated
//	std::string						Name;

	// Expended in the tree view
//	bool							Expanded;

	enum
	{
		NotAnArray,
		AtTheEnd = 0xffffffff,
	};

	/// \name Hierarchy
	IPrimitive ();

	virtual ~IPrimitive ();

	IPrimitive (const IPrimitive &node);

	virtual void operator= (const IPrimitive &node);

	/** Get the children primitive count */
	uint				getNumChildren () const
	{
		return (uint)_Children.size ();
	}

	/** Get a child primitive */
	bool				getChild (const IPrimitive *&result, uint childId) const;

	/** Get a child primitive */
	bool				getChild (IPrimitive *&result, uint childId);

	/** Get the parent primitive */
	IPrimitive			*getParent ()
	{
		return _Parent;
	}
	const IPrimitive	*getParent () const
	{
		return _Parent;
	}

	/**	Get the primitive relative to this and the given path	 */
	const	IPrimitive	*getPrimitive	(const	std::string	&absoluteOrRelativePath)	const;

	/** Get the id of the child, return 0xffffffff if not found */
	bool				getChildId (uint &childId, const IPrimitive *child) const;

	/** Remove and delete a child primitive */
	bool				removeChild (IPrimitive *child);

	/** Remove and delete a child primitive */
	bool				removeChild (uint childId);

	/// Remove the child primitive from the children list, don't delete it
	bool				unlinkChild(IPrimitive *child);

	/** Remove and delete all children primitives */
	void				removeChildren ();

	/**
	  * Insert a child primitive before the index.
	  * The pointer will be deleted by the parent primitive using the ::delete operator.
	  * return false if the index is invalid
	  */
	bool				insertChild (IPrimitive *primitive, uint index = AtTheEnd);

	/// \name Properties

	/**
	  * Get a num properties
	  **/
	uint				getNumProperty () const;

	/**
	  * Get a properties by its index
	  * This method (iterate a list) is slower than getPropertyByName (look up in a map).
	  **/
	bool				getProperty (uint index, std::string &property_name, const IProperty *&result) const;

	/**
	  * Get a properties by its index
	  * This method (iterate a list) is slower than getPropertyByName (look up in a map).
	  **/
	bool				getProperty (uint index, std::string &property_name, IProperty *&result);

	/** Check the existence of a named property */
	bool				checkProperty(const std::string &property_name) const;

	/**
	  * Add a property
	  * If the property already exist, the method does nothing and returns false.
	  * The pointer will be deleted by the primitive using the ::delete operator.
	  **/
	bool				addPropertyByName (const char *property_name, IProperty *result);

	/**
	  * Get a property with its name
	  **/
	bool				getPropertyByName (const char *property_name, const IProperty *&result) const;

	/**
	  * Get a property with its name
	  **/
	bool				getPropertyByName (const char *property_name, IProperty *&result) const;

	/**
	  * Get a string property with its name. Return false if the property is not found or is not a string property.
	  **/
	bool				getPropertyByName (const char *property_name, std::string *&result) const;

	/**
	  * Get a string array property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, std::vector<std::string> *&result) const;

	/**
	  * Get a string property with its name. Return false if the property is not found or is not a string property.
	  **/
	bool				getPropertyByName (const char *property_name, std::string &result) const;

	/**
	  * Get a string array property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, const std::vector<std::string> *&result) const;

	/**
	  * Get a color property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, NLMISC::CRGBA &result) const;

	/**
	  * Remove a property
	  * This is method (iterate a list) is slower than removePropertyByName (look up in a map).
	  **/
	bool				removeProperty (uint index);

	/**
	  * Remove a property by its name
	  **/
	bool				removePropertyByName (const char *property_name);

	/**
	  * Remove all the properties
	  **/
	void				removeProperties ();

	/* Init default primitive's parameters
	 *
	 * This method will add all the properties declared in the primitive class and create default properties.
	 */
	void				initDefaultValues (CLigoConfig &config);

	// Read the primitive, calls initDefaultValue (CLigoConfig &config)
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;

	// Get the vertices
	virtual uint				getNumVector () const = 0;
	virtual const CPrimVector	*getPrimVector () const = 0;
	virtual CPrimVector			*getPrimVector () = 0;

	// Make a copy
	virtual IPrimitive *copy () const = 0;

	// used for fast binary save/load (exploitation mode)
	void serial(NLMISC::IStream &f);

	// shortcut to getPropertyByName("name", ret); return ret;
	std::string					getName() const;

	const std::string			&getUnparsedProperties() const;
	void						setUnparsedProperties(const std::string &unparsedProperties) const;

private:

	// callback called just after the node is attach under a parent
	virtual void onLinkToParent() {}
	// callback called just before the node is removed from it's parent
	virtual void onUnlinkFromParent() {}

	/// Callback called just after an ancestor is linked
	virtual void onBranchLink() {}
	/// Callback called just before an ancestor is unlinked
	virtual void onBranchUnlink() {}

	/// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive(CPrimitives &/* primitives */) const {}

	// internal recusive call
	void branchLink();
	void branchUnlink();

	// Update child Id
	void updateChildId (uint index);

	// Child id
	uint32									_ChildId;

	// Parent
	IPrimitive								*_Parent;

	// Children
	std::vector<IPrimitive*>				_Children;

	// Single properties
	std::map<std::string, IProperty*>		_Properties;

	// Editor specific properties (unparsed)
	mutable std::string						_UnparsedProperties;


#ifdef NLLIGO_DEBUG
	std::string								_DebugClassName;
	std::string								_DebugPrimitiveName;
#endif


};

// ***************************************************************************

// Simple primitive node
class CPrimNode : public IPrimitive
{
public:
	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimNode)

protected:

	// void operator= (const CPrimNode &node);


	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);

	// \name From IPrimitive
	virtual IPrimitive *copy () const;
};

// ***************************************************************************

class CPrimPoint : public IPrimitive
{

public:

	CPrimPoint ()
	{
		Angle = 0;
	}


	CPrimVector				Point;
	float					Angle;	// Angle on OZ, CCW

public:

	void serial (NLMISC::IStream &f);

	// void operator= (const CPrimPoint &node);

	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimPoint);

protected:

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;

	// \name From IPrimitive
	virtual IPrimitive *copy () const;
};


// ***************************************************************************
class CPrimPath : public IPrimitive
{

public:

	std::vector<CPrimVector>	VPoints;

public:

	void serial (NLMISC::IStream &f);

	// void operator= (const CPrimPath &node);

	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimPath);

protected:

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;

	// \name From IPrimitive
	virtual IPrimitive *copy () const;
};


// ***************************************************************************

class CPrimZone : public IPrimitive
{

public:

	std::vector<CPrimVector>	VPoints;

	static float getSegmentDist(const NLMISC::CVector v, const NLMISC::CVector &p1, const NLMISC::CVector &p2, NLMISC::CVector &nearPos);

public:

	bool contains (const NLMISC::CVector &v) const { return CPrimZone::contains(v, VPoints); }
	bool contains(const NLMISC::CVector &v, float &distance, NLMISC::CVector &nearPos, bool isPath) const { return CPrimZone::contains(v, VPoints, distance, nearPos, isPath); }

	// void operator= (const CPrimZone &node);

	void serial (NLMISC::IStream &f);

	// Returns true if the vector v is inside of the patatoid
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points);
	// Returns true if the vector v is inside of the patatoid and set the distance of the nearest segment and the position of the nearest point.
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath);
	// Returns true if the vector v is inside of the patatoid
	static bool contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points);
	// Returns true if the vector v is inside of the patatoid and set the distance of the nearest segment and the position of the nearest point.
	static bool contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath);

	/// Returns the barycenter of the zone (warning, it may be outside of the zone if it is not convex). Returns CVector::Null if there is no vertex.
	NLMISC::CVector		getBarycentre() const;

	/// Returns the smallest axis-aligned box containing the zone (z is always set to 0)
	void				getAABox( NLMISC::CVector& cornerMin, NLMISC::CVector& cornerMax ) const;

	/// Return the area of the axis-aligned box containing the zone
	float				getAreaOfAABox() const;

	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimZone);

protected:

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;

	// \name From IPrimitive
	virtual IPrimitive *copy () const;
};


// ***************************************************************************

/** This primitive type is used to handle unique alias across a primitive file.
 *	Usage of this primitive imply the setting of the appropriate 'ligo context'
 *	before reading or copy/pasting alias.
*/
class CPrimAlias : public IPrimitive
{
	friend class CPrimitives;

	/// The 'dynamic' part of the alias
	uint32				_Alias;
	/// The primitive container
	class CPrimitives	*_Container;

	// Needed overloads (not used)
	virtual uint				getNumVector () const
	{
		return 0;
	};
	virtual const CPrimVector	*getPrimVector () const
	{
		return NULL;
	}
	virtual CPrimVector			*getPrimVector ()
	{
		return NULL;
	}


	virtual void onBranchLink();
	// callback called just before the node is removed from it's parent
	virtual void onBranchUnlink();

	void regenAlias();

public:
	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimAlias);

	// private default constructor
	CPrimAlias();
	// copy constructor needed
	CPrimAlias(const CPrimAlias &other);

	~CPrimAlias();

	// return the dynamic part of the alias
	uint32	getAlias() const;

	// Return the full alias, merge of the static and dynamic part
	uint32	getFullAlias() const;

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config);
	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Create a copy of this primitive
	virtual IPrimitive *copy () const;
	// serial for binary save
	virtual void serial (NLMISC::IStream &f);

};

// ***************************************************************************

/*
This class is deprecated.
*/
class CPrimRegion
{

public:

	std::string				Name;
	std::vector<CPrimPoint> VPoints;
	std::vector<CPrimZone>	VZones;
	std::vector<CPrimPath>	VPaths;

	std::vector<bool>		VHidePoints;
	std::vector<bool>		VHideZones;
	std::vector<bool>		VHidePaths;

public:

	void serial (NLMISC::IStream &f);
};

// ***************************************************************************

/**
  * This class is a ligo primitives set
  */
class CPrimitives
{
public:

	CPrimitives ();
	CPrimitives (const CPrimitives &other);
	~CPrimitives ();

	// Operator copy
	CPrimitives&	operator= (const CPrimitives &other);

	// Convert from old format to the new one
	void			convert (const CPrimRegion &region);

	// Read the primitive
	bool			read (xmlNodePtr xmlNode, const char *filename, CLigoConfig &config);

	// Write the primitive
	void			write (xmlDocPtr xmlNode, const char *filename) const;

	// Write the primitive
	void			write (xmlNodePtr root, const char *filename) const;

	// serial the primitive. Used for binary files.
	void			serial(NLMISC::IStream &f);

	// Root primitive hierarchy
	CPrimNode		*RootNode;

	// get the static alias part for this primitive
	uint32			getAliasStaticPart();

	// set the static alias part for this primitive
	void			setAliasStaticPart(uint32 staticPart);

	// Build an alias by combining the static and dynamic part
	uint32			buildFullAlias(uint32 dynamicPart);

	// Generate a new unique alias (dynamic part only)
	uint32			genAlias(IPrimitive *prim, uint32 preferedAlias = 0);
	// Reserve an alias and store it in the used alias list (dynamic part only)
//	void			reserveAlias(uint32 dynamicAlias);
	// Remove an alias from the list of alias in use (dynamic part only)
	void			releaseAlias(IPrimitive *prim, uint32 dynamicAlias);

	// Force the assignation of the specified alias to the primitive. If another primitive
	// already hold the alias, this other primitive is assigned a new alias.
	void			forceAlias(CPrimAlias *prim, uint32 alias);

	// get the last generated alias value (for debug only)
	uint32			getLastGeneratedAlias();

	// Return the primitive indexed by the given alias (ie, it doesn't return the alias primitive, but its first parent)
	IPrimitive		*getPrimitiveByAlias(uint32 primAlias);

	// Build the complete list of indexed primitive (ie all primitive that have a primalias child)
	void			buildPrimitiveWithAliasList(std::map<uint32, IPrimitive*> &result);


private:
	// Conversion internal methods
	void			convertAddPrimitive (IPrimitive *child, const IPrimitive *prim, bool hidden);
	void			convertPrimitive (const IPrimitive *prim, bool hidden);

	/// Optional context information
	CLigoConfig			*_LigoConfig;
	/// Static part alias mapping (can be 0 if no mapping is defined)
	uint32				_AliasStaticPart;
	/// Last generated Alias, used to compute the next alias
	uint32				_LastGeneratedAlias;
	/// List of alias in use in the primitive (dynamic part only)
	std::map<uint32, IPrimitive*>	_AliasInUse;
	// Store the filename
	// This allows to retrieve the static alias when reloading from binary file
	std::string			_Filename;
};

// ***************************************************************************

/** Singleton to manage special loading feature related to
 *	unique alias assignment
 */
class CPrimitiveContext
{
	static CPrimitiveContext	*_Instance;

	// private ctor
	CPrimitiveContext();
public:

	// get the singleton reference
	static CPrimitiveContext	&instance()
	{
		if (!_Instance)
		{
			_Instance = new CPrimitiveContext;
		}

		return *_Instance;
	}

	/// The current ligo configuration file.
	CLigoConfig		*CurrentLigoConfig;
	/// The current primitives container.
	CPrimitives		*CurrentPrimitive;

};


} // namespace NLLIGO

#endif // __PRIMITIVE_H__

