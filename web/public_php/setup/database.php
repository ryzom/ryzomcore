<?php

// php 8.1 defaults mysqli to exception reporting; the helpers below check
// mysqli_connect_errno()/return values themselves, keep the classic mode
if (function_exists('mysqli_report'))
	mysqli_report(MYSQLI_REPORT_OFF);

// Service
$db_nel = 8;
$db_nel_tool = 3;

// Domain
$db_ring_domain = 3;


function set_db_version($continue_r, $name, $version) {
	$continue = $continue_r;

	if ($continue) {
		if (file_put_contents("db_version_" . $name, (string)$version)) {
		} else {
			printalert("danger", "Failed to set database version");
			$continue = false;
		}
	}

	return $continue;
}

function get_db_version($name) {
	if (!file_exists("db_version_" . $name)) {
		return 0;
	}
	return (int)file_get_contents("db_version_" . $name);
}

function connect_database($continue, $name) {
	$con = null;

	global $cfg;
	if ($continue) {
		$con = mysqli_connect(
			$cfg['db'][$name]['host'],
			$cfg['db'][$name]['user'],
			$cfg['db'][$name]['pass'],
			$cfg['db'][$name]['name'],
			$cfg['db'][$name]['port']);
		if (mysqli_connect_errno()) {
			printalert("danger", "Failed to connect to the <em>" . htmlentities($name) . "</em> SQL server: " . htmlentities(mysqli_connect_error()));
			$con = null;
		} else {
			printalert("success", "Connected to the <em>" . htmlentities($name) . "</em> SQL server");
		}
	}

	return $con;
}

function disconnect_database($con, $name) {
	if ($con) {
		mysqli_close($con);
		printalert("info", "Disconnected from the <em>" . htmlentities($name) . "</em> SQL server");
	}
}

function upgrade_service_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "shard");
	$continue = ($con != null);
	global $db_nel;
	for ($i = 1; $i <= $db_nel; $i++) {
		if ($continue && get_db_version("shard") < $i) {
			$continue = update_database_structure($continue, $con, "nel_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "shard", $i);
		}
	}
	disconnect_database($con, "shard");

	$con = null;
	$con = connect_database($continue, "tool");
	$continue = ($con != null);
	global $db_nel_tool;
	for ($i = 1; $i <= $db_nel_tool; $i++) {
		if ($continue && get_db_version("tool") < $i) {
			$continue = update_database_structure($continue, $con, "nel_tool_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "tool", $i);
		}
	}
	disconnect_database($con, "tool");

	return $continue;
}

function upgrade_domain_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "ring");
	$continue = ($con != null);
	global $db_ring_domain;
	for ($i = 1; $i <= $db_ring_domain; $i++) {
		if ($continue && get_db_version("ring") < $i) {
			$continue = update_database_structure($continue, $con, "ring_domain_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "ring", $i);
		}
	}
	disconnect_database($con, "ring");

	return $continue;
}

function configure_shard_dev($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "ring");
	$continue = ($con != null);
	$continue = update_database_configure($continue, $con, "configure_shard_dev.sql");
	disconnect_database($con, "ring");

	return $continue;
}

// The administrator account is the first game account of a fresh install: a
// row in the nel user table with the :DEV: privilege, made by the account
// tool's own code (tools/account_common.php), so it is an account like any
// registered one. Registration is closed by default and only a :DEV: account
// can open it, so without this account the first one has to be written into
// the database by hand.

// What is wrong with the fields of the administrator account, as a message,
// or '' when they can be used. Does not touch the database: the installers
// ask before they create anything, since an install cannot be taken back
// once config.php is written.
function admin_account_error($login, $password, $email) {
	require_once(dirname(__DIR__) . '/tools/account_common.php');
	// the installers ask for the password once
	return accountNewFieldsError($login, $email, $password, $password);
}

// Create the administrator account. Install only, never part of an upgrade.
// An account that already has the login or the e-mail address is left as it
// is, and the step fails. The account gets a permission row for every domain
// that exists by now, which is the dev domain when the development shard was
// configured. A new production install has no domain yet, so the account can
// sign in to the account tool and cannot enter a shard: a domain that is
// added later needs its own permission row (admin_account_notes).
function create_admin_account($continue_r, $login, $password, $email) {
	$continue = $continue_r;

	global $cfg;
	if ($continue) {
		require_once(dirname(__DIR__) . '/tools/account_common.php');
		try {
			$db = getDatabase(
				$cfg['db']['shard']['host'],
				$cfg['db']['shard']['port'],
				$cfg['db']['shard']['user'],
				$cfg['db']['shard']['pass'],
				$cfg['db']['shard']['name']);
			$created = accountCreate($db, $login, $email, $password, $password,
				':DEV:', array('ds_open', 'ds_dev', 'ds_restricted'));
			if ($created['error'] !== '') {
				printalert("danger", "Administrator account <em>" . htmlentities($login) . "</em> not created: " . htmlentities($created['error']));
				$continue = false;
			} else {
				printalert("success", "Administrator account <em>" . htmlentities($login) . "</em> (" . htmlentities($email) . ") created with the :DEV: privilege");
			}
		} catch (PDOException $e) {
			printalert("danger", "Failed to create the administrator account <em>" . htmlentities($login) . "</em>: " . htmlentities($e->getMessage()));
			$continue = false;
		}
	}

	return $continue;
}

