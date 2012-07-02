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




#ifndef PRIMITIVE_UTIL_H
#define PRIMITIVE_UTIL_H

#include "primitive.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include <limits>
#include <vector>
#include <string>

namespace NLLIGO
{

typedef std::vector<IPrimitive*>	TPrimitiveSet;

/** Default predicate for primitive enumerator.
 *	This predicate test the class name of each primitive against a
 *	given class name.
 */
struct TPrimitiveClassPredicate : public std::unary_function<IPrimitive*, bool>
{
	TPrimitiveClassPredicate(const std::string &className)
		:	ClassName(className)
	{}

	bool operator () (const IPrimitive *prim) const
	{
		std::string *s;
		if (prim->getPropertyByName("class", s) && *s == ClassName)
			return true;
		return false;
	}

	/// The primitive class name to check
	const std::string		ClassName;
};

/** Predicate for primitive enumerator.
 *	This predicate test the name of each primitive against a
 *	given name.
 */
struct TPrimitiveNamePredicate
{
	TPrimitiveNamePredicate(const std::string &name)
		:	Name(name)
	{}

	bool operator () (const IPrimitive *prim) const
	{
		std::string *s;
		if (prim->getPropertyByName("name", s) && *s == Name)
			return true;
		return false;
	}

	/// The primitive name to check
	const std::string		Name;
};

/** Predicate for primitive enumerator.
 *	This predicate test the class name and name of the primitive.
 */
struct TPrimitiveClassAndNamePredicate
{
	TPrimitiveClassAndNamePredicate(const std::string &className, const std::string &name)
		:	ClassName(className),
		Name(name)
	{}

	bool operator () (const IPrimitive *prim) const
	{
		std::string *s;
		if (prim->getPropertyByName("class", s) && *s == ClassName)
		{
			if (prim->getPropertyByName("name", s) && *s == Name)
				return true;
		}
		return false;
	}

	/// The primitive class name to check
	const std::string		ClassName;
	/// The primitive name to check
	const std::string		Name;
};

/** Predicate for primitive enumerator.
 *	This predicate test the string value of a given property
 */
struct TPrimitivePropertyPredicate
{
	TPrimitivePropertyPredicate(const std::string &propName, const std::string &value)
		:	PropName(propName), PropValue(value)
	{}

	bool operator () (const IPrimitive *prim) const
	{
		std::string *s;
		if (prim->getPropertyByName(PropName.c_str(), s) && *s == PropValue)
			return true;
		return false;
	}

	/// The property name
	const std::string		PropName;
	/// The property value
	const std::string		PropValue;
};

/** The primitive enumerator class is used to iterate over primitive node that
 *	match a given predicate.
 *	The primitive are tested recursively starting at an arbitrary node in the
 *	primitive tree.
 *	Application code just need to call getNextMatch until it return NULL indicating
 *	there is no more node that match the predicate.
 */
template <class Pred = TPrimitiveClassPredicate>
class CPrimitiveEnumerator
{
public:
	/** Construct a primitive enumerator.
	 *	startPrim is the primitive where the enumeration start. Even if the startPrimitive
	 *	is not at the root of the primitive tree, the enumerator will not
	 *	try to parse the parent of startPrim.
	 *	predicate is the functor predicate. Each node is tested against the bool operator()(IPrimitive *)
	 *	method of the predicate. If the predicate return true, then the node is returned by getNextMatch.
	 */
	CPrimitiveEnumerator(IPrimitive *startPrim, Pred &predicate)
		: _StartPrim(startPrim),
		_Predicate(predicate),
		_CurrentPrim(startPrim)
	{
		// mark the root node as non checked
		_IndexStack.push_back(std::numeric_limits<uint>::max());
	}

	/** Each call to this method will return a primitive pointer that match
	 *	the predicate functor.
	 *	Return NULL when there is no more primitive matching the predicate.
	 */
	IPrimitive *getNextMatch()
	{
		while (!_IndexStack.empty())
		{
			if (_IndexStack.back() == std::numeric_limits<uint>::max())
			{
				_IndexStack.back() = 0;
				// we need to check the current node.
				if (_Predicate(_CurrentPrim))
				{
					// this one match !
					return _CurrentPrim;
				}
			}
			if (_IndexStack.back() < _CurrentPrim->getNumChildren())
			{
				IPrimitive *child;
				if (_CurrentPrim->getChild(child, _IndexStack.back()++))
				{
					// go down into this node
					_IndexStack.push_back(std::numeric_limits<uint>::max());
					_CurrentPrim = child;
				}
			}
			else
			{
				// no more child to test, pop one level
				_IndexStack.pop_back();
				_CurrentPrim = _CurrentPrim->getParent();
			}
		}
		// no more match
		return NULL;
	}


private:

