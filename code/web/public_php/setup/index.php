<!DOCTYPE html>
<html lang="en">
	<head>
		<meta charset="utf-8">
		<meta http-equiv="X-UA-Compatible" content="IE=edge">
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<title>Ryzom Core | Setup</title>
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
				while (mysqli_next_result($con)) {
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
				<h1>Ryzom Core <small>Setup</small></h1>
			</div>

<?php if (file_exists( '../is_installed')) { ?>

			<div class="alert alert-info" role="alert">
				Already installed. Upgrading not available yet.
			</div>

<?php } else if ($_POST) { ?>

			<div class="alert alert-info" role="alert">
				<?php var_dump($_POST); ?>
			</div>

<?php
	$continue = true;

	// Change to root directory
	if (!chdir("../")) {
		printalert("Cannot change to public PHP root directory");
		$continue = false;
	}

	// Validate basics
	if ($continue) {
		if (file_exists($_POST["privatePhpDirectory"])) {
			printalert("success", "Private PHP Directory found");
		} else {
			printalert("danger", "Private PHP Directory not found (NOTE: This directory is relative to the root of the public PHP directory)");
			$continue = false;
		}
	}
	if ($continue) {
		$continue = validate_writable($continue, "login/logs/");
		$continue = validate_writable($continue, "admin/graphs_output/");
		$continue = validate_writable($continue, "admin/templates/default_c/");
		$continue = validate_writable($continue, "ams/cache/");
		$continue = validate_writable($continue, "ams/templates_c/");
		$continue = validate_writable($continue, "./");
		if ($continue) {
			printalert("success", "Paths are writable");
		}
	}
	$con = null;
	if ($continue) {
		$con = mysqli_connect($_POST["sqlHostname"], $_POST["sqlUsername"], $_POST["sqlPassword"]);
		if (mysqli_connect_errno()) {
			printalert("danger", "Failed to connect to SQL: " . mysqli_connect_error());
			$continue = false;
			$con = null;
		} else {
			printalert("success", "Connected to the SQL server");
		}
	}

	// Create NeL database
	$continue = create_use_database($continue, $con, $_POST["nelDatabase"]);
	$continue = update_database_structure($continue, $con, "nel_00001.sql");

	// Create NeL Tools database
	$continue = create_use_database($continue, $con, $_POST["toolDatabase"]);
	$continue = update_database_structure($continue, $con, "nel_tool_00001.sql");

	// Create AMS database
	$continue = create_use_database($continue, $con, $_POST["amsDatabase"]);
	$continue = update_database_structure($continue, $con, "nel_ams_00001.sql");

	// Create AMS Library database
	$continue = create_use_database($continue, $con, $_POST["amsLibDatabase"]);
	$continue = update_database_structure($continue, $con, "nel_ams_lib_00001.sql");

	// Write config.php
	if ($continue) {
		$config = file_get_contents($_POST["privatePhpDirectory"] . "/setup/config/config.php");
		if (!$config) {
			printalert("danger", "Cannot read <em>config.php</em>");
			$continue = false;
		} else {
			$cwd = getcwd();
			$config = str_replace("%privatePhpDirectory%", realpath($cwd . "/" . $_POST["privatePhpDirectory"]), $config);
			$config = str_replace("%publicPhpDirectory%", realpath($cwd), $config);
			$config = str_replace("%sqlHostname%", $_POST["sqlHostname"], $config);
			$config = str_replace("%sqlUsername%", $_POST["sqlUsername"], $config);
			$config = str_replace("%sqlPassword%", $_POST["sqlPassword"], $config);
			$config = str_replace("%nelDatabase%", $_POST["nelDatabase"], $config);
			$config = str_replace("%toolDatabase%", $_POST["toolDatabase"], $config);
			$config = str_replace("%amsDatabase%", $_POST["amsDatabase"], $config);
			$config = str_replace("%amsLibDatabase%", $_POST["amsLibDatabase"], $config);
			$config = str_replace("%amsAdminUsername%", $_POST["amsAdminUsername"], $config);
			$config = str_replace("%amsAdminPassword%", $_POST["amsAdminPassword"], $config);
			if (file_put_contents("config.php", $config)) {
				printalert("success", "Generated <em>config.php</em>");
			} else {
				printalert("danger", "Cannot write to <em>config.php</em>");
				$continue = false;
			}
		}
	}

	// Load config
	if ($continue) {
		try {
			require_once('config.php');
		} catch (Exception $e) {
			printalert("danger", "Failed to include <em>config.php</em>");
			$continue = false;
		}
	}

	// Load AMS Library
	if ($continue) {
		try {
			require_once($AMS_LIB . '/libinclude.php');
		} catch (Exception $e) {
			printalert("danger", "Failed to include AMS <em>libinclude.php</em>");
			$continue = false;
		}
	}

	// Create AMS Admin user
	if ($continue) {
		$hashpass = crypt($_POST["amsAdminPassword"], Users::generateSALT());
		$params = array(
		  'Login' => $_POST["amsAdminUsername"],
		  'Password' => $hashpass,
		  'Email' => "localhost@localhost", // TODO
		);
		try {
			$user_id = WebUsers::createWebuser($params['Login'], $params['Password'],$params['Email']);
			$result = Webusers::createUser($params, $user_id);
			Users::createPermissions(array($params['Login']));
			$dbl = new DBLayer("lib");
			$dbl->execute("UPDATE ticket_user SET Permission = 3 WHERE TUserId = :user_id",array('user_id' => $user_id));
			printalert("success", "AMS Admin account <em>" . htmlentities($_POST["amsAdminUsername"]) . "</em> created");
		} catch (PDOException $e) {
			printalert("danger", "Failed to create AMS Admin account");
			$continue = false;
		}
	}

	if ($continue) {
		if (file_put_contents("is_installed", "1")) {
			printalert("success", "Success!");
		} else {
			printalert("danger", "Failed to flag installation success");
			$continue = false;
		}
	}

	if ($con) {
		mysqli_close($con);
		printalert("info", "Disconnected from the SQL server");
	}
?>

<?php } /* ENDOF: if (isset($_POST)) { */ else { /* TODO: Refill form on failure */ ?>

			<form class="form-horizontal" role="form" method="POST" action=".">
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Basics</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="privatePhpDirectory" class="col-sm-3 control-label">Private PHP Directory</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="privatePhpDirectory" name="privatePhpDirectory" value="../private_php/">
							</div>
						</div>
						<div class="form-group">
							<label for="sqlHostname" class="col-sm-3 control-label">SQL Hostname</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="sqlHostname" name="sqlHostname" value="localhost">
							</div>
						</div>
						<div class="form-group">
							<label for="sqlUsername" class="col-sm-3 control-label">SQL Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="sqlUsername" name="sqlUsername" value="root">
							</div>
						</div>
						<div class="form-group">
							<label for="sqlPassword" class="col-sm-3 control-label">SQL Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="sqlPassword" name="sqlPassword" value="">
							</div>
						</div>
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Core</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="nelDatabase" class="col-sm-3 control-label">NeL Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelDatabase" name="nelDatabase" value="nel">
							</div>
						</div>
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Admin</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="toolDatabase" class="col-sm-3 control-label">NeL Tools Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="toolDatabase" name="toolDatabase" value="nel_tool">
							</div>
						</div>
					</div>
					<!-- TODO: Initial admin user -->
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">AMS</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="amsDatabase" class="col-sm-3 control-label">AMS CMS Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsDatabase" name="amsDatabase" value="nel_ams">
							</div>
						</div>
						<div class="form-group">
							<label for="amsLibDatabase" class="col-sm-3 control-label">AMS Library Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsLibDatabase" name="amsLibDatabase" value="nel_ams_lib">
							</div>
						</div>
						<div class="form-group">
							<label for="amsAdminUsername" class="col-sm-3 control-label">AMS Admin Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsAdminUsername" name="amsAdminUsername" value="admin">
							</div>
						</div>
						<div class="form-group">
							<label for="amsAdminPassword" class="col-sm-3 control-label">AMS Admin Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="amsAdminPassword" name="amsAdminPassword" value="admin">
							</div>
						</div>
					</div>
					<!-- TODO: Initial admin user -->
				</div>
				<button type="submit" class="btn btn-primary">Configure</button>
			</form>

<?php } ?>

		</div>

		<script src="js/bootstrap.min.js"></script>
	</body>
</html>
