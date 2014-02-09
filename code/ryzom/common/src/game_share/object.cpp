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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "object.h"
#include "utils.h"

#include <zlib.h>

namespace R2
{

class CSerializeContext
{
public:
	CSerializeContext():_Indent(0){}
	sint32 getIndent() const { return _Indent;}
	void add() { ++_Indent; }
	void sub() { --_Indent; }
private:
	sint32 _Indent;
};


class CTableIntegrityChecker
{
public:
	CTableIntegrityChecker(const CObjectTable &table) : _Table(table)
	{
		_Table.checkIntegrity();
	}
	~CTableIntegrityChecker()
	{
		_Table.checkIntegrity();
	}
private:
	const CObjectTable &_Table;
};

#ifdef NL_DEBUG
	#define CHECK_TABLE_INTEGRITY CTableIntegrityChecker __cti(*this);
#else
	#define CHECK_TABLE_INTEGRITY
#endif

//CObjectFactory *CObjectSerializer::Factory = NULL;
CObjectFactory *CObjectSerializerClient::_ClientObjecFactory = NULL;


static CObject *newTable(CObjectFactory *factory)
{
	if (factory)
	{
		return factory->newBasic("Table");
	}
	else
	{
		return new CObjectTable;
	}
}

//----------------------- private implementation stuffs ----------------------------------------

static void addTab(std::string& out,  sint32 n)
{
	for (sint32 i = 0 ; i < n; ++i)
	{
		out += "  ";
	}
}

//----------------------- CObject ----------------------------------------

CObject::~CObject()
{
	BOMB_IF(_Validation != 0x01020304, "Error  (double delete)?", return);
	_Validation = 0;
}

CObject::CObject()
{
	_Parent = 0;
	_Ghost = false;
	_Validation = 0x01020304;
}

void CObject::previsit(std::vector<CObject::TRefPtr> &sons)
{
	sons.push_back(this);
}

void CObject::visit(IObjectVisitor &visitor)
{
	//H_AUTO(R2_CObjectTable_visit)
	std::vector<CObject::TRefPtr> sons;
	// must work on a copy here, because the 'visit' method may change the current list of sons of this table
	// Example of a scenario where this happened :
	// - Create a bandit camp
	// - onPostCreate (1) is called (see in client : CInstance::onPostCreate)
	// - create ghost bandit as sons of current bandit camps, their 'onPostCreate' method is called
	// - returns to previous onPostCreate (1) -> the list of sons has changed
	//   -> as a result, onPostCreated was called twice on instances that were just created, leading to a crash (the onPostCreate method registered
	//      the new object in some manager, and this registration is only allowed once per object)
	previsit(sons);
	for(uint k = 0; k < sons.size(); ++k)
	{
		if (sons[k]) // may become NULL if son was deleted during the visit callback
		{
			sons[k]->visitInternal(visitor);
		}
	}
}


void CObject::inPlaceCopy(const CObject &src)
{
	//H_AUTO(R2_CObject_inPlaceCopy)
	src.inPlaceCopyTo(*this);
}

bool CObject::getGhost() const
{
	//H_AUTO(R2_CObject_getGhost)
	return _Ghost;
}

void CObject::setGhost(bool ghost)
{
	//H_AUTO(R2_CObject_setGhost)
	_Ghost = ghost;
}


void CObject::copyMismatchMsg(const CObject &src)
{
	//H_AUTO(R2_CObject_copyMismatchMsg)
	nlwarning("Can't copy object of type %s into object of type %s", src.getTypeAsString(), this->getTypeAsString());
}


void CObject::inPlaceCopy(const CObjectString &src)
{
	//H_AUTO(R2_CObject_inPlaceCopy)
	copyMismatchMsg(src);
}

void CObject::inPlaceCopy(const CObjectNumber &src)
{
	//H_AUTO(R2_CObject_inPlaceCopy)
	copyMismatchMsg(src);
}

void CObject::inPlaceCopy(const CObjectTable &src)
{
	//H_AUTO(R2_CObject_inPlaceCopy)
	copyMismatchMsg(src);
}




bool CObject::isNumber(const std::string & prop) const
{
	//H_AUTO(R2_CObject_isNumber)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr) return false;
		return attr->doIsNumber();
	}
	return doIsNumber();
}

bool CObject::isString(const std::string & prop) const
{
	//H_AUTO(R2_CObject_isString)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr) return false;
		return attr->doIsString();
	}
	return doIsString();
}

sint32 CObject::findIndex(const std::string &/* key */) const
{
	//H_AUTO(R2_CObject_findIndex)
	BOMB("Try to use the method findIndex() on a object that is not an CObjectTable", return 0);
	return 0;
}

bool CObject::isTable(const std::string & prop) const
{
	//H_AUTO(R2_CObject_isTable)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr) return false;
		return attr->isTable();
	}
	return doIsTable();
}

bool CObject::isRefId(const std::string & prop /*=""*/) const
{
	//H_AUTO(R2_CObject_isRefId)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr) return false;
		return attr->isTable();
	}
	return doIsRefId();
}


bool CObject::doIsNumber() const
{
	//H_AUTO(R2_CObject_doIsNumber)
	return false;
}

bool CObject::doIsString() const
{
	//H_AUTO(R2_CObject_doIsString)
	return false;
}

bool CObject::doIsTable() const
{
	//H_AUTO(R2_CObject_doIsTable)
	return false;
}

bool CObject::doIsRefId() const
{
	//H_AUTO(R2_CObject_doIsRefId)
	return false;
}

double CObject::toNumber(const std::string & prop) const
{
	//H_AUTO(R2_CObject_toNumber)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr)
		{
			BOMB("Try to use the method toNumber() on a NULL Object", return 0);
		}
		return attr->doToNumber();
	}
	return doToNumber();
}


std::string CObject::toString(const std::string & prop) const
{
	//H_AUTO(R2_CObject_toString)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr)
		{
			BOMB(NLMISC::toString("Try to access to the property '%s' that does not exist.",prop.c_str()) , return "");
			return "";
		}
		return attr->doToString();
	}
	return doToString();
}


CObjectTable* CObject::toTable(const std::string & prop) const
{
	//H_AUTO(R2_CObject_toTable)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr)
		{
			BOMB("Try to use the method toTable() on a NULL Object", return 0);
		}
		return attr->doToTable();
	}
	return doToTable();
}



CObject* CObject::take(sint32 /* position */)
{
	//H_AUTO(R2_CObject_take)
	BOMB("Try to use the take function on an object that is not a table", return 0);
	return 0;
}

bool CObject::canTake(sint32 /* position */) const
{
	//H_AUTO(R2_CObject_canTake)
	return false;
}

double CObject::doToNumber() const
{
	//H_AUTO(R2_CObject_doToNumber)
	BOMB("Try to convert an objet to number without being allowed", return 0);
	return 0;

}

std::string CObject::doToString() const
{
	//H_AUTO(R2_CObject_doToString)
	BOMB("Try to convert an objet to string without being allowed", return "");
	return "";
}

CObjectTable* CObject::doToTable() const
{
	//H_AUTO(R2_CObject_doToTable)
	BOMB("Try to convert an objet to string without being allowed", return 0);
	return 0;
}

CObject* CObject::getAttr(const std::string & /* name */) const {  return 0;}

std::string CObject::getKey(uint32 /* pos */) const{ BOMB("Try to call the function getKey() on an object that is not a table", return "");  return "";}

CObject* CObject::getValue(uint32 /* pos */) const{ BOMB("Try to call the function getValue() on an object that is not a table", return 0);   return 0;}

uint32 CObject::getSize() const { BOMB("Try to call the function getSize() on an object that is not a table", return 0);  return 0; }

CObject* CObject::clone() const { BOMB("Try to call the function clone() on an object that is not a table", return 0); return 0;}

void CObject::add(const std::string & key,  CObject* value)
{
	//H_AUTO(R2_CObject_add)
	insert(key,  value,  -1);
}

void CObject::add(CObject* value){ add("",  value); }

bool CObject::set(const std::string& /* key */, const std::string & /* value */)
{
	BOMB("Try to set the value of an object with a string on an object that does not allowed it", return false);
	return false;
}

bool CObject::set(const std::string& /* key */, double /* value */){
	BOMB("Try to set the value of an object with a double on an object that does not allowed it", return false);
	return false;
}

bool CObject::setObject(const std::string& /* key */, CObject* /* value */)
{
	BOMB("Try to set the value of an object with an object that does not allowed it", return  false);
	return false;
}

void CObject::serialize(std::string& out) const
{
	//H_AUTO(R2_CObject_serialize)
	CSerializeContext context;
	doSerialize(out,  context);
}
bool CObject::insert( const std::string& /* key */,   CObject* /* value */,  sint32 /* position */)
{
	//H_AUTO(R2_CObject_insert)
	BOMB("Try to call the function insert() on an object that is not a table", return false);
	return false;

}



CObject* CObject::getParent() const
{
	//H_AUTO(R2_CObject_getParent)
	return _Parent;
}

void CObject::setParent(CObject* parent)
{
	//H_AUTO(R2_CObject_setParent)
	_Parent = parent;
}

void CObject::add(const std::string& key,  const std::string & value)
{
	//H_AUTO(R2_CObject_add)
	this->add(key,  new CObjectString(value));
}
void CObject::add(const std::string& key,  double value)
{
	//H_AUTO(R2_CObject_add)
	this->add(key,  new CObjectNumber(value));
}

CObject* CObject::findAttr(const std::string & first) const
{
	//H_AUTO(R2_CObject_findAttr)
	return getAttr(first);
}

CObject* CObject::findAttr(const std::string & first,  const std::string & second) const
{
	//H_AUTO(R2_CObject_findAttr)
	CObject* ret = getAttr(first);
	if (ret) { ret = ret->getAttr(second); }
	return ret;
}

CObject* CObject::findAttr(const std::string & first,  const std::string & second,  const std::string & third) const
{
	//H_AUTO(R2_CObject_findAttr)
	CObject* ret = getAttr(first);
	if (ret) { ret = ret->getAttr(second); }
	if (ret) { ret = ret->getAttr(third); }
	return ret;
}

CObject* CObject::findAttr(const std::vector<std::string>& attrNames ) const
{
	//H_AUTO(R2_CObject_findAttr)
	std::vector<std::string>::const_iterator first(attrNames.begin()),  last(attrNames.end());

	CObject* ret = const_cast<CObject*>(this);

	for ( ; first != last && ret; ++first)
	{
		if (ret) { ret = ret->getAttr(*first); }
	}
	return ret;
}


sint32 CObject::findIndex(const CObject* /* child */) const
{
	//H_AUTO(R2_CObject_findIndex)
	BOMB("Try to call the function findIndex() on an object that is not a table", return 0);
	return 0;
}

bool CObject::getShortestName(std::string &instanceId, std::string &attrName, sint32 &position) const
{
	//H_AUTO(R2_CObject_getShortestName)
	if (isTable() && getAttr("InstanceId"))
	{
		instanceId = getAttr("InstanceId")->toString();
		attrName = "";
		position = -1;
		return true;
	}
	CObject *parent = getParent();
	if (!parent)
	{
		nlassert(0); // TMP : want to see if may possibly happen
		return false;
	}
	if (parent->isTable() && parent->getAttr("InstanceId"))
	{
		sint32 index = parent->findIndex(this);
		nlassert(index != -1);
		instanceId = parent->getAttr("InstanceId")->toString();
		attrName = parent->getKey(index);
		if (attrName.empty())
		{
			position = index;
		}
		return true;
	}
	CObject *parent2 = parent->getParent();
	if (!parent2)
	{
		nlassert(0); // TMP : want to see if may possibly happen
		return false;
	}
	if (parent2->isTable() && parent2->getAttr("InstanceId"))
	{
		sint32 index2 = parent2->findIndex(parent);
		nlassert(index2 != -1);
		sint32 index = parent->findIndex(this);
		nlassert(index != -1);
		if (parent2->getKey(index2).empty())
		{
			nlassert(0); // TMP : want to see if may possibly happen
			return false;
		}
		instanceId = parent2->getAttr("InstanceId")->toString();
		attrName = parent2->getKey(index2);
		position = index;
		return true;
	}
	return false;
}

