<?php

// Account Management Tool - Account Settings Page

$pageTitle = 'Settings';
$uid = $_SESSION['account_uid'];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
	if (!csrfValidate()) {
		$error = 'Invalid form submission. Please try again.';
	} else {
	$action = isset($_POST['settings_action']) ? $_POST['settings_action'] : '';

	try {
		$db = getNelDatabase();

		if ($action === 'change_password') {
			$currentPass = isset($_POST['current_password']) ? $_POST['current_password'] : '';
			$newPass = isset($_POST['new_password']) ? $_POST['new_password'] : '';
			$confirmPass = isset($_POST['confirm_password']) ? $_POST['confirm_password'] : '';

			if ($currentPass === '' || $newPass === '' || $confirmPass === '') {
				$error = 'All password fields are required.';
			} elseif (strlen($newPass) < 5) {
				$error = 'New password must be at least 5 characters.';
			} elseif ($newPass !== $confirmPass) {
				$error = 'New passwords do not match.';
			} else {
				// Verify current password
				$stmt = $db->prepare('SELECT Password FROM user WHERE UId = :uid');
				$stmt->execute(array(':uid' => $uid));
				$user = $stmt->fetch();

				if ($user && verifyPassword($currentPass, $user['Password'])) {
					$hashedPassword = hashPassword($newPass);
					$stmt = $db->prepare('UPDATE user SET Password = :pass WHERE UId = :uid');
					$stmt->execute(array(':pass' => $hashedPassword, ':uid' => $uid));
					$success = 'Password updated successfully.';
				} else {
					$error = 'Current password is incorrect.';
				}
			}
		} elseif ($action === 'change_email') {
			$newEmail = isset($_POST['new_email']) ? trim($_POST['new_email']) : '';
			$password = isset($_POST['email_password']) ? $_POST['email_password'] : '';

			if ($newEmail === '' || $password === '') {
				$error = 'Email and password are required.';
			} elseif (!filter_var($newEmail, FILTER_VALIDATE_EMAIL)) {
				$error = 'Please enter a valid email address.';
			} else {
				// Verify password
				$stmt = $db->prepare('SELECT Password FROM user WHERE UId = :uid');
				$stmt->execute(array(':uid' => $uid));
				$user = $stmt->fetch();

				if ($user && verifyPassword($password, $user['Password'])) {
					// Check if email is already in use
					$stmt = $db->prepare('SELECT UId FROM user WHERE Email = :email AND UId != :uid');
					$stmt->execute(array(':email' => $newEmail, ':uid' => $uid));
					if ($stmt->fetch()) {
						$error = 'This email is already in use by another account.';
					} else {
						$stmt = $db->prepare('UPDATE user SET Email = :email WHERE UId = :uid');
						$stmt->execute(array(':email' => $newEmail, ':uid' => $uid));
						$_SESSION['account_email'] = $newEmail;
						$success = 'Email updated successfully.';
					}
				} else {
					$error = 'Password is incorrect.';
				}
			}
		}
	} catch (PDOException $e) {
		$error = 'Update failed. Please try again later.';
	}
	}
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

	<div class="grid grid-2">
		<div class="card">
			<h2>Change Password</h2>
			<form method="post" action="index.php?page=settings">
				<?php echo csrfField(); ?>
				<input type="hidden" name="settings_action" value="change_password">
				<div class="form-group">
					<label for="current_password">Current Password</label>
					<input type="password" id="current_password" name="current_password" required>
				</div>
				<div class="form-group">
					<label for="new_password">New Password</label>
					<input type="password" id="new_password" name="new_password" required>
				</div>
				<div class="form-group">
					<label for="confirm_password">Confirm New Password</label>
					<input type="password" id="confirm_password" name="confirm_password" required>
				</div>
				<div class="form-group">
					<button type="submit" class="btn btn-primary">Update Password</button>
				</div>
			</form>
		</div>

		<div class="card">
			<h2>Change Email</h2>
			<p style="color:#8899a6; font-size:0.85rem; margin-bottom:1rem;">Current email: <?php echo h($_SESSION['account_email']); ?></p>
			<form method="post" action="index.php?page=settings">
				<?php echo csrfField(); ?>
				<input type="hidden" name="settings_action" value="change_email">
				<div class="form-group">
					<label for="new_email">New Email</label>
					<input type="email" id="new_email" name="new_email" required>
				</div>
				<div class="form-group">
					<label for="email_password">Confirm Password</label>
					<input type="password" id="email_password" name="email_password" required>
				</div>
				<div class="form-group">
					<button type="submit" class="btn btn-primary">Update Email</button>
				</div>
			</form>
		</div>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
