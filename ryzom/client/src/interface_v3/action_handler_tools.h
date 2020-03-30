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

#ifndef CL_ACTION_HANDLER_TOOLS_H
#define CL_ACTION_HANDLER_TOOLS_H

#include <string>


// sendMsgToServer Helper
void sendMsgToServer(const std::string &sMsg);
void sendMsgToServer(const std::string &sMsg, uint8 u8);
void sendMsgToServer(const std::string &sMsg, uint8 u8n1, uint8 u8n2);
void sendMsgToServer(const std::string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3);
void sendMsgToServer(const std::string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3, uint8 u8n4);
void sendMsgToServer(const std::string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3, uint8 u8n4, uint8 u8n5);
void sendMsgToServer(const std::string &sMsg, uint8 u8n1, uint32 u32n2);
void sendMsgToServer(const std::string &sMsg, uint32 u32n1);
void sendMsgToServer(const std::string &sMsg, uint32 u32n1, uint8 u8n2);
void sendMsgToServer(const std::string &sMsg, uint32 u32n1, uint8 u8n2, uint8 u8n3);
void sendMsgToServer(const std::string &sMsg, uint32 u32n1, uint32 u32n2);

#endif // CL_ACTION_HANDLER_TOOLS_H
