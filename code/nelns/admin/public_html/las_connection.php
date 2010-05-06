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

	//include_once('../config.php');

	// Functions

	// This function connect to the LAS
	$LASPort = 49899;
	function connectToLAS($LASHost, &$fp, &$res)
	{
		global $LASPort;

		// connect to the login service
		$fp = fsockopen ($LASHost, $LASPort, $errno, $errstr, 30);
		if (!$fp)
		{
			$res = "Can't connect to the log analyser service '$LASHost:$LASPort' ($errno: $errstr) (error code 1)";
			return false;
		}
		else
		{
			$res = "";
			return true;
		}
	}

	function disconnectFromLAS(&$fp)
	{
		fclose($fp);
	}

	function	logQuery($LASHost, $query, &$result, &$queryId)
	{
		if (!connectToLAS($LASHost, $fp, $result))
		{
			$result = "Failed to connect to LAS $LASHost (ec 1)";
			return false;
		}
			
		// send the message that say that we want to add a user
		$msgout = new CMemStream;

		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet

		$messageType = 0;
		$msgout->serialuint8 ($messageType);

		$msgout->serialstring($query);

		if (!sendMessage ($fp, $msgout))
		{
			$result = "Failed to send query to LAS $LASHost (ec 2)";
			return false;
		}

		if (!waitMessage ($fp, $msgin))
		{
			$result = "Failed to wait for LAS $LASHost (ec 3)";
			return false;
		}

		$result = '';
		if (!$msgin->serialstring($result))
		{
			$result = "Failed to decode LAS message $LASHost (ec 4)";
			return false;
		}
			
		fclose ($fp);
		
		$pos = strpos($result, ':');
		if ($pos === FALSE)
		{
			$result = "Failed to decode LAS message $LASHost (ec 5)";
			return false;
		}

		$success = (substr($result, 0, $pos) == '1');

		$result = substr($result, $pos+1);

		if ($success)
		{
			$pos = strpos($result, ':');
			if ($pos == FALSE)
			{
				$result = "Failed to decode LAS message $LASHost (ec 6)";
				return false;
			}

			$queryId = substr($result, 0, $pos);
			$result = substr($result, $pos+1);
		}

		return $success;
	}

	function	getQueryResult($LASHost, $id, &$result, &$page, &$numpages)
	{
		if (!connectToLAS($LASHost, $fp, $result))
		{
			$result = "Failed to connect to LAS $LASHost (ec 7)";
			return false;
		}
			
		// send the message that say that we want to add a user
		$msgout = new CMemStream;

		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet

		$messageType = 1;
		$msgout->serialuint8 ($messageType);
		
		$str = $id.":".$page;
		$msgout->serialstring($str);

		if (!sendMessage ($fp, $msgout))
		{
			$result = "Failed to send query to LAS $LASHost (ec 8)";
			return false;
		}

		if (!waitMessage ($fp, $msgin))
		{
			$result = "Failed to wait for LAS $LASHost (ec 9)";
			return false;
		}

		$result = '';
		if (!$msgin->serialstring($result))
		{
			$result = "Failed to decode LAS message $LASHost (ec 10)";
			return false;
		}
			
		fclose ($fp);
		
		$pos = strpos($result, ':');
		if ($pos === FALSE)
		{
			$result = "Failed to decode LAS message $LASHost (ec 11)";
			return false;
		}

		$success = (substr($result, 0, $pos) == '1');

		if ($success)
		{
			++$pos;
			$npos = strpos($result, ':', $pos);
			$numpages = substr($result, $pos, $npos-$pos);
			++$npos;
			$pos = strpos($result, ':', $npos);
			$page = substr($result, $npos, $pos-$npos);

			$result = substr($result, $pos+1);
		}
		else
		{
			$result = substr($result, $pos+1);
		}

		return $success;
	}

	function	displayLASQueries($LASHost, &$result)
	{
		if (!connectToLAS($LASHost, $fp, $result))
		{
			$result = "Failed to connect to LAS $LASHost (ec 12)";
			return false;
		}
			
		// send the message that say that we want to add a user
		$msgout = new CMemStream;

		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet

		$messageType = 2;
		$msgout->serialuint8 ($messageType);
		
		if (!sendMessage ($fp, $msgout))
		{
			$result = "Failed to send query to LAS $LASHost (ec 13)";
			return false;
		}

		if (!waitMessage ($fp, $msgin))
		{
			$result = "Failed to wait for LAS $LASHost (ec 14)";
			return false;
		}

		$result = '';
		if (!$msgin->serialstring($result))
		{
			$result = "Failed to decode LAS message $LASHost (ec 15)";
			return false;
		}
			
		fclose ($fp);
		
		$pos = strpos($result, ':');
		if ($pos === FALSE)
		{
			$result = "Failed to decode LAS message $LASHost (ec 16)";
			return false;
		}

		$success = (substr($result, 0, $pos) == '1');

		if ($success)
		{
			++$pos;
			$npos = strpos($result, "\n", $pos);
			$result = substr($result, $npos);
		}
		else
		{
			$result = substr($result, $pos+1);
		}

		return $success;
	}
?>