bool CObject::getNameInParent(std::string &instanceId,
										std::string &attrName,
										sint32 &position) const
{
	//H_AUTO(R2_CObject_getNameInParent)
	CObject *currParent = this->getParent();
	if (!currParent) return false;
	if (currParent->findAttr("InstanceId"))
	{
		// i'm a property in my parent
		instanceId = currParent->findAttr("InstanceId")->toString();
		position = -1;
		attrName = currParent->getKey(currParent->findIndex(this));
		return true;
	}
	sint32 tmpPosition = currParent->findIndex(this);
	CObject *nextParent = currParent->getParent();
	if (!nextParent) return false;
	if (nextParent->findAttr("InstanceId"))
	{
		instanceId = nextParent->findAttr("InstanceId")->toString();
		// i'm a property in my parent
		position = tmpPosition;
		attrName = nextParent->getKey(nextParent->findIndex(currParent));
		return true;
	}
	return false;
}



//----------------------- CObjectString ----------------------------------------

CObjectString::CObjectString(const std::string & value) : CObject(), _Value(value){}


CObjectRefId::~CObjectRefId()
{
	//nlwarning("# Destroying CObjectString 0x%x", (int) this);
}

const char *CObjectString::getTypeAsString() const
{
	//H_AUTO(R2_CObjectString_getTypeAsString)
	return "String";
}


void CObjectString::visitInternal(IObjectVisitor &visitor)
{
	//H_AUTO(R2_CObjectString_visit)
	visitor.visit(*this);
}

void CObjectString::doSerialize(std::string& out,  CSerializeContext& /* context */) const
{
	//H_AUTO(R2_CObjectString_doSerialize)
	nlassert(!getGhost());
	std::string copy;

	std::string::size_type first(0), last(_Value.size());
	for (; first != last ; ++first)
	{
		char c = _Value[first];
		if (c != ']' && c != '[')
		{
			copy += c;
		}
		else
		{
			copy += NLMISC::toString("]]..\"%c\"..[[", c);
		}
	}
	out += "[[" + copy + "]]";
}

bool CObjectString::set(const std::string& key,  const std::string & value)
{
	//H_AUTO(R2_CObjectString_set)

	BOMB_IF( key != "", "Try to set the a sub value of an object that does not allowed it", return false);
	_Value = value;
	return true;
}

bool CObjectString::setObject(const std::string& key,  CObject* value)
{
	//H_AUTO(R2_CObjectString_setObject)
	BOMB_IF( !key.empty() || ! (value->isString() || value->isNumber()) , "Try to set the a sub value of an object that does not allowed it", return false);
	bool canSet = set(key, value->toString());
	if (canSet)
	{
		setGhost(value->getGhost());
	}

	return canSet;

}


void CObjectString::inPlaceCopyTo(CObject &dest) const
{
	//H_AUTO(R2_CObjectString_inPlaceCopyTo)
	dest.inPlaceCopy(*this);
}

void CObjectString::inPlaceCopy(const CObjectString &src)
{
	//H_AUTO(R2_CObjectString_inPlaceCopy)
	_Value = src._Value;
	setGhost(src.getGhost());
}


std::string CObjectString::doToString() const { return _Value;}


CObject* CObjectString::clone() const
{
	//H_AUTO(R2_CObjectString_clone)
	CObjectString *result = new CObjectString(_Value);
	result->setGhost(getGhost());
	return result;
}

bool CObjectString::doIsString() const { return true;}


bool CObjectString::equal(const CObject* other) const
{
	//H_AUTO(R2_CObjectString_equal)
	if (!other || !other->isString()) return false;
	std::string otherValue = other->toString();
	return _Value == otherValue;
}


//----------------------- CObjectRefId ----------------------------------------

CObjectRefId::CObjectRefId(const std::string & value) : CObjectString(value)
{
}

const char *CObjectRefId::getTypeAsString() const
{
	//H_AUTO(R2_CObjectRefId_getTypeAsString)
	return "RefId";
}

CObject* CObjectRefId::clone() const
{
	//H_AUTO(R2_CObjectRefId_clone)
	return new CObjectRefId(*this);
}

void CObjectRefId::visitInternal(IObjectVisitor &visitor)
{
	//H_AUTO(R2_CObjectRefId_visit)
	visitor.visit(*this);
}


bool CObjectRefId::equal(const CObject* other) const
{
	//H_AUTO(R2_CObjectRefId_equal)
	if (!other || !other->isRefId()) return false;
	std::string otherValue = other->toString();
	if (getValue() == otherValue ) return true;

	return false;
}

bool CObjectRefId::doIsRefId() const
{
	//H_AUTO(R2_CObjectRefId_doIsRefId)
	return true;
}

void CObjectRefId::doSerialize(std::string& out,  CSerializeContext& /* context */) const
{
	//H_AUTO(R2_CObjectRefId_doSerialize)
	nlassert(!getGhost());
	//out << "r2.RefId([[" << getValue() << "]])";
	out += "r2.RefId([[" + getValue() + "]])";
}


//----------------------- CObjectNumber ----------------------------------------


CObjectNumber::CObjectNumber(double value) : CObject(), _Value(value){}

const char *CObjectNumber::getTypeAsString() const
{
	return "Number";
}

void CObjectNumber::visitInternal(IObjectVisitor &visitor)
{
	//H_AUTO(R2_CObjectNumber_visit)
	visitor.visit(*this);
}


void CObjectNumber::inPlaceCopyTo(CObject &dest) const
{
	//H_AUTO(R2_CObjectNumber_inPlaceCopyTo)
	dest.inPlaceCopy(*this);
}

void CObjectNumber::inPlaceCopy(const CObjectNumber &src)
{
	//H_AUTO(R2_CObjectNumber_inPlaceCopy)
	_Value = src._Value;
	setGhost(src.getGhost());
}


std::string CObjectNumber::doToString() const { return NLMISC::toString("%d", _Value);}

void CObjectNumber::doSerialize(std::string& out,  CSerializeContext& /* context */) const
{
	//H_AUTO(R2_CObjectNumber_doSerialize)
	nlassert(!getGhost());
	//out.precision(15);
	std::string value = NLMISC::toString(double(sint64(_Value * 1000.0 + (_Value>=0.0?0.5:-0.5)))/1000.0);
	// search for first not 0 value from the end
	std::string::size_type pos = value.find_last_not_of('0');
	if (pos != std::string::npos)
	{
		// don't remove character at pos if it's another digit
		if (value[pos] != '.') ++pos;
		value.erase(pos);
	}
	out += value;
}

bool CObjectNumber::set(const std::string& key,  double value)
{
	//H_AUTO(R2_CObjectNumber_set)

	BOMB_IF(key != "", "Try to set an element of a table on an object that is not a table", return false);

	_Value = value;
	return true;
}


bool CObjectNumber::set(const std::string& key,  const std::string & value)
{
	//H_AUTO(R2_CObjectNumber_set)
	//XXX
	BOMB_IF(key != "", "Try to set an element of a table on an object that is not a table", return false);
//	std::stringstream ss ;
//	ss << value;
//	ss >> _Value;
	NLMISC::fromString(value, _Value);
	return true;
}


double CObjectNumber::doToNumber() const {  return _Value; }

CObject* CObjectNumber::clone() const
{
	//H_AUTO(R2_CObjectNumber_clone)
	CObjectNumber *result = new CObjectNumber(_Value);
	result->setGhost(getGhost());
	return result;
}

bool CObjectNumber::doIsNumber() const  { return true;}



bool CObjectNumber::setObject(const std::string& /* key */,  CObject* value)
{
	//H_AUTO(R2_CObjectNumber_setObject)
	BOMB_IF(!value->isNumber(), NLMISC::toString("Try to set an element of a type '%s' with  a value of type '%s' on an object that is not a number", this->getTypeAsString(), value->getTypeAsString()), return false);

	_Value = value->toNumber();
	setGhost(value->getGhost());
	return true;
}



bool CObjectNumber::equal(const CObject* other) const
{
	//H_AUTO(R2_CObjectNumber_equal)
	if (!other || !other->isNumber()) return false;
	double otherValue = other->toNumber();
	if (_Value == otherValue ) return true;
	/*
	fabs + epsilon trick
	*/
	return false;
}


//----------------------- CObjectTable ----------------------------------------



CObjectTable::CObjectTable() : CObject(){}

void CObjectTable::checkIntegrity() const
{
	//H_AUTO(R2_CObjectTable_checkIntegrity)
	static volatile bool testWanted = true;
	if (!testWanted) return;
	if (_Ghost)
	{
		for(uint k = 0; k < getSize(); ++k)
		{
			CObject *subObj = getValue(k);
			if (subObj)
			{
				if (!subObj->getGhost())
				{
					// dump whole hierarchy
					const CObject *topParent = this;
					while (topParent->getParent()) topParent = topParent->getParent();
					topParent->dump();
					nlwarning("Check Integrity failed.");
					return;
				}
				subObj->checkIntegrity();
			}
		}
	}
}


void CObjectTable::setGhost(bool ghost)
{
	//H_AUTO(R2_CObjectTable_setGhost)
	CHECK_TABLE_INTEGRITY

	_Ghost = ghost;

	CObject* parent = this->getParent();
	if (!ghost && parent)
	{
		sint32 index = parent->findIndex(this);
		if (index != -1)
		{
			std::string key = parent->getKey( static_cast<uint32>(index));

			if (key == "Ghosts" )
			{
				return;
			}
		}
	}

	for(uint k = 0; k < getSize(); ++k)
	{
		CObject *subObj = getValue(k);
		if (subObj)
		{
			subObj->setGhost(ghost);
		}
	}
}

void CObjectTable::previsit(std::vector<CObject::TRefPtr> &sons)
{
	sons.push_back(this);
	sons.reserve(sons.size() + getSize());
	for(uint k = 0; k < getSize(); ++k)
	{
		CObject *subObj = getValue(k);
		if (subObj)
		{
			subObj->previsit(sons);
		}
	}
}


void CObjectTable::visitInternal(IObjectVisitor &visitor)
{
	visitor.visit(*this);
}


void CObjectTable::inPlaceCopyTo(CObject &dest) const
{
	//H_AUTO(R2_CObjectTable_inPlaceCopyTo)
	CHECK_TABLE_INTEGRITY
	dest.inPlaceCopy(*this);
}

void CObjectTable::inPlaceCopy(const CObjectTable &src)
{
	//H_AUTO(R2_CObjectTable_inPlaceCopy)
	CHECK_TABLE_INTEGRITY
	for(uint k = 0; k < src.getSize(); ++k)
	{
		if (!src.getKey(k).empty())
		{
			CObject *dest = getAttr(src.getKey(k));
			if (!dest)
			{
				nlwarning("No dest object %s found when copying in place", src.getKey(k).c_str());
			}
			else
			{
				src.getValue(k)->inPlaceCopyTo(*dest);
			}
		}
		else
		{
			nlwarning("In place copy of objects with a number as key not supported");
		}
	}
}


