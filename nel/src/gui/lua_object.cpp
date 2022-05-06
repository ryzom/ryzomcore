// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/lua_object.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/lua_helper.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	// *************************************************
	CLuaObject::CLuaObject(CLuaState &state, const char *id)
	{
		pop(state, id);
	}

	// *************************************************
	CLuaObject::CLuaObject(CLuaState &state, const std::string &id)
	{
		pop(state, id.c_str());
	}


	// *************************************************
	bool CLuaObject::isValid() const
	{
		return getLuaState() != NULL;
	}

	// *************************************************
	CLuaState *CLuaObject::getLuaState() const
	{
		return _LuaState;
	}

	// *************************************************
	CLuaObject::CLuaObject(const CLuaObject &other)
	{
		if (other.isValid())
		{
			other.push();
			nlassert(other.getLuaState());
			pop(*other.getLuaState());
			_Id = other._Id;
		}
		// else ... copy of an invalid CLuaObject( is an invalid CLuaObject
	}

	// *************************************************
	CLuaObject &CLuaObject::operator=(const CLuaObject &other)
	{
		if (!other.isValid())
		{
			release();
			return *this;
		}
		other.push();
		pop(*other.getLuaState());
		_Id = other._Id;
		return *this;
	}

	// *************************************************
	bool CLuaObject::rawEqual(const CLuaObject &other) const
	{
		nlassert(other.getLuaState() == getLuaState());
		push();
		other.push();
		bool equal = other.getLuaState()->rawEqual(-1, -2);
		getLuaState()->pop(2);
		return equal;
	}

	// *************************************************
	CLuaObject::~CLuaObject()
	{
		release();
	}

	// *************************************************
	void CLuaObject::release()
	{
		if (_LuaState)
		{
			CLuaStackChecker lsc(_LuaState);
			_LuaState->pushLightUserData((void *) this);
			_LuaState->pushNil();
			_LuaState->setTable(LUA_REGISTRYINDEX);
			_LuaState = NULL;
		}
	}

	// *************************************************
	void CLuaObject::pop(CLuaState &luaState, const char *id)
	{
		release();
		nlassert(luaState.getTop() >= 1);
		{
			CLuaStackChecker lsc(&luaState);
			luaState.pushLightUserData((void *) this);
			luaState.pushValue(-2); // copy original object
			luaState.setTable(LUA_REGISTRYINDEX);
		}
		luaState.pop();
		_LuaState = &luaState;
		_Id = id;
	}

	// *************************************************
	void CLuaObject::push() const
	{
		nlassert(isValid());
		_LuaState->pushLightUserData((void *) this);
		_LuaState->getTable(LUA_REGISTRYINDEX);
	}

	// *************************************************
	int CLuaObject::type() const
	{
		push();
		int type = _LuaState->type();
		_LuaState->pop();
		return type;
	}

	// *************************************************
	const char *CLuaObject::getTypename() const
	{
		push();
		const char *typeName = _LuaState->getTypename(-1);
		_LuaState->pop();
		return typeName;
	}

	// *************************************************
	bool CLuaObject::isNil() const           { push(); bool result = _LuaState->isNil();           _LuaState->pop(); return result; }
	bool CLuaObject::isNumber() const        { push(); bool result = _LuaState->isNumber();        _LuaState->pop(); return result; }
	bool CLuaObject::isInteger() const       { push(); bool result = _LuaState->isInteger();       _LuaState->pop(); return result; }
	bool CLuaObject::isBoolean() const       { push(); bool result = _LuaState->isBoolean();       _LuaState->pop(); return result; }
	bool CLuaObject::isString() const        { push(); bool result = _LuaState->isString();        _LuaState->pop(); return result; }
	bool CLuaObject::isFunction() const      { push(); bool result = _LuaState->isFunction();      _LuaState->pop(); return result; }
	bool CLuaObject::isCFunction() const     { push(); bool result = _LuaState->isCFunction();     _LuaState->pop(); return result; }
	bool CLuaObject::isTable() const         { push(); bool result = _LuaState->isTable();         _LuaState->pop(); return result; }
	bool CLuaObject::isUserData() const      { push(); bool result = _LuaState->isUserData();      _LuaState->pop(); return result; }
	bool CLuaObject::isLightUserData() const { push(); bool result = _LuaState->isLightUserData(); _LuaState->pop(); return result; }
	bool CLuaObject::isRGBA() const
	{
		if (!isUserData()) return false;
		push();
		NLMISC::CRGBA dummy;
		return CLuaIHM::pop(*_LuaState, dummy);
	}

	// *************************************************
	bool            CLuaObject::toBoolean() const    {	push(); bool          result  = _LuaState->toBoolean();   _LuaState->pop(); return result; }
	lua_Number		CLuaObject::toNumber() const     {	push(); lua_Number    result  = _LuaState->toNumber();    _LuaState->pop(); return result; }
	lua_Integer		CLuaObject::toInteger() const     {	push(); lua_Integer   result  = _LuaState->toInteger();    _LuaState->pop(); return result; }
	std::string		CLuaObject::toString() const
	{
		push();
		const char *str = _LuaState->toString();
		std::string result = str ? str : "";
		_LuaState->pop();
		return result;
	}
	lua_CFunction	CLuaObject::toCFunction() const  {	push(); lua_CFunction result  = _LuaState->toCFunction(); _LuaState->pop(); return result; }
	void			*CLuaObject::toUserData() const  {  push(); void          *result = _LuaState->toUserData();  _LuaState->pop(); return result; }
	const void		*CLuaObject::toPointer() const   {  push(); const void    *result = _LuaState->toPointer();   _LuaState->pop(); return result; }
	NLMISC::CRGBA CLuaObject::toRGBA() const
	{
		NLMISC::CRGBA result;
		push();
		if (CLuaIHM::pop(*_LuaState, result))
		{
			return result;
		}
		return NLMISC::CRGBA::Black;
	}

	// *************************************************
	CLuaObject::operator bool() const         { return toBoolean(); }
	CLuaObject::operator float() const        { return (float) toNumber(); }
	CLuaObject::operator double() const       { return (double) toNumber(); }
	CLuaObject::operator sint32() const        { return (sint32) toInteger(); }
	CLuaObject::operator sint64() const       { return (sint64) toInteger(); }
	CLuaObject::operator std::string() const  { return toString(); }


	// *************************************************
	bool CLuaObject::isEnumerable() const
	{
		if (isTable()) return true;
		CLuaStackRestorer lsr(_LuaState, _LuaState->getTop());
		push();
		if (_LuaState->getMetaTable(-1))
		{
			_LuaState->remove(-2);
			_LuaState->push("__next");
			_LuaState->getTable(-2);
			return _LuaState->isFunction();
		}
		return false;
	}

	// *************************************************
	CLuaEnumeration CLuaObject::enumerate()
	{
		if (!isEnumerable())
		{
			throw ELuaNotATable(NLMISC::toString("Called CLuaObject::enumerate on an object that has no enumeration (not a table or no '__next' method in the metatable). Object is '%s' with type '%s'", getId().c_str(), getTypename()).c_str());
		}
		return CLuaEnumeration(*this);
	}

	// *************************************************
	CLuaObject CLuaObject::operator[](const char *key) const
	{
		nlassert(key);
		nlassert(isValid());
		if (!isEnumerable())
		{
			_LuaState->pushNil();
			CLuaObject result(*_LuaState);
			return result;
		}
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->getTable(-2);
		CLuaObject subObject(*_LuaState, concatId(getId(), key));
		_LuaState->pop(); // pop the sub object
		return subObject;
	}

	// *************************************************
	CLuaObject CLuaObject::operator[](double key) const
	{
		nlassert(isValid());
		if (!isEnumerable())
		{
			_LuaState->pushNil();
			CLuaObject result(*_LuaState);
			return result;
		}
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->getTable(-2);
		CLuaObject subObject(*_LuaState, concatId(getId(), NLMISC::toString(key)));
		_LuaState->pop(); // pop the sub object
		return subObject;
	}

	// *************************************************
	CLuaObject CLuaObject::at(const char *key) const
	{
		if (!isEnumerable()) throw ELuaNotATable(NLMISC::toString("Can't get key '%s' in object '%s' because type is '%s', it is not a table.", key, getId().c_str(), getTypename()).c_str());
		return operator[](key);
	}

	// *************************************************
	bool CLuaObject::hasKey(const char *key) const
	{
		if (!isEnumerable()) throw ELuaNotATable(NLMISC::toString("Trying to access key '%s' on object '%s' of type %s.", key, getId().c_str(), getTypename()).c_str());
		CLuaObject value = operator[](key);
		return (!value.isNil());
	}

	// *************************************************
	CLuaObject CLuaObject::newTable(const char *tableName)
	{
		nlassert(tableName);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to create a subtable '%s' on object '%s' of type %s (not a table).", tableName, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(tableName);
		_LuaState->newTable();
		_LuaState->setTable(-3);
		_LuaState->pop();
		return at(tableName); //\TODO nico double copy here ...
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  const CLuaObject &value)
	{
		nlassert(key);
		nlassert(isValid());
		nlassert(value.isValid());
		nlassert(getLuaState() == value.getLuaState());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%s' on object '%s' of type %s (not a table).", key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		value.push();
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setNil(const char *key)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value 'nil' at key %s on object '%s' of type %s (not a table).", key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->pushNil();
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  const char *value)
	{
		nlassert(value);
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%s' at key %s on object '%s' of type %s (not a table).", value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  const std::string &value)
	{
		setValue(key, value.c_str());
	}

	// *************************************************
	void CLuaObject::setValue(const char *key, bool value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%s' at key %s on object '%s' of type %s (not a table).", value ? "true" : "false", key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key, TLuaWrappedFunction value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a function value '%p' at key %s on object '%s' of type %s (not a table).", (void *)value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  double value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%lf' at key %s on object '%s' of type %s (not a table).", value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  uint32 value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%u' at key %s on object '%s' of type %s (not a table).", value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  sint32 value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%d' at key %s on object '%s' of type %s (not a table).", value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::setValue(const char *key,  sint64 value)
	{
		nlassert(key);
		nlassert(isValid());
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to set a value '%" NL_I64 "d' at key %s on object '%s' of type %s (not a table).", value, key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->push(value);
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	void CLuaObject::eraseValue(const char *key)
	{
		nlassert(isValid());
		nlassert(key);
		if (!isTable()) throw ELuaNotATable(NLMISC::toString("Trying to erase a value with key '%s' on object '%s' of type %s (not a table).", key, getId().c_str(), getTypename()));
		CLuaStackChecker lsc(_LuaState);
		push();
		_LuaState->push(key);
		_LuaState->pushNil();
		_LuaState->setTable(-3);
		_LuaState->pop();
	}

	// *************************************************
	bool CLuaObject::callNoThrow(int numArgs, int numRet)
	{
		nlassert(isValid());
		if (!isFunction())
		{
			nlwarning("Calling a non function object (id = %s, type = %s)", getId().c_str(), getTypename());
			_LuaState->pop(numArgs);
			return false;
		}
		// TMP TMP
		static volatile bool dumpFunction = false;
		if (dumpFunction)
		{
			CLuaStackRestorer lsr(_LuaState, _LuaState->getTop());
			push();
			lua_Debug ar;
			lua_getinfo (_LuaState->getStatePointer(), ">lS", &ar);
			nlwarning((std::string(ar.what) + ", at line " + NLMISC::toString(ar.linedefined) + " in " + std::string(ar.source)).c_str());
		}
		push();
		_LuaState->insert(-1 - numArgs); // put the function before its arguments
		int result = _LuaState->pcall(numArgs, numRet);
		switch (result)
		{
			case LUA_ERRRUN:
			case LUA_ERRMEM:
			case LUA_ERRERR:
				nlwarning(_LuaState->toString(-1));
				_LuaState->pop();
				return false;
			break;
			case 0:
				return true;
			break;
			default:
				nlassert(0);
			break;
		}
		return false;
	}

	// *************************************************
	bool CLuaObject::callMethodByNameNoThrow(const char *name, int numArgs, int numRet)
	{
		nlassert(isValid());
		int initialStackSize = _LuaState->getTop();
		if (!isTable() && !isUserData())
		{
			nlwarning("Can't call method : object is not a table (id = %s)", getId().c_str());
			_LuaState->setTop(std::max(0, initialStackSize - numArgs));
			return false;
		}
		CLuaObject method = (*this)[name];
		push();				  // the 'self' parameter
		_LuaState->insert(-1 - numArgs); // put 'self' before the arguments
		if (method.callNoThrow(numArgs + 1, numRet))
		{
			return true;
		}
		_LuaState->setTop(std::max(0, initialStackSize - numArgs));
		return false;
	}

	/////////////////////
	// CLuaEnumeration //
	/////////////////////

	// *************************************************
	CLuaEnumeration::CLuaEnumeration(CLuaObject &table)
	{
		nlassert(table.isEnumerable());
		CLuaState *luaState = table.getLuaState();
		CLuaStackChecker lsc(luaState);
		// get pointer to the 'next' function
#if LUA_VERSION_NUM >= 502
		luaState->pushGlobalTable();
#else
		luaState->pushValue(LUA_GLOBALSINDEX);
#endif
		_NextFunction = CLuaObject(*luaState)["next"];
		//
		nlassert(luaState);
		table.push();
		luaState->pushNil();
		_HasNext = false;
		if (_NextFunction.callNoThrow(2, 2))
		{
			_Value.pop(*luaState);
			_Key.pop(*luaState);
			_HasNext = !_Key.isNil();
			_Value.setId(CLuaObject::concatId(table.getId(), _Key.toString()));
			_Table = table;
		}

	}

	// *************************************************
	const CLuaObject &CLuaEnumeration::nextKey() const
	{
		nlassert(_HasNext);
		return _Key;
	}

	// *************************************************
	CLuaObject &CLuaEnumeration::nextValue()
	{
		nlassert(_HasNext);
		return _Value;
	}

	// *************************************************
	CLuaObject CLuaObject::getMetaTable() const
	{
		nlassert(isValid());
		CLuaStackChecker lsc(_LuaState);
		push();
		if (_LuaState->getMetaTable(-1))
		{
			_LuaState->remove(-2);
			return CLuaObject(*_LuaState);
		}
		_LuaState->pop();
		_LuaState->pushNil();
		return CLuaObject(*_LuaState);
	}

	// *************************************************
	bool CLuaObject::setMetaTable(CLuaObject &metatable)
	{
		nlassert(isValid());
		nlassert(metatable.isValid());
		nlassert(this->getLuaState() == metatable.getLuaState());
		CLuaStackChecker lsc(_LuaState);
		push();
		metatable.push();
		bool ok = _LuaState->setMetaTable(-2);
		_LuaState->pop(1);
		return ok;
	}

	// *************************************************
	std::string CLuaObject::toStringRecurse(uint depth /*=0*/, uint maxDepth /*= 20*/,  std::set<const void *> *alreadySeen /*= NULL */) const
	{
		if (maxDepth != 0 && depth > maxDepth) return "";
		const uint INDENT_NUM_BLANK = 2;
		std::string indentStr(depth * INDENT_NUM_BLANK, ' ');
		nlassert(_LuaState);
		if (isEnumerable()) // is enumeration possible on that object ?
		{
			std::string result;
			if (alreadySeen)
			{
				if (alreadySeen->count(toPointer())) // avoid cyclic graph (infinite recursion else)
				{
					result += indentStr +"<cycle>";
					return result;
				}
				alreadySeen->insert(toPointer());
			}
			result += indentStr + "{\n";
			CLuaObject *table = const_cast<CLuaObject *>(this);
			uint numElem = 0;
			ENUM_LUA_TABLE(*table, it)
			{
				//nlwarning("entering table %s", it.nextKey().toString().c_str());
				result += std::string((depth + 1) * INDENT_NUM_BLANK, ' ') + it.nextKey().toString() + " = ";
				if (it.nextValue().isEnumerable())
				{
					result += "\n" + it.nextValue().toStringRecurse(depth + 1, maxDepth, alreadySeen);
				}
				else
				{
					result += it.nextValue().toStringRecurse();
				}
				result += ",\n";
				++ numElem;
				if (numElem > 4000)
				{
					throw NLMISC::Exception("possible infinite loop, aborting enumeration");
				}
			}
			result += indentStr + "}";
			return result;
		}
		else if (isNil())
		{
			return (indentStr + "nil").c_str();
		}
		else if (isString())
		{
			return (indentStr + "\"" + toString() + "\"").c_str();
		}
		else if (isFunction())
		{
			return (indentStr + "<function>").c_str();
		}
		else
		{
			return ((indentStr + toString()).c_str());
		}
	}

	// *************************************************
	void CLuaObject::dump(uint maxDepth /*= 20*/, std::set<const void *> *alreadySeen /*= NULL */) const
	{
		try
		{
			std::string str = toStringRecurse(0, maxDepth, alreadySeen);
			std::vector<std::string> res;
			NLMISC::explode(str, std::string("\n"), res);
			for(uint k = 0; k < res.size(); ++k)
			{
				NLMISC::InfoLog->forceDisplayRaw((res[k] + "\n") .c_str());
			}
		}
		catch(const std::exception &e)
		{
			//CLuaIHMRyzom::dumpCallStack();
			nlwarning(e.what());
		}
	}

	// *************************************************
	std::string CLuaObject::concatId(const std::string &left,const std::string &right)
	{
		if (!right.empty() && isdigit(right[0]))
		{
			if (left.empty()) return "[" + right + "]";
			return left + "[" + right + "]";
		}
		if (left.empty()) return right;
		return left + "." + right;

	}

	// *************************************************
	void CLuaEnumeration::next()
	{
		nlassert(_HasNext);
		CLuaState *luaState = _Table.getLuaState();
		nlassert(luaState);
		CLuaStackChecker lsc(luaState);
		_Table.push();
		_Key.push();
		_HasNext = false;
		if (_NextFunction.callNoThrow(2, 2))
		{
			_Value.pop(*luaState);
			_Key.pop(*luaState);
			_HasNext = !_Key.isNil();
			_Value.setId(_Table.getId() + "." + _Key.toString());
		}
	}

}
