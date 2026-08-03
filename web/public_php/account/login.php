<?php

// Account Management Tool - Login Page

$pageTitle = 'Sign In';

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['login_submit'])) {
	if (!csrfValidate()) {
		$error = 'Invalid form submission. Please try again.';
	} else {
		$login = isset($_POST['login']) ? trim($_POST['login']) : '';
		$password = isset($_POST['password']) ? $_POST['password'] : '';

		if ($login === '' || $password === '') {
			$error = 'Please enter your username and password.';
		} else {
			try {
				$db = getNelDatabase();
				$stmt = $db->prepare('SELECT UId, Login, Password, Email, Privilege FROM user WHERE Login = :login');
				$stmt->execute(array(':login' => $login));
				$user = $stmt->fetch();

				if ($user && verifyPassword($password, $user['Password'])) {
					session_regenerate_id(true);
					$_SESSION['account_uid'] = (int)$user['UId'];
					$_SESSION['account_login'] = $user['Login'];
					$_SESSION['account_email'] = $user['Email'];
					$_SESSION['account_privilege'] = isset($user['Privilege']) ? $user['Privilege'] : '';
					redirect('home');
				} else {
					$error = 'Invalid username or password.';
				}
			} catch (PDOException $e) {
				$error = 'Database connection failed. Please try again later.';
			}
		}
	}
}

ob_start();
?>
<div class="auth-container">
	<div class="auth-card">
		<h1>Sign In</h1>
		<p class="subtitle">Ryzom Core Account Management</p>
		<?php if ($error): ?>
			<div class="alert alert-error"><?php echo h($error); ?></div>
		<?php endif; ?>
		<form method="post" action="index.php?page=login">
			<?php echo csrfField(); ?>
			<div class="form-group">
				<label for="login">Username</label>
				<input type="text" id="login" name="login" required autofocus
					value="<?php echo isset($_POST['login']) ? h($_POST['login']) : ''; ?>">
			</div>
			<div class="form-group">
				<label for="password">Password</label>
				<input type="password" id="password" name="password" required>
			</div>
			<div class="form-group">
				<button type="submit" name="login_submit" class="btn btn-primary">Sign In</button>
			</div>
		</form>
		<?php if (getSetting('registration_open', '1') !== '0'): ?>
		<div class="auth-links">
			Don't have an account? <a href="index.php?page=register">Create one</a>
		</div>
		<?php endif; ?>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
