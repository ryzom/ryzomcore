// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "scenario_text_parser.h"
#include "utils.h"

#include <zlib.h>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

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
CObjectFactory *CObjectSerializerClient::_ClientObjecFactory = nullptr;

// a scenario that is loaded from its text form can be sent, and one that was received can be saved and loaded
nlctassert(CObjectSerializer::MaxDepth == CScenarioTextParser::MaxDepth);
nlctassert(CObjectSerializer::MaxNodes == CScenarioTextParser::MaxNodes);


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
	_Parent = nullptr;
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

bool CObject::isInteger(const std::string & prop) const
{
	//H_AUTO(R2_CObject_isInteger)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr) return false;
		return attr->doIsInteger();
	}
	return doIsInteger();
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
		return attr->isRefId();
	}
	return doIsRefId();
}


bool CObject::doIsNumber() const
{
	//H_AUTO(R2_CObject_doIsNumber)
	return false;
}

bool CObject::doIsInteger() const
{
	//H_AUTO(R2_CObject_doIsInteger)
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

sint64 CObject::toInteger(const std::string & prop) const
{
	//H_AUTO(R2_CObject_toInteger)
	if (!prop.empty())
	{
		CObject* attr = getAttr(prop);
		if (!attr)
		{
			BOMB("Try to use the method toInteger() on a NULL Object", return 0);
		}
		return attr->doToInteger();
	}
	return doToInteger();
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
			BOMB("Try to use the method toTable() on a NULL Object", return nullptr);
		}
		return attr->doToTable();
	}
	return doToTable();
}



