<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('nel_message.php');

	class CAdminServiceWeb extends CCallbackClient
	{

		function globalCmd($command)
		{
			$msg = new CMessage;
			$msg->setName("GCMD");


			$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function controlCmd($serviceAlias, $command)
		{
			$msg = new CMessage;
			$msg->setName("CCMD");


			$msg->serialString($serviceAlias);
				$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function serviceCmd($serviceAlias, $command)
		{
			$msg = new CMessage;
			$msg->setName("SCMD");


			$msg->serialString($serviceAlias);
				$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function getShardOrders()
		{
			$msg = new CMessage;
			$msg->setName("GSO");



			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getShardOrders", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getShardOrders", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_GSO"))
			{
				// error during send
				$this->invokeError("getShardOrders", "Invalid response, awaited 'R_GSO', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}

		function getStates()
		{
			$msg = new CMessage;
			$msg->setName("GS");



			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getStates", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getStates", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_GS"))
			{
				// error during send
				$this->invokeError("getStates", "Invalid response, awaited 'R_GS', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}

		function getHighRezGraph($varAddr, $startDate, $endDate, $milliStep)
		{
			$msg = new CMessage;
			$msg->setName("GHRG");


			$msg->serialString($varAddr);
				$msg->serialUint32($startDate);
				$msg->serialUint32($endDate);
				$msg->serialUint32($milliStep);

			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_GHRG"))
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Invalid response, awaited 'R_GHRG', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}


		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "CMDR":
				$this->commandResult_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}


		function commandResult_skel(&$message)
		{
$message->serialString($serviceAlias);
	$message->serialString($result);

			$this->commandResult($serviceAlias, $result);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////

		function commandResult($serviceAlias, $result)
		{
		}

	}
?>
