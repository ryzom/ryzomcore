<?php

error_reporting(E_ALL);
ini_set('display_errors', '0');
ini_set('log_errors', '1');

class SystemExit extends Exception {}
try {

$pageTitle = "Install";
include('header.php');

require_once('setup/version.php');

$shardDev = isset($_GET['dev']);
$shardWin = (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN');

?>

<?php if (file_exists('config.php')) { ?>

			<div class="alert alert-danger" role="alert">
				Already installed.
			</div>

<?php } else if ($_POST) { ?>

<?php

	$roleService = isset($_POST["roleService"]) && $_POST["roleService"] == "on";
	$roleDomain = isset($_POST["roleDomain"]) && $_POST["roleDomain"] == "on";
	$configureShardDev = isset($_POST["configureShardDev"]) && $_POST["configureShardDev"] == "on";
	
	$continue = true;

	if (!$roleService && !$roleDomain) {
		printalert("danger", "No server roles selected");
		$continue = false;
	}

	// Every password field on this form used to arrive pre-filled with
	// "admin", so clicking through the wizard shipped an install whose setup
	// page -- the gate on the installer and on every later upgrade -- and
	// whose shard admin account both had a password everybody knows. The
	// fields are empty now; refuse the old value and the empty one here too,
	// since the browser is not the thing to trust with that.
	$passwordFields = array('nelSetupPassword' => 'Setup Password');
	if ($roleService)	$passwordFields['toolsAdminPassword'] = 'Admin Password (shard tools)';
	foreach ($passwordFields as $field => $label) {
		$value = isset($_POST[$field]) ? (string)$_POST[$field] : '';
		if ($value === '' || strtolower($value) === 'admin' || strlen($value) < 8) {
			printalert("danger", htmlentities($label) . " must be set, at least 8 characters, and not <em>admin</em>");
			$continue = false;
		}
	}

	// The administrator account, the first game account, goes into the NeL
	// database of the service role. Its fields are checked here, before
	// anything is created: once config.php is written the install cannot be
	// run again. The password is never printed back.
	$accountAdminLogin = (isset($_POST["accountAdminLogin"]) && is_string($_POST["accountAdminLogin"])) ? trim($_POST["accountAdminLogin"]) : '';
	$accountAdminEmail = (isset($_POST["accountAdminEmail"]) && is_string($_POST["accountAdminEmail"])) ? trim($_POST["accountAdminEmail"]) : '';
	$accountAdminPassword = (isset($_POST["accountAdminPassword"]) && is_string($_POST["accountAdminPassword"])) ? $_POST["accountAdminPassword"] : '';
	// database.php also puts mysqli back into its classic reporting mode,
	// which the connection check below counts on: required further down
	// only, an unreachable SQL server ended this page in an uncaught
	// exception on php 8.1 instead of the alert.
	require_once('database.php');
	if ($roleService) {
		$accountAdminError = admin_account_error($accountAdminLogin, $accountAdminPassword, $accountAdminEmail);
		if ($accountAdminError !== '') {
			printalert("danger", "Administrator Account: " . htmlentities($accountAdminError));
			$continue = false;
		}
	}

	// The development shard is configured by a script that names its
	// databases itself. Other names used to fail in that script, after
	// config.php was written; refuse them now.
	if ($roleDomain && $configureShardDev) {
		$shardDevProblems = shard_dev_database_problems(
			isset($_POST["nelDatabase"]) ? (string)$_POST["nelDatabase"] : '',
			isset($_POST["toolDatabase"]) ? (string)$_POST["toolDatabase"] : '',
			isset($_POST["domainDatabase"]) ? (string)$_POST["domainDatabase"] : '');
		if (!empty($shardDevProblems)) {
			printalert("danger", "Configure Development Shard: " . htmlentities(implode(', ', $shardDevProblems)) . ". Keep these database names, or untick <em>Configure Development Shard</em>.");
			$continue = false;
		}
	}

	// set once config.php is on disk: a failure after that leaves an
	// install behind that has to be undone by hand
	$configWritten = false;
    
	if ($continue) {
		try {
            if (!in_array("mysql",PDO::getAvailableDrivers(),TRUE))
            {
                throw new PDOException ("Cannot work without a proper database setting up");
            }
        }
        catch (PDOException $pdoEx)
        {
            printalert("danger", "PHP PDO seems to be missing the mysql driver");
			$continue = false;
        }
	}    

	// Validate basics. privatePhpDirectory is later opened as a path; keep
	// it under the public php root or its immediate parent (the usual
	// public_php / private_php sibling layout) and refuse anything else.
	if ($continue) {
		$privDirIn = isset($_POST["privatePhpDirectory"]) ? (string)$_POST["privatePhpDirectory"] : '';
		$privDirResolved = ($privDirIn !== '') ? realpath($privDirIn) : false;
		$publicRoot = realpath('.');
		$parentRoot = ($publicRoot !== false) ? realpath($publicRoot . '/..') : false;
		$privPrefixOk = false;
		if ($privDirResolved !== false && $publicRoot !== false) {
			$privWithSep = $privDirResolved . DIRECTORY_SEPARATOR;
			$publicPrefix = rtrim($publicRoot, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
			$parentPrefix = ($parentRoot !== false)
				? (rtrim($parentRoot, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR)
				: null;
			$privPrefixOk = (strpos($privWithSep, $publicPrefix) === 0)
				|| ($parentPrefix !== null && strpos($privWithSep, $parentPrefix) === 0);
		}
		if ($privDirResolved === false || $publicRoot === false
			|| !is_dir($privDirResolved)
			|| !$privPrefixOk
			|| !is_file($privDirResolved . '/setup/config/config.php')) {
			printalert("danger", "Private PHP Directory not found (NOTE: This directory is relative to the root of the public PHP directory)");
			$continue = false;
		} else {
			// store the cleaned path so later file_get_contents cannot be
			// pointed at a different tree than the one we just checked
			$_POST["privatePhpDirectory"] = $privDirResolved;
			printalert("success", "Private PHP Directory found");
		}
	}
	if ($continue) {
		if ($roleService) {
			$continue = validate_writable($continue, "login/logs/") && $continue;
			$continue = validate_writable($continue, "admin/graphs_output/") && $continue;
			$continue = validate_writable($continue, "admin/templates/default_c/") && $continue;
		}
		$continue = validate_writable($continue, "./");
		if ($continue) {
			printalert("success", "Paths are writable");
		}
	}

	$con = null;
	if ($continue) { // NOTE: Also test if this is reachable when not Service role
		$con = mysqli_connect($_POST["nelSqlHostname"], $_POST["nelSqlUsername"], $_POST["nelSqlPassword"], NULL, $_POST["nelSqlPort"]);
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
		$continue = create_use_database($continue, $con, $_POST["nelDatabase"]) && $continue;

		// Create NeL Tools database
		$continue = create_use_database($continue, $con, $_POST["toolDatabase"]) && $continue;
	}

	if ($roleDomain) {
		// Create Ring database
		$continue = create_use_database($continue, $con, $_POST["domainDatabase"]) && $continue;
	}

	if ($con && !$continue && !empty($installCreatedDatabases)) {
		// stopped between two databases: the next attempt refuses the ones
		// that are there, so say which ones this attempt left
		printalert("danger", "This run created " . htmlentities(implode(", ", $installCreatedDatabases)) . " before it stopped. The installer does not use a database that is there already: drop what this run created before running the install again.");
	}

	if ($con) {
		mysqli_close($con);
		printalert("info", "Disconnected from the Service SQL server");
	}

	// Write config.php
	if ($continue) {
		require_once('config_generation.php');
		$cwd = getcwd();
		$config = generate_install_config(
			$_POST["privatePhpDirectory"] . "/setup/config/config.php",
			array(
				// already resolved to an absolute path by the check above;
				// joining it onto the cwd again resolved to nothing and wrote
				// an empty private php path into the generated config
				'privatePhpDirectory' => $_POST["privatePhpDirectory"],
				'publicPhpDirectory'  => realpath($cwd),
				'nelSqlHostname'      => $_POST["nelSqlHostname"],
				'nelSqlPort'          => $_POST["nelSqlPort"],
				'nelSqlUsername'      => $_POST["nelSqlUsername"],
				'nelSqlPassword'      => $_POST["nelSqlPassword"],
				'nelDatabase'         => $_POST["nelDatabase"],
				'toolDatabase'        => $_POST["toolDatabase"],
				'nelSetupPassword'    => $_POST["nelSetupPassword"],
				'domainDatabase'      => $_POST["domainDatabase"],
				'domainUsersDir'      => $_POST["domainUsersDir"],
				'nelDomainName'       => $_POST["nelDomainName"],
				'nelSetupVersion'     => $NEL_SETUP_VERSION,
			)
		);
		if (!$config) {
			printalert("danger", "Cannot read <em>config.php</em>");
			$continue = false;
		} else {
			if (file_put_contents("config.php", $config)) {
				$configWritten = true;
				forget_cached_script("config.php");
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

	if ($roleService) {
		$continue = upgrade_service_databases($continue) && $continue;
	}

	if ($roleDomain) {
		$continue = upgrade_domain_databases($continue) && $continue;
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

	if ($continue && $roleDomain) {
		if ($configureShardDev) {
			$continue = configure_shard_dev($continue) && $continue;
		}
	}

	if ($roleService) {
		// Create the administrator account, after the development shard so
		// that its domain is there to get a permission row
		$continue = create_admin_account($continue, $accountAdminLogin, $accountAdminPassword, $accountAdminEmail) && $continue;
	}

	if ($continue && $roleService) {
		if (file_put_contents("role_service", "1")) {
			printalert("success", "Service role successfully installed");
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

	if ($continue && $roleService) {
		// the last word of the install: the one game account that exists
		// now, and what it can and cannot do
		$accountAdminNotes = admin_account_notes($accountAdminLogin, $cfg['db']['shard']['name']);
		$accountAdminStatement = array_pop($accountAdminNotes);
		printalert("info", "The game account <em>" . htmlentities($accountAdminLogin) . "</em> (" . htmlentities($accountAdminEmail) . ") is the administrator, with the :DEV: privilege. "
			. htmlentities(implode(' ', $accountAdminNotes)) . "<br><code>" . htmlentities($accountAdminStatement) . "</code>");
	}

	if (!$continue && $configWritten) {
		// the install cannot simply be run again from here. This installer
		// only gets this far with databases it created itself.
		printalert("danger", htmlentities(failed_install_notes($installCreatedDatabases, array())));
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
				document.getElementById("panelAccount").className = panelClass;
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
										<input id="roleDomain" name="roleDomain" type="checkbox" onclick="checkDomain();" checked> Domain <small>(Ring Database, ...)</small>
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
								<input type="password" class="form-control" id="nelSetupPassword" name="nelSetupPassword" value="" required autocomplete="new-password" placeholder="Required">
							</div>
						</div>
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">SQL Database</h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="nelSqlHostname" class="col-sm-3 control-label">SQL Hostname</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelSqlHostname" name="nelSqlHostname" value="localhost">
							</div>
						</div>
						<div class="form-group">
							<label for="nelSqlPort" class="col-sm-3 control-label">SQL Port</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelSqlPort" name="nelSqlPort" value="<?php if ($shardDev) { print 9040; } else { print 3306; } ?>">
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
					</div>
				</div>
				<div class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Login Service</h2>
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
								<input type="password" class="form-control" id="toolsAdminPassword" name="toolsAdminPassword" value="" required autocomplete="new-password" placeholder="Required">
							</div>
						</div>
					</div>
				</div>
				<div id="panelAccount" class="panel panel-default">
					<div class="panel-heading">
						<h2 class="panel-title">Administrator Account <small>(the first game account, with the :DEV: privilege)</small></h2>
					</div>
					<div class="panel-body">
						<div class="form-group">
							<label for="accountAdminLogin" class="col-sm-3 control-label">Login</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="accountAdminLogin" name="accountAdminLogin" value="" autocomplete="off" placeholder="Required, 3 to 64 letters, digits or underscores">
							</div>
						</div>
						<div class="form-group">
							<label for="accountAdminEmail" class="col-sm-3 control-label">E-mail</label>
							<div class="col-sm-6">
								<input type="email" class="form-control" id="accountAdminEmail" name="accountAdminEmail" value="" autocomplete="off" placeholder="Required">
							</div>
						</div>
						<div class="form-group">
							<label for="accountAdminPassword" class="col-sm-3 control-label">Password</label>
							<div class="col-sm-6">
								<input type="password" class="form-control" id="accountAdminPassword" name="accountAdminPassword" value="" autocomplete="new-password" placeholder="Required, at least 8 characters">
							</div>
						</div>
					</div>
				</div>
				<div id="panelDomain" class="panel panel-default hide">
					<div class="panel-heading">
						<h2 class="panel-title">Domain <small>(Multiple domains require separate installations, as they may run different versions)</small></h2>
					</div>
					<div class="panel-body">
						<?php if (!$shardDev) { ?>
						<div class="panel panel-danger">
							<div class="panel-heading"><span class="glyphicon glyphicon-info-sign"></span> Database Only</div>
							<div class="panel-body">
								<p>The setup script is intended only for installation and upgrade of the domain specific database.</p>
								<p>This will only configure the database for a single domain. It will not register the domain in the domain configuration.</p>
								<p>The domain must be manually configured in the databases according to the procedure for configuring your server park with patchman, as it depends on several values defined within the patchman configuration.</p>
								<!-- NOTE: For future implementation, it would be practical to configure the nel database directly from the patchman configuration script, as this has all the required values ready. -->
								<!-- IMPORTANT: This setup must NEVER be modified to configure the domains, it is not it's responsibility. -->
								<p>It is required to use separate virtual hosts for multiple domains, in order to allow domain-specific script to run at different versions.</p>
								<p>It is recommended, when planning to use multiple domains, to use a separate virtual host for each web service role.</p>
								<p>There can be multiple instances of the domain role, there can only be one service role setup.</p>
							</div>
						</div>
						<?php } ?>
						<?php if ($shardDev) { ?>
						<div class="form-group">
							<div class="col-sm-offset-3 col-sm-8">
								<div class="checkbox">
									<label>
										<input id="configureShardDev" name="configureShardDev" type="checkbox" checked> Configure Development Shard</small>
									</label>
								</div>
							</div>
						</div>
						<?php } ?>
						<div class="form-group">
							<label for="nelDomainName" class="col-sm-3 control-label">Name</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="nelDomainName" name="nelDomainName" value="<?php if ($shardDev) { print "dev"; } else { print gethostname(); } ?>">
							</div>
						</div>
						<div class="form-group">
							<label for="domainDatabase" class="col-sm-3 control-label">Database</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="domainDatabase" name="domainDatabase" value="ring_<?php if ($shardDev) { print "dev"; } else { print gethostname(); } ?>">
							</div>
						</div>
						<div class="form-group">
							<label for="domainUsersDir" class="col-sm-3 control-label">Users Directory (MFS, etc)</label>
							<div class="col-sm-6">
								<input type="text" class="form-control" id="domainUsersDir" name="domainUsersDir" value="<?php
									if ($shardDev && $shardWin) {
										print str_replace("\\", "/", str_replace("\\code\\web\\public_php\\setup\\install.php", "/pipeline/shard_dev/www", __FILE__));
									}
									else if ($shardDev) {
										print str_replace("/code/web/public_php/setup/install.php", "/pipeline/shard_dev/www", __FILE__);
									}
									else if ($shardWin) {
										print "C:/nevrax/" . gethostname() . "/www";
									}
									else {
										print "/home/nevrax/" . gethostname() . "/www";
									}
								?>">
							</div>
						</div>
					</div>
				</div>
				<input name="submit" type="submit" value="Configure" class="btn btn-primary">
			</form>

			<script>

			checkService();
			checkDomain();

			</script>

<?php } ?>

<?php

include('footer.php');

}
catch (SystemExit $e) { /* do nothing */ }

?>
