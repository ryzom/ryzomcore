<?php

	include_once('../config.php');

	// Functions

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

	// This function connect to the LS, ask it if the user uid can connect
	// to shard shardid and return a bool that say it s ok or not.
	// If true, $res contains the url to connect.
	// If false, $res contains the reason why it s not okay.

	function connectToLS(&$fp, &$res)
	{
		global $LSHost, $LSPort;

		// connect to the login service
		$fp = fsockopen ($LSHost, $LSPort, $errno, $errstr, 30);
		if (!$fp)
		{
			$res = "Can't connect to the login service '$LSHost:$LSPort' ($errno: $errstr) (error code 41)";
		}
		else
		{
			$res = "";
		}
	}

	function sendMessage ($fp, $msgout)
	{
		$size = $msgout->Pos;
		$buffer  = chr(($size>>24)&0xFF);
		$buffer .= chr(($size>>16)&0xFF);
		$buffer .= chr(($size>>8)&0xFF);
		$buffer .= chr($size&0xFF);
		$buffer .= $msgout->Buffer;

		if (!fwrite ($fp, $buffer))
			return false;

		//printf ("sent packet size '%d'<br>", strlen($buffer));
		
		// strange but flush returns false even if it s ok
		fflush ($fp);

		return true;
	}

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

		$buffer = fread ($fp, $size);
		if (feof ($fp)) return false;
		$msgin = new CMemStream;
		$msgin->setBuffer ($buffer);

		return true;
	}

	function askClientConnection ($shardid, $uid, $name, $priv, $extended, &$res, &$patchURLS)
	{
		$ok = false;

		connectToLS($fp, $res);
		if($res!="")
			return $ok;

		// send the message that say that we want to add a user
		$msgout = new CMemStream;
		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet
		$messageType = 0;
		$msgout->serialuint8 ($messageType);
		$msgout->serialuint32 ($shardid);
		$msgout->serialuint32 ($uid);
		$msgout->serialstring ($name);
		$msgout->serialstring ($priv);
		$msgout->serialstring ($extended);

		if (!sendMessage ($fp, $msgout))
		{
			$res = "Can't send message connection to the Login Service (error code 42)";
			return $ok;
		}

		if (!waitMessage ($fp, $msgin))
		{
			$res = "Can't receive the answer from the Login Service (error code 43)";
			return $ok;
		}

		if (!$msgin->serialstring($reason))
		{
			$res = "Can't read the reason (error code 44)";
			return $ok;
		}
		//printf("reason size %d", strlen($reason));
			
		if(strlen($reason) == 0)
		{
			// it s ok, let's connect
			if (!$msgin->serialstring($cookie))
			{
				$res = "Can't read the cookie (error code 45)";
				return $ok;
			}

			if (!$msgin->serialstring($addr))
			{
				$res = "Can't read the addr (error code 46)";
				return $ok;
			}

			$patchURLS = '';
			/*
			if (!$msgin->serialstring($patchURLS))
			{
				$patchURLS = '';
			}
			*/

			$res = $cookie.' '.$addr;

			$ok = true;
		}
		else
		{
			// can't accept it, display the error
			// echo "Can't connect to the shard: $reason";
			$res = $reason;
		}

		//printf("receive response '$reason' '$cookie' '$addr'<br>");

		fclose ($fp);
		//echo "sent OK.<br><br>";

		return $ok;
	}

	// This function connect to the LS, ask the LS to disconnect the user

	function disconnectClient($shardid, $uid, &$res)
	{
		$ok = false;

		connectToLS($fp, $res);
		if(strlen($res) != 0)
			return $ok;

		// send the message that say that we want to add a user
		$msgout = new CMemStream;
		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet
		$messageType = 1;
		$msgout->serialuint8 ($messageType);
		$msgout->serialuint32 ($shardid);
		$msgout->serialuint32 ($uid);

		if (!sendMessage ($fp, $msgout))
		{
			$res = "Can't send message disconnect to the Login Service (error code 47)";
			return $ok;
		}

		if (!waitMessage ($fp, $msgin))
		{
			$res = "Can't receive the answer from the Login Service (error code 48)";
			return $ok;
		}

		if (!$msgin->serialstring($res))
		{
			$res = "Can't read the string (error code 49)";
			return $ok;
		}
		//printf("reason size %d", strlen($res));
			
		//if(strlen($res) == 0)
		{
			$ok = true;
		}

		//printf("receive response '$reason' '$cookie' '$addr'<br>");

		fclose ($fp);
		//echo "sent OK.<br><br>";

		return $ok;
	}

?>
