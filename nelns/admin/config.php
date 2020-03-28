<?php
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

	if (!isset($GLOBALS["NEL_TOOL_CONFIG_PHP"]))
	{
		$GLOBALS["dbhost"] = "localhost";
		$GLOBALS["dbname"] = "nel";
		$GLOBALS["dblogin"] = "";
		$GLOBALS["dbpassword"] = "";

		$GLOBALS["allowrootdebug"] = true;

		$GLOBALS["userlogpath"] = "/var/log/nelns";				// without final /

		$GLOBALS["rrdrootpath"] = "/var/log/nelns/rrds";				// without final /
		$GLOBALS["gifoutputpath"] = "graph";				// absolute path where to store gif files, without final /
		$GLOBALS["gifhttplocation"] = "graph";			// relative path to the http root, where to find gif files, without final /
		$GLOBALS["gifpersistence"] = 180;			// in number of seconds

		$GLOBALS["NEL_TOOL_CONFIG_PHP"] = true;

		$GLOBALS["ASHost"] = "localhost";
		$GLOBALS["ASPort"] = "49995";

		$GLOBALS["enablelock"] = true;

		$GLOBALS["allowDownload"] = true;
		$GLOBALS["allowUpload"] = true;
	}
?>
