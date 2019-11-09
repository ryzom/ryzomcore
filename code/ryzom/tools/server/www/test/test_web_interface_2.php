<?php

	include('test_web_interface_itf.php');

	// force output immediatly
	ob_implicit_flush();
	echo "Starting web interface test..<br>";
	
	// declare a class that overide the callbacks
		
	class CMyInterface extends CTestInterfaceWeb2
	{		
		function returnVectorUInt32($i32)
		{
			echo "CMyInterface::returnVectorUint32 called<br>";
			
			print_r($i32);echo"<br>";
		}
		
		function returnVectorString($vstr)
		{
			echo "CMyInterface::returnVectorString called<br>";
			
			print_r($vstr);echo"<br>";
		}

		function returnMixedVector($param1, $vstr, $vi32)
		{
			echo "CMyInterface::returnMixedVevtor called<br>";
			
			echo "$param1<br>";
			print_r($vstr);echo"<br>";
			print_r($vi32);echo"<br>";
		}
		
	}

	$interf = new CMyInterface;
	
	// connect to the server
	echo "Connecting...<br>";
	$res = "";
	$interf->connect("192.168.0.1", 8062, $res);
	echo "Conn result : '$res'<br>";
	
	$arr = array(1,2,3,4,5,6,7,8,9);
	echo "Calling sendVectorUInt32...<br>";
	$interf->sendVectorUInt32($arr);
	
	echo "Waiting callback...<br>";
	$interf->waitCallback();

	$arr = array("a ldkjfmdslk", "b slekjf lef", "c smdlkjfs", "d slkdjfdsmfkj", "e mdlfkdsm");
	echo "Calling sendVectorUInt32...<br>";
	$interf->sendVectorString($arr);
	
	echo "Waiting callback...<br>";
	$interf->waitCallback();
	
	echo "Calling simple twoway...<br>";
	$ret = $interf->twoWayCall("toto");
	$ret = $interf->twoWayCall("toto");
	assert($ret === "toto");
	$ret = $interf->twoWayCall("titi");
	assert($ret === "titi");
	$ret = $interf->twoWayCall("qsmflkjsdgoishdfgoiuh ihf isqouhf skqrgbiq qo'zhto'èt_yzaèhtàaéç't_hpg");
	assert($ret === "qsmflkjsdgoishdfgoiuh ihf isqouhf skqrgbiq qo'zhto'èt_yzaèhtàaéç't_hpg");
	echo "Twoway call returned $ret<br>";

	echo "Calling int twoway<br>";
	$ret = $interf->twoWayCallInt(1234);
	assert($ret == 1234);

	echo "Calling enum twoway<br>";
	$ret = $interf->twoWayCallInt("enum_b");
	assert($ret == "enum_b");
	
	echo "Calling int vector twoway<br>";
	$arr = array(1,2,3,4,5,6);
	$ret = $interf->twoWayCallVector($arr);
	print_r($ret);echo "<br>";
	print_r($arr);echo "<br>";
	assert($ret == $arr);
	
	echo "Calling mixed vector<br>";
	$interf->mixedVector(1234, array("a", "b", "c", "d"), array(1,2,3,4,5,6));
	$interf->waitCallback();
	
	echo "Calling mixed vector twoway<br>";
	$ret = $interf->mixedTwoWayVector(1234, array("a", "b", "c", "d"), array(1,2,3,4,5,6));
	assert($ret == 1234);
	
	echo "Test ended<br>";	
?>