CObject::TSmartPtr CObject::take(sint32 /* position */)
{
	//H_AUTO(R2_CObject_take)
	BOMB("Try to use the take function on an object that is not a table", return NULL);
	return nullptr;
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

sint64 CObject::doToInteger() const
{
	//H_AUTO(R2_CObject_doToInteger)
	BOMB("Try to convert an objet to integer without being allowed", return 0);
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
	BOMB("Try to convert an objet to string without being allowed", return nullptr);
	return nullptr;
}

static const CObject::TSmartPtr s_nullSmartPtr;

const CObject::TSmartPtr& CObject::getAttr(const std::string & /* name */) const {  return s_nullSmartPtr;}

std::string CObject::getKey(uint32 /* pos */) const{ BOMB("Try to call the function getKey() on an object that is not a table", return "");  return "";}

const CObject::TSmartPtr& CObject::getValueAtPos(uint32 /* pos */) const{ BOMB("Try to call the function getValueAtPos() on an object that is not a table", return s_nullSmartPtr);   return s_nullSmartPtr;}

uint32 CObject::getSize() const { BOMB("Try to call the function getSize() on an object that is not a table", return 0);  return 0; }

CObject::TSmartPtr CObject::clone() const { BOMB("Try to call the function clone() on an object that is not a table", return NULL); return nullptr;
}

void CObject::add(const std::string & key,  const CObject::TSmartPtr &value)
{
	//H_AUTO(R2_CObject_add)
	insert(key,  value,  -1);
}

void CObject::add(const CObject::TSmartPtr &value){ add("",  value); }

bool CObject::set(const std::string& /* key */, const std::string & /* value */)
{
	BOMB("Try to set the value of an object with a string on an object that does not allowed it", return false);
	return false;
}

bool CObject::set(const std::string& /* key */, double /* value */)
{
	BOMB("Try to set the value of an object with a double on an object that does not allowed it", return false);
	return false;
}

bool CObject::set(const std::string& /* key */, sint64 /* value */)
{
	BOMB("Try to set the value of an object with an integer on an object that does not allowed it", return false);
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
bool CObject::insert( const std::string& /* key */,   const CObject::TSmartPtr& /* value */,  sint32 /* position */)
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
	this->add(key,  CObject::TSmartPtr(new CObjectString(value)));
}
void CObject::add(const std::string& key,  double value)
{
	//H_AUTO(R2_CObject_add)
	this->add(key,  CObject::TSmartPtr(new CObjectNumber(value)));
}
void CObject::add(const std::string& key,  sint64 value)
{
	//H_AUTO(R2_CObject_add)
	this->add(key,  CObject::TSmartPtr(new CObjectNumber(value)));
}

const CObject::TSmartPtr& CObject::findAttr(const std::string & first) const
{
	//H_AUTO(R2_CObject_findAttr)
	return getAttr(first);
}

const CObject::TSmartPtr& CObject::findAttr(const std::string & first,  const std::string & second) const
{
	//H_AUTO(R2_CObject_findAttr)
	const TSmartPtr& obj = getAttr(first);
	if (!obj) return s_nullSmartPtr;
	return obj->getAttr(second);
}

const CObject::TSmartPtr& CObject::findAttr(const std::string & first,  const std::string & second,  const std::string & third) const
{
	//H_AUTO(R2_CObject_findAttr)
	const TSmartPtr& obj = getAttr(first);
	if (!obj) return s_nullSmartPtr;
	const TSmartPtr& obj2 = obj->getAttr(second);
	if (!obj2) return s_nullSmartPtr;
	return obj2->getAttr(third);
}

const CObject::TSmartPtr& CObject::findAttr(const std::vector<std::string>& attrNames ) const
{
	//H_AUTO(R2_CObject_findAttr)
	const CObject* current = this;
	const TSmartPtr* result = &s_nullSmartPtr;
	for (std::vector<std::string>::const_iterator first = attrNames.begin(), last = attrNames.end(); first != last; ++first)
	{
		result = &current->getAttr(*first);
		if (!*result) return s_nullSmartPtr;
		current = result->getPtr();
	}
	return *result;
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
		attrName.clear();
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

	BOMB_IF(!key.empty(), "Try to set the a sub value of an object that does not allowed it", return false);
	_Value = value;
	return true;
}

bool CObjectString::setObject(const std::string& key,  CObject* value)
{
	//H_AUTO(R2_CObjectString_setObject)
	BOMB_IF( !key.empty() || ! (value->isString() || value->isNumber() || value->isInteger()) , "Try to set the a sub value of an object that does not allowed it", return false);
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


CObject::TSmartPtr CObjectString::clone() const
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

CObject::TSmartPtr CObjectRefId::clone() const
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


CObjectNumber::CObjectNumber(double value) : CObject(), m_IsInteger(false) 
{
	m_Value.Number = value;
}

CObjectNumber::CObjectNumber(sint64 value) : CObject(), m_IsInteger(true) 
{
	m_Value.Integer = value;
}

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
	m_IsInteger = src.m_IsInteger;
	m_Value.Integer = src.m_Value.Integer;
	setGhost(src.getGhost());
}


std::string CObjectNumber::doToString() const { return m_IsInteger ? NLMISC::toString(m_Value.Integer) : NLMISC::toString("%lf", m_Value.Number);}

void CObjectNumber::doSerialize(std::string& out,  CSerializeContext& /* context */) const
{
	//H_AUTO(R2_CObjectNumber_doSerialize)
	nlassert(!getGhost());
	if (doIsInteger())
	{
		out += NLMISC::toString(getIntegerValue());
	}
	else
	{
		//out.precision(15);
		double num = getNumberValue();
		// TODO: WTF is this for?!
		std::string value = NLMISC::toString(double(sint64(num * 1000.0 + (num>=0.0?0.5:-0.5)))/1000.0);
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
}

bool CObjectNumber::set(const std::string& key, sint64 value)
{
	//H_AUTO(R2_CObjectNumber_set)

	BOMB_IF(!key.empty(), "Try to set an element of a table on an object that is not a table", return false);

	m_IsInteger = true;
	m_Value.Integer = value;
	return true;
}

bool CObjectNumber::set(const std::string& key,  double value)
{
	//H_AUTO(R2_CObjectNumber_set)

	BOMB_IF(!key.empty(), "Try to set an element of a table on an object that is not a table", return false);

	m_IsInteger = false;
	m_Value.Number = value;
	return true;
}

bool CObjectNumber::set(const std::string& key,  const std::string & value)
{
	//H_AUTO(R2_CObjectNumber_set)
	//XXX
	BOMB_IF(!key.empty(), "Try to set an element of a table on an object that is not a table", return false);
	double num;
	sint64 i;
	if (NLMISC::fromString(value, i))
	{
		if (NLMISC::toString(i) == value)
		{
			m_IsInteger = true;
			m_Value.Integer = i;
			return true;
		}
	}
	if (NLMISC::fromString(value, num))
	{
		m_IsInteger = false;
		m_Value.Number = num;
		return true;
	}
	return false;
}

CObject::TSmartPtr CObjectNumber::clone() const
{
	//H_AUTO(R2_CObjectNumber_clone)
	CObjectNumber *result = m_IsInteger 
		? new CObjectNumber(m_Value.Integer) 
		: new CObjectNumber(m_Value.Number);
	result->setGhost(getGhost());
	return result;
}

bool CObjectNumber::doIsNumber() const  { return !m_IsInteger || (sint64)getNumberValue() == m_Value.Integer;}

double CObjectNumber::doToNumber() const {  return getNumberValue(); }

bool CObjectNumber::doIsInteger() const  { return m_IsInteger || (double)getIntegerValue() == m_Value.Number;}

sint64 CObjectNumber::doToInteger() const {  return getIntegerValue(); }

bool CObjectNumber::setObject(const std::string& /* key */,  CObject* value)
{
	//H_AUTO(R2_CObjectNumber_setObject)
	BOMB_IF(!value->isNumber() && !value->isInteger(), NLMISC::toString("Try to set an element of a type '%s' with  a value of type '%s' on an object that is not a number", this->getTypeAsString(), value->getTypeAsString()), return false);

	if (value->isInteger())
	{
		m_IsInteger = true;
		m_Value.Integer = value->toInteger();
	}
	else
	{
		m_IsInteger = false;
		m_Value.Number = value->toNumber();
	}
	setGhost(value->getGhost());
	return true;
}

bool CObjectNumber::equal(const CObject* other) const
{
	if (!other) return false;
	if (isInteger() && other->isInteger())
	{
		return (getIntegerValue() == other->toInteger());
	}

	if (!other->isNumber()) return false;
	double value = getNumberValue();
	double otherValue = other->toNumber();
	if (value == otherValue) return true;

	// if difference between 2 values less than epsilon, we consider they are equals
	return fabs(value - otherValue) <= std::numeric_limits<double>::epsilon();
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
			CObject *subObj = getValueAtPos(k);
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
		CObject *subObj = getValueAtPos(k);
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
		CObject *subObj = getValueAtPos(k);
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
				src.getValueAtPos(k)->inPlaceCopyTo(*dest);
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
	TContainer data;
	std::vector<std::string> keyVector;

	CObject* keys = getAttr("Keys");
	if (!keys || !keys->isTable()) return;

	uint32 firstKey = 0;
	uint32 lastKey = keys->getSize();
	for (; firstKey != lastKey ; ++firstKey)
	{
		CObject* keyObject = keys->getValueAtPos(firstKey);
		if (! keyObject->isString()) return;
		std::string key = keyObject->toString();

		uint32 firstValue = 0;
		uint32 lastValue = (uint32)_Value.size();
		for (; firstValue != lastValue; ++firstValue)
		{
			if ( key == _Value[firstValue].first)
			{
				data.push_back( _Value[firstValue]);
				_Value[firstValue].second = nullptr;
			}
		}
	}
	{
		uint32 firstValue = 0;
		uint32 lastValue = (uint32)_Value.size();
		for (; firstValue != lastValue; ++firstValue)
		{
			if (  _Value[firstValue].first != "Keys" && _Value[firstValue].second != nullptr)
			{
				data.push_back( _Value[firstValue]);
			}
		}

	}
	_Value.swap(data);
}

CObject::TSmartPtr CObjectTable::clone() const
{
	//H_AUTO(R2_CObjectTable_clone)
	CHECK_TABLE_INTEGRITY
	CObject::TSmartPtr ret = new CObjectTable();
	TContainer::const_iterator first(_Value.begin()),  last(_Value.end());
	for ( ;first != last; ++first )
	{
		BOMB_IF(!first->second, "Try to clone a table with an NULL component", return NULL);
		nlassert(first->second->getGhost() == this->getGhost());
		CObject::TSmartPtr clone = first->second->clone();
		if (clone) { clone->setParent(nullptr); }
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

const CObject::TSmartPtr& CObjectTable::getAttr(const std::string & name) const
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

	return s_nullSmartPtr;
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

const CObject::TSmartPtr& CObjectTable::getValueAtPos(uint32 pos) const
{
	//H_AUTO(R2_CObjectTable_getValue)
	if (pos >= _Value.size())
	{
		nlwarning("<CObjectTable::getValue> bad index %d", pos);
		return s_nullSmartPtr;
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
			CObject* value1 = table->getValueAtPos(first);
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

CObject::TSmartPtr CObjectTable::take(sint32 position)
{
	//H_AUTO(R2_CObjectTable_take)
	CHECK_TABLE_INTEGRITY
	BOMB_IF(!( -1 <= position && position < static_cast<sint32>(_Value.size())), "Try to take an element that does not exist", return NULL);

	if (0 <=  position && position < static_cast<sint32>(_Value.size()))
	{
		CObject::TSmartPtr child = _Value[ static_cast<uint32>(position) ].second;
		_Value.erase(_Value.begin() + static_cast<uint32>(position));
		child->setParent(nullptr);
		return child;
	}
	else if (position == -1)
	{
		CObject* parent = getParent();
		if (!parent)
		{
			return CObject::TSmartPtr(this);
		}

		uint32 pos = getParent()->findIndex(this);
		BOMB_IF(pos >= getParent()->getSize(), "Try to take an element that does not exist", return NULL);
		CObject::TSmartPtr child = getParent()->take(pos);
		return child;
	}

	return nullptr;
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
		if (!parent) //try to take the root of a tree
		{
			return true;
		}
		uint32 pos = parent->findIndex(this);
		BOMB_IF(pos >= getParent()->getSize(), "Try to take an element that does not exist", return false);
		return getParent()->canTake(pos);
	}

	return true;
}



bool CObjectTable::insert(const std::string& key,  const CObject::TSmartPtr &value,  sint32 position)
{
	//H_AUTO(R2_CObjectTable_insert)
	CHECK_TABLE_INTEGRITY
	uint32 count = (uint32)_Value.size();

	BOMB_IF(!( -1 <= position && position <= static_cast<sint32>(count)), "Try to take an element that does not exist", return false);
	BOMB_IF(!value, "Try to insert a Null value", return false);
	BOMB_IF(value->getParent() != nullptr, "Try to insert an element that not at the root of the tree.", return false);

	value->setParent(this);
	// inherit the 'ghost' flag
	if (this->getGhost())
	{
		value->setGhost(true);
	}


	if (0<= position && position < static_cast<sint32>(count))
	{

		_Value.insert(_Value.begin() + position,  std::pair<std::string,  CObject::TSmartPtr>(key,  value));

	}
	else
	{
		_Value.push_back(std::pair<std::string,  CObject::TSmartPtr>(key,  value));
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
		CObject* value = other->getValueAtPos(i);
		if (key != this->getKey(i)) return false;
		if ( !value->equal(this->getValueAtPos(i))) return false;
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
		return new CObjectNumber((sint64)0);
	}
	else if (type == "Table")
	{
		return new CObjectTable();
	}
	return nullptr;
}

CObject* CObjectFactory::newAvanced(const std::string & type)
{
	//H_AUTO(R2_CObjectFactory_newAvanced)

	CObjectGenerator* ret = getGenerator(type);
	if (ret) { return ret->instanciate(this); }
	return nullptr;
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
	CObject* ret = nullptr;
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
	return nullptr;
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
		CObject* found = prop->getValueAtPos(first);

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

const CObject::TSmartPtr& CObjectGenerator::getDefaultValue(const std::string & propName) const
{
	//H_AUTO(R2_CObjectGenerator_getDefaultValue)
	TDefaultValues::const_iterator found ( _DefaultValues.find(propName) );
	if (found != _DefaultValues.end())
	{
		return found->second;
	}
	return s_nullSmartPtr;
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
		CObject* found = prop->getValueAtPos(first);

		if (found && found->isString("Type") )
		{
			CObject*defaultInBase = found->getAttr("DefaultInBase");

			if (!defaultInBase || (defaultInBase->isInteger() && defaultInBase->toInteger() != 1))
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
	nlwarning("%s", result.c_str());
}

void CObjectNumber::dump(const std::string prefix, uint depth) const
{
	//H_AUTO(R2_CObjectNumber_dump)
	std::string result(depth * 4, ' ');
	if (m_IsInteger)
		result += NLMISC::toString("%sNumber, ptr = 0x%p, isNumber, value = %lf, ghost = %s", prefix.c_str(), this, m_Value.Integer, _Ghost ? "true" : "false");
	else
		result += NLMISC::toString("%sNumber, ptr = 0x%p, isInteger, value = %lf, ghost = %s", prefix.c_str(), this, m_Value.Number, _Ghost ? "true" : "false");
	nlwarning("%s", result.c_str());
}

void CObjectTable::dump(const std::string prefix, uint depth) const
{
	//H_AUTO(R2_CObjectTable_dump)
	std::string result(depth * 4, ' ');
	result += NLMISC::toString("%sTable, ptr = 0x%p, , ghost = %s", prefix.c_str(), this, _Ghost ? "true" : "false");
	nlwarning("%s", result.c_str());
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

static void writeInteger( NLMISC::IStream& stream, sint64 theValue)
{
	sint64 value = theValue;
	uint8 type;

	// It's 0
	if (value == 0)
	{
		type = ObjectNumberZero;
		stream.serial(type);
		return;
	}

	bool pos = value >= 0;
	// positif
	if (pos)
	{
		if (value <=  uint8Max)
		{
			uint8 uint8value = static_cast<uint8>(value);
			type = ObjectNumberUInt8;
			stream.serial(type);
			stream.serial( uint8value);
			return;
		}

		if (value <= uint16Max)
		{
			uint16 uint16value = static_cast<uint16>(value);
			type = ObjectNumberUInt16;
			stream.serial(type);
			stream.serial(uint16value);
			return;
		}

		if (value <= uint32Max)
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
		if ( sint8Min <= value && value <=  sint8Max)
		{
			sint8 sint8value = static_cast<sint8>(value);
			type = ObjectNumberSInt8;
			stream.serial(type);
			stream.serial( sint8value);
			return;
		}

		if ( sint16Min <= value && value <=  sint16Max)
		{
			sint16 sint16value = static_cast<sint16>(value);
			type = ObjectNumberSInt16;
			stream.serial(type);
			stream.serial( sint16value);
			return;
		}
	}

	// Default case
	if (sint32Min <= value && value <= sint32Max)
	{
		type = ObjectNumberSInt32;
		sint32 fValue = (sint32)value;
		stream.serial(type);
		stream.serial(fValue);
		return;
	}

	// No form holds an integer of more than 32 bits. It is written as a double, which is exact up to 2^53,
	// rather than cut to its low 32 bits.
	type = ObjectNumberDouble;
	double dValue = (double)value;
	stream.serial(type);
	stream.serial(dValue);
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
		// the form of old scenario sessions, which are only read: the read says so once (readTree)
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

// The class forms below write some properties in a form of their own (an instance id as a number, a coordinate
// as a fixed point number, the id of a run time object as its number). A tree comes from the other side of a
// connection and is written again by the side that received it, so a property can hold anything: these tell
// whether a value has the form its property is written in. A table with a value that has not is written as a
// table without a class (CClassSerializer::fitsClass).

// true for a string that is not a reference id, which is read back as a plain string
static bool isPlainString(const CObject *value)
{
	return value && value->isString() && !value->isRefId();
}

// true for the decimal form of a number up to maxValue, as "%d" prints it: digits only, no leading zero
static bool isCanonicalNumber(const std::string &str, uint32 maxValue)
{
	if (str.empty() || str.size() > 10) return false;
	if (str.size() > 1 && str[0] == '0') return false;
	uint64 number = 0;
	for (std::string::size_type i = 0; i != str.size(); ++i)
	{
		if (str[i] < '0' || str[i] > '9') return false;
		number = number * 10 + (uint64)(str[i] - '0');
	}
	return number <= maxValue;
}

// true for an instance id that serialStringInstanceId reads back as it was: the empty string, or
// "Client<0..255>_<0..16777215>" other than "Client0_0", whose number stands for the empty string
static bool fitsInstanceId(const CObject *value)
{
	if (!isPlainString(value)) return false;
	std::string str = value->toString();
	if (str.empty()) return true;
	if (str.compare(0, 6, "Client") != 0) return false;
	std::string::size_type separator = str.find('_', 6);
	if (separator == std::string::npos) return false;
	if (!isCanonicalNumber(str.substr(6, separator - 6), 0xff)) return false;
	if (!isCanonicalNumber(str.substr(separator + 1), 0x00ffffff)) return false;
	return CObject::uint32ToInstanceId(CObject::instanceIdToUint32(str)) == str;
}

// true for a number that serialNumberFixedPoint can write: its thousandths fit in 32 bits (it is read back
// rounded to the thousandth, which is what the form is for)
static bool fitsFixedPoint(const CObject *value)
{
	if (!value || !(value->isNumber() || value->isInteger())) return false;
	double number = value->toNumber();
	// what serialNumberFixedPoint converts to 32 bits, which cuts the fraction: false for a NaN too
	double thousandths = number * 1000.0 + (number >= 0.0 ? 0.5 : -0.5);
	return -2147483649.0 < thousandths && thousandths < 2147483648.0;
}

// A table that is being read, which belongs to no table yet: deleted with all it holds when the read of one of
// its values throws, unless it was released
class CReadHold
{
public:
	explicit CReadHold(CObject *object) : m_Object(object) { }
	~CReadHold() { delete m_Object; }
	CObject *get() const { return m_Object; }
	CObject *release()
	{
		CObject *object = m_Object;
		m_Object = nullptr;
		return object;
	}

private:
	CReadHold(const CReadHold &);
	CReadHold &operator=(const CReadHold &);

	CObject *m_Object;
};

// One more table open in the tree that is being read, for as long as this object lives. The read and
// everything that walks the tree afterwards (clone, destructor, instance map, the writer) recurse once per
// table, so a tree nested deeper than CObjectSerializer::MaxDepth is refused.
class CReadDepth
{
public:
	explicit CReadDepth(CObjectSerializer *serializer) : m_Serializer(serializer)
	{
		if (m_Serializer->m_ReadDepth >= CObjectSerializer::MaxDepth)
		{
			nlwarning("R2Share: the tree read has tables nested deeper than %u, refused", (uint)CObjectSerializer::MaxDepth);
			throw NLMISC::EStream("R2 tree: tables nested too deep");
		}
		++m_Serializer->m_ReadDepth;
	}
	~CReadDepth() { --m_Serializer->m_ReadDepth; }

private:
	CReadDepth(const CReadDepth &);
	CReadDepth &operator=(const CReadDepth &);

	CObjectSerializer *m_Serializer;
};

// the needed properties without a value that are logged one by one in a read; the read gives the count of the others
static const uint32 s_MaxMissingLogged = 8;

// the key of the entries of a table that have none
static const std::string s_NoKey;

class CClassSerializer;

class CObjectSerializerImpl : public NLMISC::CSingleton<CObjectSerializerImpl>
{
public:
	~CObjectSerializerImpl() NL_OVERRIDE;

	CObjectSerializerImpl();

	// release singleton
	static void releaseInstance();

	void registerSerializer(CClassSerializer* serializer);

	void serialImpl(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, bool init = false);

	// Reads a whole tree, with its header, into data: NULL for the tree that is no value. Throws
	// NLMISC::EStream, after a warning that says why, when the stream is not a tree within the limits of
	// CObjectSerializer; nothing of what was read is kept then.
	void readTree(NLMISC::IStream &stream, CObject *&data, CObjectSerializer *serializer);

	// Adds a value that was read to the table that is being read, and counts it. A stream with an entry that
	// has no value, a key that is too long or more entries than CObjectSerializer::MaxNodes is refused: the
	// value is deleted and NLMISC::EStream is thrown.
	static void addReadValue(CObject *table, const std::string &key, CObject *value, CObjectSerializer *serializer);

private:
	// Writes, or reads when its kind readType was read, a value that holds no other value. The tables are
	// read and written by the functions below, which serialImpl calls.
	void serialValue(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, uint8 readType);

	// the header that is in front of a tree: the version of the form
	void writeHeader(NLMISC::IStream &stream, CObjectSerializer *serializer);
	// reads the kind of the value that comes, after the header if this is the start of a tree (init)
	uint8 readKind(NLMISC::IStream &stream, CObjectSerializer *serializer, bool init);

	void writeTable(NLMISC::IStream &stream, CObject *data, CObjectSerializer *serializer);
	// the form of the class that the table names, if it names one and fits its form (fitsClass), else NULL
	CClassSerializer *findClassForm(const CObject *data) const;
	void logClassSent(const CClassSerializer *classSerializer, CObjectSerializer *serializer, uint length);
	// the form of the class that has this id on the wire, NULL if the id is of no class
	CClassSerializer *findClassForm(uint8 type) const;
	void writeTableKey(NLMISC::IStream &stream, const CObject *data, uint32 index, CObjectSerializer *serializer);
	void logTableSent(const CObject *data, CObjectSerializer *serializer, uint length);

	void readTable(NLMISC::IStream &stream, CObject *&data, CObjectSerializer *serializer);
	void readTableEntry(NLMISC::IStream &stream, CObject *table, CObject *&value, CObjectSerializer *serializer);

private:
	std::map<uint8, CClassSerializer*> _ClassSerializersById;
	std::map<std::string, CClassSerializer*> _ClassSerializersByName;
};


class CClassSerializer
{
public:
	// True if writeClass can write this table and readClass read back the same entries (the named ones in the order of
	// the class). The form has one "Class" that is a string, each needed property once at most, 8 bit and 16
	// bit counts for the other kinds of entries, and forms of their own for some properties (fitsProperty).
	// The writer writes a table that does not fit as a table without a class, which holds anything.
	bool fitsClass(const CObject *data) const
	{
		uint32 classCount = 0;
		uint32 optionalCount = 0;
		uint32 valueCount = 0;
		uint32 otherCount = 0;
		std::vector<bool> neededFound(_NeededProp.size(), false);

		uint32 size = data->getSize();
		for (uint32 first = 0; first != size; ++first)
		{
			std::string key = data->getKey(first);
			const CObject *value = data->getValueAtPos(first);
			if (!value) return false;

			if (key == "Class")
			{
				if (++classCount > 1 || !isPlainString(value)) return false;
			}
			else if (key.empty())
			{
				++valueCount;
			}
			else
			{
				std::vector<std::string>::const_iterator neededIt = std::find(_NeededProp.begin(), _NeededProp.end(), key);
				if (neededIt != _NeededProp.end())
				{
					// only the first one of a needed property that is there twice would be written
					std::vector<bool>::size_type index = neededIt - _NeededProp.begin();
					if (neededFound[index]) return false;
					neededFound[index] = true;
					if (!fitsProperty(key, value)) return false;
				}
				else if (std::find(_OptionalProp.begin(), _OptionalProp.end(), key) != _OptionalProp.end())
				{
					++optionalCount;
					if (!fitsProperty(key, value)) return false;
				}
				else
				{
					// A property that is not of the class is written as any value. The readers before this
					// one read it by its key all the same, so a key that has a form of its own in onSerial
					// (the "InstanceId" in a run time class) is misread by them: not in the class form.
					if (!fitsProperty(key, nullptr)) return false;
					++otherCount;
				}
			}
		}

		if (classCount != 1 || optionalCount > 0xff || valueCount > 0xffff || otherCount > 0xffff) return false;

		// A needed property that is not there is written as the value that is none, which the forms of their own
		// do not have. (A name that is twice in the list of a class is found at its first place only.)
		for (std::vector<std::string>::size_type index = 0; index != _NeededProp.size(); ++index)
		{
			if (!neededFound[index]
				&& std::find(_NeededProp.begin(), _NeededProp.begin() + index, _NeededProp[index]) == _NeededProp.begin() + index
				&& !fitsProperty(_NeededProp[index], nullptr))
			{
				return false;
			}
		}
		return true;
	}

	// True if onSerial writes this value of a needed or optional property in a form that it reads back; value
	// is NULL for a needed property that is not in the table, so false with NULL tells a key that onSerial has
	// a form of its own for. To override with onSerial.
	virtual bool fitsProperty(const std::string &key, const CObject *value) const
	{
		if (key == "InstanceId") return fitsInstanceId(value);
		return true;
	}

private:
	// the entries of a table that are not its needed properties, by kind, as positions in the table
	struct CEntries
	{
		std::vector<uint32> Optional;
		std::vector<uint32> Other;
		std::vector<uint32> Value;
	};

public:
	// writeClass and readClass are on the stack once per table of a class that is open, as serialImpl is: what
	// needs room (the keys, the log) is in the functions below them, which return before a value is read or
	// written.
	void writeClass(NLMISC::IStream& stream, CObject* data, CObjectSerializer* serializer)
	{
		CEntries entries;
		uint initLength = writeHead(stream, data, entries);

		/*
			Serial needed property
		*/
		if (!_NeededProp.empty())
		{
			//no need to put a number we already known how meany object must be
			serializer->Level +=2;
			for (uint32 first = 0, last = (uint32)_NeededProp.size(); first != last ; ++first)
			{
				//_NeededProp[first] => Property key ()
				CObject* value = getNeeded(data, first, serializer);
				onSerial(stream, _NeededProp[first], value, serializer);
			}
			serializer->Level -=2;
			if (serializer->Log) { initLength = logSent(stream, serializer, "Needed", initLength); }
		}

		// serial optional properties
		if (!entries.Optional.empty())
		{
			serializer->Level +=2;
			uint32 first = 0, last = (uint32)entries.Optional.size();
			uint8 optionalSize = static_cast<uint8>(last);
			stream.serial(optionalSize);
			for (; first != last; ++first)
			{
				uint32 uiKey = writeOptionalKey(stream, data, entries.Optional[first], serializer);
				CObject*value = data->getValueAtPos(entries.Optional[first]);
				onSerial(stream, _OptionalProp[uiKey], value, serializer);
			}
			serializer->Level -=2;
			if (serializer->Log) { initLength = logSent(stream, serializer, "Optional", initLength); }
		}

		// serial value properties
		if (!entries.Value.empty())
		{
			serializer->Level +=2;
			uint32 first = 0, last = (uint32)entries.Value.size();
			uint16 last16 = static_cast<uint16>(last);
			stream.serial(last16);
			for (; first != last; ++first)
			{
				CObject* value = data->getValueAtPos(entries.Value[first]);
				if (serializer->Log) { logValue(serializer, first); }
				CObjectSerializerImpl::getInstance().serialImpl(stream, value, serializer);
			}
			serializer->Level -=2;
			if (serializer->Log) { initLength = logSent(stream, serializer, "Values", initLength); }
		}

		// serial other properties
		if (!entries.Other.empty())
		{
			serializer->Level +=2;
			uint32 first = 0, last = (uint32)entries.Other.size();
			uint16 last16 = static_cast<uint16>(last);
			stream.serial(last16);
			for (; first != last; ++first)
			{
				writeOtherKey(stream, data, entries.Other[first], serializer);
				CObject* value = data->getValueAtPos(entries.Other[first]);
				CObjectSerializerImpl::getInstance().serialImpl(stream, value, serializer);
			}
			serializer->Level -=2;
			if (serializer->Log) { initLength = logSent(stream, serializer, "Other", initLength); }
		}
	}

private:
	void logValue(CObjectSerializer* serializer, uint32 index)
	{
		nldebug("R2NET: (%u) Field [%u]", serializer->Level, index);
	}

	// Sorts the entries of the table by kind and writes the id of the class and the kinds it has entries of.
	// Returns the position of the stream after them, for the log.
	uint writeHead(NLMISC::IStream& stream, const CObject* data, CEntries &entries)
	{
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
					entries.Value.push_back(first);
				}
				else if (std::find(_NeededProp.begin(), _NeededProp.end(), key) == _NeededProp.end())
				{
					std::vector<std::string>::const_iterator optionalFound = std::find(_OptionalProp.begin(), _OptionalProp.end(), key);
					if (optionalFound != _OptionalProp.end())
					{
						entries.Optional.push_back(first);
					}
					else
					{
						entries.Other.push_back(first);
					}
				}

			}
		}

		uint8 id = getId();
		stream.serial(id);

		uint8 next = 0;
		if (!_NeededProp.empty()) next |= 1;
		if (!entries.Optional.empty()) next |= 2;
		if (!entries.Value.empty()) next |= 4;
		if (!entries.Other.empty()) next |= 8;


		stream.serial(next);
		return stream.getPos();
	}

	// the value of a needed property of the table, NULL if the table does not have it
	CObject *getNeeded(const CObject *data, uint32 index, CObjectSerializer* serializer)
	{
		const std::string &key = _NeededProp[index];
		CObject* value = data->getAttr(key);
		if( !value )
		{
			nlwarning( "About to serialize a NULL value at key %s, className %s!",
				key.c_str(), _ClassName.c_str() );
		}
		if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
		return value;
	}

	// writes which optional property the entry of the table is, and returns it as an index in the list of the class
	uint32 writeOptionalKey(NLMISC::IStream& stream, const CObject *data, uint32 position, CObjectSerializer* serializer)
	{
		std::string key = data->getKey(position);
		uint32 uiKey = 0, lastLocal = (uint32)_OptionalProp.size();

		for ( ; uiKey != lastLocal && _OptionalProp[uiKey]!=key ; ++uiKey){}

		uint8 shortKey = static_cast<uint8>(uiKey);
		stream.serial(shortKey);
		if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
		return uiKey;
	}

	void writeOtherKey(NLMISC::IStream& stream, const CObject *data, uint32 position, CObjectSerializer* serializer)
	{
		std::string key = data->getKey(position);
		stream.serial(key);
		if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level, key.c_str());}
	}

	// logs the size of a kind of entries that were written since the position initLength; returns the position now
	uint logSent(NLMISC::IStream& stream, CObjectSerializer* serializer, const char *kind, uint initLength)
	{
		uint endLength = stream.getPos();
		nldebug("R2NET: (%u) %s Properties sent %u bytes", serializer->Level +1, kind, endLength - initLength);
		return endLength;
	}

public:
	void readClass(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer)
	{
		// The table is held until it is complete: if the read of one of its values throws, it is deleted with
		// what it holds and data is left as it was.
		CReadDepth depth(serializer);
		CReadHold hold(newTable(serializer->Factory));
		CObject *table = hold.get();
		addClassName(table, serializer);

		uint8 next;
		stream.serial(next);

		// Needed pop
		if( (next & 1) )
		{
			for (uint32 first = 0, last = (uint32)_NeededProp.size(); first != last ; ++first)
			{
				//_NeededProp[first] => Property key ()
				CObject* value = nullptr;
				onSerial(stream, _NeededProp[first], value, serializer);
				addNeeded(table, first, value, serializer);
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
				CObject* value = nullptr;
				uint8 keyIndex = readOptionalKey(stream);
				onSerial(stream, _OptionalProp[keyIndex], value, serializer);
				CObjectSerializerImpl::addReadValue(table, _OptionalProp[keyIndex], value, serializer);
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
				CObject* value = nullptr;
				onSerial(stream, s_NoKey, value, serializer);
				CObjectSerializerImpl::addReadValue(table, s_NoKey, value, serializer);
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
				readOther(stream, table, serializer);
			}

		}

		data = hold.release();
	}

private:
	void addClassName(CObject* table, CObjectSerializer* serializer)
	{
		CObjectSerializerImpl::addReadValue(table, "Class", new CObjectString(_ClassName), serializer);
	}

	// Adds the value that was read for a needed property to the table that is being read
	void addNeeded(CObject* table, uint32 index, CObject* value, CObjectSerializer* serializer)
	{
		const std::string &key = _NeededProp[index];
		if (!value)
		{
			// this is how a needed property that the table does not have is written
			if (serializer->m_ReadMissing < s_MaxMissingLogged)
			{
				nlwarning("R2Share: the needed property '%s' of class '%s' has no value in the tree read",
					key.c_str(), _ClassName.c_str());
			}
			++serializer->m_ReadMissing;
		}
		else if (std::find(_NeededProp.begin(), _NeededProp.begin() + index, key) != _NeededProp.begin() + index)
		{
			// a name that is twice in the list of the class has its value written twice: one entry is made of it
			CObject::TSmartPtr drop(value);
		}
		else
		{
			CObjectSerializerImpl::addReadValue(table, key, value, serializer);
		}
	}

	// reads which optional property of the class comes, as an index in its list
	uint8 readOptionalKey(NLMISC::IStream& stream)
	{
		uint8 keyIndex;
		stream.serial(keyIndex);
		if (keyIndex >= _OptionalProp.size())
		{
			nlwarning("CClassSerializer: optional property index %u out of %u", (uint)keyIndex, (uint)_OptionalProp.size());
			throw NLMISC::EStream("optional property index out of range");
		}
		return keyIndex;
	}

	void readOther(NLMISC::IStream& stream, CObject* table, CObjectSerializer* serializer)
	{
		std::string key;
		stream.serial(key);
		CObject* value=nullptr;
		// As the writer does, whatever the key: onSerial would read a property that has a form of its own in
		// another class (the "InstanceId" of a run time class) in a form it was not written in.
		CObjectSerializerImpl::getInstance().serialImpl(stream, value, serializer);
		CObjectSerializerImpl::addReadValue(table, key, value, serializer);
	}

public:
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

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer) NL_OVERRIDE;
	virtual bool fitsProperty(const std::string &key, const CObject *value) const NL_OVERRIDE;
};


CNpcSerializer::CNpcSerializer()
{
	static const char* neededProp[] = { "InstanceId", "Base", "Position", "Angle", "Behavior" };
	static const char* optionalProp[] = { "GabaritHeight", "GabaritTorsoWidth","GabaritArmsWidth", "GabaritLegsWidth", "GabaritBreastSize",
		"HairType", "HairColor", "Tattoo", "EyesColor", "MorphTarget1", "MorphTarget2", "MorphTarget3", "MorphTarget4", "MorphTarget5",
		"MorphTarget6", "MorphTarget7", "MorphTarget8", "Sex", "JacketModel", "TrouserModel", "FeetModel", "HandsModel", "ArmModel",
		"WeaponRightHand", "WeaponLeftHand", "JacketColor", "ArmColor", "HandsColor", "TrouserColor", "FeetColor", "Function",
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

bool CNpcSerializer::fitsProperty(const std::string &key, const CObject *value) const
{
	if (key == "Angle") return fitsFixedPoint(value);
	return this->CClassSerializer::fitsProperty(key, value);
}
//------------------
class CNpcCustomSerializer : public CClassSerializer
{
public:
	CNpcCustomSerializer();

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer) NL_OVERRIDE;
	virtual bool fitsProperty(const std::string &key, const CObject *value) const NL_OVERRIDE;
};


CNpcCustomSerializer::CNpcCustomSerializer()
{
	static const char* neededProp[] = { "InstanceId", "Base", "Position", "Angle", "Behavior",
		"GabaritHeight", "GabaritTorsoWidth","GabaritArmsWidth", "GabaritLegsWidth", "GabaritBreastSize",
		"HairType", "HairColor", "Tattoo", "EyesColor", "MorphTarget1", "MorphTarget2", "MorphTarget3", "MorphTarget4", "MorphTarget5",
		"MorphTarget6", "MorphTarget7", "MorphTarget8", "JacketModel", "TrouserModel", "FeetModel", "HandsModel", "ArmModel",
		"JacketColor", "ArmColor", "HandsColor", "TrouserColor", "FeetColor" };
	static const char* optionalProp[] = {
		"Name", "Profile", "Speed", "Aggro", "PlayerAttackable", "BotAttackable", "Function", "WeaponRightHand", "WeaponLeftHand", "Sex" };

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

bool CNpcCustomSerializer::fitsProperty(const std::string &key, const CObject *value) const
{
	if (key == "Angle") return fitsFixedPoint(value);
	return this->CClassSerializer::fitsProperty(key, value);
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

	virtual void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer) NL_OVERRIDE;
	virtual bool fitsProperty(const std::string &key, const CObject *value) const NL_OVERRIDE;
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

bool CPositionSerializer::fitsProperty(const std::string &key, const CObject *value) const
{
	if (key == "x" || key == "y" || key == "z") return fitsFixedPoint(value);
	return this->CClassSerializer::fitsProperty(key, value);
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

	void onSerial(NLMISC::IStream& stream, const std::string& key, CObject*& data, CObjectSerializer* serializer) NL_OVERRIDE
	{
		if (key == "Id")  { serialRtId(stream, data); return; }
		this->CClassSerializer::onSerial(stream, key, data, serializer);
	}

	// The id of a run time object is written as the number in "<class>_<number>" and read back printed with
	// "%d": any other string, or a number that has another decimal form, is not read back as it was.
	bool fitsProperty(const std::string &key, const CObject *value) const NL_OVERRIDE
	{
		if (key == "Id")
		{
			if (!isPlainString(value)) return false;
			std::string str = value->toString();
			std::string prefix = _ClassName + "_";
			return str.compare(0, prefix.size(), prefix) == 0 && isCanonicalNumber(str.substr(prefix.size()), 0x7fffffff);
		}
		return this->CClassSerializer::fitsProperty(key, value);
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
			Instance = nullptr;
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



// a string of the tree read that the writer would refuse to write again is refused
static void checkReadString(const std::string &value)
{
	if (value.size() > CObjectSerializer::MaxStringLength)
	{
		nlwarning("R2Share: the tree read has a string of %u bytes, more than %u, refused",
			(uint)value.size(), (uint)CObjectSerializer::MaxStringLength);
		throw NLMISC::EStream("R2 tree: string too long");
	}
}

void CObjectSerializerImpl::addReadValue(CObject *table, const std::string &key, CObject *value, CObjectSerializer *serializer)
{
	// the value is held from here, and deleted if it does not go in the table
	CObject::TSmartPtr hold(value);

	if (!table)
	{
		nlwarning("R2Share: no table could be made for the tree read, refused");
		throw NLMISC::EStream("R2 tree: no table");
	}
	if (!value)
	{
		// only a needed property of a class is written without a value, and it is not given here
		nlwarning("R2Share: the tree read has a table entry without a value, refused");
		throw NLMISC::EStream("R2 tree: table entry without a value");
	}
	if (key.size() > CObjectSerializer::MaxStringLength)
	{
		nlwarning("R2Share: the tree read has a key of %u bytes, more than %u, refused",
			(uint)key.size(), (uint)CObjectSerializer::MaxStringLength);
		throw NLMISC::EStream("R2 tree: key too long");
	}
	if (serializer->m_ReadNodes >= CObjectSerializer::MaxNodes)
	{
		nlwarning("R2Share: the tree read has more than %u table entries, refused", (uint)CObjectSerializer::MaxNodes);
		throw NLMISC::EStream("R2 tree: too many table entries");
	}
	++serializer->m_ReadNodes;

	table->add(key, hold);
}

void CObjectSerializerImpl::readTree(NLMISC::IStream &stream, CObject *&data, CObjectSerializer *serializer)
{
	serializer->m_ReadDepth = 0;
	serializer->m_ReadNodes = 0;
	serializer->m_ReadMissing = 0;
	// a stream without a header is in the first version
	serializer->setVersion(0);

	CObject *tree = nullptr;
	serialImpl(stream, tree, serializer, true);
	data = tree;

	if (serializer->m_ReadMissing > s_MaxMissingLogged)
	{
		nlwarning("R2Share: %u needed properties have no value in the tree read, the first %u are given above",
			serializer->m_ReadMissing, s_MaxMissingLogged);
	}
	if (serializer->getVersion() == 0)
	{
		nlwarning("R2Share: a tree was read in the first version of its binary form, which is only kept to load old scenario sessions");
	}
}

static bool measureImpl(const CObject *tree, uint32 level, uint32 &depth, uint32 &nodes, const CObject *part, uint32 *partNodes)
{
	if (!tree || !tree->isTable()) return true;

	if (level >= CObjectSerializer::MaxDepth) return false;
	++level;
	if (level > depth) depth = level;

	uint32 nodesBefore = nodes;
	uint32 size = tree->getSize();
	if (size > CObjectSerializer::MaxNodes - nodes)
	{
		nodes = CObjectSerializer::MaxNodes;
		return false;
	}
	nodes += size;

	for (uint32 first = 0; first != size; ++first)
	{
		if (!measureImpl(tree->getValueAtPos(first), level, depth, nodes, part, partNodes)) return false;
	}
	if (tree == part && partNodes)
	{
		*partNodes = nodes - nodesBefore;
	}
	return true;
}

bool CObjectSerializer::measure(const CObject *tree, uint32 &depth, uint32 &nodes, const CObject *part, uint32 *partNodes)
{
	depth = 0;
	nodes = 0;
	if (partNodes) *partNodes = 0;
	return measureImpl(tree, 0, depth, nodes, part, partNodes);
}

// the stream has a header where it has a value, see serialImpl
static void refuseHeader()
{
	nlwarning("R2Share: the tree read has a header where a value is expected, refused");
	throw NLMISC::EStream("R2 tree: header where a value is expected");
}

void CObjectSerializerImpl::serialImpl(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, bool init)
{
	//H_AUTO(R2_CObjectSerializerImpl_serialImpl)

	// This function is on the stack once per table that is open, with the function that reads or writes the
	// table, and a tree has up to CObjectSerializer::MaxDepth tables in each other: they hold as little as they
	// can. What needs room (the strings, the log) is in the functions that return before a table goes deeper.
	if (!stream.isReading())
	{
		if (init)
		{
			writeHeader(stream, serializer);
		}

		if (data != nullptr && data->isTable())
		{
			writeTable(stream, data, serializer);
		}
		else
		{
			serialValue(stream, data, serializer, ObjectNull);
		}
	}
	else
	{
		uint8 type = readKind(stream, serializer, init);

		if (type == ObjectTable)
		{
			readTable(stream, data, serializer);
		}
		else
		{
			CClassSerializer *classSerializer = findClassForm(type);
			if (classSerializer)
			{
				classSerializer->readClass(stream, data, serializer);
			}
			else
			{
				serialValue(stream, data, serializer, type);
			}
		}
	}
}

void CObjectSerializerImpl::writeHeader(NLMISC::IStream &stream, CObjectSerializer *serializer)
{
	uint8 type = ObjectHeaderTag;
	uint32 version = 1;
	stream.serial(type);
	stream.serial(version);
	serializer->setVersion(version);
}

uint8 CObjectSerializerImpl::readKind(NLMISC::IStream &stream, CObjectSerializer *serializer, bool init)
{
	uint8 type = ObjectNull;
	stream.serial(type);

	if (type == ObjectHeaderTag)
	{
		// The header is written once, in front of the tree. It is not read anywhere else, and not by a call
		// of serialImpl for each header of a stream of headers.
		if (!init) refuseHeader();
		uint32 version = 0;
		stream.serial(version);
		serializer->setVersion(version);
		stream.serial(type);
		if (type == ObjectHeaderTag) refuseHeader();
	}
	return type;
}

CClassSerializer *CObjectSerializerImpl::findClassForm(uint8 type) const
{
	std::map<uint8, CClassSerializer*>::const_iterator found(_ClassSerializersById.find(type));
	return found != _ClassSerializersById.end() ? found->second : nullptr;
}

void CObjectSerializerImpl::writeTable(NLMISC::IStream &stream, CObject *data, CObjectSerializer *serializer)
{
	uint initLength = stream.getPos();
	CClassSerializer *classSerializer = findClassForm(data);
	if (classSerializer)
	{
		classSerializer->writeClass(stream, data, serializer);
		if (serializer->Log)
		{
			logClassSent(classSerializer, serializer, stream.getPos() - initLength);
		}
		return;
	}

	uint8 type = ObjectTable;
	stream.serial(type);
	uint32 size = data->getSize();
	stream.serial(size);
	for (uint first = 0; first != size; ++first)
	{
		writeTableKey(stream, data, first, serializer);
		CObject* value = data->getValueAtPos(first);
		++ (serializer->Level);
		serialImpl(stream, value, serializer);
		-- (serializer->Level);
	}
	if (serializer->Log)
	{
		logTableSent(data, serializer, stream.getPos() - initLength);
	}
}

void CObjectSerializerImpl::logClassSent(const CClassSerializer *classSerializer, CObjectSerializer *serializer, uint length)
{
	nldebug("R2NET: (%u) Class '%s' sent %u bytes", serializer->Level, classSerializer->getClassName().c_str(), length);
}

CClassSerializer *CObjectSerializerImpl::findClassForm(const CObject *data) const
{
	std::string className;
	if ( data->isString("Class"))
	{
		className = data->toString("Class");
	}
	if (className.empty())
	{
		return nullptr;
	}

	// a table that names a class and has not the form of the class is written as a table without a class
	std::map<std::string, CClassSerializer*>::const_iterator found(_ClassSerializersByName.find(className));
	if (found != _ClassSerializersByName.end() && found->second->fitsClass(data))
	{
		return found->second;
	}
	return nullptr;
}

void CObjectSerializerImpl::writeTableKey(NLMISC::IStream &stream, const CObject *data, uint32 index, CObjectSerializer *serializer)
{
	std::string key = data->getKey(index);
	stream.serial(key);
	if (serializer->Log) { nldebug("R2NET: (%u) Field '%s'", serializer->Level + 1, key.c_str());}
}

void CObjectSerializerImpl::logTableSent(const CObject *data, CObjectSerializer *serializer, uint length)
{
	std::string className;
	if ( data->isString("Class"))
	{
		className = data->toString("Class");
	}
	if (className.empty())
	{
		nldebug("R2NET: (%u) Table sent %u bytes",serializer->Level, length);
	}
	else
	{
		nldebug("R2NET: (%u) Generic Class(%s) sent %u bytes", serializer->Level, className.c_str(), length);
	}
}

void CObjectSerializerImpl::readTable(NLMISC::IStream &stream, CObject *&data, CObjectSerializer *serializer)
{
	uint32 size;
	stream.serial(size);
	// The table is held until it is complete: if the read of one of its values throws, it is deleted with what
	// it holds and data is left as it was. The size is not trusted for anything: a table that is said larger
	// than it is ends with the stream, or with the limit of entries.
	CReadDepth depth(serializer);
	CReadHold table(newTable(serializer->Factory));
	for (uint32 first = 0 ; first != size; ++first)
	{
		CObject* value=nullptr;
		readTableEntry(stream, table.get(), value, serializer);
	}
	data = table.release();
}

void CObjectSerializerImpl::readTableEntry(NLMISC::IStream &stream, CObject *table, CObject *&value, CObjectSerializer *serializer)
{
	std::string key;
	stream.serial(key);
	serialImpl(stream, value, serializer);
	addReadValue(table, key, value, serializer);
}

void CObjectSerializerImpl::serialValue(NLMISC::IStream& stream, CObject*& data, CObjectSerializer* serializer, uint8 readType)
{
	uint8 type;


	if (!stream.isReading())
	{
		if (data == nullptr)
		{
			type = ObjectNull;
			uint initLength = stream.getPos();
			stream.serial(type);
			uint endLength = stream.getPos();
			if (serializer->Log) { nldebug("R2NET: (%u) Null sent %u bytes", serializer->Level, endLength - initLength);}
		}
		else if (data->isInteger())
		{
			uint initLength = stream.getPos();
			writeInteger(stream, data->toInteger());
			uint endLength = stream.getPos();
			if (serializer->Log) { nldebug("R2NET: (%u) Integer sent %u bytes", serializer->Level, endLength - initLength); }
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
		// a table is written by writeTable
	}
	else
	{
		// the kind was read by serialImpl, which reads the tables and the header itself
		uint8 type = readType;

		// NB nico : use factory here instead of plain 'new' because client may use derived classes internally
		// NB 2 : if server is created locally, not a problem because derived object fonctinnality


		switch(type)
		{
			case ObjectNull:
			{
				data = nullptr;
				return;
			}
			case ObjectNumber:
			{
				double value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", value);
				return;
			}
			case ObjectNumberZero:
			{
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", 0.0);
				return;
			}
			case ObjectNumberSInt32:
			{
				sint32 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}

			case ObjectNumberUInt32:
			{
				uint32 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}

			case ObjectNumberSInt16:
			{
				sint16 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}

			case ObjectNumberUInt16:
			{
				uint16 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}

			case ObjectNumberSInt8:
			{
				sint8 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}

			case ObjectNumberUInt8:
			{
				uint8 value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (sint64)value);
				return;
			}
			// Do not remove this or it would be impossible to load old session
			case ObjectNumberFloat:
			{
				float value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
				((CObjectNumber *) data)->set("", (double)value);
				return;
			}

			case ObjectNumberDouble:
			{
				double value;
				stream.serial(value);
				data = serializer->Factory ? serializer->Factory->newBasic("Number") : new CObjectNumber((sint64)0);
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
				checkReadString(value);
				data = serializer->Factory ? serializer->Factory->newBasic("String") : new CObjectString("");
				((CObjectString *) data)->set("", value);
				break;
			}

			case ObjectRefId:
			{
				std::string value;
				stream.serial(value);
				checkReadString(value);
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
			default:
				// not a table, not a class that is registered (serialImpl), and none of the kinds above
				nlwarning("R2Share: the tree read has a value of the unknown kind %u, refused", (uint)type);
				throw NLMISC::EStream("R2 tree: unknown kind of value");
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
	if ( stream.isReading() )
	{
		// What comes is from the other side of a connection. What this object held is dropped first, and it only
		// holds something again once that was read whole: a read that throws leaves it without a tree.
		_Data = nullptr;
		delete [] _CompressedBuffer;
		_CompressedBuffer = nullptr;
		_CompressedLen = 0;
		_UncompressedLen = 0;
		_Compressed = false;
		_MustUncompress = false;

		bool compressed = false;
		stream.serial(compressed);
		if (!compressed)
		{
			CObject* rawData = nullptr;
			CObjectSerializerImpl::getInstance().readTree(stream, rawData, this);
			_Data = rawData;
		}
		else
		{
			uint32 compressedLen = 0;
			uint32 uncompressedLen = 0;
			stream.serial(compressedLen);
			stream.serial(uncompressedLen);

			// The sizes are checked before anything is allocated for them. The data is only expanded when the
			// tree is asked for (uncompressImpl), or never when it is sent on as it is.
			if (compressedLen > MaxCompressedSize || uncompressedLen > MaxUncompressedSize
				|| (uint64)uncompressedLen > (uint64)compressedLen * 1032)
			{
				nlwarning("R2Share: compressed tree of %u bytes said to expand to %u bytes, refused (at most %u bytes, expanding to %u bytes and to 1032 times the size)",
					compressedLen, uncompressedLen, (uint)MaxCompressedSize, (uint)MaxUncompressedSize);
				throw NLMISC::EStream("R2 tree: size of the compressed data out of range");
			}

			// the buffer belongs to this object from here: it is not lost if the stream ends before its end
			_CompressedBuffer = new uint8[compressedLen];
			stream.serialBuffer(_CompressedBuffer, compressedLen);
			_CompressedLen = compressedLen;
			_UncompressedLen = uncompressedLen;
			_Compressed = true;
			_MustUncompress = true;
		}
		return;
	}

	stream.serial(_Compressed);

	if (!_Compressed)
	{
		CObject* rawData = _Data.getPtr();
		CObjectSerializerImpl::getInstance().serialImpl(stream, rawData, this, true);
	}
	else
	{
		stream.serial(_CompressedLen);
		stream.serial(_UncompressedLen);
		stream.serialBuffer(_CompressedBuffer, _CompressedLen);
	}
}

CObject::TSmartPtr CObjectSerializer::getData() const
{
	//H_AUTO(R2_CObjectSerializer_getData)
	if (_Compressed && _MustUncompress) { uncompress(); };
	if (_Data) return CObject::TSmartPtr(_Data->clone());
	return nullptr;
}


CObjectSerializer::CObjectSerializer(CObjectFactory *factory, CObject* data)
	:	Factory(factory),
		Level(0),
		Log(false)
{

	Log = false;
	_CompressedBuffer = nullptr;
	_CompressedLen = 0;
	_UncompressedLen = 0;
	_Compressed = false;
	_MustUncompress = false;
	_Version = 0;
	m_ReadDepth = 0;
	m_ReadNodes = 0;
	m_ReadMissing = 0;
	if (data)
	{
		_Data = data->clone();
	}
	else
	{
		_Data = nullptr;
	}

}

CObjectSerializer::~CObjectSerializer()
{
	if (_CompressedBuffer) { delete [] _CompressedBuffer; _CompressedBuffer = nullptr;}
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
		_Data = nullptr;
	}
}






void CObjectSerializer::compress()
{
	//H_AUTO(R2_CObjectSerializer_compress)

	NLMISC::CMemStream buffer;
	if (buffer.isReading()) buffer.invert();
	uint32 init =  buffer.length();
	CObject* rawData = _Data.getPtr();
	CObjectSerializerImpl::getInstance().serialImpl(buffer, rawData, this, true);
	uint32 length =  buffer.length() - init;

	// The reader refuses compressed data above these sizes, and it reads this tree when it is not compressed
	// (it has its limits of tables and entries, none on the size): the tree is then sent as it is.
	if (length > MaxUncompressedSize)
	{
		nlwarning("R2Share: a tree of %u bytes is not compressed, the reader expands %u bytes at most", length, (uint)MaxUncompressedSize);
		return;
	}

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
	if (destLen > MaxCompressedSize)
	{
		delete [] dest;
		nlwarning("R2Share: a tree of %u bytes is not compressed, it compresses to %u bytes and the reader takes %u bytes at most",
			length, (uint)destLen, (uint)MaxCompressedSize);
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
		_Data = nullptr;

		// The data was read from a stream (serial checks the sizes it gives) and is from the other side of a
		// connection. Whatever is wrong with it, this object is left without a tree, which is what the
		// callers of getData() test: nothing is thrown from here.
		if (!_CompressedBuffer || _CompressedLen == 0 || _UncompressedLen == 0
			|| _CompressedLen > MaxCompressedSize || _UncompressedLen > MaxUncompressedSize
			|| (uint64)_UncompressedLen > (uint64)_CompressedLen * 1032)
		{
			nlwarning("R2Share: compressed tree of %u bytes said to expand to %u bytes, refused", _CompressedLen, _UncompressedLen);
			return;
		}

		std::vector<uint8> data(_UncompressedLen);
		uLongf dataLen = _UncompressedLen;
		sint32 state = ::uncompress (reinterpret_cast<Bytef*>(&data[0]), &dataLen ,
			reinterpret_cast<Bytef*>(_CompressedBuffer), _CompressedLen);

		if (state != Z_OK)
		{
			nlwarning("R2Share: compressed tree of %u bytes said to expand to %u bytes does not expand (zlib error %d), refused",
				_CompressedLen, _UncompressedLen, (int)state);
			return;
		}
		if (_UncompressedLen != dataLen)
		{
			nlwarning("R2Share: compressed tree of %u bytes said to expand to %u bytes expands to %u bytes, refused",
				_CompressedLen, _UncompressedLen, (uint)dataLen);
			return;
		}

		NLMISC::CMemStream buffer;
		if (buffer.isReading()) buffer.invert();
		buffer.serialBuffer(&data[0], _UncompressedLen);
		buffer.invert();
		buffer.seek(0, NLMISC::IStream::begin);

		try
		{
			CObject* rawData = nullptr;
			CObjectSerializerImpl::getInstance().readTree(buffer, rawData, this);
			_Data = rawData;
		}
		catch (const NLMISC::EStream &e)
		{
			// the read has said what was wrong, and has deleted what it had read
			nlwarning("R2Share: the tree of the compressed data (%u bytes, expanded to %u bytes) is refused: %s",
				_CompressedLen, _UncompressedLen, e.what());
		}
	}
}
}
