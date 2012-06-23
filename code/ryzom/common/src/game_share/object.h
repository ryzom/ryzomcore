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

#ifndef R2_OBJECT_H
#define R2_OBJECT_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/mem_stream.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>

namespace R2
{


class CObject;
class CObjectTable;
class CObjectString;
class CObjectRefId;
class CObjectNumber;
class CSerializeContext;


// vistor triggered at traversal of object tree. see CObject::visit
struct IObjectVisitor
{
	virtual ~IObjectVisitor() { }

	virtual void visit(CObjectRefId &/* obj */) {}
	virtual void visit(CObjectString &/* obj */) {}
	virtual void visit(CObjectNumber &/* obj */) {}
	virtual void visit(CObjectTable &/* obj */) {}
};


class CObject : public NLMISC::CRefCount
{
public:
	typedef NLMISC::CRefPtr<CObject> TRefPtr;
	typedef NLMISC::CSmartPtr<CObject> TSmartPtr;

	// recursively visit hierarchy of objects
	void visit(IObjectVisitor &visitor);

	virtual ~CObject();

	virtual CObject* clone() const;

	void serialize(std::string& out) const;

	virtual void doSerialize(std::string& out, CSerializeContext& context) const = 0;

	virtual const char *getTypeAsString() const = 0;

	CObject* findAttr(const std::string & first) const;

	CObject* findAttr(const std::string & first, const std::string & second) const;

	CObject* findAttr(const std::string & first, const std::string & second, const std::string & third) const;

	CObject* findAttr(const std::vector<std::string>& attrName ) const;
	// test type
	bool isNumber(const std::string & prop="") const;

	bool isString(const std::string & prop="") const;

	bool isTable(const std::string & prop="") const;

	bool isRefId(const std::string & prop="") const;

	virtual bool insert(const std::string & key, CObject * value, sint32 position = -1);

	// to Value
	double toNumber(const std::string & prop="") const;

	std::string toString(const std::string & prop="") const;

	CObjectTable* toTable(const std::string & prop="") const;


	virtual sint32 findIndex(const CObject* child) const;

	// find index from a key, or -1 if not found
	virtual sint32 findIndex(const std::string &key) const;

	// as table
	virtual CObject* getAttr(const std::string & name) const;

	virtual std::string getKey(uint32 pos) const;

	virtual CObject* getValue(uint32 pos) const;

	virtual uint32 getSize() const;

	virtual CObject* take(sint32 pos);

	virtual bool canTake(sint32 pos) const;


	// add Value

	void add(const std::string & key, CObject* value);

	void add(CObject* value);

	void add(const std::string& key, const std::string & value);

	void add(const std::string& key, double value);


	// set Value
	virtual bool set(const std::string& key, const std::string & value);

	virtual bool set(const std::string& key, double value);

	virtual bool setObject(const std::string& key, CObject* value);

	CObject* getParent() const;

	void setParent(CObject* parent);

	// tmp for debug
	virtual void dump(const std::string prefix = "", uint depth = 0) const = 0;

	/** In place copy of object "src" into this object
	  * Type and fields must match, otherwise error msg are printed
	  */
	void inPlaceCopy(const CObject &src);

	virtual bool equal(const CObject* /* other */) const { return false; }

	static std::string uint32ToInstanceId(uint32 id);

	static uint32 instanceIdToUint32(const std::string& instanceId);

	// set / get the 'ghost' flag, meaning that this objet only exist for the client
	bool   getGhost() const;
	virtual void  setGhost(bool ghost);

	virtual void checkIntegrity() const {}

	/** Find the shortest name for this object as a (instanceId, attrName, position) triplet
      * \return false if such a name could not be found
	  */
	bool getShortestName(std::string &instanceId, std::string &attrName, sint32 &position) const;
	bool getNameInParent(std::string &instanceId, std::string &attrName, sint32 &position) const;



protected:
	explicit CObject();

	virtual bool doIsNumber() const;

	virtual bool doIsString() const;

	virtual bool doIsTable() const;

	virtual bool doIsRefId() const;

	virtual double doToNumber() const;

	virtual std::string doToString() const;

	virtual CObjectTable* doToTable() const;


public:
	virtual void previsit(std::vector<TRefPtr> &sons);

public:
	virtual void inPlaceCopyTo(CObject &dest) const = 0;
	virtual void inPlaceCopy(const CObjectString &src);
	virtual void inPlaceCopy(const CObjectNumber &src);
	virtual void inPlaceCopy(const CObjectTable &src);
protected:
	void		 copyMismatchMsg(const CObject &src);
	virtual void visitInternal(IObjectVisitor &visitor) = 0;

private:
	CObject* _Parent;

protected:
	bool	 _Ghost;

