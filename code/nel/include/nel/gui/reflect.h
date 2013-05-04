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

#ifndef CL_REFLECT_H
#define CL_REFLECT_H

#include "nel/misc/rgba.h"
#include "nel/gui/lua_object.h"
#include <string>

namespace NLGUI
{
	class CReflectable;
	class CLuaState;
	struct CClassInfo;

	/** A property of a reflectable object
	  * NB: multiple inheritance not supported
	  */
	class CReflectedProperty
	{
	public:
		enum TType { Boolean = 0,
					 SInt32,
					 UInt32,
					 Float,
					 String,
					 UCString,
					 RGBA,
					 LuaMethod
					}; // other types will be added when needed
		// define some pointer-to-member types
		typedef bool			(CReflectable::* TGetBool) () const;
		typedef sint32			(CReflectable::* TGetSInt32) () const;
		typedef uint32			(CReflectable::* TGetUInt32) () const;
		typedef float			(CReflectable::* TGetFloat) () const;
		typedef std::string		(CReflectable::* TGetString) () const;
		typedef ucstring		(CReflectable::* TGetUCString) () const;
		typedef NLMISC::CRGBA	(CReflectable::* TGetRGBA) () const;
		//
		typedef void   (CReflectable::* TSetBool) (bool);
		typedef void   (CReflectable::* TSetSInt32) (sint32);
		typedef void   (CReflectable::* TSetUInt32) (uint32);
		typedef void   (CReflectable::* TSetFloat) (float);
		typedef void   (CReflectable::* TSetString) (const std::string &);
		typedef void   (CReflectable::* TSetUCString) (const ucstring &);
		typedef void   (CReflectable::* TSetRGBA) (NLMISC::CRGBA col);
		//
		typedef int   (CReflectable:: *TLuaMethod) (CLuaState &luaState);

	public:
		TType Type;
		// In each union we have method pointers to retrieve / set the data of the desired type (as told in 'Type')
		union
		{
			TGetBool		GetBool;
			TGetSInt32		GetSInt32;
			TGetUInt32		GetUInt32;
			TGetFloat		GetFloat;
			TGetString		GetString;
			TGetUCString	GetUCString;
			TGetRGBA		GetRGBA;
			TLuaMethod		GetLuaMethod; // lua method can only be obtained, not written ...
		} GetMethod;
		union
		{
			TSetBool		SetBool;
			TSetSInt32		SetSInt32;
			TSetUInt32		SetUInt32;
			TSetFloat		SetFloat;
			TSetString		SetString;
			TSetUCString	SetUCString;
			TSetRGBA		SetRGBA;
		} SetMethod;
		// name of the property
		std::string Name;
		mutable CLuaObject LuaMethodRef; // cache pointer to function call if type == LuaMethod
		const CClassInfo *ParentClass; // filled when 'registerClass' is called
	};

	// a vector of reflected properties
	typedef std::vector<CReflectedProperty> TReflectedProperties;


	struct CClassInfo;

	/** Base class for a reflectable object
	  * NB: multiple inheritance not supported
	  */
	class CReflectable
	{
	public:
		virtual ~CReflectable() {}
		virtual const char *getReflectedClassName() const { return "CReflectable"; }
		virtual const char *getRflectedParentClassName() const { return ""; }

		/** When registering classes, the reflect system will call this function on each class
		  * to know which properties they exports.
		  * To defines which properties are exported use the REFLECT_EXPORT_** macros.
		  * By doing so, a new 'getReflectedProperties' function will be defined
		  */
		static void getReflectedProperties(TReflectedProperties &/* props */)
		{
		}
		// get class infos for this reflectable object
		const CClassInfo *getClassInfo();

		/** get a property from this object by name
		  * TODO nico : optimized version for lua string (found in CLuaIHM) would maybe fit better here ...
		  */
		const CReflectedProperty *getReflectedProperty(const std::string &propertyName, bool dspWarning= true) const;
	};

	struct CLuaIndexedProperty
	{
		const CReflectedProperty *Prop;
		CLuaString		   Id; // must keep id here, so that we are sure the string is not gc in lua and its pointer remains valid
	};


	struct CClassInfo
	{
		TReflectedProperties Properties;  // the properties exported by this class
		const CClassInfo     *ParentClass; // pointer to infos of the parent class, or NULL if it is a root class
		std::string			 ClassName;
		/** For lua speedup (used by CLuaIHM) : because lua string are unique, we can use them to access property directly.
		  */
		typedef CHashMap<const char *, CLuaIndexedProperty, CLuaHashMapTraits> TLuaStrToPropMap;
		mutable TLuaStrToPropMap LuaStrToProp;
	};

