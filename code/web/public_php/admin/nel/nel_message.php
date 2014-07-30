<?php
	
	$SockTimeOut = 10;
	
	function debug($text)
	{
//		flush();
//		echo $text;

//		$file = "logs/debug.txt";
//		$str = file_get_contents($file);
//		$str .= $text."\n";
//		file_put_contents($file, $str);
	}

	class CMemStream
	{
		var $Buffer;
		var $InputStream;
		var $Pos;

		function CMemStream ()
		{
			$this->InputStream = false;
			$this->Pos = 0;
			$this->Buffer = "";
			debug("A : ".gettype($this->Buffer)."<br>");
		}

		function setBuffer ($Buffer)
		{
			$this->InputStream = true;
			$this->Buffer = $Buffer;
			$this->Pos = 0;
		}

		function isReading () { return $this->InputStream; }

		function serialUInt8 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				debug(sprintf ("read uint8 '%d'<br>\n", $val));
			}
			else
			{
			debug("B".gettype($this->Buffer)."<br>");
			debug(sprintf ("write uint8 Buffer size before = %u<br>\n", strlen($this->Buffer)));
				$this->Buffer = $this->Buffer . chr($val & 0xFF);
				$this->Pos++;
			debug("C".gettype($this->Buffer)."<br>");
			debug(sprintf ("write uint8 '%d' %d<br>\n", $val, $this->Pos));
			debug(sprintf ("write uint8 Buffer size after = %u<br>\n", strlen($this->Buffer)));
			}
		}

		function serialUInt32 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				$val += ord($this->Buffer{$this->Pos++})*256;
				$val += ord($this->Buffer{$this->Pos++})*(double)256*256;
				$val += ord($this->Buffer{$this->Pos++})*(double)256*256*256;
				debug(sprintf ("read uint32 '%d'<br>\n", $val));
//				var_dump($val);
			}
			else
			{
			debug("D".gettype($this->Buffer)."<br>");
				$this->Buffer .= chr($val & 0xFF);
				$this->Buffer .= chr(($val>>8) & 0xFF);
				$this->Buffer .= chr(($val>>16) & 0xFF);
				$this->Buffer .= chr(($val>>24) & 0xFF);
				$this->Pos += 4;
			debug("E".gettype($this->Buffer)."<br>");
			debug(sprintf ("write uint32 '%d' %d<br>\n", $val, $this->Pos));
			}
		}

		function serialString (&$val)
		{
			if ($this->isReading())
			{
				$this->serialUInt32($size);
				debug(sprintf ("read string : size = %u<br>\n", $size));
				$val = substr ($this->Buffer, $this->Pos, $size);
				debug(sprintf ("read string '%s'<br>\n", $val));
				$this->Pos += strlen($val);
			}
			else
			{
				$this->serialUInt32(strlen($val));
				$this->Buffer .= $val;
				$this->Pos += strlen($val);
				debug(sprintf ("write string '%s' %d<br>\n", $val, $this->Pos));
			}
		}
	}
	
	class CMessage extends CMemStream
	{
		var $MsgName;
		
		function CMessage()
		{
			$this->CMemStream();
		}
		
		function setName($name)
		{
			$this->MsgName = $name;
		}
	}
	
	class CCallbackClient
	{
		var	$ConSock = false;

		var $MsgNum = 0;
		
		function connect($addr, $port, &$res)
		{
			global $SockTimeOut;
			
			debug(sprintf("Connect<br>"));
			$this->MsgNum = 0;
			
			$this->ConSock = fsockopen ($addr, $port, $errno, $errstr, $SockTimeOut);
			debug("H".gettype($this->ConSock)."<br>");

			if (!$this->ConSock)
			{
				$res = "Can't connect to the callback server '$addr:$port' ($errno: $errstr)";
				
				return false;
			}
			else
			{
				// set time out on the socket to 2 secondes
				stream_set_timeout($this->ConSock, $SockTimeOut);	
				$res = "";
				return true;
			}
		}
		
		function close()
		{
			if ($this->ConSock)
			{
				fclose($this->ConSock);
				debug(sprintf("Close<br>"));
			}
			else
				debug(sprintf("Already Closed !<br>"));
		}
		
		function sendMessage(&$message)
		{
			if (!$this->ConSock)
			{
				debug(sprintf ("Socket is not valid\n"));
				return false;
			}
			debug(sprintf ("sendMessage : message Buffer is '%d'<br>\n", $message->Pos));
			debug(sprintf ("sendMessage : message Buffer is '%d'<br>\n", strlen($message->Buffer)));
			$hd = new CMemStream;
			debug(sprintf("SendMessage number %u<br>", $this->MsgNum));
			$hd->serialUInt32 ($this->MsgNum);			// number the packet
			$this->MsgNum += 1;
			debug(sprintf("After SendMessage, number %u<br>", $this->MsgNum));
			$messageType = 0;
			$hd->serialUInt8 ($messageType);
			$hd->serialString ($message->MsgName);

			debug(sprintf ("sendMessage : header size is '%d'<br>\n", $hd->Pos));
			
//			$sb .= $message->Buffer;

			$size = $hd->Pos + $message->Pos;
			$Buffer = (string) chr(($size>>24)&0xFF);
			$Buffer .= chr(($size>>16)&0xFF);
			$Buffer .= chr(($size>>8)&0xFF);
			$Buffer .= chr($size&0xFF);
			debug( "E".gettype($hd->Buffer)."<br>");
			debug("F".gettype($message->Buffer)."<br>");
			$Buffer .= (string) $hd->Buffer;
			$Buffer .= (string) $message->Buffer;
			
			debug("G".gettype($this->ConSock)."<br>");

			if (!fwrite ($this->ConSock, $Buffer))
			{
				debug(sprintf ("Error writing to socket\n"));
				return false;
			}
			debug(sprintf ("sent packet size '%d' (written size = %d) <br>\n", strlen($Buffer), $size));
			fflush ($this->ConSock);
			
			return true;
		}
		
		function waitMessage()
		{
			if (!$this->ConSock)
			{
				debug(sprintf ("Socket is not valid\n"));
				return false;
			}
			

			$size = 0;
			$val = fread ($this->ConSock, 1);
			$info = stream_get_meta_data($this->ConSock);
			if ($info['timed_out']) 
			{
				debug('Connection timed out!');
				return false;
			}
			$size = ord($val) << 24;
			$val = fread ($this->ConSock, 1);
			$info = stream_get_meta_data($this->ConSock);
			if ($info['timed_out']) 
			{
				debug('Connection timed out!');
				return false;
			}
			$size = ord($val) << 16;
			$val = fread ($this->ConSock, 1);
			$info = stream_get_meta_data($this->ConSock);
			if ($info['timed_out']) 
			{
				debug('Connection timed out!');
				return false;
			}
			$size += ord($val) << 8;
			$val = fread ($this->ConSock, 1);
			$info = stream_get_meta_data($this->ConSock);
			if ($info['timed_out']) 
			{
				debug('Connection timed out!');
				return false;
			}
			$size += ord($val);
			debug(sprintf ("receive packet size '%d'<br>\n", $size));
			$fake = fread ($this->ConSock, 5);
			$info = stream_get_meta_data($this->ConSock);
			if ($info['timed_out']) 
			{
				debug('Connection timed out!');
				return false;
			}
			$size -= 5; // remove the fake

			$Buffer = "";
			while ($size > 0 && strlen($Buffer) != $size)
			{
				$Buffer .= fread ($this->ConSock, $size - strlen($Buffer));
				$info = stream_get_meta_data($this->ConSock);
				if ($info['timed_out']) 
				{
					debug('Connection timed out!');
					return false;
				}
			}
			$msgin = new CMemStream;
			$msgin->setBuffer ($Buffer);
			
			// decode msg name
			$msgin->serialString($name);
			
			debug(sprintf("Message name = '%s'<BR>", $name));
			$message = new CMessage;
			$message->setBuffer(substr($msgin->Buffer, $msgin->Pos));
			$message->setName($name);
			
			debug(sprintf("In message name = '%s'<br>", $message->MsgName));
			
			return $message;
		}
	}
	
	
