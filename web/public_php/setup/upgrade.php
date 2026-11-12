<?php

error_reporting(E_ALL);
ini_set('display_errors', '0');
ini_set('log_errors', '1');

class SystemExit extends Exception {}
try {

$pageTitle = "Upgrade";
include('header.php');

// Read config.php as it is on disk: right after an upgrade the opcode cache
// may still hold the copy from before it, and this page would take the
// installation for one that still has to be upgraded.
require_once('config_generation.php');
forget_cached_script('config.php');
require_once('config.php');
require_once('setup/version.php');
if (!isset($NEL_SETUP_VERSION_CONFIGURED)) {
	$NEL_SETUP_VERSION_CONFIGURED = 1;
}
$upToDate = ($NEL_SETUP_VERSION_CONFIGURED >= $NEL_SETUP_VERSION);

?>

			<div style="margin-left: auto; margin-right: auto; max-width: 1024px;">

<?php if ($upToDate) { ?>

<?php

	// Nothing to do, and nothing is run: not the database steps, and
	// config.php is not written again.
	printalert("info", "This installation is up to date, at setup version " . (int)$NEL_SETUP_VERSION_CONFIGURED . "."
		. ($_POST ? " The upgrade was not run again." : ""));

?>

				<p>
					<a class="btn btn-primary" href="index.php">Continue</a>
				</p>

<?php } else if ($_POST) { ?>

<?php

	// NOTE: If a config upgrade requires new information, modify the
	// upgrade confirmation form at the bottom of this script.

	require_once('database.php');

	if (file_exists("role_service")) {
		$continue = upgrade_service_databases($continue);
	}

	if (file_exists("role_domain")) {
		$continue = upgrade_domain_databases($continue);
	}

	if (file_exists("role_service") && $NEL_SETUP_VERSION_CONFIGURED < 14 && $NEL_DOMAIN_NAME === 'dev') {
		$continue = upgrade_shard_dev_localhost($continue);
	}

	// Rewrite config.php
	if ($continue) {
		require_once('config_generation.php');
		$config = generate_upgrade_config(
			$PRIVATE_PHP_PATH . "/setup/config/config.php",
			$cfg, $PRIVATE_PHP_PATH, $PUBLIC_PHP_PATH,
			$NEL_SETUP_PASSWORD, $NEL_DOMAIN_NAME, $NEL_SETUP_VERSION,
			$cfg['db']['ring']['name'], $USERS_DIR,
			$NEL_SETUP_VERSION_CONFIGURED
		);
		if (!$config) {
			printalert("danger", "Cannot read <em>config.php</em>");
			$continue = false;
		} else {
			if (file_put_contents("config.php", $config)) {
				forget_cached_script("config.php");
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