	uint32 _Validation;
};

/*inline std::ostream& operator<<( std::ostream& os, const CObject& c )
{
	c.serialize(os);
	return os;
}*/

class CObjectString : public CObject
{
public:
	explicit CObjectString(const std::string & value);

	virtual const char *getTypeAsString() const;

	virtual CObject* clone() const;

	virtual bool set(const std::string& key, const std::string & value);

	virtual bool setObject(const std::string& key, CObject* value);

	const std::string &getValue() const { return _Value; }

	virtual void dump(const std::string prefix = "", uint depth = 0) const;

	virtual bool equal(const CObject* other) const;

protected:
	virtual void visitInternal(IObjectVisitor &visitor);

	virtual void doSerialize(std::string& out, CSerializeContext& context) const;

	virtual std::string doToString() const;

	virtual bool doIsString() const;

	virtual void inPlaceCopyTo(CObject &dest) const;
	virtual void inPlaceCopy(const CObjectString &src);

private:
	std::string _Value;

};


// A single string from the server viewpoint
// From client viewpoint, derived class 'observes' a target object and tells when it has been deleted
// or created
class CObjectRefId : public CObjectString
{
public:
	explicit CObjectRefId(const std::string & value);
	~CObjectRefId();
	virtual const char *getTypeAsString() const;
	virtual CObject* clone() const;
	virtual bool equal(const CObject* other) const;
protected:
	virtual void visitInternal(IObjectVisitor &visitor);
	virtual bool doIsRefId() const;
	virtual void doSerialize(std::string& out, CSerializeContext& context) const;
};


class CObjectNumber : public CObject
{

public:
	explicit CObjectNumber(double value);

	virtual const char *getTypeAsString() const;

	virtual bool set(const std::string& key, double value);
	virtual bool set(const std::string& key, const std::string&value);

	virtual bool setObject(const std::string& key, CObject* value);

	virtual CObject* clone() const;

	double getValue() const { return _Value; }

	virtual void dump(const std::string prefix = "", uint depth = 0) const;

	virtual bool equal(const CObject* other) const;

protected:
	virtual void doSerialize(std::string& out, CSerializeContext& context) const;

	virtual bool doIsNumber() const;

	virtual double doToNumber() const;

	virtual std::string doToString() const;

	virtual void inPlaceCopyTo(CObject &dest) const;
	virtual void inPlaceCopy(const CObjectNumber &src);

	virtual void visitInternal(IObjectVisitor &visitor);

private:
	double _Value;

};

class CObjectTable: public CObject
{

public:
	typedef NLMISC::CRefPtr<CObjectTable> TRefPtr;
	typedef NLMISC::CRefPtr<const CObjectTable> TRefPtrConst;

	explicit CObjectTable();

	virtual ~CObjectTable();

	virtual const char *getTypeAsString() const;

	virtual bool insert(const std::string & key, CObject * value, sint32 pos);

	virtual CObject* clone() const;

	virtual void doSerialize(std::string& out, CSerializeContext& context) const;

	virtual CObject* getAttr(const std::string & name) const;


	virtual std::string getKey(uint32 pos) const;

	virtual CObject* getValue(uint32 pos) const;

	virtual sint32 findIndex(const CObject* child) const;

	// find index from a key, or -1 if not found
	virtual sint32 findIndex(const std::string &key) const;

	virtual uint32 getSize() const;

	virtual bool set(const std::string& key, const std::string & value);

	virtual bool set(const std::string& key, double value);

	virtual bool setObject(const std::string& key, CObject* value);

	virtual CObject* take(sint32 pos);

	virtual bool canTake(sint32 pos) const;

	void clear();

	void sort();

	virtual void dump(const std::string prefix = "", uint depth = 0) const;

	virtual bool equal(const CObject* other) const;

	virtual void  setGhost(bool ghost);

	virtual void checkIntegrity() const;

protected:
	virtual void visitInternal(IObjectVisitor &visitor);

	virtual bool doIsTable() const;

	virtual CObjectTable* doToTable() const;

	virtual void inPlaceCopyTo(CObject &dest) const;
	virtual void inPlaceCopy(const CObjectTable &src);


	virtual void previsit(std::vector<CObject::TRefPtr> &sons);

	/** Compute absolute position, return true if index is valid
	  * A negative index indicate an offset from the end of the table (-1 for the last element)
	  */
protected:
	typedef std::vector< std::pair<std::string, CObject*> >  TContainer;

protected:
	TContainer _Value;

};


class CTypedObject : public CObjectTable
{
public:

