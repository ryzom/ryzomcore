<?php

// This file contains all variables needed by other php scripts

require_once dirname(__DIR__).'/config.php';

// !! IMPORTANT !! keep these outside public webroot
$LogRelativePath = '../../logs/';

// ----------------------------------------------------------------------------------------
// Variables for nel database access
// ----------------------------------------------------------------------------------------

if ($cfg['db']['shard']['host'] !=  $cfg['db']['ring']['host'])
	throw new Exception("Invalid configuration");

// where we can find the mysql database
$DBHost         = $cfg['db']['shard']['host'];
$DBPort         = $cfg['db']['shard']['port'];
$DBUserName     = $cfg['db']['shard']['user'];
$DBPassword     = $cfg['db']['shard']['pass'];
$DBName         = $cfg['db']['shard']['name'];

$RingDBUserName = $cfg['db']['ring']['user'];
$RingDBPassword = $cfg['db']['ring']['pass'];
$RingDBName     = $cfg['db']['ring']['name'];

// If true, the server will add automatically unknown user in the database
// (in nel.user, nel.permission, ring.ring_user and ring.characters
$AcceptUnknownUser = $ALLOW_UNKNOWN;
// If true, the login service automaticaly create a ring user and a editor character if needed
$AutoCreateRingInfo = $CREATE_RING;

// If true, clients that request &dbg=1 (development builds always do)
// receive the debug variant of the login error messages: the failing
// query, database host and user, and raw service result codes. Keep it
// off on any deployment reachable by players. Accounts with the :DEV:
// privilege get the debug detail without this switch for any error
// after their password has been verified; the switch only adds the
// pre-authentication errors, which anyone can trigger.
if (!isset($LoginAllowDbg))
	$LoginAllowDbg = false;

// stats_query.php: optional shared secret (query param or header). When set,
// a matching token grants access without relying on the private-network regex.
if (!isset($StatsQuerySecret))
	$StatsQuerySecret = isset($cfg['stats_query']['secret']) ? $cfg['stats_query']['secret'] : '';
// stats_query.php / stats.php die2(): regex matched against REMOTE_ADDR.
// Override in config_user.php for your LAN; default keeps the historical
// 192.168.1.* allowlist used by the installer stats viewer.
if (!isset($StatsPrivateNetwork))
	$StatsPrivateNetwork = isset($cfg['stats_query']['private_network'])
		? $cfg['stats_query']['private_network']
		: '/^192\\.168\\.1\\./';

/*
 * Connect with utf8mb4 so mysqli_real_escape_string and multi-byte
 * input agree. Call after every mysqli_connect in the login/ring stack.
 */
function nel_mysqli_set_charset($link)
{
	if ($link)
		@mysqli_set_charset($link, 'utf8mb4');
	return $link;
}

?>