	/** Simple reflection system.
	  * Used by the GUI and some other objects.
	  * It is used to export some properties so that we can easily manipulate them in the GUI scripts (either lua or with CInterfaceExpr).
	  * NB: multiple inheritance not supported
	  *
	  * Example of use : a class exporting a boolean
	  *
	  * class CTestClass : public CReflectable
	  * {
	  *    public:
	  *       void setValue(bool value) { _Value = value; }
	  *       bool getValue() const { return _Value; }
	  *       \\ export the bool value
	  *       REFLECT_EXPORT_START(CTestClass, CReflectable)
	  *			REFLECT_BOOL("myValue", setValue, getValue)
	  *       REFLECT_EXPORT_END
	  *    private:
	  *       bool _Value;
	  * };
	  *
	  * The class must then be registered with :
	  *
	  * REGISTER_REFLECTABLE_CLASS(CTestClass, CReflectable)
	  *
	  * NB: It should be registered after its parents
	  *
	  *
	  * \author Nicolas Vizerie
	  * \author Nevrax France
	  * \date 2002
	  */
	class CReflectSystem
	{
	public:

		typedef std::map<std::string, CClassInfo> TClassMap;

	public:
		// release memory
		static void release();

		/** register a class and its properties
		  * NB : class should be registered after their parent have been, or an assertion will be raised
		  */
		static void registerClass(const std::string &className, const std::string &parentName, const TReflectedProperties properties);

		// retrieve a property of a reflectable class, or NULL if unknown
		static const CReflectedProperty *getProperty(const std::string &className, const std::string &propertyName, bool dspWarning= true);

		// get the list of class for debug or read purpose (NULL if no register has been called)
		static const TClassMap	*getClassMap() {return _ClassMap;}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		static TClassMap *_ClassMap; // each class and its infos
	};


	/** Helper macros to export properties of a reflectable class
	  */

	/** Start a declaration of a reflectable class exports
	  * Should be placed inside the class
	  */
	#define REFLECT_EXPORT_START(className, parentName)                                        \
		virtual const char *getReflectedClassName() const { return #className; }			   \
		virtual const char *getReflectedParentClassName() const { return #parentName; }		   \
		static void getReflectedProperties(TReflectedProperties &props)                        \
		{                                                                                      \
			typedef className A;                                                               \
			typedef bool			(className::* TGetBoola) () const;                         \
			typedef sint32			(className::* TGetSInt32a) () const;                       \
			typedef uint32			(className::* TGetUInt32a) () const;                       \
			typedef float			(className::* TGetFloata) () const;                        \
			typedef std::string		(className::* TGetStringa) () const;                       \
			typedef ucstring		(className::* TGetUCStringa) () const;                     \
			typedef NLMISC::CRGBA	(className::* TGetRGBAa) () const;                         \
			typedef void   (className::* TSetBoola) (bool);                                    \
			typedef void   (className::* TSetSInt32a) (sint32);                                \
			typedef void   (className::* TSetUInt32a) (uint32);                                \
			typedef void   (className::* TSetFloata) (float);                                  \
			typedef void   (className::* TSetStringa) (const std::string &);                   \
			typedef void   (className::* TSetUCStringa) (const ucstring &);                    \
			typedef void   (className::* TSetRGBAa) (NLMISC::CRGBA col);                       \
			typedef int   (className:: *TLuaMethoda) (CLuaState &luaState);                    \
			nlunreferenced(props);


	// export a boolean value, by giving the name of the get and the set method
	#define REFLECT_BOOL(exportName, getMethod, setMethod)                  \
	{                                                                       \
		CReflectedProperty prop;                                            \
		prop.Name = exportName;                                             \
		prop.Type = CReflectedProperty::Boolean;                            \
		prop.GetMethod.GetBool = (CReflectedProperty::TGetBool) (TGetBoola) &A::getMethod; \
		prop.SetMethod.SetBool = (CReflectedProperty::TSetBool) (TSetBoola) &A::setMethod; \
		props.push_back(prop);                                              \
	}

	// export a sint32 value, by giving the name of the get and the set method
	#define REFLECT_SINT32(exportName, getMethod, setMethod)                           \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::SInt32;                                        \
		prop.GetMethod.GetSInt32 = (CReflectedProperty::TGetSInt32) (TGetSInt32a) &A::getMethod;        \
		prop.SetMethod.SetSInt32 = (CReflectedProperty::TSetSInt32) (TSetSInt32a) &A::setMethod;        \
		props.push_back(prop);                                                         \
	}

	// export a sint32 value, by giving the name of the get and the set method
	#define REFLECT_UINT32(exportName, getMethod, setMethod)                           \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::UInt32;                                        \
		prop.GetMethod.GetUInt32 = (CReflectedProperty::TGetUInt32) (TGetUInt32a) &A::getMethod;        \
		prop.SetMethod.SetUInt32 = (CReflectedProperty::TSetUInt32) (TSetUInt32a) &A::setMethod;        \
		props.push_back(prop);                                                         \
	}

	// export a float value, by giving the name of the get and the set method
	#define REFLECT_FLOAT(exportName, getMethod, setMethod)                            \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::Float;                                         \
		prop.GetMethod.GetFloat = (CReflectedProperty::TGetFloat) (TGetFloata) &A::getMethod;          \
		prop.SetMethod.SetFloat = (CReflectedProperty::TSetFloat) (TSetFloata) &A::setMethod;          \
		props.push_back(prop);                                                         \
	}