//	class CSessionManagerProxy
//	{
//		function createSession($userId, $sessionType, $callbackClient)
//		{
//			debug(sprintf("Creating session for user %u, type %s<BR>", $userId, $sessionType));
//			$msg = new CMessage;
//			$msg->setName("CSS");
//			$msg->serialUInt32($userId);
//			$msg->serialString($sessionType);
//			
//			$callbackClient->sendMessage($msg);
//		}
//	}
	
//	class CSessionManagerClientSkel
//	{	
//		function waitCallback($callbackClient)
//		{
//			$message = $callbackClient->waitMessage();
//
//			debug(sprintf("Received message '%s'<BR>", $message->MsgName));
//			
//			switch($message->MsgName)
//			{
//			case "CSSR":
//				debug(sprintf("Create session result<BR>"));
//				$this->createSessionResult_skel($message);
//				break;
//					
//			case "CSNR":
//				debug(sprintf("Create scenario result<BR>"));
//				$this->createScenarioResult_skel($message);
//				break;
//			};
//		}
//		
//		function createSessionResult_skel($message)
//		{
//			$userId = 0;
//			$sessionId = 0;
//			$result = false;
//			
//			$message->serialUInt32($userId);
//			$message->serialUInt32($sessionId);
//			$message->serialUInt8($result);
//			
//			createSessionResult($userId, $sessionId, $result);
//		}
//	}
	
//	printf("creating callback client...<BR>");
//	
//	$cb = new CCallbackClient;
//	$ret = "";
//	$cb->connect("192.168.0.1", "8060", $ret);
//	
//	$smp = new CSessionManagerProxy;
//	
//	printf("creating a new sessions...<BR>");
//	$smp->createSession(10, "st_edit", $cb);
//	
//	$smcs = new CSessionManagerClientSkel;
//	$smcs->waitCallback($cb);
//	
//
//	function createSessionResult($userId, $sessionId, $result)
//	{
//		echo "The session result for user $userId is the session $sessionId with a result of $result\n";
//	}
//	

	// This function connect to the AS.
	// If true, $res contains the url to connect.
	// If false, $res contains the reason why it s not okay.

