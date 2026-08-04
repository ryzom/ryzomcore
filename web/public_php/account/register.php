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
	if ($login === '' || $email === '' || $password === '' || $confirm === '') {
		$error = 'All fields are required.';
	} elseif (!nel_account_name_length_ok($login)) {
		$error = 'Username must be between ' . nel_account_name_min_length()
			. ' and ' . nel_account_name_max_length() . ' characters.';
	} elseif (!nel_is_valid_account_name($login)) {
		$error = 'Username may only contain letters, numbers, and underscores.';
	} elseif (!filter_var($email, FILTER_VALIDATE_EMAIL) || strlen($email) > 255) {
		$error = 'Please enter a valid email address.';
	} elseif (!accountPasswordAcceptable($password)) {
		$error = 'Password must be between ' . accountPasswordMinLength()
			. ' and ' . accountPasswordMaxLength() . ' characters.';
	} elseif ($password !== $confirm) {
		$error = 'Passwords do not match.';
	} else {
		// Count this attempt before the uniqueness probes so enumeration
		// still burns the rate limit.
		accountActionRecordFailure('register');
		try {
			$db = getNelDatabase();

			// Check if username already exists
			$stmt = $db->prepare('SELECT UId FROM user WHERE Login = :login');
			$stmt->execute(array(':login' => $login));
			if ($stmt->fetch()) {
				$error = 'This username is already taken.';
			} else {
				// Check if email already exists
				$stmt = $db->prepare('SELECT UId FROM user WHERE Email = :email');
				$stmt->execute(array(':email' => $email));
				if ($stmt->fetch()) {
					$error = 'This email is already registered.';
				} else {
					// Create the user directly in the nel user table
					$hashedPassword = hashPassword($password);

					// Default privileges for self-registration: known codes
					// only, and never staff ranks (those come from admin).
					$lowRisk = array_values(array_diff(knownPrivilegeCodes(), highRiskPrivilegeCodes()));
					$defaultPriv = sanitizePrivilegeString(
						getSetting('default_privileges', ''),
						$lowRisk
					);

					$stmt = $db->prepare('INSERT INTO user (Login, Password, Email, Privilege) VALUES (:login, :pass, :email, :priv)');
					$stmt->execute(array(
						':login' => $login,
						':pass' => $hashedPassword,
						':email' => $email,
						':priv' => $defaultPriv,
					));
					$uid = (int)$db->lastInsertId();

					// Create default permissions based on domain access setting
					$accessSetting = getSetting('default_access_domains', 'ds_open');
					$allowedStatuses = array('ds_open', 'ds_dev', 'ds_restricted', 'ds_close');
					$accessStatuses = array();
					foreach (array_map('trim', explode(',', $accessSetting)) as $st) {
						if (in_array($st, $allowedStatuses, true)) {
							$accessStatuses[] = $st;
						}
					}
					if (!empty($accessStatuses)) {
						$domains = $db->query("SELECT domain_id, status FROM domain");
						foreach ($domains as $domain) {
							if (in_array($domain['status'], $accessStatuses, true)) {
								// The login service turns the domain status into the
								// access privilege it then looks for (ds_dev => DEV),
								// so a row written as OPEN on a dev domain refuses
								// the very account it was meant to admit. It is also
								// a SET column: only its own members may be stored.
								$accessPriv = strtoupper(substr((string)$domain['status'], 3));
								if (!in_array($accessPriv, array('OPEN', 'DEV', 'RESTRICTED'), true)) {
									continue;
								}
								$pstmt = $db->prepare('INSERT INTO permission (UId, DomainId, AccessPrivilege) VALUES (:uid, :did, :priv)');
								$pstmt->execute(array(
									':uid' => $uid,
									':did' => (int)$domain['domain_id'],
									':priv' => $accessPriv,
								));
							}
						}
					}

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
