<?php

// Service
$db_nel = 3;
$db_nel_tool = 1;

// Support
$db_nel_ams = 1
$db_nel_ams_lib = 3;

// Domain
$db_ring_domain = 1;


function set_db_version($continue_r, $name, $version) {
	$continue = $continue_r;

	if (file_put_contents("db_version_" . $name, (string)$version)) {
	} else {
		printalert("danger", "Failed to set database version");
		$continue = false;
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
			$cfg['db'][$name]['name']);
		if (mysqli_connect_errno()) {
			printalert("danger", "Failed to connect to the <em>" . $name . "</em> SQL server: " . mysqli_connect_error());
			$con = null;
		} else {
			printalert("success", "Connected to the <em>" . $name . "</em> SQL server");
		}
	}

	return $con;
}

function disconnect_database($con, $name) {
	if ($con) {
		mysqli_close($con);
		printalert("info", "Disconnected from the <em>" . $name . "</em> SQL server");
	}
}

function upgrade_service_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "shard");
	$continue = ($con != null);
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
	for ($i = 1; $i <= $db_nel_tool; $i++) {
		if ($continue && get_db_version("tool") < $i) {
			$continue = update_database_structure($continue, $con, "nel_tool_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "tool", $i);
		}
	}
	disconnect_database($con, "tool");

	return $continue;
}

function upgrade_support_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "web");
	$continue = ($con != null);
	for ($i = 1; $i <= $db_nel_ams; $i++) {
		if ($continue && get_db_version("web") < $i) {
			$continue = update_database_structure($continue, $con, "nel_ams_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "web", $i);
		}
	}
	disconnect_database($con, "web");

	$con = null;
	$con = connect_database($continue, "lib");
	$continue = ($con != null);
	for ($i = 1; $i <= $db_nel_ams_lib; $i++) {
		if ($continue && get_db_version("lib") < $i) {
			$continue = update_database_structure($continue, $con, "nel_ams_lib_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "lib", $i);
		}
	}
	disconnect_database($con, "lib");

	return $continue;
}

function upgrade_domain_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "ring");
	$continue = ($con != null);
	for ($i = 1; $i <= $db_ring_domain; $i++) {
		if ($continue && get_db_version("ring") < $i) {
			$continue = update_database_structure($continue, $con, "ring_domain_" . str_pad($i, 5, "0", STR_PAD_LEFT) . ".sql");
			$continue = set_db_version($continue, "ring", $i);
		}
	}
	disconnect_database($con, "ring");

	return $continue;
}

?>
