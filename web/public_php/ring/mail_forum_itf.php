<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('../tools/nel_message.php');

	class CMailForumWeb extends CCallbackClient
	{

		function notifyMail($charId)
		{
			$msg = new CMessage;
			$msg->setName("MFS_NM");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function notifyForumMessage($guildId, $forumId)
		{
			$msg = new CMessage;
			$msg->setName("MFS_NFM");


			$msg->serialUint32($guildId);
				$msg->serialUint32($forumId);
	
			return parent::sendMessage($msg);


		}
	

		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "MFS_RET":
				$this->invokeResult_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}
		

		function invokeResult_skel(&$message)
		{
			$message->serialUint32($resultCode);
				$message->serialString($resultString);
				
			$this->invokeResult($resultCode, $resultString);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class 
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////
		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error

		function invokeResult($resultCode, $resultString)
		{
		}

	}
?>
