<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('../tools/nel_message.php');

	class CLoginServiceWeb extends CCallbackClient
	{

		function login($userId, $ipAddress, $domainId)
		{
			$msg = new CMessage;
			$msg->setName("LG");


			$msg->serialUint32($userId);
				$msg->serialString($ipAddress);
				$msg->serialUint32($domainId);
	
			return parent::sendMessage($msg);


		}

		function logout($userId)
		{
			$msg = new CMessage;
			$msg->setName("LO");


			$msg->serialUint32($userId);
	
			return parent::sendMessage($msg);


		}
	

		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "LGR":
				$this->loginResult_skel($message);
				break;
			case "LGOR":
				$this->logoutResult_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}
		

		function loginResult_skel(&$message)
		{
			$message->serialUint32($userId);
				$message->serialString($cookie);
				$message->serialUint32($resultCode);
				$message->serialString($errorString);
				
			$this->loginResult($userId, $cookie, $resultCode, $errorString);
		}

		function logoutResult_skel(&$message)
		{
			$message->serialUint32($errorCode);
				$message->serialString($reason);
				
			$this->logoutResult($errorCode, $reason);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class 
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////
		// Return the cookie generated for this user session
		// Eventualy, return an empty string as cookie in case of error
		//   resultCode : 0 - ok, login success
		//       1 - invalid user
		//       2 - user already online, must relog
		//  errorString contain a stringified description in case of error

		function loginResult($userId, $cookie, $resultCode, $errorString)
		{
		}

		// Return an error code for the logout attemp
		// If return is not 0, then reason contains a debug string
		// Return values : 0 - ok
		//                 1 - invalid user
		//                 2 - user already offline

		function logoutResult($errorCode, $reason)
		{
		}

	}
?>
