// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef SEVEN_ZIP_H
#define SEVEN_ZIP_H

#include <string>
#include <nel/misc/sha1.h>

namespace NLMISC {

// utility func to decompress a monofile 7zip archive
bool unpack7Zip(const std::string &sevenZipFileName, const std::string &destFileName);

// utility func to decompress a single LZMA packed file
bool unpackLZMA(const std::string &lzmaFileName, const std::string &destFileName);

// utility func to decompress a single LZMA packed file
bool unpackLZMA(const std::string &lzmaFileName, const std::string &destFileName, CHashKey &sha1);

// utility func to compress a single file to LZMA packed file
bool packLZMA(const std::string &srcFileName, const std::string &lzmaFileName);

}

#endif