CObjectTable::~CObjectTable()
{
	{
		CHECK_TABLE_INTEGRITY
	}
	TContainer::iterator first(_Value.begin()),  last(_Value.end());
	for ( ;first != last; ++first )
	{
		delete first->second;
		first->second = 0;
	}
	_Value.clear();
}

const char *CObjectTable::getTypeAsString() const
{
	//H_AUTO(R2_CObjectTable_getTypeAsString)
	return "Table";
}


//complexity  less than
void CObjectTable::sort()
{
	//H_AUTO(R2_CObjectTable_sort)
	CHECK_TABLE_INTEGRITY
	std::vector < std::pair<std::string,  CObject*> > data;
	std::vector<std::string> keyVector;

	CObject* keys = getAttr("Keys");
	if (!keys || !keys->isTable()) return;

	uint32 firstKey = 0;
	uint32 lastKey = keys->getSize();
	for (; firstKey != lastKey ; ++firstKey)
	{
		CObject* keyObject = keys->getValue(firstKey);
		if (! keyObject->isString()) return;
		std::string key = keyObject->toString();

		uint32 firstValue = 0;
		uint32 lastValue = (uint32)_Value.size();
		for (; firstValue != lastValue; ++firstValue)
		{
			if ( key == _Value[firstValue].first)
			{
				data.push_back( _Value[firstValue]);
				_Value[firstValue].second = 0;
			}
		}
	}
	{
		uint32 firstValue = 0;
		uint32 lastValue = (uint32)_Value.size();
		for (; firstValue != lastValue; ++firstValue)
		{
			if (  _Value[firstValue].first != "Keys" && _Value[firstValue].second != 0)
			{
				data.push_back( _Value[firstValue]);
			}
		}

	}
	_Value.swap(data);

	delete keys;
}

CObject* CObjectTable::clone() const
{
	//H_AUTO(R2_CObjectTable_clone)
	CHECK_TABLE_INTEGRITY
	CObject* ret = new CObjectTable();
	TContainer::const_iterator first(_Value.begin()),  last(_Value.end());
	for ( ;first != last; ++first )
	{
		BOMB_IF(!first->second, "Try to clone a table with an NULL component", return 0)
		nlassert(first->second->getGhost() == this->getGhost());
		CObject* clone = first->second->clone();
		if (clone) { clone->setParent(0); }
		ret->add(first->first,  clone);
	}
	ret->setGhost(getGhost());
	#ifdef NL_DEBUG
		ret->checkIntegrity();
	#endif
	return ret;
}


namespace
{
	struct CValueIndex
	{
		CValueIndex(unsigned int index, const std::string& name,  bool isTable, bool isDefaultFeature)
			:Index(index), Name(name), IsTable(isTable), IsDefaultFeature(isDefaultFeature){}

		bool operator < (const CValueIndex& rh) const
		{
			// container after values
			if ( !IsTable && rh.IsTable ) { return true; }
			if ( IsTable && !rh.IsTable ) { return false; }


			if ( !IsTable)
			{
				if ( Name == "InstanceId" &&  (rh.Name != "InstanceId" ) ) { return true;}
				if ( Name != "InstanceId" &&  (rh.Name == "InstanceId" ) ) { return false;}

				if ( Name == "Class" &&  (rh.Name != "Class" ) ) { return true;}
				if ( Name != "Class" &&  (rh.Name == "Class" ) ) { return false;}

				if ( Name == "Version" &&  (rh.Name != "Version" ) ) { return true;}
				if ( Name != "Version" &&  (rh.Name == "Version" ) ) { return false;}

			}

			if (IsTable && rh.IsTable)
			{

				if (IsDefaultFeature) { return true; }
				if (rh.IsDefaultFeature) { return false; }

				return Name < rh.Name;
			}
			return Name < rh.Name;
		}

		uint32 Index;
		std::string Name;
		bool IsTable;
		bool IsDefaultFeature;
	};
}
void CObjectTable::doSerialize(std::string& out,  CSerializeContext& context) const
{
	//H_AUTO(R2_CObjectTable_doSerialize)


	std::vector<CValueIndex> indexs;
	uint32 i = 0;
	uint32 size = (uint32)_Value.size();
	indexs.reserve(size);
	for (; i < size ; ++i)
	{
		bool isDefault= false;
		if (_Value[i].second->isTable() && _Value[i].second->isString("Class"))
		{
			std::string cl = _Value[i].second->toString("Class");
			if (cl == "DefaultFeature")
			{
				isDefault = true;
			}
			else if ( cl == "Act")
			{
				if ( _Value[i].second->isString("Name")
					&&  _Value[i].second->isString("LocationId")
					&& _Value[i].second->toString("Name") == "Permanent"
					&& _Value[i].second->toString("LocationId").empty())
				{
					isDefault = true;
				}
			}

		}
		indexs.push_back( CValueIndex(i, _Value[i].first, _Value[i].second->isTable(), isDefault) );
	}
	std::stable_sort(indexs.begin(), indexs.end());

	CHECK_TABLE_INTEGRITY
	if (getGhost())
	{
		nlwarning("Try to serialize a ghost Component");
		return;
	}
	nlassert(!getGhost());
	sint32 indent = context.getIndent();

	out += "{";

	uint32 j = 0;
	context.add();
	for (; j < size ; ++j)
	{

		uint32 i = indexs[j].Index;
		static const std::string ghostStr = "Ghost_";
		static const std::string::size_type ghostStrSize = ghostStr.size();
		std::string key = _Value[i].first;

		if (!_Value[i].second->getGhost() && !(key.size() > ghostStrSize && key.substr(0, ghostStrSize) == ghostStr))
		{
			out += "\n";
			addTab(out,  context.getIndent());

			if (!_Value[i].first.empty())
			{
				out += _Value[i].first + " = ";
			}

			_Value[i].second->doSerialize(out,  context);

		//	if (j  != size -1)
			{
				out += ",  ";
			}
		}
	}
	context.sub();

	out += "\n";
	addTab(out,  indent);
	out += "}";
}

CObject* CObjectTable::getAttr(const std::string & name) const
{
	//H_AUTO(R2_CObjectTable_getAttr)
	CHECK_TABLE_INTEGRITY
	//search by position
	//XXX
	if (name.size() >= 1 && '0' <= name[0] && name[0] <='9')
	{
		uint32 first2 = 0;
		uint32 end2 = (uint32)name.size();
		for ( ; first2 != end2 && '0' <= name[first2] && name[first2] <= '9'; ++first2)	{}
		if (first2 == end2)
		{
			uint32 position;
			NLMISC::fromString(name, position);
			if (position < _Value.size())
			{
				return _Value[position].second;
			}
		}
	}


	//search by name
	TContainer::const_iterator first(_Value.begin());
	TContainer::const_iterator last(_Value.end());
	for (; first != last ;++first)
	{
		if (first->first == name)
		{
			return first->second;
		}

	}

	return 0;
}

std::string CObjectTable::getKey(uint32 pos) const
{
	//H_AUTO(R2_CObjectTable_getKey)
	CHECK_TABLE_INTEGRITY
	if (pos >= _Value.size())
	{
		nlwarning("<CObjectTable::getKey> bad index %d", pos);
		return "";
	}
	return _Value[pos].first;
}

CObject* CObjectTable::getValue(uint32 pos) const
{
	//H_AUTO(R2_CObjectTable_getValue)
	if (pos >= _Value.size())
	{
		nlwarning("<CObjectTable::getValue> bad index %d", pos);
		return NULL;
	}
	return _Value[pos].second;
}

bool CObjectTable::set(const std::string& key,  const std::string & value)
{
	//H_AUTO(R2_CObjectTable_set)
	CHECK_TABLE_INTEGRITY
	CObject *attr = getAttr(key);
	if (!attr)
	{
		CObject *str = new CObjectString(value);
		str->setGhost(this->getGhost());
		this->add(key, str);
		//nlwarning("Can't find attribute '%s' in object of class '%s'",  key.c_str(),  toString("Class").c_str());
		return true;
	}
	return attr->set("",  value);

}

bool CObjectTable::set(const std::string& key,  double value)
{
	//H_AUTO(R2_CObjectTable_set)
	CHECK_TABLE_INTEGRITY
	CObject *attr = getAttr(key);
	if (!attr)
	{
		//nlwarning("Can't find attribute '%s' in object of class '%s'",  key.c_str(),  toString("Class").c_str());
		CObject *nbr = new CObjectNumber(value);
		nbr->setGhost(this->getGhost());
		this->add(key, nbr);
		return true;
	}
	return attr->set("",  value);
}

bool CObjectTable::setObject(const std::string& key,  CObject* value)
{
	//H_AUTO(R2_CObjectTable_setObject)
	CHECK_TABLE_INTEGRITY
	value->setGhost(this->getGhost());
	if (key.empty())
	{

		clear();
		uint32 first = 0;
		CObject* table = value;
		//check
		uint32 last = table->getSize();
		for ( ; first != last ; ++first)
		{
			std::string key1 = table->getKey(first);
			CObject* value1 = table->getValue(first);
			add(key1,  value1->clone());
		}
		return true;


		//getAttrId
		//remove
	}
	else
	{

		CObject* attr=	getAttr(key);
		if (!attr)
		{
			this->add(key, value->clone());
			return true;
		}
		if (attr){ return attr->setObject("",  value); }
	}
	return false;
}

void CObjectTable::clear()
{
	//H_AUTO(R2_CObjectTable_clear)
	CHECK_TABLE_INTEGRITY
	TContainer::iterator first(_Value.begin());
	TContainer::iterator last(_Value.end());
	for (; first != last  ;++first)
	{
		delete first->second;
	}
	_Value.clear();
}

uint32 CObjectTable::getSize() const
{
	//H_AUTO(R2_CObjectTable_getSize)
	return (uint32)_Value.size();
}

bool CObjectTable::doIsTable() const { return true;}

CObjectTable* CObjectTable::doToTable() const { return const_cast<CObjectTable*>(this);}


sint32 CObjectTable::findIndex(const CObject* child) const
{
	//H_AUTO(R2_CObjectTable_findIndex)
	CHECK_TABLE_INTEGRITY
	uint32 first = 0, last = (uint32)_Value.size();
	for (; first != last && _Value[first].second != child ; ++first){}
	if (first == last) return -1;
	return first;
}

sint32 CObjectTable::findIndex(const std::string &key) const
{
	//H_AUTO(R2_CObjectTable_findIndex)
	CHECK_TABLE_INTEGRITY
	uint32 first = 0, last = (uint32)_Value.size();
	for (; first != last && _Value[first].first != key ; ++first){}
	if (first == last) return -1;
	return first;
}

CObject* CObjectTable::take(sint32 position)
{
	//H_AUTO(R2_CObjectTable_take)
	CHECK_TABLE_INTEGRITY
	BOMB_IF(!( -1 <= position && position < static_cast<sint32>(_Value.size())), "Try to take an element that does not exist", return 0);

	if (0 <=  position && position < static_cast<sint32>(_Value.size()))
	{
		CObject* child = _Value[ static_cast<uint32>(position) ].second;
		_Value.erase(_Value.begin() + static_cast<uint32>(position));
		child->setParent(0);
		return child;
	}
	else if (position == -1)
	{
		CObject* parent = getParent();
		if (!parent)
		{
			return this;
		}

		uint32 pos = getParent()->findIndex(this);
		BOMB_IF(pos >= getParent()->getSize(), "Try to take an element that does not exist", return 0);
		CObject* child = getParent()->take(pos);
		return child;
	}

	return 0;
}


