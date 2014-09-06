<?php

error_reporting(E_ALL);
ini_set('display_errors', 'on');

class SystemExit extends Exception {}
try {

$pageTitle = "Upgrade";
include('header.php');

require_once('config.php');

?>

			<div style="margin-left: auto; margin-right: auto; max-width: 1024px;">

<?php if ($_POST) { ?>

<?php

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
			// $config = str_replace("%domainDatabase%", addslashes($_POST["domainDatabase"]), $config); // TODO
			$config = str_replace("%nelDomainName%", addslashes($NEL_DOMAIN_NAME), $config);
			if (file_put_contents("config.php", $config)) {
				printalert("success", "Generated <em>config.php</em>");
			} else {
				printalert("danger", "Cannot write to <em>config.php</em>");
				$continue = false;
			}
		}
	}

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

?>

				<p>
					<a class="btn btn-primary" href="index.php">Continue</a>
				</p>

<?php } else { ?>

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
