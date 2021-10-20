<?php

error_reporting(E_ALL);
ini_set('display_errors', 'on');

class SystemExit extends Exception {}
try {

$pageTitle = "Upgrade";
include('header.php');

require_once('config.php');
require_once('setup/version.php');
if (!isset($NEL_SETUP_VERSION_CONFIGURED)) {
	$NEL_SETUP_VERSION_CONFIGURED = 1;
}

?>

			<div style="margin-left: auto; margin-right: auto; max-width: 1024px;">

<?php if ($_POST) { ?>

<?php

	// NOTE: If a config upgrade requires new information, modify the
	// upgrade confirmation form at the bottom of this script.

	require_once('database.php');

	if (file_exists("role_support")) {
		$continue = upgrade_support_databases($continue);
	}

	if (file_exists("role_service")) {
		$continue = upgrade_service_databases($continue);
	}

	if (file_exists("role_domain")) {
		$continue = upgrade_domain_databases($continue);
	}

	// Rewrite config.php
	if ($continue) {
		$config = file_get_contents($PRIVATE_PHP_PATH . "/setup/config/config.php");
		if (!$config) {
			printalert("danger", "Cannot read <em>config.php</em>");
			$continue = false;
		} else {
			$cwd = getcwd();
			$config = str_replace("%privatePhpDirectory%", addslashes($PRIVATE_PHP_PATH), $config);
			$config = str_replace("%publicPhpDirectory%", addslashes($PUBLIC_PHP_PATH), $config);
			$config = str_replace("%nelSqlHostname%", addslashes($cfg['db']['shard']['host']), $config);
			$config = str_replace("%nelSqlUsername%", addslashes($cfg['db']['shard']['user']), $config);
			$config = str_replace("%nelSqlPassword%", addslashes($cfg['db']['shard']['pass']), $config);
			$config = str_replace("%nelDatabase%", addslashes($cfg['db']['shard']['name']), $config);
			$config = str_replace("%toolDatabase%", addslashes($cfg['db']['tool']['name']), $config);
			$config = str_replace("%amsSqlHostname%", addslashes($cfg['db']['lib']['host']), $config);
			$config = str_replace("%amsSqlUsername%", addslashes($cfg['db']['lib']['user']), $config);
			$config = str_replace("%amsSqlPassword%", addslashes($cfg['db']['lib']['pass']), $config);
			$config = str_replace("%amsDatabase%", addslashes($cfg['db']['web']['name']), $config);
			$config = str_replace("%amsLibDatabase%", addslashes($cfg['db']['lib']['name']), $config);
			$config = str_replace("%nelSetupPassword%", addslashes($NEL_SETUP_PASSWORD), $config);
			$config = str_replace("%nelDomainName%", addslashes($NEL_DOMAIN_NAME), $config);
			$config = str_replace("%nelSetupVersion%", addslashes($NEL_SETUP_VERSION), $config);
			$config = str_replace("%cryptKey%", addslashes($cfg['crypt']['key']), $config);
			$config = str_replace("%cryptKeyIMAP%", addslashes($SUPPORT_GROUP_IMAP_CRYPTKEY), $config);
			if ($NEL_SETUP_VERSION_CONFIGURED < 2) {
				$config = str_replace("%domainDatabase%", "mini01", $config);
			} else {
				$config = str_replace("%domainDatabase%", addslashes($cfg['db']['ring']['name']), $config);
			}
			if (file_put_contents("config.php", $config)) {
				printalert("success", "Generated <em>config.php</em>");
			} else {
				printalert("danger", "Cannot write to <em>config.php</em>");
				$continue = false;
			}
		}
	}

	// Create config_user.php if it doesn't exist yet
	if ($continue && !file_exists("config_user.php")) {
		$configUser = file_get_contents($PRIVATE_PHP_PATH . "/setup/config/config_user.php");
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

	if ($continue) {
		printalert("success", "Upgrade complete");
	}

?>

				<p>
					<a class="btn btn-primary" href="index.php">Continue</a>
				</p>

<?php } else { // NOTE: This is where you may also ask for new configuration fields ?>

				<div class="panel panel-danger">
					<div class="panel-heading"><span class="glyphicon glyphicon-hdd"></span> Backup</div>
					<div class="panel-body">
						It is strongly recommended to create a backup of your installation before proceeding with an upgrade!
					</div>
				</div>

				<form class="form" role="form" method="POST" action="" enctype="application/x-www-form-urlencoded">
					<div class="input-group">
						<input name="submit" type="submit" value="Upgrade" class="btn btn-primary">
					</div>
				</form>

<?php } ?>

			</div>

<?php

include('footer.php');

}
catch (SystemExit $e) { /* do nothing */ }

?>
