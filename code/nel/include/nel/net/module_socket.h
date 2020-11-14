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


#ifndef NL_FILE_MODULE_SOCKET_H
#define NL_FILE_MODULE_SOCKET_H

#include "nel/net/message.h"
#include "module_common.h"

namespace NLNET
{
	class IModuleSocket
	{
	public:
		virtual ~IModuleSocket() {}
		/** Register the socket in the module manager socket registry
		 */
		virtual void registerSocket() =0;
		/** Unregister the socket in the module manager socket registry
		 */
		virtual void unregisterSocket() =0;

		/** Ask derived class to obtain the socket name.
		 */
		virtual const std::string &getSocketName() =0;

		/** A plugged module send a message to another module.
		 *	If the destination module is not accessible through this socket,
		 *	an exception is thrown.
		 */
		virtual void sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message) =0;
		/** A plugged module send a message to all the module reachable
		 *	with this socket.
		 */
		virtual void broadcastModuleMessage(IModule *senderModule, const NLNET::CMessage &message) =0;

		/** Fill the resultList with the list of module that are
		 *	reachable with this socket.
		 *	Note that the result vector is not cleared before filling.
		 */
		virtual void getModuleList(std::vector<IModuleProxy*> &resultList) =0;

		//@name Callback for socket implementation
		//@{
		/// Called just after a module is plugged in the socket.
		virtual void onModulePlugged(IModule *pluggedModule) =0;
		/// Called just before a module is unplugged from the socket.
		virtual void onModuleUnplugged(IModule *unpluggedModule) =0;
		//@}
	};

//	const TModuleSocketPtr	NullModuleSocket;


	/** A base class for socket.
	 *	It provide plugged module management to
	 *	implementors.
	 */
	class CModuleSocket : public IModuleSocket
	{
	protected:
		typedef NLMISC::CTwinMap<TModuleId, TModulePtr>	TPluggedModules;
		/// The list of plugged modules
		TPluggedModules			_PluggedModules;

		bool					_SocketRegistered;

		friend class CModuleBase;

		CModuleSocket();
		~CModuleSocket();

		virtual void registerSocket();
		virtual void unregisterSocket();


		virtual void _onModulePlugged(const TModulePtr &pluggedModule);
		virtual void _onModuleUnplugged(const TModulePtr &pluggedModule);

		virtual void _sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message) =0;

		virtual void _broadcastModuleMessage(IModule *senderModule, const NLNET::CMessage &message) =0;

		virtual void sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message);
		/** A plugged module send a message to all the module reachable
		 *	with this socket.
		 */
		virtual void broadcastModuleMessage(IModule *senderModule, const NLNET::CMessage &message);

	};


} // namespace NLNET

#endif // NL_FILE_MODULE_SOCKET_H
