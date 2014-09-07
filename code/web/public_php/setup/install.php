<?php

error_reporting(E_ALL);
ini_set('display_errors', 'on');

class SystemExit extends Exception {}
try {

$pageTitle = "Install";
include('header.php');

?>

<?php if (file_exists('config.php')) { ?>

			<div class="alert alert-danger" role="alert">
				Already installed.
			</div>

<?php } else if ($_POST) { ?>

			<div class="alert alert-info" role="alert">
				<?php var_dump($_POST); ?>
			</div>

<?php

	$roleService = isset($_POST["roleService"]) && $_POST["roleService"] == "on";
	$roleSupport = isset($_POST["roleSupport"]) && $_POST["roleSupport"] == "on";
	$roleDomain = isset($_POST["roleDomain"]) && $_POST["roleDomain"] == "on";

	if (!$roleService && !$roleSupport && !$roleDomain) {
		printalert("danger", "No server roles selected");
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
		if ($roleService) {
			$continue = validate_writable($continue, "login/logs/");
			$continue = validate_writable($continue, "admin/graphs_output/");
			$continue = validate_writable($continue, "admin/templates/default_c/");
		}
		if ($roleSupport) {
			$continue = validate_writable($continue, "ams/cache/");
			$continue = validate_writable($continue, "ams/templates_c/");
		}
		$continue = validate_writable($continue, "./");
		if ($continue) {
			printalert("success", "Paths are writable");
		}
	}

	$con = null;
	if ($continue) { // NOTE: Also test if this is reachable when not Service role
		$con = mysqli_connect($_POST["nelSqlHostname"], $_POST["nelSqlUsername"], $_POST["nelSqlPassword"]);
		if (mysqli_connect_errno()) {
			printalert("danger", "Failed to connect to Service SQL: " . mysqli_connect_error());
			$continue = false;
			$con = null;
		} else {
			printalert("success", "Connected to the Service SQL server");
		}
	}

	if ($roleService) {
		// Create NeL database
		$continue = create_use_database($continue, $con, $_POST["nelDatabase"]);

		// Create NeL Tools database
		$continue = create_use_database($continue, $con, $_POST["toolDatabase"]);
	}

	if ($roleDomain) {
		// Create Ring database
		$continue = create_use_database($continue, $con, $_POST["domainDatabase"]);
	}

	if ($con) {
		mysqli_close($con);
		printalert("info", "Disconnected from the Service SQL server");
	}

	if ($roleSupport) {
		if ($continue) {
			$con = mysqli_connect($_POST["amsSqlHostname"], $_POST["amsSqlUsername"], $_POST["amsSqlPassword"]);
			if (mysqli_connect_errno()) {
				printalert("danger", "Failed to connect to Support SQL: " . mysqli_connect_error());
				$continue = false;
				$con = null;
			} else {
				printalert("success", "Connected to the Support SQL server");
			}
		}

		// Create AMS database
		$continue = create_use_database($continue, $con, $_POST["amsDatabase"]);

		// Create AMS Library database
		$continue = create_use_database($continue, $con, $_POST["amsLibDatabase"]);

		if ($con) {
			mysqli_close($con);
			printalert("info", "Disconnected from the Support SQL server");
		}
	}

	// Write config.php
	if ($continue) {
		$config = file_get_contents($_POST["privatePhpDirectory"] . "/setup/config/config.php");
		if (!$config) {
			printalert("danger", "Cannot read <em>config.php</em>");
			$continue = false;
		} else {
			$cwd = getcwd();
			$config = str_replace("%privatePhpDirectory%", addslashes(realpath($cwd . "/" . $_POST["privatePhpDirectory"])), $config);
			$config = str_replace("%publicPhpDirectory%", addslashes(realpath($cwd)), $config);
			$config = str_replace("%nelSqlHostname%", addslashes($_POST["nelSqlHostname"]), $config);
			$config = str_replace("%nelSqlUsername%", addslashes($_POST["nelSqlUsername"]), $config);
			$config = str_replace("%nelSqlPassword%", addslashes($_POST["nelSqlPassword"]), $config);
			$config = str_replace("%nelDatabase%", addslashes($_POST["nelDatabase"]), $config);
			$config = str_replace("%toolDatabase%", addslashes($_POST["toolDatabase"]), $config);
			$config = str_replace("%amsSqlHostname%", addslashes($_POST["amsSqlHostname"]), $config);
			$config = str_replace("%amsSqlUsername%", addslashes($_POST["amsSqlUsername"]), $config);
			$config = str_replace("%amsSqlPassword%", addslashes($_POST["amsSqlPassword"]), $config);
			$config = str_replace("%amsDatabase%", addslashes($_POST["amsDatabase"]), $config);
			$config = str_replace("%amsLibDatabase%", addslashes($_POST["amsLibDatabase"]), $config);
			$config = str_replace("%nelSetupPassword%", addslashes($_POST["nelSetupPassword"]), $config);
			$config = str_replace("%domainDatabase%", addslashes($_POST["domainDatabase"]), $config);
			$config = str_replace("%nelDomainName%", addslashes($_POST["nelDomainName"]), $config);
			$cryptKeyLength = 16;
			$cryptKey = str_replace("=", "", base64_encode(mcrypt_create_iv(ceil(0.75 * $cryptKeyLength), MCRYPT_DEV_URANDOM)));
			$cryptKeyIMAP = str_replace("=", "", base64_encode(mcrypt_create_iv(ceil(0.75 * $cryptKeyLength), MCRYPT_DEV_URANDOM)));
			$config = str_replace("%cryptKey%", addslashes($cryptKey), $config);
			$config = str_replace("%cryptKeyIMAP%", addslashes($cryptKeyIMAP), $config);
			if (file_put_contents("config.php", $config)) {
				printalert("success", "Generated <em>config.php</em>");
			} else {
				printalert("danger", "Cannot write to <em>config.php</em>");
				$continue = false;
			}
		}
	}

	if ($continue) {
		$configUser = file_get_contents($_POST["privatePhpDirectory"] . "/setup/config/config_user.php");
		if (!$configUser) {
			printalert("danger", "Cannot read <em>config_user.php</em>");
			$continue = false;
		} else {
			if (file_put_contents("config_user.php", $configUser)) {
				printalert("success", "Copied <em>config_user.php</em>");
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

	require_once('database.php');

	if ($roleSupport) {
		$continue = upgrade_support_databases($continue);
	}

	if ($roleService) {
		$continue = upgrade_service_databases($continue);
	}

	if ($roleDomain) {
		$continue = upgrade_domain_databases($continue);
	}

	if ($roleService) {
		// Create the default shard admin user
		if (!chdir("admin/")) {
			printalert("danger", "Cannot change to admin tools directory");
			$continue = false;
		}
		if ($continue) {
			try {
				require_once('common.php');
			} catch (Exception $e) {
				printalert("danger", "Failed to include NeL <em>admin/common.php</em>");
				$continue = false;
			}
		}
		if ($continue) {
			try {
				require_once('functions_tool_administration.php');
			} catch (Exception $e) {
				printalert("danger", "Failed to include NeL <em>admin/functions_tool_administration.php</em>");
				$continue = false;
			}
		}
		if ($continue) {
			$adminGroup = 1;
			$result = tool_admin_users_add($_POST["toolsAdminUsername"], $_POST["toolsAdminPassword"], (string)$adminGroup, (string)1);
			if ($result == "") {
				printalert("success", "Added shard admin to NeL tools database");
			} else {
				printalert("danger", "Failed to add shard admin to NeL tools database<br>" . htmlentities($result));
				$continue = false;
			}
		}
		if (!chdir("../")) {
			printalert("danger", "Cannot change to public PHP root directory");
			$continue = false;
		}
	}

	if ($roleSupport) {
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
	}

	if ($roleDomain) {
		// TODO: Register the domain with the nel database etc
	}

	if ($continue && $roleService) {
		if (file_put_contents("role_service", "1")) {
			printalert("success", "Service role successfully installed");
		} else {
			printalert("danger", "Failed to flag installation success");
			$continue = false;
		}
	}

	if ($continue && $roleSupport) {
		if (file_put_contents("role_support", "1")) {
			printalert("success", "Support role successfully installed");
		} else {
			printalert("danger", "Failed to flag installation success");
			$continue = false;
		}
	}

	if ($continue && $roleDomain) {
		if (file_put_contents("role_domain", "1")) {
			printalert("success", "Domain role successfully installed");
		} else {
			printalert("danger", "Failed to flag installation success");
			$continue = false;
		}
	}
?>

			<p>
				<a class="btn btn-primary" href="index.php">Continue</a>
			</p>

<?php } /* ENDOF: if (isset($_POST)) { */ else { /* TODO: Refill form on failure */ ?>

			<script>

			var visiblePanelClass = "panel panel-default";
			var hiddenPanelClass = "panel panel-default hide";

			function checkService() {
				var panelClass = document.getElementById('roleService').checked
					? visiblePanelClass
					: hiddenPanelClass;
				document.getElementById("panelAdmin").className = panelClass;
			}

			function checkSupport() {
				var panelClass = document.getElementById('roleSupport').checked
					? visiblePanelClass
					: hiddenPanelClass;
				document.getElementById("panelAMS").className = panelClass;
			}

			function checkDomain() {
				var panelClass = document.getElementById('roleDomain').checked
					? visiblePanelClass
					: hiddenPanelClass;
				document.getElementById("panelDomain").className = panelClass;
			}

			</script>

			<form class="form-horizontal" role="form" method="POST" action="" enctype="application/x-www-form-urlencoded">
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Roles</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<div class="col-sm-offset-3 col-sm-8">
								<div class="checkbox">
									<label>
										<input id="roleService" name="roleService" type="checkbox" onclick="checkService();" checked> Service <small>(NeL Login, Shard Admin, Domain Database, ...)</small>
									</label>
								</div>
							</div>
						</div>
						<div class="form-group">
							<div class="col-sm-offset-3 col-sm-8">
								<div class="checkbox">
									<label>
										<input id="roleSupport" name="roleSupport" type="checkbox" onclick="checkSupport();" checked> Support <small>(AMS, ...)</small>
									</label>
								</div>
							</div>
						</div>
						<div class="form-group">
							<div class="col-sm-offset-3 col-sm-8">
								<div class="checkbox">
									<label>
										<input id="roleDomain" name="roleDomain" type="checkbox" onclick="checkDomain();" disabled> Domain <small>(Ring Database, ...) <em>TODO</em></small>
									</label>
								</div>
							</div>
						</div>
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Basics <small>(Paths relative to the public root directory)</small></h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="privatePhpDirectory" class="col-sm-3 control-label">Private PHP Directory</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="privatePhpDirectory" name="privatePhpDirectory" value="../private_php/">
							</div>
						</div>
						<div class="form-group">
							<label for="nelSetupPassword" class="col-sm-3 control-label">Setup Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="nelSetupPassword" name="nelSetupPassword" value="admin">
							</div>
						</div>
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Service Database <small>(Used for NeL login, admin tools and domain databases)</small></h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="nelSqlHostname" class="col-sm-3 control-label">SQL Hostname</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelSqlHostname" name="nelSqlHostname" value="localhost">
							</div>
						</div>
						<div class="form-group">
							<label for="nelSqlUsername" class="col-sm-3 control-label">SQL Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelSqlUsername" name="nelSqlUsername" value="root">
							</div>
						</div>
						<div class="form-group">
							<label for="nelSqlPassword" class="col-sm-3 control-label">SQL Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="nelSqlPassword" name="nelSqlPassword" value="">
							</div>
						</div>
						<div class="form-group">
							<label for="nelDatabase" class="col-sm-3 control-label">NeL Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelDatabase" name="nelDatabase" value="nel">
							</div>
						</div>
					</div>
				</div>
				<div id="panelAdmin" class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Shard Admin</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="toolDatabase" class="col-sm-3 control-label">NeL Tools Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="toolDatabase" name="toolDatabase" value="nel_tool">
							</div>
						</div>
						<div class="form-group">
							<label for="toolsAdminUsername" class="col-sm-3 control-label">Admin Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="toolsAdminUsername" name="toolsAdminUsername" value="admin">
							</div>
						</div>
						<div class="form-group">
							<label for="toolsAdminPassword" class="col-sm-3 control-label">Admin Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="toolsAdminPassword" name="toolsAdminPassword" value="admin">
							</div>
						</div>
					</div>
				</div>
				<div id="panelAMS" class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">AMS <small>(Account Management System)</small></h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="amsSqlHostname" class="col-sm-3 control-label">SQL Hostname</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsSqlHostname" name="amsSqlHostname" value="localhost">
							</div>
						</div>
						<div class="form-group">
							<label for="amsSqlUsername" class="col-sm-3 control-label">SQL Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsSqlUsername" name="amsSqlUsername" value="root">
							</div>
						</div>
						<div class="form-group">
							<label for="amsSqlPassword" class="col-sm-3 control-label">SQL Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="amsSqlPassword" name="amsSqlPassword" value="">
							</div>
						</div>
						<div class="form-group">
							<label for="amsDatabase" class="col-sm-3 control-label">CMS Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsDatabase" name="amsDatabase" value="nel_ams">
							</div>
						</div>
						<div class="form-group">
							<label for="amsLibDatabase" class="col-sm-3 control-label">Library Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsLibDatabase" name="amsLibDatabase" value="nel_ams_lib">
							</div>
						</div>
						<div class="form-group">
							<label for="amsAdminUsername" class="col-sm-3 control-label">Admin Username</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="amsAdminUsername" name="amsAdminUsername" value="admin">
							</div>
						</div>
						<div class="form-group">
							<label for="amsAdminPassword" class="col-sm-3 control-label">Admin Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="amsAdminPassword" name="amsAdminPassword" value="admin">
							</div>
						</div>
					</div>
				</div>
				<div id="panelDomain" class="panel panel-default hide">
					<div class="panel-heading">
						<h2 class="panel-title">Domain <small>(Multiple domains require separate installations, as they may run different versions)</small></h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="nelDomainName" class="col-sm-3 control-label">Name</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelDomainName" name="nelDomainName" value="mini01">
							</div>
						</div>
						<div class="form-group">
							<label for="domainDatabase" class="col-sm-3 control-label">Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="domainDatabase" name="domainDatabase" value="ring_mini01">
							</div>
						</div>
					</div>
				</div>
				<input name="submit" type="submit" value="Configure" class="btn btn-primary">
			</form>

			<script>

			checkService();
			checkSupport();
			checkDomain();

			</script>

<?php } ?>

<?php

include('footer.php');

}
catch (SystemExit $e) { /* do nothing */ }

?>
