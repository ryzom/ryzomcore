<?php

error_reporting(E_ALL);
ini_set('display_errors', 'on');

class SystemExit extends Exception {}
try {

$pageTitle = "Setup";
include('header.php');

?>

			<div class="panel panel-default" style="margin-left: auto; margin-right: auto; max-width: 256px;">
				<div class="panel-body">

					<p>
						<a class="btn btn-default" style="width: 100%;" href="upgrade.php"><span class="glyphicon glyphicon-wrench"></span> Upgrade</a>
					</p>

<?php if (file_exists('role_domain')) { ?>

					<!--<p>
						<a class="btn btn-default" style="width: 100%;" href="domain.php"><span class="glyphicon glyphicon-globe"></span> Add Domain</a>
					</p>-->

					<!--<p>
						<a class="btn btn-default" style="width: 100%;" href="upgrade.php"><span class="glyphicon glyphicon-tower"></span> Add Shard</a>
					</p>-->

<?php } ?>

					<!--<p>-->
						<a class="btn btn-default" style="width: 100%;" href="status.php"><span class="glyphicon glyphicon-info-sign"></span> Status</a>
					<!--</p>-->

				</div>
			</div>

<?php

include('footer.php');

}
catch (SystemExit $e) { /* do nothing */ }

?>