bool CObjectTable::canTake(sint32 position) const
{
	//H_AUTO(R2_CObjectTable_canTake)
	CHECK_TABLE_INTEGRITY
	if(!( -1 <= position && position < static_cast<sint32>(_Value.size())))
	{
		return false;
	}

	if (position == -1)
	{

		CObject* parent = getParent();
		if (parent) //try to take the root of a tree
		{
			return true;
		}
		uint32 pos = parent->findIndex(this);
		BOMB_IF(pos >= getParent()->getSize(), "Try to take an element that does not exist", return false);
		return getParent()->canTake(pos);
	}

	return true;
}



bool CObjectTable::insert(const std::string& key,  CObject* value,  sint32 position)
{
	//H_AUTO(R2_CObjectTable_insert)
	CHECK_TABLE_INTEGRITY
	uint32 count = (uint32)_Value.size();

	BOMB_IF(!( -1 <= position && position <= static_cast<sint32>(count)), "Try to take an element that does not exist", return false);
	BOMB_IF(!value, "Try to insert a Null value", return false);
	BOMB_IF(value->getParent() != 0, "Try to insert an element that not at the root of the tree.", return false);

	value->setParent(this);
	// inherit the 'ghost' flag
	if (this->getGhost())
	{
		value->setGhost(true);
	}


	if (0<= position && position < static_cast<sint32>(count))
	{

		_Value.insert(_Value.begin() + position,  std::pair<std::string,  CObject*>(key,  value));

	}
	else
	{
		_Value.push_back(std::pair<std::string,  CObject*>(key,  value));
	}
	return true;

}

bool CObjectTable::equal(const CObject* other) const
{
	//H_AUTO(R2_CObjectTable_equal)
	CHECK_TABLE_INTEGRITY
	if (!other || !other->isTable()) return false;
	#ifdef NL_DEBUG
		other->checkIntegrity();
	#endif
	uint32 size;
	size = getSize();
	if (size != other->getSize()) return false;

	uint i = 0;
	for ( i=0; i!= size ; ++i )
	{
		std::string key = other->getKey(i);
		CObject* value = other->getValue(i);
		if (key != this->getKey(i)) return false;
		if ( !value->equal(this->getValue(i))) return false;
	}
	return true;
}



//----------------------- CObjectTable ----------------------------------------


CTypedObject::CTypedObject(const std::string & type): CObjectTable()
{
	if (!type.empty())
	{
		add("Class",  type);
	}
}
bool CTypedObject::isOk() const { return true;}



//-------------------------------
//----------------------
std::string CNameGiver::getNewName(const std::string & type,  sint32 id)
{
	//H_AUTO(R2_CNameGiver_getNewName)
	//XX To change
//	std::stringstream ss;
//	std::string ret = type;
//	if (id == -1)
//	{
//		id = getNewId(type);
//	}
//	ss << ret << "_" << id;
//	return ss.str();

	std::string ret = type;
	if (id == -1)
	{
		id = getNewId(type);
	}

	return ret + "_" + NLMISC::toString(id);
}

sint32 CNameGiver::getNewId(const std::string & type)
{
	//H_AUTO(R2_CNameGiver_getNewId)

	std::map< std::string,  sint32>::const_iterator found = _Value.find(type);
	if (found == _Value.end())
	{
		_Value[type] = 0;
	}
	return ++_Value[type];
}

CNameGiver::CNameGiver()
{

}

void CNameGiver::setMaxId(const std::string& eid,sint32 id)
{
	//H_AUTO(R2_CNameGiver_setMaxId)
	_Value[eid]=id;
}

sint32 CNameGiver::getMaxId(const std::string& eid)
{
	//H_AUTO(R2_CNameGiver_getMaxId)
	return _Value[eid];
}

void CNameGiver::clear()
{
	//H_AUTO(R2_CNameGiver_clear)
	_Value.clear();
}
//----------------------------------------------
void CObjectFactory::clear()
{
	//H_AUTO(R2_CObjectFactory_clear)
	_NameGiver->clear();
}

CObject* CObjectFactory::newBasic(const std::string & type)
{
	//H_AUTO(R2_CObjectFactory_newBasic)
	if (type == "RefId")
	{
		return new CObjectRefId("");
	}
	else
	if (type == "String")
	{
		return new CObjectString("");
	}
	else if (type == "Number")
	{
		return new CObjectNumber(0);
	}
	else if (type == "Table")
	{
		return new CObjectTable();
	}
	return 0;
}

CObject* CObjectFactory::newAvanced(const std::string & type)
{
	//H_AUTO(R2_CObjectFactory_newAvanced)

	CObjectGenerator* ret = getGenerator(type);
	if (ret) { return ret->instanciate(this); }
	return 0;
}

void CObjectFactory::registerGenerator(CObject* objectClass)
{
	//H_AUTO(R2_CObjectFactory_registerGenerator)
	nlassert(objectClass->isString("Name"));
	std::string classType = objectClass->toString("Name");
	bool exist=_Map.insert(std::make_pair(classType,new CObjectGenerator(objectClass, this))).second;
	if (!exist)
	{
		nlwarning("Generator with name %s already exists", classType.c_str());
	}
}

CObjectFactory::CObjectFactory(const std::string & prefix)
{
	//H_AUTO(R2_CObjectFactory_CObjectFactory)
	_NameGiver = new CNameGiver();
	_Prefix = prefix;
}

void CObjectFactory::setPrefix(const std::string & prefix)
{
	//H_AUTO(R2_CObjectFactory_setPrefix)
	_Prefix = prefix;
}


CObjectFactory::~CObjectFactory()
{
	delete _NameGiver;
	//delete map
	std::map<std::string, CObjectGenerator*>::iterator it = _Map.begin();
	while( it != _Map.end() )
		delete (*it++).second;
	_Map.clear();
}


void CObjectFactory::setMaxId(const std::string& eid,sint32 id)
{
	//H_AUTO(R2_CObjectFactory_setMaxId)
	_NameGiver->setMaxId(eid,id);
}


sint32 CObjectFactory::getMaxId(const std::string& eid) const
{
	//H_AUTO(R2_CObjectFactory_getMaxId)
	return _NameGiver->getMaxId(eid);
}


CObject* CObjectFactory::newComponent(const std::string & type)
{
	//H_AUTO(R2_CObjectFactory_newComponent)
	CObject* ret = 0;
	ret = newBasic(type);
	if (ret) return ret;
	ret = newAvanced(type);

	if (!ret)
	{
		nlwarning("Component not found : %s",  type.c_str());
	}

	return ret;
}
std::string CObjectFactory::getNewName(const std::string& type) const
{
	//H_AUTO(R2_CObjectFactory_getNewName)
	if (type.empty())
	{
		return _NameGiver->getNewName(_Prefix);
	}
	return _NameGiver->getNewName(type);
}

//-------------------------

CClass::CClass(const std::string & classType):CObjectTable()
{
	CObjectTable* prop = new CObjectTable(); // never accessed by client
	add("name",  classType);
	add("prop",  prop);
	_ClassType = classType;
}


void CClass::addAttribute(const std::string & name,  const std::string & type)
{
	//H_AUTO(R2_CClass_addAttribute)
	findAttr("Prop")->add( "",  new CClassAttribute(name,  type));
}

void CClass::addAttribute(const std::string & name,  const std::string & type,  const std::string & value)
{
	//H_AUTO(R2_CClass_addAttribute)
	findAttr("Prop")->add( "",  new CClassAttribute(name,  type,  value));
}


//-------------

CObjectGenerator::~CObjectGenerator()
{
	delete _ObjectClass;
	TDefaultValues::iterator first(_DefaultValues.begin()), last(_DefaultValues.end());
	for (;first != last; ++first)
	{
		CObject* data = first->second;
		delete data;
	}
}


CObjectGenerator * CObjectFactory::getGenerator(const std::string & type)
{
	//H_AUTO(R2_CObjectFactory_getGenerator)
	std::map<std::string,  CObjectGenerator*>::const_iterator found(_Map.find(type));
	if (found != _Map.end())
	{
		CObjectGenerator* ret = found->second;
		return ret;
	}
	return 0;
}

void CObjectGenerator::createDefaultValues(CObjectFactory* factory)
{
	//H_AUTO(R2_CObjectGenerator_createDefaultValues)
	if (!_ObjectClass) return;


	CObject* prop = _ObjectClass->getAttr("Prop");
	nlassert(prop);
	uint32 first = 0;
	uint32 last = prop->getSize();
	for ( ; first != last ; ++first )
	{
		CObject* found = prop->getValue(first);

		if (found && found->isString("DefaultValue") && found->isString("Name") )
		{

			std::string type = found->toString("Type");
			std::string name = found->toString("Name");

			CObject* instance = factory->newComponent(type);
			if (!instance)
			{
				nlwarning("<CObjectGenerator::instanciate> Can't create component of type %s", type.c_str());
				return;
			}

			std::string value =	found->toString("DefaultValue");
			instance->set("", value);
			_DefaultValues.insert( std::make_pair(name, instance));
		}
	}

}

std::string CObjectGenerator::getBaseClass() const
{
	//H_AUTO(R2_CObjectGenerator_getBaseClass)
	if (!_ObjectClass) return "";
	if ( _ObjectClass->isString("BaseClass"))
	{
		return _ObjectClass->toString("BaseClass");
	}
	return "";
}

CObject* CObjectGenerator::getDefaultValue(const std::string & propName) const
{
	//H_AUTO(R2_CObjectGenerator_getDefaultValue)
	TDefaultValues::const_iterator found ( _DefaultValues.find(propName) );
	if (found != _DefaultValues.end())
	{
		return found->second;
	}
	return 0;
}

CObject* CObjectGenerator::instanciate(CObjectFactory* factory) const
{
	//H_AUTO(R2_CObjectGenerator_instanciate)
	CObject* toRet = newTable(factory);
	CObject* objectClassType = _ObjectClass->getAttr("Name");
	nlassert( objectClassType && objectClassType->isString());
	std::string classType =  objectClassType->toString();
	toRet->add("Class",  classType);

	CObject* prop = _ObjectClass->getAttr("Prop");
	nlassert(prop);
	uint32 first = 0;
	uint32 last = prop->getSize();
	for ( ; first != last ; ++first )
	{
		CObject* found = prop->getValue(first);

		if (found && found->isString("Type") )
		{
			CObject*defaultInBase = found->getAttr("DefaultInBase");

			if (!defaultInBase || (defaultInBase->isNumber() && defaultInBase->toNumber() != 1))
			{

				std::string type = found->toString("Type");
				/*
					:XXX:
					Default Value = NIL
				*/
				CObject* instance = factory->newComponent(type);
				if (!instance)
				{
					nlwarning("<CObjectGenerator::instanciate> Can't create component of type %s", type.c_str());
				}
				else
				{
					nlassert(instance);

					std::string name = found->toString("Name");
					toRet->add(name,  instance);

					if (found->isString("DefaultValue"))
					{
						std::string value =  found->toString("DefaultValue");
						instance->set("",  value);
					}

					if (name == "InstanceId")
					{
						std::string value = factory->getNewName();
						instance->set("",  value);
					}
					else if (name == "Id")
					{
						std::string value = factory->getNewName(classType);
						instance->set("",  value);
					}
				}
			}
		}
		else
		{
			if (!found)
			{
				nlwarning("Field 'Type' not found for a property in class %s",  classType.c_str());
			}
			else
			{
				nlwarning("Field 'Type' shoud has type 'String'  (class = %s)",  classType.c_str());
			}
		}

	}
	return toRet;
}


void CObjectString::dump(const std::string prefix, uint depth) const
{
	//H_AUTO(R2_CObjectString_dump)
	std::string result(depth * 4, ' ');
	result += NLMISC::toString("%sString, ptr = Ox%p, value = %s, ghost = %s", prefix.c_str(), this, _Value.c_str(), _Ghost ? "true" : "false");
	nlwarning(result.c_str());
}

