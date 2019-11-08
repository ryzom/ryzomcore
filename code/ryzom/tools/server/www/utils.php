<?php

if (!isset($UTILS_PHP))
{

// import HTTP_GET_VARS as _GET if _GET doesn't exist
if (!isset($_GET) && isset($HTTP_GET_VARS))
	$_GET = &$HTTP_GET_VARS;

// import HTTP_POST_VARS as _GET if _POST doesn't exist
if (!isset($_POST) && isset($HTTP_POST_VARS))
	$_POST = &$HTTP_POST_VARS;

$allowCookies = false;

// import HTTP_POST_VARS as _GET if _POST doesn't exist
if ($allowCookies && !isset($_COOKIE) && isset($HTTP_COOKIE_VARS))
	$_COOKIE = &$HTTP_COOKIE_VARS;

// -------------------------------------
// import HTTP param (from a GET or POST, or left if already set...)
// -------------------------------------
function importParam($var, $secureValue = true)
{
	if (!$secureValue && isset($GLOBALS[$var]))
		return;

	global	$allowCookies, $_GET, $_POST, $_COOKIE;

	if (isset($_GET[$var]))
		$GLOBALS[$var] = $_GET[$var];
	else if (isset($_POST[$var]))
		$GLOBALS[$var] = $_POST[$var];
	else if ($allowCookies && isset($_COOKIE[$var]))
		$GLOBALS[$var] = $_COOKIE[$var];
	else
	{
		//die("Missing arg '$var'");
		unset($GLOBALS[$var]);
	}
}

// -------------------------------------
// export HTTP param, as if user sent var in a POST
// -------------------------------------
function exportParam($var, $value)
{
	global	$_POST;
	$_POST[$var] = $value;
}


// always sent by client
importParam('user_login');
importParam('shard');
importParam('session_cookie');

$user_login = trim($user_login);
$session_cookie = stripslashes($session_cookie);

include_once('config.php');

$UTILS_PHP = 1;

// -------------------------------------
// read an index
// -------------------------------------
function read_index($file, &$header, &$array)
{
	if (!file_exists($file))
	{
		$header = '';
		$array = array();
		return;
	}

	$f = fopen($file, 'r');

	// read header
	$header = fgets($f, 512);
	
	while (!feof($f))
	{
		// get a new line
		$line = trim(fgets($f, 10240));

		// check line is not empty
		if ($line == "")
			continue;

		// explode line
		$array[] = explode('%%', $line);
	}

	fclose($f);
}

// -------------------------------------
// write an index
// -------------------------------------
function write_index($file, $header, &$array)
{
	$f = fopen($file, 'w');

	fwrite($f, str_pad(trim($header), 256));
	fwrite($f, "\n");

	if (count($array) > 0)
	{
		foreach ($array as $l)
		{
			fwrite($f, trim(join('%%', $l)));
			fwrite($f, "\n");
		}
	}

	fclose($f);
}


// -------------------------------------
// write an index
// -------------------------------------
function use_index($file)
{
	if (!file_exists($file))
	{
		$f = fopen($file, 'w');
		update_next_index($f, 0);
		fclose($f);
	}
}



// -------------------------------------
// write an index
// -------------------------------------
function read_next_index($f, &$index)
{
	fseek($f, 0, SEEK_SET);
	$index = (int)fgets($f, 512);
}

// -------------------------------------
// write an index
// -------------------------------------
function update_next_index($f, $index)
{
	fseek($f, 0, SEEK_SET);

	fwrite($f, str_pad($index, 256));
	fwrite($f, "\n");
}

// -------------------------------------
// write an index
// -------------------------------------
function append_to_index($f, $line)
{
	fseek($f, 0, SEEK_END);

	fwrite($f, $line, 8192);
	fwrite($f, "\n");
}




// -------------------------------------
// write html prolog
// -------------------------------------
function write_prolog($f, $title)
{
	fwrite($f, "<html><head><title>$title</title></head><body>\n");
}

// -------------------------------------
// write html epilog
// -------------------------------------
function write_epilog($f)
{
	fwrite($f, "</body></html>\n");
}




// -------------------------------------
// read template file
// -------------------------------------
function read_template($file, &$template)
{
	global	$TEMPLATE_DIR;
	$filename = $TEMPLATE_DIR.'/'.$file;
	$f = fopen($filename, 'r');
	$template = fread($f, filesize($filename));
	fclose($f);
}



// -------------------------------------
// redirect
// -------------------------------------
function redirect($url, $time=0)
{
	echo "<html><head><title>Redirecting...</title>\n";
	echo "<meta http-equiv='refresh' content='$time; URL=$url'>\n";
	echo "</head>\n";
	echo "<body></body></html>\n";
}



// -------------------------------------
// convert to forum name
// -------------------------------------
function convert_forum_name($str)
{
	return ucfirst(strtr($str, '_', ' '));
}


// -------------------------------------
// clean string
// -------------------------------------
function clean_string($str)
{
	return strtr($str, array("\n" => '', '%' => '\%'));
}

// -------------------------------------
// clean content
// -------------------------------------
function clean_content($str)
{
	return strtr($str, array("\n" => '\n', '%' => '\%'));
}

// -------------------------------------
// displayable string
// -------------------------------------
function displayable_string($str)
{
	return nl2br(htmlspecialchars(stripslashes($str), ENT_QUOTES));
}

// -------------------------------------
// displayable string
// -------------------------------------
function displayable_content($str)
{
	return htmlspecialchars(stripcslashes($str), ENT_QUOTES);
}

// -------------------------------------
// displayable string
// -------------------------------------
function displayable_date()
{
	return "<i>date#".date("y/m/d")."</i> ".date("H:i");
}





function matchParam($var, $param, &$value)
{
	$plen = strlen($param);
	if (!strncmp($param, $var, $plen))
	{
		$value = trim(substr($var, $plen));
		return true;
	}
	return false;
}


function nameToFile($name)
{
	$r = '';
	for ($i=0; $i<strlen($name); ++$i)
	{
		if ($name[$i] == ' ')
			$r .= '_';
		else if ($name[$i] == '%' || $name[$i] <= chr(32) || $name[$i] >= chr(127))
			$r .= sprintf("%%%02x", ord($name[$i]));
		else
			$r .= $name[$i];
	}
	return $r;
}

function nameToURL($name)
{
	$r = '';
	for ($i=0; $i<strlen($name); ++$i)
	{
		if ($name[$i] == ' ')
			$r .= '%20';
		else
			$r .= $name[$i];
	}
	return $r;
}

function nameFromURL($name)
{
	return rawurldecode($name);
}

function fileToName($file)
{
	$n = '';
	for ($p=0; $p<strlen($file); ++$p)
	{
		if ($file[$p] == '%' && $file[$p+1] != '%')
		{
			$b = $file[++$p];
			$b .= $file[++$p];
			list($c) = sscanf($b, "%x");
			$n .= chr($c);
		}
		else if ($file[$p] == '_')
		{
			$n .= ' ';
		}
		else
		{
			$n .= $file[$p];
		}
	}
	return $n;
}


// -------------------------------------
// get user home directory
// -------------------------------------
function get_user_dir($user, $shard)
{
	if ($user == "" || $shard == "")
		die("INTERNAL ERROR CODE 1");
		
	global	$USERS_DIR;
	
	$user = nameToFile($user);

	return $USERS_DIR.'/'.strtolower($shard).'/'.substr(strtolower($user), 0, 2).'/'.strtolower($user).'/';
}

// -------------------------------------
// build user home directory
// -------------------------------------
function build_user_dir($user, $shard)
{
	$dir = get_user_dir($user, $shard);

	$p = 0;
	while (!is_dir($dir))
	{
		$p = strpos($dir, '/', $p+1);
		if ($p == 0)
		{
			die("INTERNAL ERROR CODE 3");
		}
		else
		{
			$interm = substr($dir, 0, $p);
			if (!is_dir($interm))
			{
				if (!mkdir($interm, 0777))
					die("INTERNAL ERROR CODE 3");
			}
		}
	}
	return $dir;
}

include_once('login/config.php');

// -------------------------------------
// connect to DB server and select ring DB
// -------------------------------------
function connect_to_ring_db()
{
	global $DBHost, $RingDBUserName, $RingDBPassword, $RingDBName;
	$ringDb = mysql_connect($DBHost, $RingDBUserName, $RingDBPassword) or die("can't connect to ring db @'".$DBHost."' with user '".$RingDBUserName."'");
	mysql_select_db($RingDBName, $ringDb) or die("can't select ring db: '$RingDBName' Host=$DBHost User=$RingDBUserName (not enough privilege?)");
	return $ringDb;
}

// -------------------------------------
// user_login=character_name => ring_live.characters[user_login].guild_id
// test ring_live.guilds[guild_id].guild_name == $forum
// -------------------------------------
function check_character_belongs_to_guild($charName, $guildName)
{
	connect_to_ring_db();
	$res = mysql_query(
	"SELECT guilds.guild_name FROM guilds
	JOIN characters ON characters.guild_id=guilds.guild_id
	WHERE char_name='$charName'")
		or die("Can't query guild for $charName in DB");
	if (false === ($row = mysql_fetch_row($res)))
		die("Guild not found for char $charName in DB");
	if ($row[0] != $guildName)
		die("ACCESS DENIED: $charName is not a member of $guildName");
}

$remote_addr = $HTTP_SERVER_VARS['REMOTE_ADDR'];
if ($remote_addr == "213.208.119.226" || $remote_addr == "38.117.236.132")
{
	importParam('internal_check');
	if ($internal_check)
	{
		echo "INTERNAL CHECK\n";
		die("$internal_check:1");
	}
}


/*
 * check user is valid
 */
if ($user_login == "support" && ($remote_addr == "192.168.1.153" || $remote_addr == "192.168.3.1") ||
	$remote_addr == "127.0.0.1" )
{ 
	echo "SUPPORT MODE!";
	// do not check "support" email that come from rsweb 
	//echo $HTTP_SERVER_VARS['REMOTE_ADDR']; 
	//die();
	importParam('translate_user_login');
	if ($translate_user_login)
		$user_login = $translate_user_login;
} 
else 
{
    if (!strstr($HTTP_SERVER_VARS['HTTP_USER_AGENT'], 'Ryzom'))
        die("ERROR: Bad parameters");
	$udir = get_user_dir($user_login, $shard); 
	$ufile = $udir.'session'; 
	if (is_dir($udir) && file_exists($ufile)) 
	{ 
		$file = fopen($ufile, 'r'); 
		if (!$file) 
			die("ERROR: Not logged"); 
		$server_cookie = trim(fgets($file, 1024)); 
		if ($server_cookie != $session_cookie) 
			die("ERROR: Authentication failed"); 
	} 
	else 
	{ 
		die("ERROR: Directory not found: ".$udir); 
	} 
} 
 
}
?>