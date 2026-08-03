<?php

// Account Management Tool - Dev Settings Page
// Requires the settings privilege (default: :DEV: only)

$pageTitle = 'Settings (Dev)';
$uid = $_SESSION['account_uid'];

$settings = array();
$allSettings = array(
	'admin_privileges' => array(
		'label' => 'Admin Privileges',
		'help' => 'Colon-delimited privilege codes that grant access to the Admin page (e.g. :DEV:SGM:GM:).',
		'default' => ':DEV:SGM:GM:',
	),
	'settings_privilege' => array(
		'label' => 'Settings Privilege',
		'help' => 'Colon-delimited privilege codes that grant access to this Dev Settings page (e.g. :DEV:).',
		'default' => ':DEV:',
	),
	'default_privileges' => array(
		'label' => 'Default New Account Privileges',
		'help' => 'Privileges automatically assigned to newly registered accounts (e.g. empty for none, or :G: for guide). Leave empty for no default privileges.',
		'default' => '',
	),
	'default_access_domains' => array(
		'label' => 'Default Domain Access for New Accounts',
		'help' => 'Which domain statuses automatically grant access to new accounts. Comma-separated list of: ds_open, ds_dev, ds_restricted. Default: ds_open.',
		'default' => 'ds_open',
	),
	'registration_open' => array(
		'label' => 'Account Registration',
		'help' => 'Set to 1 to let visitors create their own accounts, or 0 to close the registration page (accounts then come from the admin page or the database). Default is closed.',
		'default' => '0',
	),
);

try {
	$db = getNelDatabase();

	// Handle POST actions
	if ($_SERVER['REQUEST_METHOD'] === 'POST' && csrfValidate()) {
		if (isset($_POST['save_settings'])) {
			foreach ($allSettings as $key => $meta) {
				if (isset($_POST['setting_' . $key])) {
					$val = trim($_POST['setting_' . $key]);
					setSetting($key, $val);
				}
			}
			$success = 'Settings saved.';
		}
	}

	// Load current settings
	$stmt = $db->query('SELECT setting, value FROM setting');
	$rows = $stmt->fetchAll();
	foreach ($rows as $row) {
		$settings[$row['setting']] = $row['value'];
	}
} catch (PDOException $e) {
	$error = 'Database error. Please try again later.'; // the mysql text quotes host, user and query
}

ob_start();
?>
<div class="container">
	<?php if ($error): ?>
		<div class="alert alert-error"><?php echo h($error); ?></div>
	<?php endif; ?>
	<?php if ($success): ?>
		<div class="alert alert-success"><?php echo h($success); ?></div>
	<?php endif; ?>

	<div class="card">
		<h2>Dev Settings</h2>
		<p style="font-size:0.85rem; color:#8899a6; margin-bottom:1rem;">
			These settings are stored in the <code style="color:#5dade2;">setting</code> table and
			control account management behavior. Changes take effect immediately.
		</p>
		<form method="post" action="index.php?page=dev_settings">
			<?php echo csrfField(); ?>
			<?php foreach ($allSettings as $key => $meta): ?>
				<div class="form-group">
					<label for="setting_<?php echo h($key); ?>"><?php echo h($meta['label']); ?></label>
					<input type="text" id="setting_<?php echo h($key); ?>" name="setting_<?php echo h($key); ?>"
						value="<?php echo h(isset($settings[$key]) ? $settings[$key] : $meta['default']); ?>"
						placeholder="<?php echo h($meta['default']); ?>">
					<p style="font-size:0.8rem; color:#8899a6; margin-top:0.25rem;"><?php echo h($meta['help']); ?></p>
				</div>
			<?php endforeach; ?>
			<div class="form-group">
				<button type="submit" name="save_settings" class="btn btn-primary">Save Settings</button>
			</div>
		</form>
	</div>

	<div class="card">
		<h2>All Settings</h2>
		<?php if (empty($settings)): ?>
			<div class="empty-state"><p>No settings stored yet. Save above to initialize.</p></div>
		<?php else: ?>
			<table>
				<thead>
					<tr>
						<th>Setting</th>
						<th>Value</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($settings as $k => $v): ?>
					<tr>
						<td style="color:#ecf0f1;"><code><?php echo h($k); ?></code></td>
						<td><?php echo ($v !== '' && $v !== null) ? h($v) : '<span style="color:#8899a6;">(empty)</span>'; ?></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		<?php endif; ?>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
