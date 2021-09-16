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

#ifndef R2_SCENARIO_H
#define R2_SCENARIO_H

#include "nel/misc/types_nl.h"

#include "nel/misc/md5.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/smart_ptr.h"
#include <list>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "game_share/r2_types.h"

#include "r2_share_itf.h"

//#include "game_share/far_position.h"

namespace R2
{


class CObject;
class CInstanceMap;
class CInstanceMap;
class CIdMaker;






/*! Access Time optimised container Object Tree.
* Transforms an object tree into a map for time access optimization to the different nodes.
* The key used to find an element is set in the ctor
*/
class CInstanceMap
{
public:
	/// Set the id Name (b.e. if "InstanceId" is used then look at the "InstanceId" attribute to Identify a node)
	CInstanceMap(const std::string& idName):_IdName(idName){}

	/// Sets the initial tree (clear the container and add)
	void set(CObject* root);

	/// Add a new tree (register all its node)
	void add(CObject* root);

	/// Remove a tree (unregister all its node)
	void remove(CObject* root);

	/// Find a element by its id
	CObject* find (const std::string& instanceId);


	//:XXX: Global functionality and specific must be in two different class

	/// Returns the maximum  of id used by a character .
	sint32 getMaxId(const std::string& eid);

	void setMaxId(const std::string &eid, sint32 maxId);
private:
	/// Id to Object
	std::map<std::string, CObject*> _Map;

	//:XXX: Must not be here
	/// UserId -> maximum id used
	std::map<std::string, sint32> _MapEids;

	// The Name of the attribute used to identify a node.
	const std::string _IdName;
};



/*! Hold a scenario
* Contain a scenario
* - Edition data / Animation data
* - Palette used to create elements
* - InstanceMap to give quick access to tree Node
* - List of Current characters connect to this scenario
*/

//:XXX: _CurrentUsers will still be useful after session manager?
class CScenario : public NLMISC::CRefCount
{




public:
	/*! ctor
	*	/param property the CObject node of the edition data.
	*/
	explicit CScenario(CObject* property, TScenarioSessionType sessionType = st_edit);

	///dtor
	virtual ~CScenario();



	/*! Reset the Edition data.
	* /param /highLevle the edition data
	*/
	void setHighLevel(CObject* highLevel);

	/*! Reset the Animation data.
	* /param /highLevle the edition data
	*/
	void setRtData(CObject* rtScenario);



	/*!
	Add, set, erase, move node of a scenario
	/see CObject definition
	*/
	///@{

	bool setNode( const std::string& instanceId, const std::string& attrName, CObject* value);

	bool insertNode(const std::string&  instanceId, const std::string & attrName, sint32 position,
		const std::string& key, CObject* value);

	bool eraseNode(const std::string&  instanceId, const std::string & attrName, sint32 position);

	bool moveNode( const std::string& instanceId1, const std::string& attrName1, int position1,
		const std::string& instanceId2, const std::string& attrName2, int position2);

	///@}

	/// Find a node from the scenario tree by its InstanceId.
	CObject *find(const std::string& instanceId, const std::string& attrName = "", sint32 position = -1, const std::string &key ="");

	/// Get the Edition Data tree
	CObject* getHighLevel() const;

	/// Get the Animation Data tree
	CObject* getRtData() const;

	/*! return the maxId used by a specific user
	 *  /param eid the id of the pioneer
	 *  /return The max InstanceId used by this user
	 */
	sint32 getMaxId(const std::string& eid);

	void setMaxId(const std::string& eid, sint32 range);



	uint32 getMode() const { return _Mode; }

	void setMode(uint32 mode) { _Mode = mode; }

	bool isEditing() const;

	bool isRunning() const;

	bool isWaiting() const;

	void setSessionType(const TScenarioSessionType& sessionType ) {  _SessionType = sessionType; }

	void registerUserComponent(const NLMISC::CHashKeyMD5& md5) { _CUserComponent.insert(md5); }

	void unregisterUserComponent(const NLMISC::CHashKeyMD5& md5) { _CUserComponent.erase(md5); }

	uint32 getInitialActIndex() const { return _InitialActIndex;}

	void setInitialActIndex(uint32 initialActIndex) { _InitialActIndex = initialActIndex; }

	void serial( NLMISC::IStream &f);

	void setClean(bool clean){ _Clean = clean; }
	bool getClean() const { return _Clean; }

private:
	//time stamp?

	/// Palette used
	// :XXX: useful?
	CObject* _Palette;

/// list of chars
	/// :XXX: useful?
	uint32			_Mode;

	// Runtime Data
	CObject* _BasicBricks;


	CObject* _HighLevel;
	CInstanceMap* _InstanceMap;
	TScenarioSessionType _SessionType;
	uint32 _InitialActIndex;
	bool _Clean;