void CObjectNumber::dump(const std::string prefix, uint depth) const
{
	//H_AUTO(R2_CObjectNumber_dump)
	std::string result(depth * 4, ' ');
	result += NLMISC::toString("%sNumber, ptr = 0x%p, value = %f, ghost = %s", prefix.c_str(), this, _Value, _Ghost ? "true" : "false");
	nlwarning(result.c_str());
}

void CObjectTable::dump(const std::string prefix, uint depth) const
{
	//H_AUTO(R2_CObjectTable_dump)
	std::string result(depth * 4, ' ');
	result += NLMISC::toString("%sTable, ptr = 0x%p, , ghost = %s", prefix.c_str(), this, _Ghost ? "true" : "false");
	nlwarning(result.c_str());
	for(uint k = 0; k < _Value.size(); ++k)
	{
		std::string prefix = NLMISC::toString("Index = %d, key = %s ", (int) k, _Value[k].first.c_str());
		_Value[k].second->dump(prefix, depth + 1);
	}
}

	enum
	{
		ObjectNull, ObjectString, ObjectNumber, ObjectTable,
		ObjectNumberZero, ObjectNumberSInt32, ObjectNumberUInt32, ObjectNumberSInt16, ObjectNumberUInt16, ObjectNumberSInt8, ObjectNumberUInt8, ObjectNumberFloat,
		ObjectStringEmpty, ObjectString8, ObjectString16,
		ObjectTablePosition, ObjectTableNpc, ObjectTableNpcCustom, ObjectTableWayPoint, ObjectTableRegionVertex,
		ObjectTableRegion, ObjectTableRoad,
		ObjectTableNpcGrpFeature, ObjectTableBehavior,

		ObjectTableActivitySequence, ObjectTableActivityStep,
		ObjectTableChatSequence, ObjectTableChatStep,ObjectTableChatAction,
		ObjectTableLogicEntityAction,
		ObjectTableActionStep, ObjectTableActionType, ObjectTableEventType, ObjectTableLogicEntityReaction,
		ObjectTableTextManagerEntry, ObjectTableConditionStep, ObjectTableConditionType,


		ObjectTableRtAct, ObjectTableRtNpcGrp, ObjectTableRtNpc, ObjectTableRtPosition,
		RtAiState, ObjectTableRtAiState, ObjectTableRtNpcEventHandler, ObjectTableRtNpcEventHandlerAction,
		ObjectRefIdEmpty, ObjectRefId8, ObjectRefId16, ObjectRefId,

		ObjectNumberDouble, ObjectHeaderTag // If we "sort" this list we bust remove old session save
	};


const double sint8Min = -128.;
const double sint8Max = 127.;
const double uint8Max = 255.;
const double sint16Min = -32768.;
const double sint16Max = 32767.;
const double uint16Max = 65535.;

const double sint32Min = -2147483648.;
const double sint32Max = 2147483647.;
const double uint32Max = 4294967295.;


uint32 CObject::instanceIdToUint32(const std::string& instanceId)
{
	//H_AUTO(R2_CObject_instanceIdToUint32)
	if (instanceId.empty()) return 0;
	uint32 size = (uint32)instanceId.size();
	if ( instanceId.substr(0, 6) != "Client")
	{
		nlwarning("R2Share: Wrong InstanceId(%s)", instanceId.c_str());
		return 0;
	}

	std::string::size_type clientIdIt= instanceId.find("_", 6);

	if (clientIdIt == std::string::npos)
	{
		nlwarning("R2Share: Wrong InstanceId(%s)", instanceId.c_str());
		return 0;
	}

	std::string clientIdStr = instanceId.substr(6,  clientIdIt-6);
	std::string componentIdStr = instanceId.substr(clientIdIt+1,  size - clientIdIt);
	bool ko;
	uint32 clientId;
	ko = NLMISC::fromString(clientIdStr, clientId);
	if (!ko)
	{
		nlwarning("R2Share: Wrong InstanceId(%s)", instanceId.c_str());
		return 0;
	}

	uint32 componentId;
	ko = NLMISC::fromString(componentIdStr, componentId);
	if (!ko)
	{
		nlwarning("R2Share: Wrong InstanceId(%s)", instanceId.c_str());
		return 0;
	}
	return (0xff000000 &  clientId << 24) | (componentId & 0x00ffffff);
}


std::string CObject::uint32ToInstanceId(uint32 id)
{
	//H_AUTO(R2_CObject_uint32ToInstanceId)
	if (id == 0) return "";
	uint32 clientId = (id>> 24) & 0x000000ff;
	uint32 componentId = (id & 0x00ffffff);
	return NLMISC::toString("Client%d_%d", clientId, componentId);
}

static void writeNumber( NLMISC::IStream& stream, double theValue)
{
		double value = theValue;
		double absValue = fabs(value);
		uint8 type;

		// It's 0
		if (absValue <= std::numeric_limits<double>::epsilon())
		{
			type = ObjectNumberZero;
			stream.serial(type);
			return;
		}

		double integral;

		double fractional = modf(absValue, &integral);

		// It is an integral type (no fractional part)
		if ( fractional <= std::numeric_limits<double>::epsilon() )
		{
			bool pos = 0.0 <= value;
			// positif
			if (pos)
			{
				if (integral <=  uint8Max)
				{
					uint8 uint8value = static_cast<uint8>(value);
					type = ObjectNumberUInt8;
					stream.serial(type);
					stream.serial( uint8value);
					return;
				}

				if (integral <= uint16Max)
				{
					uint16 uint16value = static_cast<uint16>(value);
					type = ObjectNumberUInt16;
					stream.serial(type);
					stream.serial(uint16value);
					return;
				}

				if (integral <= uint32Max)
				{
					uint32 uint32value = static_cast<uint32>(value);
					type = ObjectNumberUInt32;
					stream.serial(type);
					stream.serial(uint32value);
					return;
				}
			}
			//negatif
			else
			{
				if ( sint8Min <= integral && integral <=  sint8Max)
				{
					sint8 sint8value = static_cast<sint8>(value);
					type = ObjectNumberSInt8;
					stream.serial(type);
					stream.serial( sint8value);
					return;
				}

				if ( sint16Min <= integral && integral <=  sint16Max)
				{
					sint16 sint16value = static_cast<sint16>(value);
					type = ObjectNumberSInt16;
					stream.serial(type);
					stream.serial( sint16value);
					return;
				}

				if ( sint32Min <= integral && integral <=  sint32Max)
				{
					sint32 sint32value = static_cast<sint32>(value);
					type = ObjectNumberSInt32;
					stream.serial(type);
					stream.serial( sint32value);
					return;
				}

			}

		}

		//Default case
		// Float are evil: you loose too much precision
		type = ObjectNumberDouble;
		double fValue = value;
		stream.serial(type);
		stream.serial(fValue);
}

static void serialStringInstanceId( NLMISC::IStream& stream, CObject*& data)
{
	if (!stream.isReading())
	{
		uint32 instanceId = CObject::instanceIdToUint32(data->toString());
		stream.serial(instanceId);
	}
	else
	{
		uint32 instanceId;
		stream.serial(instanceId);
		std::string strInstanceId = CObject::uint32ToInstanceId(instanceId);
		data = new CObjectString(strInstanceId);
	}
}

void CObjectSerializer::serialStringInstanceId( NLMISC::IStream& stream, std::string& data)
{
	//H_AUTO(R2_CObjectSerializer_serialStringInstanceId)
	if (!stream.isReading())
	{
		uint32 instanceId = CObject::instanceIdToUint32(data);
		stream.serial(instanceId);
	}
	else
	{
		uint32 instanceId;
		stream.serial(instanceId);
		data= CObject::uint32ToInstanceId(instanceId);
	}
}


void CObjectSerializer::swap(CObjectSerializer& other)
{
	//H_AUTO(R2_CObjectSerializer_swap)
	std::swap(this->_Data, other._Data);
	std::swap(this->_Compressed, other._Compressed);
	std::swap(this->_MustUncompress, other._MustUncompress);
	std::swap(this->_CompressedBuffer, other._CompressedBuffer);
	std::swap(this->_CompressedLen, other._CompressedLen);
	std::swap(this->_UncompressedLen, other._UncompressedLen);
	std::swap(this->Level, other.Level);
	std::swap(this->Log, other.Log);
}



static void serialNumberFixedPoint( NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer )
{
	if (serializer->getVersion() == 0)
	{
		nlwarning("Using oldScenario Version (must only be use when loading) old scenarios scenario session");
		if (!stream.isReading())
		{
			sint32 v = static_cast<sint32>(data->toNumber()* 64);
			stream.serial(v);
		}
		else
		{
			sint32 v;
			stream.serial(v);
			data = new CObjectNumber( 1.0 * v / 64.0);
		}
	}
	else
	{
		if (!stream.isReading())
		{

			double val = data->toNumber();
			sint32 v = static_cast<sint32>( val * 1000.0  + (val >= 0.0? 0.5: -0.5));
			stream.serial(v);
//			nldebug("serial > %f %d ", val, v);
		}
		else
		{
			sint32 v;
			stream.serial(v);
			double val = double(v) / 1000.0;
			data = new CObjectNumber( val );
//			nldebug("serial < %f %d", val, v);
		}
	}

}

class CClassSerializer;

class CObjectSerializerImpl : public NLMISC::CSingleton<CObjectSerializerImpl>
{
public:
	~CObjectSerializerImpl();

	CObjectSerializerImpl();

	// release singleton
	static void releaseInstance();

	void registerSerializer(CClassSerializer* serializer);

	void serialImpl(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, bool init = false);

private:
	std::map<uint8, CClassSerializer*> _ClassSerializersById;
	std::map<std::string, CClassSerializer*> _ClassSerializersByName;
};


class CClassSerializer
{
public:
	virtual void serialClass(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer)
	{
		if (!stream.isReading())
		{
			//serial other prop;

			std::vector<uint32> optionalPropFoundIndex;
			std::vector<uint32> otherPropFoundIndex;
			std::vector<uint32> valuePropFoundIndex;


			/*
				make difference between properties
			*/
			{
				uint32  first=0;
				uint	last=data->getSize();

				for (; first != last; ++first)
				{
					std::string key = data->getKey(first);
					if (key == "Class")
					{

					}
					else if ( key.empty() )
					{
						valuePropFoundIndex.push_back(first);
					}
					else if (std::find(_NeededProp.begin(), _NeededProp.end(), key) == _NeededProp.end())
					{
						std::vector<std::string>::const_iterator optionalFound = std::find(_OptionalProp.begin(), _OptionalProp.end(), key);
						if (optionalFound != _OptionalProp.end())
						{
							optionalPropFoundIndex.push_back(first);
						}
						else
						{
							otherPropFoundIndex.push_back(first);
						}
					}

				}
			}

			uint8 id = getId();
			stream.serial(id);

			uint8 next = 0;
			if (!_NeededProp.empty()) next |= 1;
			if (!optionalPropFoundIndex.empty()) next |= 2;
			if (!valuePropFoundIndex.empty()) next |= 4;
			if (!otherPropFoundIndex.empty()) next |= 8;


			stream.serial(next);
			/*
				Serial needed property
			*/
			if (!_NeededProp.empty())
			{
				//no need to put a number we already known how meany object must be
				std::vector<std::string>::const_iterator  first(_NeededProp.begin()), last(_NeededProp.end());

				uint initLength = stream.getPos();
				serializer->Level +=2;
				for ( ; first != last ; ++first)
				{
					//*first => Property key ()
					CObject* value = data->getAttr(*first);
					if( !value )
					{
						nlwarning( "About to serialize a NULL value at key %s, className %s!",
							first->c_str(), _ClassName.c_str() );
					}
					if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, first->c_str());}
					onSerial(stream, *first, value, serializer);
				}
				serializer->Level -=2;
				uint endLength = stream.getPos();
				if (serializer->Log) { nldebug("R2NET: (%u) Needed Properties sent %u bytes", serializer->Level +1, endLength - initLength);}

			}

