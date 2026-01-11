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

	class CMemStream
	{
		var $Buffer;
		var $InputStream;
		var $Pos;

		function CMemStream ()
		{
			$this->InputStream = false;
			$this->Pos = 0;
		}

		function setBuffer ($buffer)
		{
			$this->InputStream = true;
			$this->Buffer = $buffer;
			$this->Pos = 0;
		}

		function isReading () { return $this->InputStream; }

/*
		function serialuint8 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				//printf ("read uint8 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Pos++;
				//printf ("write uint8 '%d' %d<br>", $val, $this->Pos);
			}
		}

		function serialuint32 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				$val += ord($this->Buffer{$this->Pos++})<<8;
				$val += ord($this->Buffer{$this->Pos++})<<16;
				$val += ord($this->Buffer{$this->Pos++})<<32;
				//printf ("read uint32 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Buffer .= chr(($val>>8) & 0xFF);
				$this->Buffer .= chr(($val>>16) & 0xFF);
				$this->Buffer .= chr(($val>>24) & 0xFF);
				$this->Pos += 4;
				//printf ("write uint32 '%d' %d<br>", $val, $this->Pos);
			}
		}

		function serialstring (&$val)
		{
			if ($this->isReading())
			{
				$this->serialuint32($size);
				$val = substr ($this->Buffer, $this->Pos, $size);
				//printf ("read string '%s'<br>", $val);
				$this->Pos += strlen($val);
			}
			else
			{
				$this->serialuint32(strlen($val));
				$this->Buffer .= $val;
				$this->Pos += strlen($val);
				//printf ("write string '%s' %d<br>", $val, $this->Pos);
			}
		}
*/

		function serialuint8 (&$val)
		{
			if ($this->isReading())
			{
				if ($this->Pos+1 > strlen($this->Buffer))
					return false;

				$val = ord($this->Buffer{$this->Pos++});
				//printf ("read uint8 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Pos++;
				//printf ("write uint8 '%d' %d<br>", $val, $this->Pos);
			}
			return true;
		}

		function serialuint32 (&$val)
		{
			if ($this->isReading())
			{
				if ($this->Pos+4 > strlen($this->Buffer))
					return false;

				$val = ord($this->Buffer{$this->Pos++});
				$val += ord($this->Buffer{$this->Pos++})<<8;
				$val += ord($this->Buffer{$this->Pos++})<<16;
				$val += ord($this->Buffer{$this->Pos++})<<32;
				//printf ("read uint32 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Buffer .= chr(($val>>8) & 0xFF);
				$this->Buffer .= chr(($val>>16) & 0xFF);
				$this->Buffer .= chr(($val>>24) & 0xFF);
				$this->Pos += 4;
				//printf ("write uint32 '%d' %d<br>", $val, $this->Pos);
			}
			return true;
		}

		function serialstring (&$val)
		{
			if ($this->isReading())
			{
				if (!$this->serialuint32($size))
					return false;

				if ($this->Pos+$size > strlen($this->Buffer))
					return false;

				$val = substr ($this->Buffer, $this->Pos, $size);
				//printf ("read string '%s'<br>", $val);
				$this->Pos += strlen($val);
			}
			else
			{
				$this->serialuint32(strlen($val));
				$this->Buffer .= $val;
				$this->Pos += strlen($val);
				//printf ("write string '%s' %d<br>", $val, $this->Pos);
			}
			return true;
		}

	}

	// This function connect to the AS.
	// If true, $res contains the url to connect.
	// If false, $res contains the reason why it s not okay.

	function connectToAS(&$fp, &$res, $asHost, $asPort)
	{
		// connect to the login service that must be $ASHost:$ASPort
		$fp = fsockopen ($asHost, $asPort, $errno, $errstr, 30);
		if (!$fp)
		{
			$res = "Can't connect to the admin service '$ASHost:$ASPort' ($errno: $errstr)";
		}
		else
		{
			$res = "";
		}
	}
	
	function disconnectFromAS(&$fp)
	{
		fclose($fp);
	}

	function sendMessage ($fp, $msgout)
	{
		$size = $msgout->Pos;
		$buffer = chr(($size>>24)&0xFF);
		$buffer .= chr(($size>>16)&0xFF);
		$buffer .= chr(($size>>8)&0xFF);
		$buffer .= chr($size&0xFF);
		$buffer .= $msgout->Buffer;

		fwrite ($fp, $buffer);

		fflush ($fp);
		
		return true;
	}
	
	function logToFile($msg)
	{
		$f = fopen("../log_admin_tool.txt", "a");
		if ($f)
		{
			fwrite($f, "$msg\n");
			fflush($f);
			fclose($f);
		}
	}