	CTypedObject(const std::string & type);
	virtual bool isOk() const;
};

class CNameGiver
{
public:
	CNameGiver();
	sint32 getNewId(const std::string& type="") ;
	std::string getNewName(const std::string& type="", sint32 id = -1);
	void setMaxId(const std::string& eid,sint32 id);
	sint32 getMaxId(const std::string& eid);
	void clear();
private:
	std::map< std::string, sint32> _Value;

};

class CObjectGenerator;

// NB nico : added 'virtual' because client derives its own factory
class CObjectFactory
{
public:
	CObjectFactory(const std::string & prefix);
	void registerGenerator(CObject* classObject);
	virtual ~CObjectFactory();
	virtual CObject* newBasic(const std::string & type);
	CObject* newAvanced(const std::string & type);
	CObject* newComponent(const std::string & type);
	CObjectGenerator* getGenerator(const std::string & type);
	std::string getNewName(const std::string& type="") const;
	void setMaxId(const std::string&  eid,sint32 id);
	void setPrefix(const std::string & prefix);
	sint32 getMaxId(const std::string& eid) const;
	void clear();
private:
	CNameGiver* _NameGiver;
	std::string _Prefix;
	std::map<std::string, CObjectGenerator*> _Map;
};


class CObjectGenerator
{
public:
	CObject* instanciate(CObjectFactory* factory) const;
	CObjectGenerator(CObject* objectClass, CObjectFactory* factory):
		_ObjectClass(objectClass){ createDefaultValues(factory);}
	~CObjectGenerator();
	CObject* getDefaultValue(const std::string & propName) const;
	std::string getBaseClass() const;
protected:
	void createDefaultValues(CObjectFactory* factory);
private:
	typedef std::map<std::string, CObject* > TDefaultValues;
private:
	CObject* _ObjectClass;
	TDefaultValues _DefaultValues;
};

class CClass : public CObjectTable
{
public:
	CClass(const std::string & classType);
	void addAttribute(const std::string & name, const std::string & type);
	void addAttribute(const std::string & name, const std::string & type, const std::string & defaultValue);

private:
	std::string _ClassType;
};

class CClassAttribute: public CObjectTable
{
	public:

	CClassAttribute(const std::string & propName, const std::string& propType)
	{
		add("Name", propName);
		add("Type", propType);
	}
	CClassAttribute(const std::string & propName, const std::string& propType, const std::string & defaultValue)
	{
		add("Name", propName);
		add("Type", propType);
		add("DefaultValue", defaultValue);
	}
	//virtual CObject* instanciate() const = 0;

	virtual bool verify(CObject* /* prop */) const { return true;}
};

// Don't take ownership
class CObjectSerializer
{
public:
	// for client : factory to use when reading objects
	CObjectFactory *Factory;

	// to force static serializer memory cleanup
	static void releaseInstance();
protected:
	CObjectSerializer(CObjectFactory *factory, CObject* data = 0);

public:
	void serial(NLMISC::IStream& stream);
	CObject* getData() const;
	// make a copy of data (the caller must handle data)
	void setData(CObject* data);
	// :XXX: don't delete _Data
	~CObjectSerializer();
	static void serialStringInstanceId( NLMISC::IStream& stream, std::string& data);

	bool isCompresed() const { return _Compressed;}

	void compress();

	void uncompress() const;

	void swap(CObjectSerializer& other);
	void setVersion(uint32 version) { _Version = version; }
	uint32 getVersion() const { return _Version; }


private:
	CObjectSerializer(const CObjectSerializer& lh);
	CObjectSerializer& operator=(const CObjectSerializer& rh);
	void uncompressImpl();


private:
	CObject*	_Data;
	bool		_Compressed;
	bool		_MustUncompress;
	uint8*		_CompressedBuffer;
	uint32		_CompressedLen;
	uint32		_UncompressedLen;
	uint32		_Version;
public:
	uint32		Level;
	bool		Log;
};

class CObjectSerializerClient : public CObjectSerializer
{
	static CObjectFactory	*_ClientObjecFactory;
public:
	// constructor for client side serializer, we use the client side factory
	CObjectSerializerClient(CObject *data=0)
		:	CObjectSerializer(_ClientObjecFactory, data)
	{
		nlassert(_ClientObjecFactory != NULL);
	}

	static void setClientObjectFactory(CObjectFactory *factory)
	{
		_ClientObjecFactory = factory;
	}
};

class CObjectSerializerServer: public CObjectSerializer
{
public:
	// constructor for server side serializer, we don't use object factory
	CObjectSerializerServer(CObject *data=0)
		:	CObjectSerializer(NULL, data)
	{}
};


} // namespace R2

#endif //R2_OBJECT_H