			// serial optional properties
			if (!optionalPropFoundIndex.empty())
			{
				uint initLength = stream.getPos();
				serializer->Level +=2;
				uint32 first = 0, last = (uint32)optionalPropFoundIndex.size();
				uint8 optionalSize = static_cast<uint8>(last);
				stream.serial(optionalSize);
				for (; first != last; ++first)
				{
					std::string key = data->getKey(optionalPropFoundIndex[first]);
					uint32 uiKey = 0, lastLocal = (uint32)_OptionalProp.size();

					for ( ; uiKey != lastLocal && _OptionalProp[uiKey]!=key ; ++uiKey){}

					uint8 shortKey = static_cast<uint8>(uiKey);
					stream.serial(shortKey);

					CObject*value = data->getValue(optionalPropFoundIndex[first]);
					if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
					onSerial(stream, key, value, serializer);
				}

				serializer->Level -=2;
				uint endLength = stream.getPos();
				if (serializer->Log) { nldebug("R2NET: (%u) Optional Properties sent %u bytes", serializer->Level +1, endLength - initLength);}

			}

			// serial value properties
			if (!valuePropFoundIndex.empty())
			{
				uint initLength = stream.getPos();
				serializer->Level +=2;

				uint32 first = 0, last = (uint32)valuePropFoundIndex.size();
				uint16 last16 = static_cast<uint16>(last);
				stream.serial(last16);
				for (; first != last; ++first)
				{
					CObject* value = data->getValue(valuePropFoundIndex[first]);
					if (serializer->Log) { nldebug("R2NET: (%u) Field [%u]", serializer->Level, first);}
					CObjectSerializerImpl::getInstance().serialImpl(stream, value, serializer);
				}


				serializer->Level -=2;
				uint endLength = stream.getPos();
				if (serializer->Log) { nldebug("R2NET: (%u) Values Properties sent %u bytes", serializer->Level +1, endLength - initLength);}

			}

			// serial other properties
			if (!otherPropFoundIndex.empty())
			{
				uint initLength = stream.getPos();
				serializer->Level +=2;


				uint32 first = 0, last = (uint32)otherPropFoundIndex.size();
				uint16 last16 = static_cast<uint16>(last);
				stream.serial(last16);
				for (; first != last; ++first)
				{
					uint32 keyIndex = otherPropFoundIndex[first];
					std::string key = data->getKey(keyIndex);
					CObject* value = data->getValue(keyIndex);
					stream.serial(key);
					if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
					CObjectSerializerImpl::getInstance().serialImpl(stream, value, serializer);
				}


				serializer->Level -=2;
				uint endLength = stream.getPos();
				if (serializer->Log) { nldebug("R2NET: (%u) Other Properties sent %u bytes", serializer->Level +1, endLength - initLength);}

			}

		}
		else
		{
			data = newTable(serializer->Factory);
			data->add("Class", new CObjectString(_ClassName));

			uint8 next;
			stream.serial(next);

			// Needed pop
			if( (next & 1) )
			{

				std::vector<std::string>::const_iterator  first(_NeededProp.begin()), last(_NeededProp.end());
				for ( ; first != last ; ++first)
				{
					//*first => Property key ()
					CObject* value = 0;
					std::string key = *first;
					onSerial(stream, key, value, serializer);
					if (!value)
					{
						nlwarning( "Error the needed param '%s' of Class '%s' is a NULL value.",
							_ClassName.c_str(), first->c_str() );
					}
					else
					{
						data->add(key, value);
					}

				}
			}

			// Optional value
			if ( (next & 2) )
			{
				uint8 optionalSize;
				stream.serial(optionalSize);
				uint first(0), last(optionalSize);
				for ( ; first != last ; ++first)
				{
					CObject* value;
					uint8 keyIndex;
					stream.serial(keyIndex);
					std::string key = _OptionalProp[keyIndex];
					onSerial(stream, key, value, serializer);
					if (!value)
					{
						nlwarning("R2Share: Stream error");
					}
					data->add(key, value);
				}
			}

			// Table value
			if ( (next & 4) )
			{
				uint16 arrayValuesSize;
				stream.serial(arrayValuesSize);
				uint first(0), last(arrayValuesSize);
				for ( ; first != last ; ++first)
				{
					CObject* value;
					onSerial(stream, "", value, serializer);
					if (!value)
					{
						nlwarning("R2Share: Stream error");
					}
					data->add("", value);
				}
			}

			// Other Value
			if ( (next & 8) )
			{
				uint16 otherPropSize;
				stream.serial(otherPropSize);
				uint first(0), last(otherPropSize);
				for ( ; first != last ; ++first)
				{
					std::string key;
					stream.serial(key);
					CObject* value=0;
					onSerial(stream, key, value, serializer);
					if (!value)
					{
						nlwarning("R2Share: Stream error");
					}
					else
					{
						data->add(key, value);
					}
				}

			}

		}
	}

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer)
	{
		if (key == "InstanceId") { serialStringInstanceId(stream, data); return;	}
		CObjectSerializerImpl::getInstance().serialImpl(stream, data, serializer);

	}

	virtual ~CClassSerializer(){}

	uint8 getId() const { return _Type; }

	std::string getClassName() const { return _ClassName; }


protected:
	std::vector<std::string> _NeededProp;
	std::vector<std::string> _OptionalProp;
	std::string _ClassName;
	uint8 _Type;
};

//------------------
class CNpcSerializer : public CClassSerializer
{
public:
	CNpcSerializer();

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer);
};


CNpcSerializer::CNpcSerializer()
{
	static const char* neededProp[] = { "InstanceId", "Base", "Position", "Angle", "Behavior" };
	static const char* optionalProp[] = { "GabaritHeight", "GabaritTorsoWidth","GabaritArmsWidth", "GabaritLegsWidth", "GabaritBreastSize",
		"HairType", "HairColor", "Tattoo", "EyesColor", "MorphTarget1", "MorphTarget2", "MorphTarget3", "MorphTarget4", "MorphTarget5",
		"MorphTarget6", "MorphTarget7", "MorphTarget8", "Sex", "JacketModel", "TrouserModel", "FeetModel", "HandsModel", "ArmModel",
		"WeaponRightHand", "WeaponLeftHand", "JacketColor", "ArmColor", "HandsColor", "TrouserColor", "FeetColor", "Function","Level",
		"Profile", "Speed", "Aggro", "PlayerAttackable", "BotAttackable" };

	_ClassName = "Npc";
	_Type = ObjectTableNpc;
	const uint32 last =  sizeof(neededProp) / sizeof(neededProp[0]) ;
	std::vector<std::string> tmp(&neededProp[0], &neededProp[last]);
	_NeededProp.swap(tmp);
	std::vector<std::string> tmp2(&optionalProp[0], &optionalProp[ sizeof(optionalProp) / sizeof(optionalProp[0]) ]);
	_OptionalProp.swap(tmp2);
}

void CNpcSerializer::onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer)
{
	//H_AUTO(R2_CNpcSerializer_onSerial)
	if (key == "Angle") { serialNumberFixedPoint(stream, data, serializer); return; }
	this->CClassSerializer::onSerial(stream, key, data, serializer);
}
//------------------
class CNpcCustomSerializer : public CClassSerializer
{
public:
	CNpcCustomSerializer();

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer);
};


CNpcCustomSerializer::CNpcCustomSerializer()
{
	static const char* neededProp[] = { "InstanceId", "Base", "Position", "Angle", "Behavior",
		"GabaritHeight", "GabaritTorsoWidth","GabaritArmsWidth", "GabaritLegsWidth", "GabaritBreastSize",
		"HairType", "HairColor", "Tattoo", "EyesColor", "MorphTarget1", "MorphTarget2", "MorphTarget3", "MorphTarget4", "MorphTarget5",
		"MorphTarget6", "MorphTarget7", "MorphTarget8", "JacketModel", "TrouserModel", "FeetModel", "HandsModel", "ArmModel",
		"JacketColor", "ArmColor", "HandsColor", "TrouserColor", "FeetColor" };
	static const char* optionalProp[] = {
		"Name", "Profile", "Speed", "Aggro", "PlayerAttackable", "BotAttackable", "Function", "Level", "WeaponRightHand", "WeaponLeftHand", "Sex" };

	_ClassName = "NpcCustom";
	_Type = ObjectTableNpcCustom;
	const uint32 last =  sizeof(neededProp) / sizeof(neededProp[0]) ;
	std::vector<std::string> tmp(&neededProp[0], &neededProp[last]);
	_NeededProp.swap(tmp);
	std::vector<std::string> tmp2(&optionalProp[0], &optionalProp[ sizeof(optionalProp) / sizeof(optionalProp[0]) ]);
	_OptionalProp.swap(tmp2);
}

// TODO -> Lot of property to bitstream
void CNpcCustomSerializer::onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer)
{
	//H_AUTO(R2_CNpcCustomSerializer_onSerial)
	if (key == "Angle") { serialNumberFixedPoint(stream, data, serializer); return; }
	this->CClassSerializer::onSerial(stream, key, data, serializer);
}

//--------------------------------



#define R2_CLASS_SERIALIZER_NEEDED(Name, NeededProp ) \
	class C##Name##Serializer : public CClassSerializer \
	{\
	public:\
		C##Name##Serializer()\
		{ \
			_ClassName = #Name;\
			_Type = ObjectTable##Name;\
			const uint32 last =  sizeof(NeededProp) / sizeof(NeededProp[0]) ;\
			std::vector<std::string> tmp(&NeededProp[0], &NeededProp[last]);\
			_NeededProp.swap(tmp);\
		}\
	};


static const char* ActivitySequenceNeededProp[] = {"InstanceId","Name","Repeating","Components"};
R2_CLASS_SERIALIZER_NEEDED(ActivitySequence, ActivitySequenceNeededProp);


static const char* ActivityStepNeededProp[] = {"InstanceId","Type","TimeLimitValue","EventsIds","Chat", "ActivityZoneId", "Name", "TimeLimit", "Activity"};
R2_CLASS_SERIALIZER_NEEDED(ActivityStep, ActivityStepNeededProp);

static const char* ChatSequenceNeededProp[] = {"InstanceId","Name", "Components"};
R2_CLASS_SERIALIZER_NEEDED(ChatSequence, ChatSequenceNeededProp);

static const char* ChatStepNeededProp[] = {"InstanceId","Time", "Actions", "Name"};
R2_CLASS_SERIALIZER_NEEDED(ChatStep, ChatStepNeededProp);

static const char* ChatActionNeededProp[] = {"InstanceId","Emote", "Who", "Facing","Says"	};
R2_CLASS_SERIALIZER_NEEDED(ChatAction, ChatActionNeededProp);

static const char* LogicEntityActionNeededProp[] = {"InstanceId","Conditions", "Actions", "Event", "Name"};
R2_CLASS_SERIALIZER_NEEDED(LogicEntityAction, LogicEntityActionNeededProp);

static const char* ActionStepNeededProp[] = {"InstanceId","Entity"};
R2_CLASS_SERIALIZER_NEEDED(ActionStep, ActionStepNeededProp);

