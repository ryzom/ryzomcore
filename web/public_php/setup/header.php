<?php

$NEL_SETUP_SESSION = true;

// This session is what stands between a visitor and the installer. Keep the
// cookie out of reach of page script and off cross site posts, and ask for
// the secure flag when the request arrived over tls.
ini_set('session.cookie_httponly', '1');
ini_set('session.use_only_cookies', '1');
ini_set('session.cookie_samesite', 'Lax');
if ((isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== '' && strtolower($_SERVER['HTTPS']) !== 'off')
	|| (isset($_SERVER['SERVER_PORT']) && $_SERVER['SERVER_PORT'] == 443)) {
	ini_set('session.cookie_secure', '1');
}

// Operators can drop public_php/setup.disabled next to the generated
// config.php to lock the installer and upgrade UI after go-live. The
// setup password alone is not enough when the password is weak or shared.
if (file_exists(dirname(__DIR__) . '/setup.disabled')) {
	header('HTTP/1.1 403 Forbidden');
	header('Content-Type: text/plain; charset=utf-8');
	echo "Setup is disabled on this host.\n";
	throw new SystemExit();
}

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
	// Bootstrap alert class only; refuse anything else so a caller cannot
	// open an attribute. Message is trusted to already contain intended
	// htmlentities() fragments from the install steps.
	$allowed = array('success', 'info', 'warning', 'danger');
	if (!in_array($type, $allowed, true))
		$type = 'info';
	print '<div class="alert alert-' . $type . '" role="alert">';
	print $message;
	print '</div>';
}
function is__writable($path) {
	if ($path[strlen($path) - 1] == '/' || $path[strlen($path) - 1] == '\\') {
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
		$sql = "CREATE DATABASE `" . mysqli_real_escape_string($con, $database) . "` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;";
		if (mysqli_query($con, $sql)) {
			printalert("success", "Database <em>" . htmlentities($database) . "</em> created");
		} else {
			printalert("danger", "Error creating <em>" . htmlentities($database) . "</em> database: " . htmlentities(mysqli_error($con)));
			$continue = false;
		}
	}
	if ($continue) {
		$sql = "USE `" . mysqli_real_escape_string($con, $database) . "`;";
		if (mysqli_query($con, $sql)) {
			printalert("success", "Database <em>" . htmlentities($database) . "</em> selected");
		} else {
			printalert("danger", "Error selecting <em>" . htmlentities($database) . "</em> database: " . htmlentities(mysqli_error($con)));
			$continue = false;
		}
	}
	return $continue;
}
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
