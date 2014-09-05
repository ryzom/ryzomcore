<?php

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
	if ($continue && get_db_version("shard") < 1) {
		$continue = update_database_structure($continue, $con, "nel_00001.sql");
		$continue = set_db_version($continue, "shard", 1);
	}
	disconnect_database($con, "shard");

	$con = null;
	$con = connect_database($continue, "tool");
	$continue = ($con != null);
	if ($continue && get_db_version("tool") < 1) {
		$continue = update_database_structure($continue, $con, "nel_tool_00001.sql");
		$continue = set_db_version($continue, "tool", 1);
	}
	disconnect_database($con, "tool");

	return $continue;
}

function upgrade_support_databases($continue_r) {
	$continue = $continue_r;

	$con = null;
	$con = connect_database($continue, "web");
	$continue = ($con != null);
	if ($continue && get_db_version("web") < 1) {
		$continue = update_database_structure($continue, $con, "nel_ams_00001.sql");
		$continue = set_db_version($continue, "web", 1);
	}
	disconnect_database($con, "web");

	$con = null;
	$con = connect_database($continue, "lib");
	$continue = ($con != null);
	if ($continue && get_db_version("lib") < 1) {
		$continue = update_database_structure($continue, $con, "nel_ams_lib_00001.sql");
		$continue = set_db_version($continue, "lib", 1);
	}
	if ($continue && get_db_version("lib") < 2) {
		$continue = update_database_structure($continue, $con, "nel_ams_lib_00002.sql");
		$continue = set_db_version($continue, "lib", 2);
	}
	disconnect_database($con, "lib");

	return $continue;
}

function upgrade_domain_databases($continue_r) {
	$continue = $continue_r;

	return $continue;
}

?>