// configure_shard_dev.sql names the databases it writes to itself, so the
// development shard can only be configured into an install that uses those
// names. Returns what does not match, one entry per database, none when the
// names fit.
function shard_dev_database_problems($nelDatabase, $toolDatabase, $domainDatabase) {
	$fixed = array(
		array("NeL database", $nelDatabase, "nel"),
		array("NeL tools database", $toolDatabase, "nel_tool"),
		array("domain database", $domainDatabase, "ring_dev"),
	);
	$problems = array();
	foreach ($fixed as $entry) {
		if ($entry[1] !== $entry[2]) {
			$problems[] = "the " . $entry[0] . " has to be '" . $entry[2] . "' and not '" . $entry[1] . "'";
		}
	}
	return $problems;
}

// What has to be undone before an install that failed after it wrote
// config.php can be run again, as plain text.
function failed_install_notes($databases) {
	return "config.php was written before the install failed, so the installer answers \"Already installed\" from now on. "
		. "Before running the install again: fix what failed, drop the databases of this install (" . implode(", ", $databases) . "), "
		. "and delete config.php and the db_version_* and role_* files from the public PHP directory.";
}

// What the administrator account is good for once the install is done, for
// the last lines of both installers: plain text, one line per entry, the
// statement on a line of its own.
function admin_account_notes($login, $nelDatabase) {
	return array(
		"Sign in with it in the account tool (account/). Its Dev page opens the registration for everybody else.",
		"In the game it can enter the shards of the domains that existed at install time: the development shard when this install configured one, and none on a new production install, where the domains are still to be added.",
		"For a domain that is added later, any account gets its permission row with one statement in the '" . $nelDatabase . "' database, with OPEN for a domain whose status is ds_open, DEV for ds_dev and RESTRICTED for ds_restricted. Other accounts can also be given the row on the admin page of the account tool. For this account:",
		"INSERT INTO permission (UId, DomainId, AccessPrivilege) VALUES ((SELECT UId FROM user WHERE Login = '" . $login . "'), <domain_id>, 'OPEN');",
	);
}

// These lived in header.php, which is the HTML page shell: the CLI
// installer includes this file without it and died on the undefined
// function the first time a migration had to run.
function update_database_structure($continue_r, $con, $file) {
	$continue = $continue_r;
	global $PRIVATE_PHP_PATH;
	if ($continue) {
		$sql = file_get_contents($PRIVATE_PHP_PATH . "/setup/sql/" . $file);
		if (!$sql) {
			printalert("danger", "Cannot read <em>" . htmlentities($file) . "</em>");
			$continue = false;
		} else {
			printalert("info", "Running script <em>" . htmlentities($file) . "</em>");
			if (mysqli_multi_query($con, $sql)) {
				printalert("success", "Database structure updated using <em>" . htmlentities($file) . "</em>");
				while (mysqli_more_results($con) && mysqli_next_result($con)) {
					// no-op
				}
			} else {
				printalert("danger", "Error updating database using <em>" . htmlentities($file) . "</em>: " . htmlentities(mysqli_error($con)));
				$continue = false;
			}
		}
	}
	return $continue;
}
function update_database_configure($continue_r, $con, $file) {
	$continue = $continue_r;
	global $PRIVATE_PHP_PATH;
	if ($continue) {
		$sql = file_get_contents($PRIVATE_PHP_PATH . "/setup/sql/" . $file);
		$sql = str_replace('%RC_HOSTNAME%', mysqli_real_escape_string($con, gethostname()), $sql);
		$shardDevDir = str_replace('/www', '', str_replace('\\', '/', $_POST["domainUsersDir"]));
		$sql = str_replace('%RC_SHARD_DEV%', mysqli_real_escape_string($con, $shardDevDir), $sql);
		if (!$sql) {
			printalert("danger", "Cannot read <em>" . htmlentities($file) . "</em>");
			$continue = false;
		} else {
			if (mysqli_multi_query($con, $sql)) {
				printalert("success", "Database updated using <em>" . htmlentities($file) . "</em>");
				while (mysqli_more_results($con) && mysqli_next_result($con)) {
					// no-op
				}
			} else {
				printalert("danger", "Error updating database using <em>" . htmlentities($file) . "</em>: " . htmlentities(mysqli_error($con)));
				$continue = false;
			}
		}
	}
	return $continue;
}

?>
