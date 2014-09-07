<?php

$NEL_SETUP_SESSION = true;
if (file_exists( '../config.php')) {
	session_start();
	if ((!isset($_SESSION['nelSetupAuthenticated'])) || $_SESSION['nelSetupAuthenticated'] != 1) {
		if (basename($_SERVER["SCRIPT_NAME"]) != "auth.php") {
			header("Cache-Control: max-age=1");
			header('Location: auth.php', true, 303);
			throw new SystemExit();
		}
	}
} else if (basename($_SERVER["SCRIPT_NAME"]) != "install.php") {
	header("Cache-Control: max-age=1");
	header('Location: install.php', true, 303);
	throw new SystemExit();
}

?><!DOCTYPE html>
<html lang="en">
	<head>
		<meta charset="utf-8">
		<meta http-equiv="X-UA-Compatible" content="IE=edge">
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<title>Ryzom Core | <?php print(htmlentities($pageTitle)); ?></title>
		<link href="css/bootstrap.min.css" rel="stylesheet">
	</head>

<?php
function printalert($type, $message) {
	print '<div class="alert alert-' . $type . '" role="alert">';
	print $message;
	print '</div>';
}
function is__writable($path) {
	if ($path{strlen($path) - 1} == '/') {
		return is__writable($path.uniqid(mt_rand()).'.tmp');
	}

	if (file_exists($path)) {
		if (!($f = @fopen($path, 'r+'))) {
			return false;
		}
		fclose($f);
		return true;
	}

	if (!($f = @fopen($path, 'w'))) {
		return false;
	}
	fclose($f);
	unlink($path);
	return true;
}
function validate_writable($continue, $path) {
	if (!is__writable($path)) {
		printalert("danger", "Failed to get write permissions on " . htmlentities($path));
		return false;
	}
	return $continue;
}
function create_use_database($continue_r, $con, $database) {
	$continue = $continue_r;
	if ($continue) {
		$sql = "CREATE DATABASE `" . mysqli_real_escape_string($con, $database) . "` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;";
		if (mysqli_query($con, $sql)) {
			printalert("success", "Database <em>" . $database . "</em> created");
		} else {
			printalert("danger", "Error creating <em>" . $database . "</em> database: " . mysqli_error($con));
			$continue = false;
		}
	}
	if ($continue) {
		$sql = "USE `" . mysqli_real_escape_string($con, $database) . "`;";
		if (mysqli_query($con, $sql)) {
			printalert("success", "Database <em>" . $database . "</em> selected");
		} else {
			printalert("danger", "Error selecting <em>" . $database . "</em> database: " . mysqli_error($con));
			$continue = false;
		}
	}
	return $continue;
}
function update_database_structure($continue_r, $con, $file) {
	$continue = $continue_r;
	if ($continue) {
		$sql = file_get_contents($_POST["privatePhpDirectory"] . "/setup/sql/" . $file);
		if (!$sql) {
			printalert("danger", "Cannot read <em>" . $file . "</em>");
			$continue = false;
		} else {
			if (mysqli_multi_query($con, $sql)) {
				printalert("success", "Database structure updated using <em>" . $file . "</em>");
				while (mysqli_more_results($con) && mysqli_next_result($con)) {
					// no-op
				}
			} else {
				printalert("danger", "Error updating database using <em>" . $file . "</em>: " . mysqli_error($con));
				$continue = false;
			}
		}
	}
	return $continue;
}
?>

	<body>
		<div style="margin-left: auto; margin-right: auto; padding-left: 24px; padding-right: 24px; padding-bottom: 24px; max-width: 1024px;">

			<div class="page-header">
				<h1>Ryzom Core <small><?php print(htmlentities($pageTitle)); ?></small></h1>
			</div>

<?php

$continue = true;

// Change to root directory
if (!chdir("../")) {
	printalert("danger", "Cannot change to public PHP root directory");
	$continue = false;
}

?>

<!-- --------------------------------------------------------------- -->
