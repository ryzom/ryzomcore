<?php

	$SockTimeOut = 10;

	function debug($text)
	{
//		flush();
//		echo $text;
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
				$valLen = strlen($val);
				$this->serialUInt32($valLen);
				$this->Buffer .= $val;
				$this->Pos += $valLen;
				debug(sprintf ("write string '%s' %d<br>\n", $val, $this->Pos));
			}
		}
		function serialEnum (&$val)
		{
			if ($this->isReading())
			{
				$intValue = 0;
				$this->serialUInt32($intValue);
				$val->fromInt((int)$intValue);
				debug(sprintf ("read enum '%s'<br>\n", $val->toString()));
			}
			else
			{
				$intValue = $val->toInt();
				$this->serialUInt32($intValue);
				debug(sprintf ("write enum '%s' %d<br>\n", $val->toString(), $this->Pos));
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

?>
