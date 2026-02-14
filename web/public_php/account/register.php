<?php

// Account Management Tool - Registration Page

$pageTitle = 'Create Account';

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['register_submit'])) {
	$login = isset($_POST['login']) ? trim($_POST['login']) : '';
	$email = isset($_POST['email']) ? trim($_POST['email']) : '';
	$password = isset($_POST['password']) ? $_POST['password'] : '';
	$confirm = isset($_POST['confirm']) ? $_POST['confirm'] : '';

	// Validate inputs
	if ($login === '' || $email === '' || $password === '' || $confirm === '') {
		$error = 'All fields are required.';
	} elseif (strlen($login) < 3 || strlen($login) > 64) {
		$error = 'Username must be between 3 and 64 characters.';
	} elseif (!preg_match('/^[a-zA-Z0-9_]+$/', $login)) {
		$error = 'Username may only contain letters, numbers, and underscores.';
	} elseif (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
		$error = 'Please enter a valid email address.';
	} elseif (strlen($password) < 5) {
		$error = 'Password must be at least 5 characters.';
	} elseif ($password !== $confirm) {
		$error = 'Passwords do not match.';
	} else {
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

					$stmt = $db->prepare('INSERT INTO user (Login, Password, Email) VALUES (:login, :pass, :email)');
					$stmt->execute(array(
						':login' => $login,
						':pass' => $hashedPassword,
						':email' => $email,
					));
					$uid = (int)$db->lastInsertId();

					// Create default permissions for all open domains
					$domains = $db->query("SELECT domain_id, status FROM domain");
					foreach ($domains as $domain) {
						if ($domain['status'] === 'ds_open') {
							$pstmt = $db->prepare('INSERT INTO permission (UId, DomainId, AccessPrivilege) VALUES (:uid, :did, :priv)');
							$pstmt->execute(array(
								':uid' => $uid,
								':did' => (int)$domain['domain_id'],
								':priv' => 'OPEN',
							));
						}
					}

					// Log the user in immediately
					$_SESSION['account_uid'] = $uid;
					$_SESSION['account_login'] = $login;
					$_SESSION['account_email'] = $email;
					redirect('home');
				}
			}
		} catch (PDOException $e) {
			$error = 'Registration failed. Please try again later.';
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
		<form method="post" action="index.php?page=register">
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
		<div class="auth-links">
			Already have an account? <a href="index.php?page=login">Sign in</a>
		</div>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
