<?php

error_reporting(E_ALL);
ini_set('display_errors', 'on');

class SystemExit extends Exception {}
try {

$pageTitle = "Authenticate";
include('header.php');

require_once('config.php');

?>
			<div style="margin-left: auto; margin-right: auto; max-width: 512px;">

<?php /*var_dump($_POST);*/ $showForm = true; if ($_POST) { ?>

<?php if ($_POST['nelSetupPassword'] == $NEL_SETUP_PASSWORD) { ?>

<?php

$_SESSION['nelSetupAuthenticated'] = 1;

printalert("success", "You are now authenticated");
$showForm = false;

?>
				<p>
					<a class="btn btn-primary" href="index.php">Continue</a>
				</p>

<?php } else {

printalert("danger", "Invalid password");

} ?>

<?php } if ($showForm) { ?>

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
