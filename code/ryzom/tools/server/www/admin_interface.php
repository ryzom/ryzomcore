<?php

$server = "localhost";
$port = 80;

// import HTTP_GET_VARS as _GET if _GET doesn't exist
if (!isset($_GET) && isset($HTTP_GET_VARS))
	$_GET = &$HTTP_GET_VARS;

echo "<form method='get' action='$PHP_SELF'>\n";
echo "SHARD: <input type='text' name='shard' value='".$_GET['shard']."'><br>\n";
echo "MAILBOX: <input type='text' name='mailbox' value='".$_GET['mailbox']."'><br>\n";
echo "FORUM: <input type='text' name='forum' value='".$_GET['forum']."'><br>\n";
echo "<input type='submit' value='Retrieve'><br>\n";
echo "</form>\n";

if ($_GET["mailbox"])
{
	echo "Get mailbox ".$_GET["mailbox"]." content:<br>\n";
	$s = fsockopen($server, $port, &$errno, &$errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s,"GET /websrv/admin.php?user_login=support&shard=".$_GET["shard"]."&mailbox=".$_GET["mailbox"]." HTTP/1.0\n\n");

	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (ereg("^FILE:(.*)", $l, $reg))
			echo "<a href=\"$PHP_SELF?shard=".$_GET["shard"]."&mailbox=".$_GET["mailbox"]."&mail=".$reg[1]."\">".$reg[1]."</a><br>\n";
	}

	fclose($s);
	echo "<br><br>\n";
}

if ($_GET["mail"])
{
	echo "Get mail ".$_GET["mailbox"]."/".$_GET["mail"]." content:<br>\n";
	$s = fsockopen($server, $port, &$errno, &$errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".$_GET["shard"]."&mail=".$_GET["mail"]."&mailbox=".$_GET["mailbox"]." HTTP/1.0\n\n");

	echo "Content of mail:<br>\n";
	while(!feof($s))
		echo nl2br(htmlentities(trim(fgets($s, 2048))));

	fclose($s);
	echo "<br><br>\n";
}

if ($_GET["recover_thread"])
{
	echo "Recover thread ".$_GET["forum"]." ".$_GET["recover_thread"]."<br>\n";
	$s = fsockopen($server, $port, &$errno, &$errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".$_GET["shard"]."&recover_thread=".$_GET["forum"]."&recover_threadthread=".$_GET["recover_thread"]." HTTP/1.0\n\n");
	fclose($s);
	echo "<br><br>\n";
}

if ($_GET["forum"])
{
	echo "Get forum ".$_GET["forum"]." content:<br>\n";
	$s = fsockopen($server, $port, &$errno, &$errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".$_GET["shard"]."&forum=".$_GET["forum"]." HTTP/1.0\n\n");

	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (ereg("^FILE:(.*)", $l, $reg))
		{
			echo "<a href=\"$PHP_SELF?shard=".$_GET["shard"]."&forum=".$_GET["forum"]."&thread=".trim($reg[1])."\">".trim($reg[1])."</a>\n";
			if ($reg[1]{0} == '_')
			{
				echo " <a href=\"$PHP_SELF?shard=".$_GET["shard"]."&forum=".$_GET["forum"]."&recover_thread=".trim($reg[1])."\">recover thread</a>\n";
			}
			echo "<br>\n";
		}
	}

	fclose($s);
	echo "<br><br>\n";
}

if ($_GET["thread"])
{
	echo "Get thread ".$_GET["forum"]."/".$_GET["thread"]." content:<br>\n";
	$s = fsockopen($server, $port, &$errno, &$errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".$_GET["shard"]."&forum=".$_GET["forum"]."&thread=".$_GET["thread"]." HTTP/1.0\n\n");

	echo "Content of thread:<br>\n";
	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (ereg("^TOPIC:(.*) SUBMIT:(.*)$", $l, $reg))
			echo nl2br(htmlentities(" TOPIC:".$reg[1]."SUBMITED BY: ".$reg[2]."\n"));
		if (ereg("^AUTHOR:(.*) DATE:(.*) POST:(.*)", $l, $reg))
			echo nl2br(htmlentities("AUTHOR: ".$reg[1]." DATE:".$reg[2]." POST:".$reg[3]."\n"));
	}

	fclose($s);
	echo "<br><br>\n";
}



?>