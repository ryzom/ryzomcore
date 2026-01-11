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


#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "nel/misc/factory.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/common.h"
#include "module_common.h"

namespace NLNET
{

	class CGatewayRoute;

	/** This is the interface for the module manager.
	 *	Module manager is in charge of module factoring and
	 *	referencing,
	 *	It is also an entry point when you need to retrieve
	 *	a module socket or a module gateway.
	 */
	class IModuleManager : public NLMISC::CRefCount
	{
	public:

		/// Check that the module manager is initialized
		static bool			isInitialized();
		/// The module manager is a singleton
		static IModuleManager &getInstance();

		/// Must be called before releasing the instance, will call IModule::onApplicationExit on all module instance
		virtual void applicationExit() =0;

		/// Release the singleton instance
		static void releaseInstance();

		virtual ~IModuleManager() {}

		/** set the unique name root used for fully qualified name generation
		 *	(by default, it is the host name and process id)
		 */
		virtual void setUniqueNameRoot(const std::string &uniqueNameRoot) =0;

		/** get the unique name root used for fully qualified name generation
		 *	(by default, it is the host name and process id)
		 */
		virtual const std::string &getUniqueNameRoot() =0;

		/** Load a module library.
		 *	Module library are dll or so files that contains
		 *	one or more module implementation.
		 *	Loading the library add the modules to the module
		 *	factory.
		 *	If the library can be loaded, the method return true, false otherwise.
		 *
		 *	The library name is the base name that will be 'decorated'
		 *	with the nel naming standard according to compilation mode
		 *	and platform specific file extension (.dll, .dylib or .so).
		 *
		 *	A module library can only be loaded once. If the library
		 *	is already loaded, the call is ingored.
		 */
		virtual bool loadModuleLibrary(const std::string &libraryName) =0;

		/** Unload a module library.
		 *	Any module that come from the unloaded library
		 *	are immediately deleted.
		 *	If the specified library is not loaded, the
		 *	call is ignored.
		 */
		virtual bool unloadModuleLibrary(const std::string &libraryName) =0;

		/** Unregister a module factory
		*/
		virtual void unregisterModuleFactory(class IModuleFactory *moduleFactory) =0;


		/** Fill the vector with the list of available module.
		 *	Note that the vector is not cleared before being filled.
		 */
		virtual void getAvailableModuleClassList(std::vector<std::string> &moduleClassList) =0;

		/** Create a new module instance.
		 *	The method create a module of the specified class with the
		 *	specified local name.
		 *	The class MUST be available in the factory and the
		 *	name MUST be unique OR empty.
		 *	If the name is empty, the method generate a name using
		 *	the module class and a number.
		 *  If the module class could not be found, NULL is returned.
		 *	If the module fail to initialize properly, then it
		 *	deleted and NULL is returned.
		 */
		virtual IModule *createModule(const std::string &className, const std::string &localName, const std::string &paramString) =0;

		/** Delete a module instance.
		 *	This is the only mean to delete a module instance.
		 */
		virtual void deleteModule(IModule *module) =0;

		/** Lookup in the created module for a module having the
		 *	specified local name.
		 *	Return NULL if not found.
		 */
		virtual IModule *getLocalModule(const std::string &moduleName) =0;

		/** Call this method to update all module instances.
		 *	If the application is a NeL service, then this method
		 *	is automaticly called in the service main loop.
		 *	If the application is a regular one, then you
		 *	have to call manually this method regularly
		 *	to update the modules.
		 */
		virtual void updateModules() =0;

		/** Lookup in the created socket for a socket having the
		 *	specified local name.
		 */
		virtual IModuleSocket *getModuleSocket(const std::string &socketName) =0;
		/** Register a socket in the manager.
		 *	TODO : make this method only available to CModuleSocket to prevent dramatic ERROR
		 */
		virtual void registerModuleSocket(IModuleSocket *moduleSocket) =0;
		/** UrRegister a socket in the manager.
		 *	TODO : make this method only available to CModuleSocket to prevent dramatic ERROR
		 */
		virtual void unregisterModuleSocket(IModuleSocket *moduleSocket) =0;

		/** Lookup in the created gateway for a gateway having the
		 *	specified local name.
		 */
		virtual IModuleGateway *getModuleGateway(const std::string &gatewayName) =0;
		/** Register a gateway in the manager.
		 */
		virtual void registerModuleGateway(IModuleGateway *moduleGateway) =0;
		/** UrRegister a socket in the manager.
		 */
		virtual void unregisterModuleGateway(IModuleGateway *moduleGateway) =0;

		/** Get a module proxy with the module proxy ID */
		virtual TModuleProxyPtr getModuleProxy(TModuleId moduleProxyId) =0;

		/** Called by the gateway module to create new module proxy
		 *	Module proxy are an easy way for implementor to send
		 *	message to a module, without having to retrieve
		 *	the gateway that discovered the destination module.
		 *	When module are created and plugged on a gateway,
		 *	the gateway automaticly create a proxy
		 *	for each local module and each foreing module.
		 */
		virtual IModuleProxy *createModuleProxy(	IModuleGateway *gateway,
													CGatewayRoute *route,
													uint32 distance,
													IModule *localModule,
													const std::string &moduleClassName,
													const std::string &moduleFullyQualifiedName,
													const std::string &moduleManifest,
//													const TModuleGatewayProxyPtr &foreignGateway,
													TModuleId foreignModuleId) =0;

		virtual void releaseModuleProxy(TModuleId moduleProxyId) =0;

		virtual uint32 getNbModule() =0;
		virtual uint32 getNbModuleProxy() =0;

	};

	typedef NLMISC::CFactoryIndirect<IModuleFactory, std::string>	TLocalModuleFactoryRegistry;

	/** Class for pure NeL module library */
	class CNelModuleLibrary : public NLMISC::INelLibrary
	{
	public:
		/** Return the local module factory.*/
		virtual TLocalModuleFactoryRegistry &getLocalModuleFactoryRegistry()
		{
			return TLocalModuleFactoryRegistry::instance();
		}

		virtual void onLibraryLoaded(bool /* firstTime */) {}
		virtual void onLibraryUnloaded(bool /* lastTime */) {}
	};

} // namespace NLNET

#endif // MODULE_MANAGER_H

