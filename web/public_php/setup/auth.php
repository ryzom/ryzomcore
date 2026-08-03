<?php

error_reporting(E_ALL);
ini_set('display_errors', '0');
ini_set('log_errors', '1');

class SystemExit extends Exception {}
try {

$pageTitle = "Authenticate";
include('header.php');

require_once('config.php');

?>
			<div style="margin-left: auto; margin-right: auto; max-width: 512px;">

<?php $showForm = true; if ($_POST) { ?>

<?php
// Soft throttle: setup is the post-install privileged surface and has no
// account lockout elsewhere. Ten failures in five minutes is enough to stop
// bulk guessing without locking a lone operator out for long.
$now = time();
if (!isset($_SESSION['setup_auth_failures']) || !is_array($_SESSION['setup_auth_failures']))
	$_SESSION['setup_auth_failures'] = array();
$failures = array();
foreach ($_SESSION['setup_auth_failures'] as $ts) {
	if (($now - (int)$ts) < 300)
		$failures[] = (int)$ts;
}
$_SESSION['setup_auth_failures'] = $failures;
$setupThrottled = count($failures) >= 10;

if ($setupThrottled) {
	printalert("danger", "Too many failed attempts. Please wait a few minutes and try again.");
} elseif (isset($_POST['nelSetupPassword']) && hash_equals((string)$NEL_SETUP_PASSWORD, (string)$_POST['nelSetupPassword'])) {

// drop any session id the caller may have planted before auth
session_regenerate_id(true);
$_SESSION['nelSetupAuthenticated'] = 1;
unset($_SESSION['setup_auth_failures']);

printalert("success", "You are now authenticated");
$showForm = false;

?>
				<p>
					<a class="btn btn-primary" href="index.php">Continue</a>
				</p>

<?php } else {

$_SESSION['setup_auth_failures'][] = $now;
printalert("danger", "Invalid password");

} ?>

<?php } if ($showForm && empty($setupThrottled)) { ?>

				<form class="form" role="form" method="POST" action="" enctype="application/x-www-form-urlencoded">
					<div class="input-group">
						<label for="nelSetupPassword" class="sr-only">NeL Setup Password</label>
						<input type="password" class="form-control" id="nelSetupPassword" name="nelSetupPassword" placeholder="Password">
						<span class="input-group-btn">
							<input name="submit" type="submit" value="Authenticate" class="btn btn-primary">
						</span>
					</div>
				</form>

<?php } ?>

			</div>

<?php

include('footer.php');

}
catch (SystemExit $e) { /* do nothing */ }

?>