/*
	function waitMessage ($fp, &$msgin)
	{
		$size = 0;
		$val = fread ($fp, 1);
		$size = ord($val) << 24;
		$val = fread ($fp, 1);
		$size = ord($val) << 16;
		$val = fread ($fp, 1);
		$size += ord($val) << 8;
		$val = fread ($fp, 1);
		$size += ord($val);
		$fake = fread ($fp, 4);
		$size -= 4; // remove the fake
		
		$buffer = '';
		do
		{
			$buffer .= fread ($fp, $size);
			//logToFile("read ".strlen($buffer)." bytes...");
			if (feof($fp))
				break;
		}
		while (strlen($buffer) != $size);

		//logToFile("finished read ".strlen($buffer)." bytes!!");

		$msgin = new CMemStream;
		$msgin->setBuffer ($buffer);

		//logToFile("leave wait message");
		return true;
	}
*/
	function waitMessage ($fp, &$msgin)
	{
		//echo "waiting a message";
		$size = 0;
		$val = fread ($fp, 1);
		if (feof ($fp)) return false;
		$size = ord($val) << 24;
		$val = fread ($fp, 1);
		if (feof ($fp)) return false;
		$size = ord($val) << 16;
		$val = fread ($fp, 1);
		if (feof ($fp)) return false;
		$size += ord($val) << 8;
		$val = fread ($fp, 1);
		if (feof ($fp)) return false;
		$size += ord($val);
		//printf ("receive packet size '%d'<br>", $size);
		$fake = fread ($fp, 4);
		if (feof ($fp)) return false;
		$size -= 4; // remove the fake

		$buffer = '';
		while (($stillNotRead = $size-strlen($buffer)) > 0)
		{
			$buffer .= fread ($fp, $stillNotRead);
			if (feof ($fp)) return false;
		}

		$msgin = new CMemStream;
		$msgin->setBuffer ($buffer);

		return true;
	}
	
	function logNelQuery($query)
	{
		global	$uid;
/*
		$f = fopen("./nel_queries.log", "a");
		fwrite($f, date("Y/m/d H:i:s")." ".sprintf("%-16s", $admlogin)." $query\n");
		fclose($f);
*/
		logUser($uid, "QUERY=".$query);
	}


	function	queryToAS($rawvarpath, &$result, $asAddr, $asPort)
	{
		global			$nel_queries;

		$nel_queries[] = $rawvarpath;
		$ok = false;

		connectToAS($fp, $result, $asAddr, $asPort);
		if(strlen($result) != 0)
			return $ok;

		// send the message that say that we want to add a user
		$msgout = new CMemStream;
		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet
		$messageType = 0;
		$msgout->serialuint8 ($messageType);
		$msgout->serialstring ($rawvarpath);

		sendMessage ($fp, $msgout);

		waitMessage ($fp, $msgin);

		$msgin->serialstring($result);
			
		if(strlen($result) == 0)
		{
			// it failed
		}
		else
		{
			// it's ok
			$ok = true;
		}

		disconnectFromAS(&$fp);

		return $ok;
	}


	function nel_query($rawvarpath, &$result)
	{
		global	$ASHost, $ASPort;

		$shards = getShardListFromQuery($rawvarpath);
		$as = getASList($shards);

		if (count($as) <= 1)
		{
			$asHost = $ASHost;
			$asPort = $ASPort;
	
			if (count($as) > 0)
			{
				foreach ($as as $asHost)
				{
					$pos = strpos($asHost, ':');
					if (pos != FALSE)
					{
						$asPort = substr($asHost, $pos+1);
						$asHost = substr($asHost, 0, $pos);
					}
				}
			}

			$res = queryToAS($rawvarpath, $result, $asHost, $asPort);
		}
		else
		{
			$resCols = array();
			$resArray = array();
			
			$res = false;

			foreach ($as as $asHost)
			{
				$asPort = $ASPort;
				$pos = strpos($asHost, ':');
				if (pos != FALSE)
				{
					$asPort = substr($asHost, $pos+1);
					$asHost = substr($asHost, 0, $pos);
				}

				$tmp = queryToAS($rawvarpath, $qres, $asHost, $asPort);
				if ($tmp)
				{
					$res = true;
					mergeResult($qres, $resCols, $resArray);
				}
			}

			//print_r($resCols);echo '<br>';
			//print_r($resArray);echo '<br>';
			$result = rebuildResult($resCols, $resArray);
		}
		
		return res;
	}


	function	getShardFromSimpleQuery($query, $startpos=0)
	{
		$pos = $startpos;
		while ($pos < strlen($query) && $query[$pos] != '.' && $query[$pos] != ']' && $query[$pos] != ',')
			++$pos;
		//echo 'getShardFromSimpleQuery: '.substr($query, $startpos, $pos-$startpos).'<br>';
		return substr($query, $startpos, $pos-$startpos);
	}

	function	getShardListFromQuery($query, $startpos=0)
	{
		$shards = array();

		if ($query[$startpos] != '[')
		{
			$shards[] = getShardFromSimpleQuery($query, $startpos);
			return $shards;
		}

		$pos = $startpos+1;
		$lvl = 0;
		
		while (true)
		{
			//echo 'getShardListFromQuery in '.substr($query, $pos).'<br>';
			$shards = array_merge($shards, getShardListFromQuery($query, $pos));

			while ($pos<strlen($query) && ($query[$pos]!=',' || $lvl>0))
			{
				if ($query[$pos] == '[')	++$lvl;
				if ($query[$pos] == ']')	--$lvl;
				++$pos;
			}
			
			if ($query[$pos] != ',')
				break;
				
			++$pos;
		}

		return array_unique($shards);
	}
	
	function	selectAllAS()
	{
		global	$ASHost, $ASPort;

		$as[] = $ASHost.':'.$ASPort;
		
		foreach($shardLockState as $shard)
			if ($shard['ASAddr'] != '')
				$as[] = $shard[ASAddr];

		return array_unique($as);
	}

	function	getASList($shards)
	{
		global	$ASHost, $ASPort, $shardLockState;

		if (count($shards) == 0)
			return;

		if (array_search('*', $shards) != FALSE)
			return selectAllAS();

		foreach($shards as $shard)
			if ($shardLockState[$shard]['ASAddr'] != '')
				$as[$shard] = $shardLockState[$shard]['ASAddr'];
			else
				$as[$shard] = $ASHost.':'.$ASPort;

		return array_unique($as);
	}

	function	mergeResult(&$res, &$resCols, &$resArray)
	{
		$resV = explode(' ', $res);

		$i = 0;
		$numCols = $resV[$i++];
		for ($i=1; $i<=$numCols; ++$i)
			if (!isset($resCols[$resV[$i]]))
				$resCols[$resV[$i]] = count($resCols);

		while ($i < count($resV))
		{
			//echo 'examine i='.$i.', resv='.count($resV).'<br>';

			$line = array();
			for ($j=0; $j<$numCols; ++$j)
			{
				$line[ $resCols[$resV[$j+1]] ] = $resV[$i++];
				
				if ($i > count($resV))
					break;
			}
			if ($j>=$numCols)
				$resArray[] = $line;
		}
	}

	function	rebuildResult(&$resCols, &$resArray)
	{
		$numCols = count($resCols);

		$res = $numCols;
		
		foreach($resCols as $col => $id)
			$res .= ' '.$col;

		//$res .= join(' ', $resCols);

		$numRows = count($resArray);
		if ($numRows == 0)
			return $res;
		
		for ($i=0; $i<$numRows; ++$i)
		{
			$line = &$resArray[$i];
			//print_r($line); echo '<br>';
			if (count($line) == 0)
				continue;

			foreach ($resCols as $col => $id)
			{
				$v = &$line[$id];
				if (!isset($v) || $v == '')
					$res .= ' ???';
				else
					$res .= ' '.$v;
			}
		}
		
		return $res;
	}
?>
