

<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('tools/nel_message.php');

	class CLoginServiceWebProxy
	{

		function login($userId, $ipAddress, $callbackClient)
		{
			$msg = new CMessage;
			$msg->setName("LG");
			$msg->serialUint32($userId);
			$msg->serialUint32($ipAddress);
			
			$callbackClient->sendMessage($msg);
		}
	}

	class CLoginServiceWebSkel
	{	
		function waitCallback($callbackClient)
		{
			$message = $callbackClient->waitMessage();

			switch($message->MsgName)
			{
			case "LGR":
				$this->loginResult_skel($message);
				break;
			}
		}
		
		// Return the cookie generated for this user session
		// Eventualy, return an empty string as cookie in case of error

		function loginResult_skel($message)
		{
			$msg->serialUint32($userId);
			$msg->serialString($cookie);
			
			loginResult($userId, $cookie);
		}
	}
?>
	