	struct CHashKeyMD5Less : public std::binary_function<NLMISC::CHashKeyMD5, NLMISC::CHashKeyMD5, bool>
	{
		bool operator()(const NLMISC::CHashKeyMD5& x, const NLMISC::CHashKeyMD5& y) const { return x.operator<(y); }
	};


	std::set<NLMISC::CHashKeyMD5, CHashKeyMD5Less> _CUserComponent;


};



class CUserComponent
{
public: //public type
	enum TComponentType {Compressed, Dev} ;

public://public functions
	CUserComponent();

	CUserComponent(const std::string &filename,
		uint8* uncompressedData,  uint32 uncompressedDataLength,
		uint8* compressedData = 0,  uint32 compressedDataLength = 0
		);

	~CUserComponent();

	void computeMd5();

	void  serial(NLMISC::IStream &f);

	uint8* getUncompressedData() const;

	uint32 getUncompressedDataLength() const;

	void setType(TComponentType type) { ComponentType = type; }

	TComponentType GetComponentType() const { return ComponentType; }

	std::string getFilename() const { return Filename; }


public: //public value
	NLMISC::CHashKeyMD5 Md5;
	NLMISC::CHashKeyMD5 Md5Id;
	uint32 TimeStamp;
	TComponentType ComponentType;
	std::string Filename;
	std::string Name;
	std::string Description;
	uint8* UncompressedData; //uncompressed
	uint32 UncompressedDataLength;
	uint8* CompressedData;
	uint32 CompressedDataLength;
};


class CEmoteBehavior
{
public:
	CEmoteBehavior();
	std::string get(const std::string& emoteId) const;

private:
	void load() const;

private:
	mutable std::map<std::string, std::string> _EmotesMap;

};


class CScenarioValidatorLoadSuccededCallback;

class CScenarioValidator
{
public:
	typedef std::vector< std::pair<std::string, std::string> >  TValues;

public:
	static std::string AutoSaveSignature;

public:
	TValues _Values;
	CScenarioValidator(CScenarioValidatorLoadSuccededCallback* loadCb = 0 );
	~CScenarioValidator();
	bool setScenarioToSave(const std::string& filename, CObject* scenario, const CScenarioValidator::TValues& value, std::string& md5);
	bool setScenarioToLoad(const std::string& filename, CScenarioValidator::TValues& values, std::string& md5, std::string& signature, bool checkMD5);

	bool applySave(const std::string& signature);
	void applyLoad(std::string& filename, std::string& body, CScenarioValidator::TValues& values);
	std::string getBodyMd5() const { return _BodyMd5; }

private:
	std::string _HeaderBody;
	std::string _ScenarioBody;
	std::string _HeaderMd5;
	std::string _Filename;
	std::string _BodyMd5;
	CScenarioValidatorLoadSuccededCallback* _LoadCb;


};

// Verify that a scenario is not alterated by user

class CScenarioValidatorLoadSuccededCallback
{
public:
	virtual ~CScenarioValidatorLoadSuccededCallback(){}
	virtual void doOperation(const std::string& filename, const std::string& body, const CScenarioValidator::TValues& values) = 0;


};

class CUserComponentValidatorLoadSuccededCallback;

class CUserComponentValidator
{
public:
	typedef std::vector< std::pair<std::string, std::string> >  TValues;

public:
	static std::string AutoSaveSignature;

public:
	TValues _Values;
	CUserComponentValidator(CUserComponentValidatorLoadSuccededCallback* loadCb = 0 );
	~CUserComponentValidator();
	bool setUserComponentToSave(const std::string& filename, const CUserComponentValidator::TValues& value, std::string& md5, const std::string &body);
	bool setUserComponentToLoad(const std::string& filename, CUserComponentValidator::TValues& values, std::string& md5, std::string& signature, bool checkMD5);

	bool applySave(const std::string& signature);
	void applyLoad(std::string& filename, std::string& body, CScenarioValidator::TValues& values);
	std::string getBodyMd5() const { return _BodyMd5; }

	std::string getFilename() const{ return _Filename;}

private:
	std::string _HeaderBody;
	std::string _UserComponentBody;
	std::string _HeaderMd5;
	std::string _Filename;
	std::string _BodyMd5;
	CUserComponentValidatorLoadSuccededCallback* _LoadCb;


};

class CUserComponentValidatorLoadSuccededCallback
{
public:
	virtual ~CUserComponentValidatorLoadSuccededCallback(){}
	virtual void doOperation(const std::string& filename, const std::string& body, const CUserComponentValidator::TValues& values) = 0;


};

} // namespace R2
#endif //R2_SCENARIO_H