//	function connectToAS(&$fp, &$res)
//	{
//		global	$ASHost, $ASPort;
///*
//		$sid = session_id();
//		$result = sqlquery("SELECT socket_id FROM resident_socket");
//		if (!$result || sqlnumrows($result) == 0)
//		{
//			$fp = pfsockopen ($ASHost, $ASPort, $errno, $errstr, 30);
//			echo "opened resident socket '$fp'\n";
//			
//			$result = sqlquery("SELECT socket_id FROM resident_socket WHERE socket_id='$fp'");
//			if ($result && sqlnumrows($result)>0)
//				sqlquery("DELETE FROM resident_socket WHERE socket_id='$fp'");
//			
//			sqlquery("INSERT INTO resident_socket SET socket_id='$fp', session_id='$sid', last_access=NOW()");
//		}
//		else
//		{
//			$result = sqlfetch($result);
//			$fp = $result["socket_id"];
//		}
//		
//		// remove too old sockets
//		sqlquery("SELECT socket_id FROM resident_socket WHERE NOW()-last_access > 1800");
//		while ($result && ($arr=sqlfetch($result)))
//		{
//			fclose((int)($arr["socket_id"]));
//			sqlquery("DELETE FROM resident_socket WHERE socket_id='".$arr["socket_id"]."'");
//		}
//
//		// update current socket last access
//		sqlquery("UPDATE resident_socket SET last_access=NOW() WHERE socket_id='$fp' AND session_id='$sid'");
//*/
//
//		// connect to the login service that must be $ASHost:$ASPort
//		$fp = fsockopen ($ASHost, $ASPort, $errno, $errstr, 30);
//		if (!$fp)
//		{
//			$res = "Can't connect to the admin service '$ASHost:$ASPort' ($errno: $errstr)";
//		}
//		else
//		{
//			$res = "";
//		}
//
//	}
//	
//	function disconnectFromAS(&$fp)
//	{
///*
//		$result = sqlquery("SELECT socket_id FROM resident_socket WHERE socket_id='$fp'");
//		if (!$result || sqlnumrows($socket)==0)
//			fclose($fp);
//*/
//		fclose($fp);
//	}
//
//	function sendMessage ($fp, $msgout)
//	{
//		$size = $msgout->Pos;
//		$Buffer = chr(($size>>24)&0xFF);
//		$Buffer .= chr(($size>>16)&0xFF);
//		$Buffer .= chr(($size>>8)&0xFF);
//		$Buffer .= chr($size&0xFF);
//		$Buffer .= $msgout->Buffer;
//
//		fwrite ($fp, $Buffer);
//
//		//printf ("sent packet size '%d'<br>", strlen($Buffer));
//		
//		fflush ($fp);
//	}
//
//	function waitMessage ($fp, &$msgin)
//	{
//		//echo "waiting a message";
//		$size = 0;
//		$val = fread ($fp, 1);
//		$size = ord($val) << 24;
//		$val = fread ($fp, 1);
//		$size = ord($val) << 16;
//		$val = fread ($fp, 1);
//		$size += ord($val) << 8;
//		$val = fread ($fp, 1);
//		$size += ord($val);
//		//printf ("receive packet size '%d'<br>", $size);
//		$fake = fread ($fp, 4);
//		$size -= 4; // remove the fake
//
//		$Buffer = fread ($fp, $size);
//		$msgin = new CMemStream;
//		$msgin->setBuffer ($Buffer);
//	}
//	
//	function logNelQuery($query)
//	{
//		global	$uid;
///*
//		$f = fopen("./nel_queries.log", "a");
//		fwrite($f, date("Y/m/d H:i:s")." ".sprintf("%-16s", $admlogin)." $query\n");
//		fclose($f);
//*/
//
//		logUser($uid, "QUERY=".$query);
//	}
//
//	function nel_query($rawvarpath, &$result)
//	{
//		global			$nel_queries;
//
//		$nel_queries[] = $rawvarpath;
//		$ok = false;
//		//echo "rawvarpath=$rawvarpath<br>\n";
//		
//		//logNelQuery($rawvarpath);
//
//		connectToAS($fp, $result);
//		if(strlen($result) != 0)
//			return $ok;
//
//		// send the message that say that we want to add a user
//		$msgout = new CMemStream;
//		$fake = 0;
//		$msgout->serialuint32 ($fake);			// fake used to number the packet
//		$messageType = 0;
//		$msgout->serialuint8 ($messageType);
//		$msgout->serialstring ($rawvarpath);
//
//		sendMessage ($fp, $msgout);
//
//		waitMessage ($fp, $msgin);
//
//		$msgin->serialstring($result);
//			
//		if(strlen($result) == 0)
//		{
//			// it failed
//		}
//		else
//		{
//			// it's ok
//			$ok = true;
//		}
//
//		//printf("receive response '$result'<br>\n");
//
//		disconnectFromAS(&$fp);
//		//echo "sent OK.<br><br>\n";
//
//		return $ok;
//	}
?>
