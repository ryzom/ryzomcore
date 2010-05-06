

<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('tools/nel_message.php');

	class CRingSessionManagerWebProxy
	{

		function createSession($userId, $sessionType, $callbackClient)
		{
			$msg = new CMessage;
			$msg->setName("CSS");
			$msg->serialUint32($userId);
			$msg->serialString($sessionType);

			$callbackClient->sendMessage($msg);
		}

		function createScenario($sessionId, $callbackClient)
		{
			$msg = new CMessage;
			$msg->setName("CSC");
			$msg->serialUint32($sessionId);

			$callbackClient->sendMessage($msg);
		}
	}

	class CRingSessionManagerWebSkel
	{
		function waitCallback($callbackClient)
		{
			$message = $callbackClient->waitMessage();

			switch($message->MsgName)
			{
			case "CSSR":
				$this->createSessionResult_skel($message);
				break;
			case "CSCR":
				$this->createScenarioResult_skel($message);
				break;
			}
		}


		function createSessionResult_skel($message)
		{
			$msg->serialUint32($userId);
			$msg->serialUint32($sessionId);
			$msg->serialUInt8($result);

			createSessionResult($userId, $sessionId, $result);
		}

		function createScenarioResult_skel($message)
		{
			$msg->serialUint32($sessionId);
			$msg->serialUint32($scenarioId);
			$msg->serialUInt8($result);

			createScenarioResult($sessionId, $scenarioId, $result);
		}
	}
?>
