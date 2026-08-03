<?php

// Ryzom Core - MMORPG Framework <http://ryzom.dev/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This page used to proxy as user_login=support with no authentication of
// its own. admin.php is currently hard-disabled, but keep this endpoint
// closed as well so re-enabling the backend does not open it again.
die("Access denied");

$server = "localhost";
$port = 80;

// import HTTP_GET_VARS as _GET if _GET doesn't exist
if (!isset($_GET) && isset($HTTP_GET_VARS))
	$_GET = &$HTTP_GET_VARS;

// Every value below arrives with the request and is both written into this
// page and put on the wire towards admin.php. Escape it for html on the way
// out, and url encode it on the way in -- a raw newline in one of them would
// otherwise let the caller append headers or a second request of their own.
function ai_get($name)
{
	return isset($_GET[$name]) && is_string($_GET[$name]) ? $_GET[$name] : '';
}

function ai_html($value)
{
	return htmlspecialchars($value, ENT_QUOTES);
}

function ai_query($value)
{
	return rawurlencode($value);
}

$self = ai_html($_SERVER['PHP_SELF']);

$g_shard   = ai_get('shard');
$g_mailbox = ai_get('mailbox');
$g_forum   = ai_get('forum');
$g_mail    = ai_get('mail');
$g_thread  = ai_get('thread');
$g_recover = ai_get('recover_thread');

echo "<form method='get' action='$self'>\n";
echo "SHARD: <input type='text' name='shard' value='".ai_html($g_shard)."'><br>\n";
echo "MAILBOX: <input type='text' name='mailbox' value='".ai_html($g_mailbox)."'><br>\n";
echo "FORUM: <input type='text' name='forum' value='".ai_html($g_forum)."'><br>\n";
echo "<input type='submit' value='Retrieve'><br>\n";
echo "</form>\n";

if ($g_mailbox != '')
{
	echo "Get mailbox ".ai_html($g_mailbox)." content:<br>\n";
	$s = fsockopen($server, $port, $errno, $errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s,"GET /websrv/admin.php?user_login=support&shard=".ai_query($g_shard)."&mailbox=".ai_query($g_mailbox)." HTTP/1.0\r\n\r\n");

	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (preg_match('/^FILE:(.*)/', $l, $reg))
			echo "<a href=\"$self?shard=".ai_html(ai_query($g_shard))."&mailbox=".ai_html(ai_query($g_mailbox))."&mail=".ai_html(ai_query($reg[1]))."\">".ai_html($reg[1])."</a><br>\n";
	}

	fclose($s);
	echo "<br><br>\n";
}

if ($g_mail != '')
{
	echo "Get mail ".ai_html($g_mailbox)."/".ai_html($g_mail)." content:<br>\n";
	$s = fsockopen($server, $port, $errno, $errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".ai_query($g_shard)."&mail=".ai_query($g_mail)."&mailbox=".ai_query($g_mailbox)." HTTP/1.0\r\n\r\n");

	echo "Content of mail:<br>\n";
	while(!feof($s))
		echo nl2br(htmlentities(trim(fgets($s, 2048))));

	fclose($s);
	echo "<br><br>\n";
}

if ($g_recover != '')
{
	echo "Recover thread ".ai_html($g_forum)." ".ai_html($g_recover)."<br>\n";
	$s = fsockopen($server, $port, $errno, $errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".ai_query($g_shard)."&recover_thread=".ai_query($g_forum)."&recover_threadthread=".ai_query($g_recover)." HTTP/1.0\r\n\r\n");
	fclose($s);
	echo "<br><br>\n";
}

if ($g_forum != '')
{
	echo "Get forum ".ai_html($g_forum)." content:<br>\n";
	$s = fsockopen($server, $port, $errno, $errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".ai_query($g_shard)."&forum=".ai_query($g_forum)." HTTP/1.0\r\n\r\n");

	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (preg_match('/^FILE:(.*)/', $l, $reg))
		{
			$file = trim($reg[1]);
			echo "<a href=\"$self?shard=".ai_html(ai_query($g_shard))."&forum=".ai_html(ai_query($g_forum))."&thread=".ai_html(ai_query($file))."\">".ai_html($file)."</a>\n";
			if ($file != '' && $file[0] == '_')
			{
				echo " <a href=\"$self?shard=".ai_html(ai_query($g_shard))."&forum=".ai_html(ai_query($g_forum))."&recover_thread=".ai_html(ai_query($file))."\">recover thread</a>\n";
			}
			echo "<br>\n";
		}
	}

	fclose($s);
	echo "<br><br>\n";
}

if ($g_thread != '')
{
	echo "Get thread ".ai_html($g_forum)."/".ai_html($g_thread)." content:<br>\n";
	$s = fsockopen($server, $port, $errno, $errstr, 30) or die ("ERROR: can't connect to $server:$port");
	fputs($s, "GET /websrv/admin.php?user_login=support&shard=".ai_query($g_shard)."&forum=".ai_query($g_forum)."&thread=".ai_query($g_thread)." HTTP/1.0\r\n\r\n");

	echo "Content of thread:<br>\n";
	while(!feof($s))
	{
		$l = trim(fgets($s, 2048));
		if (preg_match('/^TOPIC:(.*) SUBMIT:(.*)$/', $l, $reg))
			echo nl2br(htmlentities(" TOPIC:".$reg[1]."SUBMITED BY: ".$reg[2]."\n"));
		if (preg_match('/^AUTHOR:(.*) DATE:(.*) POST:(.*)/', $l, $reg))
			echo nl2br(htmlentities("AUTHOR: ".$reg[1]." DATE:".$reg[2]." POST:".$reg[3]."\n"));
	}

	fclose($s);
	echo "<br><br>\n";
}



?>