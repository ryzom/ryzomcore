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
			$rejected = array();
			foreach ($allSettings as $key => $meta) {
				if (!isset($_POST['setting_' . $key])) {
					continue;
				}
				$val = trim($_POST['setting_' . $key]);
				if ($key === 'registration_open') {
					if ($val !== '0' && $val !== '1') {
						$rejected[] = $key;
						continue;
					}
				} elseif ($key === 'admin_privileges' || $key === 'settings_privilege') {
					if (!isValidPrivilegeSetting($val) || $val === '') {
						// Empty would lock every account out of the page that
						// can fix it; refuse blank staff gates.
						$rejected[] = $key;
						continue;
					}
				} elseif ($key === 'default_privileges') {
					// Self-registration defaults: known codes only, no staff ranks.
					$lowRisk = array_values(array_diff(knownPrivilegeCodes(), highRiskPrivilegeCodes()));
					if ($val !== '' && !isValidPrivilegeSetting($val)) {
						$rejected[] = $key;
						continue;
					}
					$val = sanitizePrivilegeString($val, $lowRisk);
				} elseif ($key === 'default_access_domains') {
					$allowed = array('ds_open', 'ds_dev', 'ds_restricted', 'ds_close');
					$parts = array();
					foreach (array_map('trim', explode(',', $val)) as $st) {
						if ($st !== '' && in_array($st, $allowed, true)) {
							$parts[] = $st;
						}
					}
					if ($val !== '' && empty($parts)) {
						$rejected[] = $key;
						continue;
					}
					$val = implode(',', $parts);
				} elseif (strlen($val) > 255) {
					$rejected[] = $key;
					continue;
				}
				setSetting($key, $val);
			}
			if (!empty($rejected)) {
				$error = 'Some settings were not saved (invalid value): ' . implode(', ', $rejected) . '.';
				if (count($rejected) < count($allSettings)) {
					$success = 'Other settings saved.';
				}
			} else {
				$success = 'Settings saved.';
			}
		}
	}

	// Load only the keys this page manages — the setting table is a free
	// key/value store and must not dump arbitrary rows to every DEV session.
	foreach (array_keys($allSettings) as $key) {
		$settings[$key] = getSetting($key, $allSettings[$key]['default']);
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
		<h2>Effective Values</h2>
		<table>
			<thead>
				<tr>
					<th>Setting</th>
					<th>Value</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($allSettings as $k => $meta):
					$v = isset($settings[$k]) ? $settings[$k] : $meta['default'];
				?>
				<tr>
					<td style="color:#ecf0f1;"><code><?php echo h($k); ?></code></td>
					<td><?php echo ($v !== '' && $v !== null) ? h($v) : '<span style="color:#8899a6;">(empty)</span>'; ?></td>
				</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