static const char* ActionTypeNeededProp[] = {"InstanceId","Type", "Value"};
R2_CLASS_SERIALIZER_NEEDED(ActionType, ActionTypeNeededProp);

static const char* EventTypeNeededProp[] = {"InstanceId","Type", "Value"};
R2_CLASS_SERIALIZER_NEEDED(EventType, EventTypeNeededProp);

static const char* LogicEntityReactionNeededProp[] = {"InstanceId","LogicEntityAction", "ActionStep", "Name"};
R2_CLASS_SERIALIZER_NEEDED(LogicEntityReaction, LogicEntityReactionNeededProp);

static const char* TextManagerEntryNeededProp[] = {"InstanceId","Count", "Text"};
R2_CLASS_SERIALIZER_NEEDED(TextManagerEntry, TextManagerEntryNeededProp);

static const char* ConditionStepNeededProp[] = {"InstanceId","Entity", "Condition"};
R2_CLASS_SERIALIZER_NEEDED(ConditionStep, ConditionStepNeededProp);

static const char* ConditionTypeNeededProp[] = {"InstanceId","Type", "Value"};
R2_CLASS_SERIALIZER_NEEDED(ConditionType, ConditionTypeNeededProp);


//--------------------------------

class CPositionSerializer : public CClassSerializer
{
public:
	CPositionSerializer();

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer);
};


CPositionSerializer::CPositionSerializer()
{
	static const char* neededProp[] = {"InstanceId", "x", "y", "z"};
	_ClassName = "Position";
	_Type = ObjectTablePosition;
	std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[4]));
	_NeededProp.swap(tmp);
}


void CPositionSerializer::onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer)
{
	//H_AUTO(R2_CPositionSerializer_onSerial)
	if (key == "x" || key == "y" || key == "z")  { serialNumberFixedPoint(stream, data, serializer); return; }
	this->CClassSerializer::onSerial(stream, key, data, serializer);
}

//--------------------------------

class CWayPointSerializer : public CClassSerializer
{
public:
	CWayPointSerializer()
	{
		static const char* neededProp[] = {"InstanceId", "Position"};
		_ClassName = "WayPoint";
		_Type = ObjectTableWayPoint;
		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[2]));
		_NeededProp.swap(tmp);
	}
};

class CRegionVertexSerializer : public CClassSerializer
{
public:
	CRegionVertexSerializer()
	{
		static const char* neededProp[] = {"InstanceId", "Position"};
		_ClassName = "RegionVertex";
		_Type = ObjectTableRegionVertex;
		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[2]));
		_NeededProp.swap(tmp);
	}
};

class CRegionSerializer : public CClassSerializer
{
public:
	CRegionSerializer()
	{
		static const char* neededProp[] = {"InstanceId", "Name", "Points"};
		_ClassName = "Region";
		_Type = ObjectTableRegion;
		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[2]));
		_NeededProp.swap(tmp);
	}
};


class CRoadSerializer : public CClassSerializer
{
public:
	CRoadSerializer()
	{
		static const char* neededProp[] = {"InstanceId", "Name", "Points"};
		_ClassName = "Road";
		_Type = ObjectTableRoad;
		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[2]));
		_NeededProp.swap(tmp);
	}
};



