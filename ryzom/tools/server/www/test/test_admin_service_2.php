<?php

//	$ASHost = "192.168.2.2";
	$ASPort = 46700;
//	$ASHost = "borisb";
	$ASHost = "workserver";
	require_once("../../../../code_private/nel/tools/net/admin/public_html/admin_modules_itf.php");
	
	class MyAdminService extends CAdminServiceWeb
	{
		function commandResult($serviceModuleName, $result)
		{
			echo "Service $serviceModuleName returned '$result'<br>";
		}
	}

	// connect to the AS
	$adminService = new MyAdminService;
	if ($adminService->connect($ASHost, $ASPort, $res) === false)
	{
		echo "Failed to connect to AS : '".$res."'<br>";
		die;
	}
	
	if (!isset($_GET['action']))
	{
		// no request, display the form with status
		echo "sending status request...<br>";	
		$status = $adminService->getStates();
		echo "Result : <br>";
		
		echo "<form action='test_admin_service_2.php' method='get'>";
		echo "<input type='text' name='cmd' value=''>&nbsp;<input type='submit' name='action' value='GLOBAL_CMD'><br>";
		echo "<br>";
		echo "</form>";
		
		
		foreach($status as $value)
		{
			$varsStr = explode ( "\t", $value );
			$vars = array();
			foreach($varsStr as $var)
			{
				$parts = explode("=", $var);
				$vars[$parts[0]] = $parts[1];
			}
			
			echo "<form action='test_admin_service_2.php' method='get'>";
			echo "Service '".$vars['AliasName']."' :<br>";
			echo "<li>RunningState = '".$vars['RunningState']."'<br>";
			echo "<li>RunningTags = '".$vars['RunningTags']."'<br>";
			echo "<li>Longname = '".$vars['LongName']."'<br>";
			echo "<li>Shortname = '".$vars['ShortName']."'<br>";
			echo "<li>Host = '".$vars['Hostname']."'<br>";
			echo "<li>State = '".$vars['State']."'<br>";
			echo "<li>NoReportSince = '".$vars['NoReportSince']."' (seconds)<br>";
			echo "<li>RawState = '".$value."'<br>";
			echo "<input type='hidden' name='serviceAlias' value='".$vars['AliasName']."'>&nbsp;";
			echo "<input type='submit' name='action' value='START'>&nbsp;";
			echo "<input type='submit' name='action' value='STOP' >&nbsp;";
			echo "<input type='submit' name='action' value='RESTART'>&nbsp;";
			echo "<input type='submit' name='action' value='KILL'>&nbsp;";
			echo "<input type='submit' name='action' value='ABORT'>&nbsp;";
			echo "<br>";
			echo "<input type='text' name='serviceCmd' value=''>&nbsp;<input type='submit' name='action' value='SERVICE_CMD'><br>";
			echo "<input type='text' name='controlCmd' value=''>&nbsp;<input type='submit' name='action' value='CONTROL_CMD'><br>";
//			echo "<input type='text' name='cmd' value=''>&nbsp;<input type='submit' name='action' value='GLOBAL_CMD'><br>";
			echo "<br>";
			echo "</form>";
		}
	}
	else
	{
		echo "Connection result : '$res'<br>";
		// execute the request
		switch ($_GET['action'])
		{
		case "SERVICE_CMD":
			$adminService->serviceCmd($_GET['serviceAlias'], $_GET['serviceCmd']);
			echo "Waiting result...<br>";	
			if (!$adminService->waitCallback())
				echo "Error will waiting for callback<br>";
			break;
		case "CONTROL_CMD":
			$adminService->controlCmd($_GET['serviceAlias'], $_GET['controlCmd']);
			echo "Waiting result...<br>";	
			if (!$adminService->waitCallback())
				echo "Error will waiting for callback<br>";
			break;
		case "GLOBAL_CMD":
			$adminService->globalCmd($_GET['cmd']);
			echo "Waiting result...<br>";	
			if (!$adminService->waitCallback())
				echo "Error will waiting for callback<br>";
			break;
		}
			
		// back link
		echo '<a href="test_admin_service_2.php">Back to status page</a>';
	}
	
	die;
	
	if (!isset($_GET['req']))
	{
		echo "you must supply a 'req' argument as :<br>";
		echo " <li>req=cmd&service=&lt;a service module name&gt;&cmd=&lt;your command&gt;<br>";
		echo " <li>req=ctrl&service=&lt;a service module name&gt;&cmd=&lt;your command&gt;<br>";
		echo " <li>req=glob&cmd=&lt;your command&gt;<br>";
		echo " <li>req=status<br>";
		echo " <li>req=hrg&varAddr=&lt;varAddr&gt;start=&lt;unix start date&gt;end=&lt;unix end date&gt;step=&lt;millisecond step&gt;<br>";
		die();
	}
	else if ($_GET['req'] === "status")
	{
		$adminService = new MyAdminService;
		$adminService->connect($ASHost, $ASPort, $res);
		echo "Connection result : '$res'<br>";

		echo "sending status request...<br>";	
		$status = $adminService->getStates();
		echo "Status : <br>";
		foreach($status as $value)
			echo "$value<br>";
		echo "<br>";
	}
	else if ($_GET['req'] === "cmd")
	{
		$adminService = new MyAdminService;
		$adminService->connect($ASHost, $ASPort, $res);
		echo "Connection result : '$res'<br>";

		echo "sending command...<br>";	
		$adminService->serviceCmd($_GET['service'], $_GET['cmd']);
		echo "Waiting result...<br>";	
		if (!$adminService->waitCallback())
			echo "Error will waiting for callback<br>";
	}
	else if ($_GET['req'] === "ctrl")
	{
		$adminService = new MyAdminService;
		$adminService->connect($ASHost, $ASPort, $res);
		echo "Connection result : '$res'<br>";

		echo "sending command...<br>";	
		$adminService->controlCmd($_GET['service'], $_GET['cmd']);
		echo "Waiting result...<br>";	
		if (!$adminService->waitCallback())
			echo "Error will waiting for callback<br>";
	}
	else if ($_GET['req'] === "glob")
	{
		$adminService = new MyAdminService;
		$adminService->connect($ASHost, $ASPort, $res);
		echo "Connection result : '$res'<br>";

		echo "sending command...<br>";	
		$adminService->globalCmd($_GET['cmd']);
		echo "Waiting result...<br>";	
		if (!$adminService->waitCallback())
			echo "Error will waiting for callback<br>";
	}
	else if ($_GET['req'] === "hrgraph")
	{
		$adminService = new MyAdminService;
		$adminService->connect($ASHost, $ASPort, $res);
		echo "Connection result : '$res'<br>";

		echo "sending HR graph req...<br>";	
		$ret = $adminService->getHighRezGraph($_GET['varAddr'], $_GET['start'],$_GET['end'],$_GET['step']);
		echo "Result :<br>";
		foreach($ret as $value)
			echo "$value<br>";
		echo "<br>";
	}
	else
	{
		echo "invalid request '".$_GET['req']."<br>";
	}
	echo "End of page<br>";	
?>