	/// The root primitive for enumeration
	IPrimitive	*_StartPrim;
	/// The predicate functor
	Pred		_Predicate;
	/// The current primitive
	IPrimitive	*_CurrentPrim;
	/// for each recursion level, keep the index of the currently explored child.
	std::vector<uint>	_IndexStack;
};

/** Build a primitive set that match the predicate
 *	This class makes use of the CPrimitiveEnumerator class to iterate
 *	on each valid node and fill the result primitive set.
*/
template <class Pred>
class CPrimitiveSet
{
public:

	void buildSet(IPrimitive *startPrimitive, Pred &predicate, TPrimitiveSet &result)
	{
		CPrimitiveEnumerator<Pred>	enumerator(startPrimitive, predicate);

		IPrimitive *p;
		while ((p = enumerator.getNextMatch()) != NULL)
			result.push_back(p);
	}
};

/** Filter a primitive set against a predicate.
 *	Useful to refine a primitive set with another predicate.
 */
template <class Pred>
class CPrimitiveSetFilter
{
public:

	void filterSet(const std::vector<IPrimitive*> &source, Pred &predicate, TPrimitiveSet &result)
	{
		std::vector<IPrimitive*>::const_iterator first(source.begin()), last(source.end());
		for (; first != last; ++first)
		{

			if (predicate(*first))
				result.push_back(*first);
		}
	}
};

/** Utility function that load an xml primitive file into a CPrimitives object.
 *	This function deal with file IO and XML parsing call.
 *	Return false if the loading fail for some reason, true otherwise.
 */
inline bool loadXmlPrimitiveFile(CPrimitives &primDoc, const std::string &fileName, CLigoConfig &ligoConfig)
{
	try
	{
		NLMISC::CIFile	fileIn(fileName);
		NLMISC::CIXml xmlIn;
		xmlIn.init (fileIn);

		// Read it
		return primDoc.read (xmlIn.getRootNode (), NLMISC::CFile::getFilename(fileName).c_str(), ligoConfig);
	}
	catch(const NLMISC::Exception &e)
	{
		nlwarning("Error reading input file '%s': '%s'", fileName.c_str(), e.what());
		return false;
	}
}

/** Utility function that save a CPrimitives object into an xml file.
 *	This function deal with file IO and XML parsing call.
 *	Return false if the saving fail for some reason, true otherwise.
 */
inline bool saveXmlPrimitiveFile(CPrimitives &primDoc, const std::string &fileName)
{
	try
	{
		NLMISC::COFile	fileOut(fileName);
//		xmlDocPtr	xmlDoc = xmlNewDoc((xmlChar*)("1.0"));;
		NLMISC::COXml xmlOut;
		xmlOut.init (&fileOut);
//		NLMISC::CIXml xmlOut;
//		xmlOut.init (fileOut);

		// Read it
		primDoc.write(xmlOut.getDocument(), fileName.c_str());

		xmlOut.flush ();

		fileOut.close();

		return true;

//		return xmlSaveFile(fileName.c_str(), xmlDoc) != -1;
	}
	catch(const NLMISC::Exception &e)
	{
		nlwarning("Error writing output file '%s': '%s'", fileName.c_str(), e.what());
		return false;
	}
}

/** Utility function to look for the first child of a primitive node that
 *	match the predicate.
 *	Return NULL if none of the child match the predicate.
 *	There is no way to get the next matching child using this function,
 *	you must use filterPrimitiveChilds to do this.
 */
template <class Pred>
IPrimitive *getPrimitiveChild(IPrimitive *parent, Pred &predicate)
{
	for (uint i=0; i<parent->getNumChildren(); ++i)
	{
		IPrimitive *child;
		if (parent->getChild(child, i) && predicate(child))
		{
			return child;
		}
	}

	return NULL;
}

/** Utility function to look for the first parent of a primitive node that
 *	match the predicate.
 *	Return NULL if none of the parent match the predicate.
 */
template <class Pred>
IPrimitive *getPrimitiveParent(IPrimitive *prim, Pred &predicate)
{
	IPrimitive *parent = prim->getParent();
	while (parent)
	{
		if (predicate(parent))
			return parent;

		parent = parent->getParent();
	}

	return NULL;
}

/** Utility function that fill a primitive set with all the child nodes
 *	that match the predicate.
 */
template <class Pred>
void filterPrimitiveChilds(IPrimitive *parent, Pred &predicate, TPrimitiveSet &result)
{
	for (uint i=0; i<parent->getNumChildren(); ++i)
	{
		IPrimitive *child;
		if (parent->getChild(child, i) && predicate(child))
			result.push_back(child);
	}
}


/** Build a string that represent the path to a node
 *	Note that the reverse operation does not guaranty to
 *	return a unique node because there is no name
 *	uniqueness constraint in the primitive system.
 */
std::string buildPrimPath(const IPrimitive *prim);

/** Return a set of primitive that match a given path*/
void selectPrimByPath(IPrimitive *rootNode, const std::string &path, TPrimitiveSet &result);

} // namespace NLLIGO


#endif // #define PRIMITIVE_UTIL_H
