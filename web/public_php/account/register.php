<?php

// Account Management Tool - Registration Page

$pageTitle = 'Create Account';

// Sites that hand out accounts by other means can close this page from the
// dev settings; the login service has the same idea in $ALLOW_UNKNOWN.
// Default closed: an open public registration form on a freshly installed
// shard is rarely what operators want.
$registrationOpen = getSetting('registration_open', '0') !== '0';

if (!$registrationOpen) {
	$error = 'Account registration is closed on this server.';
} elseif ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['register_submit'])) {
	if (!csrfValidate()) {
		$error = 'Invalid form submission. Please try again.';
	} elseif (accountActionThrottled('register')) {
		$error = 'Too many registration attempts. Please wait a few minutes and try again.';
	} else {
		$login = isset($_POST['login']) ? trim($_POST['login']) : '';
		$email = isset($_POST['email']) ? trim($_POST['email']) : '';
		$password = isset($_POST['password']) ? $_POST['password'] : '';
		$confirm = isset($_POST['confirm']) ? $_POST['confirm'] : '';

	// Validate inputs
	$error = accountNewFieldsError($login, $email, $password, $confirm);
	if ($error === '') {
		// Count this attempt before the uniqueness probes so enumeration
		// still burns the rate limit.
		accountActionRecordFailure('register');
		try {
			$db = getNelDatabase();

			// Default privileges for self-registration: known codes
			// only, and never staff ranks (those come from admin).
			$lowRisk = array_values(array_diff(knownPrivilegeCodes(), highRiskPrivilegeCodes()));
			$defaultPriv = sanitizePrivilegeString(
				getSetting('default_privileges', ''),
				$lowRisk
			);

			// Default permissions based on domain access setting
			$accessSetting = getSetting('default_access_domains', 'ds_open');

			// Refuses a taken username or email, else creates the user in
			// the nel user table with its permissions
			$created = accountCreate($db, $login, $email, $password, $confirm,
				$defaultPriv, explode(',', $accessSetting));
			if ($created['error'] !== '') {
				$error = $created['error'];
			} else {
				$uid = $created['uid'];

				// Log the user in immediately
				accountActionClearFailures('register');
				session_regenerate_id(true);
				$_SESSION['account_uid'] = $uid;
				$_SESSION['account_login'] = $login;
				$_SESSION['account_email'] = $email;
				// The session carries the privilege everything else gates
				// on; leaving the key out is a session in a state no other
				// entry point produces
				$_SESSION['account_privilege'] = $defaultPriv;
				redirect('home');
			}
		} catch (PDOException $e) {
			$error = 'Registration failed. Please try again later.';
		}
	}
	}
}

ob_start();
?>
<div class="auth-container">
	<div class="auth-card">
		<h1>Create Account</h1>
		<p class="subtitle">Ryzom Core Account Management</p>
		<?php if ($error): ?>
			<div class="alert alert-error"><?php echo h($error); ?></div>
		<?php endif; ?>
		<?php if ($registrationOpen): ?>
		<form method="post" action="index.php?page=register">
			<?php echo csrfField(); ?>
			<div class="form-group">
				<label for="login">Username</label>
				<input type="text" id="login" name="login" required autofocus
					value="<?php echo isset($_POST['login']) ? h($_POST['login']) : ''; ?>">
			</div>
			<div class="form-group">
				<label for="email">Email</label>
				<input type="email" id="email" name="email" required
					value="<?php echo isset($_POST['email']) ? h($_POST['email']) : ''; ?>">
			</div>
			<div class="form-group">
				<label for="password">Password</label>
				<input type="password" id="password" name="password" required>
			</div>
			<div class="form-group">
				<label for="confirm">Confirm Password</label>
				<input type="password" id="confirm" name="confirm" required>
			</div>
			<div class="form-group">
				<button type="submit" name="register_submit" class="btn btn-primary">Create Account</button>
			</div>
		</form>
		<?php endif; ?>
		<div class="auth-links">
			Already have an account? <a href="index.php?page=login">Sign in</a>
		</div>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
