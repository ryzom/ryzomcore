<?php

	include('test_web_interface_itf.php');

	ob_implicit_flush();
	
	$host = "192.168.0.1";
	$port = 8061;
	$state = 0;
	
	function returnUInt32($i32)
	{
		global $state;
		
		printf("returnUInt32(%x)<br>", $i32);
		var_dump($i32);
		
		if ($state == 0)
		{
			assert($i32 == 0x12345678);
		}
		else if ($state == 3)
		{
			assert($i32 == 0x12345678);
		}
		else if ($state == 9)
		{
			assert($i32 == 0x12345678);
		}
		else if ($state == 10)
		{
			assert($i32 == 0x87654321);
		}
		else if ($state == 12)
		{
			assert($i32 == 0x87654321);
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	
	function returnUInt8($i8)
	{
		global $state;
		
		printf("returnUInt8(%x)<br>", $i8);
		var_dump($i8);

		
		if ($state == 1)
		{
			assert($i8 == 0xf1);
		}
		else if ($state == 4)
		{
			assert($i8 == 0x01);
		}
		else if ($state == 11)
		{
			// ignore
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	
	function returnString($str)
	{
		global $state;
		
		printf("returnString(\"%s\")<br>", $str);
		
		if ($state == 2)
		{
			assert($str == "hello world");
		}
		else if ($state == 5)
		{
			assert($str == "ok");
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	
	function returnComposite1($i32, $i8, $str)
	{
		global $state;
		if ($state == 6)
		{
			assert($i32 == 0x12345678);
			assert($i8 == 0x9a);
			assert($str == 'hal');
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	
	function returnComposite2($str1, $str2, $str3, $str4)
	{
		global $state;
		if ($state == 7)
		{
			assert($str1 == "ABCD");
			assert($str2 == "EFGH IJKL");
			assert($str3 == "MNO");
			assert($str4 == "PQ");
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	function returnComposite3($i81, $i321, $str1, $i82, $i322, $str2, $i83, $i323, $str3, $i84, $i324, $str4, $i325, $i85)
	{
		global $state;
		if ($state == 8)
		{
			assert($i81 == 0x9a);
			assert($i321 == 0x12345678);
			assert($str1 == "ABC");
			assert($i82 == 0x34);
			assert($i322 == 0xbcdef012);
			assert($str2 == "DEFGH");
			assert($i83 == 0xcd);
			assert($i323 == 0x567890ab);
			assert($str3 == "I");
			assert($i84 == 0x67);
			assert($i324 == 0xef012345);
			assert($str4 == "");
			assert($i325 == 0x01234567);
			assert($i85 == 0x89);	
		}
		else
		{
			assert(false);
		}
		
		$state++;
	}
	
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	
	// init the test session
	$ret = $tstProxy->beginTest();
	assert($ret);
	$tstProxy->close();
	usleep(1000*500);		// wait 10ms

	///////////////////////////
	// step 1	
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "A<br>";
	$ret = $tstProxy->sendUInt32(0x12345678);
	assert($ret);
	echo "B<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "C<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms

	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "D<br>";
	$ret = $tstProxy->sendUInt8(0xf1);
	assert($ret);
	echo "E<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "F<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
	
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "G<br>";
	$ret = $tstProxy->sendString("hello world");
	assert($ret);
	echo "H<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "I<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
	
	////////////////////////
	// step 2
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "J<br>";
	$ret = $tstProxy->sendUInt32(0xFEDCBA98);
	assert($ret);
	echo "K<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "L<br>";
	
	$ret = $tstProxy->sendUInt8(0xCC);
	assert($ret);
	echo "M<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "N<br>";

	$ret = $tstProxy->sendString("hello world");
	assert($ret);
	echo "O<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "P<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
	
	////////////////////////
	// step 3
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "Q<br>";
	$ret = $tstProxy->composite1(0x12345678, 0x9A, "Ibm");
	assert($ret);
	echo "R<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "S<br>";

	$ret = $tstProxy->composite2("ABCD", "EFGH IJKL", "MNO", "PQ");
	assert($ret);
	echo "T<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "U<br>";
	
	$ret = $tstProxy->composite3(0x9a, 0x12345678, "ABC",
								   0x34, 0xbcdef012, "DEFGH",
								   0xcd, 0x567890ab, "I",
								   0x67, 0xef012345, "",
								   0x01234567, 0x89);
	assert($ret);
	echo "V<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "W<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
	
	///////////////////////////////
	// step 4
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "X<br>";
	$ret = $tstProxy->sendUInt32(0x12345678);
	assert($ret);
	echo "Y<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "Z<br>";
	// wait another return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "AA<br>";
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
		
	/////////////////////////////////
	// step 5
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "AB<br>";
	$ret = $tstProxy->sendUInt32(0x12345678);
	assert($ret);
	echo "AC<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "AD<br>";
	// wait another return message
	$ret = $tstProxy->waitCallback();
	assert($ret);
	echo "AE<br>";
			
	// diconnect	
	$tstProxy->close();
//	usleep(1000*200);		// wait 10ms
	
	/////////////////////////////////
	// step 6
	$tstProxy = new CTestInterfaceWeb;
	$ret = $tstProxy->connect($host, $port, $res);
	assert($ret);
	echo "AF<br>";
	$ret = $tstProxy->sendUInt32(0x12345678);
	assert($ret);
	echo "AG<br>";
	// wait the return message
	$ret = $tstProxy->waitCallback();
	assert($ret == false);
	echo "AH<br>";

	// diconnect	
	$tstProxy->close();
	
	echo "All test done<br>";
//	usleep(1000*200);		// wait 10ms
?>