	// export a string value, by giving the name of the get and the set method
	#define REFLECT_STRING(exportName, getMethod, setMethod)                           \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::String;                                        \
		prop.GetMethod.GetString = (CReflectedProperty::TGetString) (TGetStringa) &A::getMethod;        \
		prop.SetMethod.SetString = (CReflectedProperty::TSetString) (TSetStringa) &A::setMethod;        \
		props.push_back(prop);                                                         \
	}

	// export a unicode string value, by giving the name of the get and the set method
	#define REFLECT_UCSTRING(exportName, getMethod, setMethod)                         \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::UCString;                                      \
		prop.GetMethod.GetUCString = (CReflectedProperty::TGetUCString) (TGetUCStringa) &A::getMethod;    \
		prop.SetMethod.SetUCString = (CReflectedProperty::TSetUCString) (TSetUCStringa) &A::setMethod;    \
		props.push_back(prop);                                                         \
	}


	// export a color value, by giving the name of the get and the set method
	#define REFLECT_RGBA(exportName, getMethod, setMethod)							   \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::RGBA;										   \
		prop.GetMethod.GetRGBA = (CReflectedProperty::TGetRGBA) (TGetRGBAa) &A::getMethod;		       \
		prop.SetMethod.SetRGBA = (CReflectedProperty::TSetRGBA) (TSetRGBAa) &A::setMethod;			   \
		props.push_back(prop);                                                         \
	}

	// export a lua method
	#define REFLECT_LUA_METHOD(exportName, method)					                   \
	{                                                                                  \
		CReflectedProperty prop;                                                       \
		prop.Name = exportName;                                                        \
		prop.Type = CReflectedProperty::LuaMethod;									   \
		prop.GetMethod.GetLuaMethod = (CReflectedProperty::TLuaMethod) (TLuaMethoda) &A::method;		   \
		props.push_back(prop);                                                         \
	}



	// ends an export declaration
	#define REFLECT_EXPORT_END  }


	// This macro registers a reflectable class to the manager
	#define REGISTER_REFLECTABLE_CLASS(className, parentName)          \
	{                                                                  \
		TReflectedProperties props;                                    \
		className::getReflectedProperties(props);                      \
		CReflectSystem::registerClass(#className, #parentName, props); \
	}



	/** Reflectable refcounted object
	  * NB nico : added this intermediate class so that the binding from lua to the reflection
	  * system that are found in CLuaIHM can be reused for other objects as well
	  * NOTE: The class is named 'CReflectableRefPtrTarget' and not 'CReflectableRefCount'
	  * because the refcount part is only used for ref pointing in the ui
	  */
	class CReflectableRefPtrTarget : public CReflectable, public NLMISC::CRefCount
	{
	public:
		virtual ~CReflectableRefPtrTarget();
	};


	class CReflectableLuaRef
	{
	public:
		CReflectableLuaRef(CReflectableRefPtrTarget *ptr = NULL) : Ptr(ptr), _ClassInfo(NULL) {}
		NLMISC::CRefPtr<CReflectableRefPtrTarget> Ptr;
		const CClassInfo						  &getClassInfo() const;
		// IMPORTANT : luaStringPtr should have been obtained from lua, see remark in CClassInfo
		const CReflectedProperty				  *getProp(const char *luaStringPtr) const;
	private:
		// cache to class definition of the pointee object (once a CReflectableLuaRef created in lua, it remains a *const* pointer)
		mutable const CClassInfo							  *_ClassInfo;
	};


}

#endif