//------------------------------
class CNpcGrpFeatureSerializer : public CClassSerializer
{
public:
	CNpcGrpFeatureSerializer()
	{
		_ClassName = "NpcGrpFeature";
		_Type = ObjectTableNpcGrpFeature;
		static const char* neededProp[] = {"InstanceId", "Name", "Components", "ActivitiesId"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};
//--------------------------------------------
class CBehaviorSerializer : public CClassSerializer
{
public:
	CBehaviorSerializer()
	{
		_ClassName = "Behavior";
		_Type = ObjectTableBehavior;
		static const char* neededProp[] = {"InstanceId", "Type", "ZoneId"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};
//----------------------------------------------
class CRtSerializer : public CClassSerializer
{
public:

	void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer)
	{
		if (key == "Id")  { serialRtId(stream, data); return; }
		this->CClassSerializer::onSerial(stream, key, data, serializer);
	}


	void serialRtId( NLMISC::IStream& stream, CObject*& data)
	{
		if (!stream.isReading())
		{
			uint32 id = 0;

			std::string str = data->toString();
			std::string toFind = _ClassName;
			toFind += "_";
			std::string::size_type pos = str.find(toFind);
			if (pos != std::string::npos)
			{
				NLMISC::fromString(str.substr(toFind.size()), id);
			}
			stream.serial(id);
		}
		else
		{
			uint32 id;
			stream.serial(id);
			std::string strInstanceId = NLMISC::toString("%s_%d", _ClassName.c_str(), id);
			data = new CObjectString(strInstanceId);
		}
	}

};


//----------------------------------------------
class CRtActSerializer : public CRtSerializer
{
public:
	CRtActSerializer()
	{
		_ClassName = "RtAct";
		_Type = ObjectTableRtAct;
		static const char* neededProp[] = {"Id", "NpcGrps", "FaunaGrps", "AiStates", "Npcs", "Events", "Actions"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};

class CRtNpcGrpSerializer : public CRtSerializer
{
public:
	CRtNpcGrpSerializer()
	{
		_ClassName = "RtNpcGrp";
		_Type = ObjectTableRtNpcGrp;
		static const char* neededProp[] = {"Id", "Name", "Children", "AutoSpawn", "BotChat_parameters", "BotEquipment",
			"BotSheetClient", "BotVerticalPos", "Count", "GrpKeywords", "GrpParameters"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};

class CRtNpcSerializer : public CRtSerializer
{
public:
	CRtNpcSerializer()
	{
		_ClassName = "RtNpc";
		_Type = ObjectTableRtNpc;
		static const char* neededProp[] = {"Id", "Name", "Children", "ChatParameters", "Equipment", "IsStuck",
			"Keywords", "Keywords", "Sheet", "SheetClient", "BotVerticalPos", "Angle", "Pt"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};

class CRtPositionSerializer : public CRtSerializer
{
public:
	CRtPositionSerializer()
	{
		_ClassName = "RtPosition";
		_Type = ObjectTableRtPosition;
		static const char* neededProp[] = {"x", "y", "z"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};


class CRtAiStateSerializer : public CRtSerializer
{
public:
	CRtAiStateSerializer()
	{
		_ClassName = "RtAiState";
		_Type = ObjectTableRtAiState;
		static const char* neededProp[] = {"Id", "Name", "Children", "AiActivity", "AiMovement", "AiProfileParams", "Keywords","VerticalPos", "Pts" };

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};

class CRtNpcEventHandlerSerializer : public CRtSerializer
{
public:
	CRtNpcEventHandlerSerializer()
	{
		_ClassName = "RtNpcEventHandler";
		_Type = ObjectTableRtNpcEventHandler;
		static const char* neededProp[] = {"Id", "Name", "Event", "StatesByName", "GroupsByName", "ActionsId"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};

class CRtNpcEventHandlerActionSerializer : public CRtSerializer
{
public:
	CRtNpcEventHandlerActionSerializer()
	{
		_ClassName = "RtNpcEventHandlerAction";
		_Type = ObjectTableRtNpcEventHandlerAction;
		static const char* neededProp[] = {"Id", "Action", "Name", "Parameters", "Children", "Weight"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};


//-------------------------------------------


/*
class CXXXSerializer : public CClassSerializer
{
public:
	CNpcGrpFeatureSerializer()
	{
		_ClassName = "XXX";
		_Type = ObjectTableXXX;
		static const char* neededProp[] = {"InstanceId", "Type", "ZoneId"};

		static uint32 nbNeededProp = sizeof(neededProp) / sizeof(neededProp[0]);

		std::vector<std::string> tmp(&(neededProp[0]), &(neededProp[nbNeededProp]));
		_NeededProp.swap(tmp);
	}
};
*/

//------------------------------

//--------------------------------

	CObjectSerializerImpl::~CObjectSerializerImpl()
	{
		std::map<uint8, CClassSerializer*>::iterator first(_ClassSerializersById.begin()), last(_ClassSerializersById.end());
		for ( ; first != last ; ++first)
		{
			delete first->second;
		}
		_ClassSerializersById.clear();
	}

	CObjectSerializerImpl::CObjectSerializerImpl()
	{
		registerSerializer( new CNpcSerializer() );
		registerSerializer( new CNpcCustomSerializer() );
		registerSerializer( new CNpcGrpFeatureSerializer());
		registerSerializer( new CPositionSerializer());
		registerSerializer( new CWayPointSerializer());
		registerSerializer( new CRegionVertexSerializer());
		registerSerializer( new CRegionSerializer());
		registerSerializer( new CRoadSerializer());
//		registerSerializer( new CNpcGrpFeatureSerializer());
		registerSerializer( new CBehaviorSerializer());
		registerSerializer( new CRtActSerializer());
		registerSerializer( new CRtNpcGrpSerializer());
		registerSerializer( new CRtNpcSerializer());
		registerSerializer( new CRtAiStateSerializer());
		registerSerializer( new CRtNpcEventHandlerSerializer());
		registerSerializer( new CRtNpcEventHandlerActionSerializer());

		registerSerializer( new CActivitySequenceSerializer());
		registerSerializer( new CActivityStepSerializer());
		registerSerializer( new CChatSequenceSerializer());
		registerSerializer( new CChatStepSerializer());
		registerSerializer( new CChatActionSerializer());
		registerSerializer( new CLogicEntityActionSerializer());
		registerSerializer( new CActionStepSerializer());
		registerSerializer( new CActionTypeSerializer());
		registerSerializer( new CEventTypeSerializer());
		registerSerializer( new CLogicEntityReactionSerializer());
		registerSerializer( new CTextManagerEntrySerializer());
		registerSerializer( new CConditionTypeSerializer());
		registerSerializer( new CConditionStepSerializer());
	}

	void CObjectSerializerImpl::releaseInstance()
	{
		if( Instance )
		{
			delete Instance;
			Instance = NULL;
		}
	}

	void CObjectSerializerImpl::registerSerializer(CClassSerializer* serializer)
	{
		bool insert = _ClassSerializersById.insert(std::make_pair(serializer->getId(), serializer)).second;
		if (!insert)
		{
			nlinfo("R2Share: Prototype register two time");
		}
		insert = _ClassSerializersByName.insert(std::make_pair(serializer->getClassName(), serializer)).second;
		if (!insert)
		{
			nlinfo("R2Share: Prototype register two time");
		}
	}



void CObjectSerializerImpl::serialImpl(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, bool init)
{
	//H_AUTO(R2_CObjectSerializerImpl_serialImpl)
	uint8 type;


	if (!stream.isReading())
	{
		if (init)
		{
			type = ObjectHeaderTag;
			uint32 version = 1;
			stream.serial(type);
			stream.serial(version);
			serializer->setVersion(version);
		}

		if (data == 0)
		{
			type = ObjectNull;
			uint initLength = stream.getPos();
			stream.serial(type);
			uint endLength = stream.getPos();
			if (serializer->Log) { nldebug("R2NET: (%u) Null sent %u bytes", serializer->Level, endLength - initLength);}
		}
		else if (data->isNumber())
		{
			uint initLength = stream.getPos();
			writeNumber(stream, data->toNumber());
			uint endLength = stream.getPos();
			if (serializer->Log) { nldebug("R2NET: (%u) Number sent %u bytes", serializer->Level, endLength - initLength); }
		}
		else if(data->isString() || data->isRefId())
		{
			uint initLength = stream.getPos();
			std::string value = data->toString();
			uint32 size = (uint32)value.size();

			if (size == 0)
			{
				/*if (data->isRefId())
				{
					nlwarning("Serializing an object of type 'empty RefId'");
				}*/
				type = data->isRefId() ? ObjectRefIdEmpty : ObjectStringEmpty; // NB : 'isString()' would be true in both test because CObjectRefId derives from CObjectString
				stream.serial(type);


			}

			else if (size  <  static_cast<uint32>(uint8Max) )
			{
				/*if (data->isRefId())
				{
					nlwarning("Serializing an object of type 'RefId8'");
				}*/
				type = data->isRefId() ? ObjectRefId8 : ObjectString8; // NB : 'isString()' would be true in both test because CObjectRefId derives from CObjectString
				uint8 size8 = static_cast<uint8>(size);
				stream.serial(type);
				stream.serial(size8);
				stream.serialBuffer((uint8*)(&(value[0])), size8);


			}
			else if (size  < static_cast<uint32>(uint16Max) )
			{
				/*if (data->isRefId())
				{
					nlwarning("Serializing an object of type 'RefId16'");
				}*/
				type = data->isRefId() ? ObjectRefId16 : ObjectString16; // NB : 'isString()' would be true in both test because CObjectRefId derives from CObjectString
				uint16 size16 = static_cast<uint16>(size);
				stream.serial(type);
				stream.serial(size16);
				stream.serialBuffer((uint8*)(&(value[0])), size16);

			}
			else
			{

				//default very big string
				type = data->isRefId() ? ObjectRefId : ObjectString; // NB : 'isString()' would be true in both test because CObjectRefId derives from CObjectString
				stream.serial(type);
				stream.serial(value);
			}

			uint endLength = stream.getPos();
			if (serializer->Log) { nldebug("R2NET: (%u) String Send %u bytes", serializer->Level, endLength - initLength); }
			return;
		}
		else if (data->isTable())
		{
			uint initLength = stream.getPos();
			std::string className;
			if ( data->isString("Class"))
			{
				className = data->toString("Class");
			}



			if (!className.empty())
			{
				std::map<std::string, CClassSerializer*>::const_iterator found(_ClassSerializersByName.find(className));
				if (found != _ClassSerializersByName.end() )
				{
					found->second->serialClass(stream, data, serializer);
					uint endLength = stream.getPos();
					if (serializer->Log) { nldebug("R2NET: (%u) Class '%s' sent %u bytes", serializer->Level, className.c_str(), endLength - initLength); }
					return;
				}
			}



			type = ObjectTable;
			stream.serial(type);
			uint32 size = data->getSize();
			stream.serial(size);
			for (uint first = 0; first != size; ++first)
			{
				std::string key = data->getKey(first);
				CObject* value = data->getValue(first);
				stream.serial(key);
				++ (serializer->Level);
				if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
				serialImpl(stream, value, serializer);
				-- (serializer->Level);
			}
			uint endLength = stream.getPos();
			if (serializer->Log)
			{
				if (className.empty())
				{
					nldebug("R2NET: (%u) Table sent %u bytes",serializer->Level, endLength - initLength);
				}
				else
				{
					nldebug("R2NET: (%u) Generic Class(%s) sent %u bytes", serializer->Level, className.c_str(), endLength - initLength);
				}
			}



		//	nlstop;
		}
	}
	else
	{
		uint8 type = ObjectNull;
		stream.serial(type);

		// NB nico : use factory here instead of plain 'new' because client may use derived classes internally
		// NB 2 : if server is created locally, not a problem because derived object fonctinnality


		switch(type)
		{
			case ObjectHeaderTag:
			{
				uint32 version = 0;
				stream.serial(version);
				serializer->setVersion(version);
				this->serialImpl(stream,  data, serializer);
				return;
			}
			case ObjectNull:
			{
				data = 0;
				return;
			}
			case ObjectNumber:
			{
				double value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}
			case ObjectNumberZero:
			{
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", 0.0);
				return;
			}
			case ObjectNumberSInt32:
			{
				sint32 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberUInt32:
			{
				uint32 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberSInt16:
			{
				sint16 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberUInt16:
			{
				uint16 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberSInt8:
			{
				sint8 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberUInt8:
			{
				uint8 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}
			// Do not remove this or it would be impossible to load old session
			case ObjectNumberFloat:
			{
				float value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectNumberDouble:
			{
				double value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber(0);
				((CObjectNumber *) data)->set("", value);
				return;
			}

			case ObjectStringEmpty:
			{
				data = serializer->Factory ? serializer->Factory->newBasic("String") : new CObjectString("");
				return;
			}

			case ObjectString8:
			{
				std::string value;
				uint8 size;

				stream.serial(size);
				value.resize(size);
				stream.serialBuffer((uint8*)(&(value[0])), size);
				data = serializer->Factory ? serializer->Factory->newBasic("String") : new CObjectString("");
				((CObjectString *) data)->set("", value);
				return;
			}

			case ObjectString16:
			{
				std::string value;
				uint16 size;

				stream.serial(size);
				value.resize(size);
				stream.serialBuffer((uint8*)(&(value[0])), size);
				data = serializer->Factory ? serializer->Factory->newBasic("String") : new CObjectString("");
				((CObjectString *) data)->set("", value);
				break;
			}

			case ObjectString:
			{
				std::string value;

				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("String") : new CObjectString("");
				((CObjectString *) data)->set("", value);
				break;
			}

			case ObjectRefId:
			{
				std::string value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("RefId") : new CObjectRefId("");
				((CObjectRefId *) data)->set("", value);
				break;
			}

			case ObjectRefIdEmpty:
			{
				data = serializer->Factory ? serializer->Factory->newBasic("RefId") : new CObjectRefId("");
				return;
			}

			case ObjectRefId8:
			{
				std::string value;
				uint8 size;

				stream.serial(size);
				value.resize(size);
				stream.serialBuffer((uint8*)(&(value[0])), size);
				data = serializer->Factory ? serializer->Factory->newBasic("RefId") : new CObjectRefId("");
				((CObjectRefId *) data)->set("", value);
				return;
			}

			case ObjectRefId16:
			{
				std::string value;
				uint16 size;

				stream.serial(size);
				value.resize(size);
				stream.serialBuffer((uint8*)(&(value[0])), size);
				data = serializer->Factory ? serializer->Factory->newBasic("RefId") : new CObjectRefId("");
				((CObjectRefId *) data)->set("", value);
			}
			break;
			case ObjectTable:
			{

				uint32 size;
				stream.serial(size);
				data = newTable(serializer->Factory);
				uint32 first;
				for (first = 0 ; first != size; ++first)
				{
					std::string key;
					stream.serial(key);
					CObject* value=0;
					serialImpl(stream,value, serializer);
					data->add(key, value);
				}
			}
			break;
			default:
				std::map<uint8, CClassSerializer*>::const_iterator found(_ClassSerializersById.find(type));
				if (found != _ClassSerializersById.end())
				{
					found->second->serialClass(stream, data, serializer);
					return;
				}

				BOMB("ClassSerializer not found: can not read data", return);
		}
	}
}

void CObjectSerializer::releaseInstance()
{
	//H_AUTO(R2_CObjectSerializer_releaseInstance)
	CObjectSerializerImpl::releaseInstance();
}

void CObjectSerializer::serial(NLMISC::IStream& stream)
{
	//H_AUTO(R2_CObjectSerializer_serial)
	stream.serial(_Compressed);


	if ( stream.isReading() )
	{
		_MustUncompress = _Compressed;
	}

	if (!_Compressed)
	{
		CObjectSerializerImpl::getInstance().serialImpl(stream, _Data, this, true);
	}
	else
	{
		stream.serial(_CompressedLen);
		stream.serial(_UncompressedLen);

		if ( stream.isReading() )
		{
			_CompressedBuffer = new uint8[_CompressedLen];
		}
		stream.serialBuffer(_CompressedBuffer, _CompressedLen);
	}
}

CObject* CObjectSerializer::getData() const
{
	//H_AUTO(R2_CObjectSerializer_getData)
	if (_Compressed && _MustUncompress) { uncompress(); };
	if (_Data) return _Data->clone();
	return 0;
}


CObjectSerializer::CObjectSerializer(CObjectFactory *factory, CObject* data)
	:	Factory(factory),
		Level(0),
		Log(false)
{

	Log = false;
	_CompressedBuffer = 0;
	_CompressedLen = 0;
	_UncompressedLen = 0;
	_Compressed = false;
	_MustUncompress = false;
	_Version = 0;
	if (data)
	{
		_Data = data->clone();
	}
	else
	{
		_Data = 0;
	}

}

CObjectSerializer::~CObjectSerializer()
{
	if (_CompressedBuffer) { delete [] _CompressedBuffer; _CompressedBuffer = 0;}
	delete _Data;
}


CObjectSerializer::CObjectSerializer(const CObjectSerializer& /* lh */)
{
	//H_AUTO(R2_CObjectSerializer_CObjectSerializer)
	nlassert(0);
}

CObjectSerializer& CObjectSerializer::operator=(const CObjectSerializer& /* rh */)
{
	nlassert(0);
	return *this;
}

void CObjectSerializer::setData(CObject* data)
{
	//H_AUTO(R2_CObjectSerializer_setData)
	nlassert(!_Compressed);
	if (data)
	{
		_Data = data->clone();
	}
	else
	{
		_Data = 0;
	}
}






void CObjectSerializer::compress()
{
	//H_AUTO(R2_CObjectSerializer_compress)

	NLMISC::CMemStream buffer;
	if (buffer.isReading()) buffer.invert();
	uint32 init =  buffer.length();
	CObjectSerializerImpl::getInstance().serialImpl(buffer, _Data, this, true);
	uint32 length =  buffer.length() - init;

	uLongf destLen = length + length / 1000 + 12;





	Bytef *dest = new Bytef[destLen];
	int ok = ::compress(dest, &destLen, (Bytef *)buffer.buffer(), buffer.length());

	if (ok != Z_OK)
	{
		delete [] dest;
		nlwarning("Error while compressing data stream");
		return;
	}
	// Compress data only if shortest
	if ( length < destLen)
	{
		delete [] dest;
		return;
	}

	_Compressed = true;
	_CompressedBuffer = (uint8*) dest;
	_CompressedLen = destLen;
	_UncompressedLen = length;

	nlinfo("Compress Data from %u to %u",_UncompressedLen, _CompressedLen );
}


void CObjectSerializer::uncompress() const
{
	//H_AUTO(R2_CObjectSerializer_uncompress)
	const_cast<CObjectSerializer*>(this)->uncompressImpl();
}

void CObjectSerializer::uncompressImpl()
{
	//H_AUTO(R2_CObjectSerializer_uncompressImpl)
	if (_Compressed && _MustUncompress)
	{
		_MustUncompress = false;
		if  ( _Data )
		{
			delete _Data;
			_Data = 0;
		}

		Bytef* data = new Bytef[_UncompressedLen];
		uLongf dataLen = _UncompressedLen;
		sint32 state = ::uncompress (data, &dataLen ,
			reinterpret_cast<Bytef*>(_CompressedBuffer), _CompressedLen);

		if (state != Z_OK)
		{
			delete[] data;
			nlwarning("Error while uncompressing data stream.");
			return;
		}
		if (_UncompressedLen != dataLen)
		{
			nlwarning("Error error in data stream.");
		}

		_UncompressedLen = dataLen;
		NLMISC::CMemStream buffer;
		if (buffer.isReading()) buffer.invert();
		buffer.serialBuffer((uint8*)data, _UncompressedLen);
		buffer.invert();
		buffer.seek(0, NLMISC::IStream::begin);

		CObjectSerializerImpl::getInstance().serialImpl(buffer, _Data, this, true);

		delete[] data;
	}
}
}
