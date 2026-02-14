<?php

// Account Management Tool - Main Entry Point

session_start();

require_once('config.php');

// Determine the requested page
$page = isset($_GET['page']) ? $_GET['page'] : '';

// Pages that don't require authentication
$public_pages = array('login', 'register');

// Check authentication
$logged_in = isset($_SESSION['account_uid']);

if (!$logged_in && !in_array($page, $public_pages)) {
	$page = 'login';
}

// Default page for logged-in users
if ($logged_in && ($page === '' || $page === 'login' || $page === 'register')) {
	$page = 'home';
}

// Route to the appropriate page
$valid_pages = array('login', 'register', 'home', 'characters', 'sessions', 'settings', 'admin', 'dev_settings', 'logout');

if (!in_array($page, $valid_pages)) {
	$page = $logged_in ? 'home' : 'login';
}

// Handle logout
if ($page === 'logout') {
	session_destroy();
	header('Location: index.php?page=login');
	exit;
}

// Handle stop impersonation
if (isset($_GET['stop_impersonate']) && $logged_in && isImpersonating()) {
	stopImpersonation();
	header('Location: index.php?page=admin');
	exit;
}

// Gate admin and dev_settings pages while impersonating (force exit first)
if (isImpersonating() && ($page === 'admin' || $page === 'dev_settings')) {
	$page = 'home';
}

// Gate admin page behind privilege check
if ($page === 'admin' && !isAdmin()) {
	$page = 'home';
}

// Gate dev settings page behind settings privilege check
if ($page === 'dev_settings' && !canEditSettings()) {
	$page = 'home';
}

// Process the page
$error = '';
$success = '';
$pageTitle = 'Account';
$content = '';

require_once($page . '.php');

// Render the layout
renderLayout($pageTitle, $content, $page, $logged_in);

/**
 * Render the page layout.
 */
function renderLayout($title, $content, $currentPage, $loggedIn)
{
	$navItems = '';
	$impersonationBanner = '';
	if ($loggedIn) {
		$user = h($_SESSION['account_login']);
		$pages = array(
			'home' => 'Home',
			'characters' => 'Characters',
			'sessions' => 'Sessions',
			'settings' => 'Settings',
		);
		if (!isImpersonating()) {
			if (isAdmin()) {
				$pages['admin'] = 'Admin';
			}
			if (canEditSettings()) {
				$pages['dev_settings'] = 'Dev';
			}
		}
		foreach ($pages as $key => $label) {
			$active = ($currentPage === $key) ? ' class="active"' : '';
			$navItems .= '<a href="index.php?page=' . $key . '"' . $active . '>' . $label . '</a>';
		}
		if (isImpersonating()) {
			$adminLogin = h($_SESSION['impersonate_admin_login']);
			$impersonationBanner = '<div class="impersonate-bar">Viewing as <strong>' . $user . '</strong> &mdash; Logged in as ' . $adminLogin . ' &mdash; <a href="index.php?stop_impersonate=1">Exit</a></div>';
			$navItems .= '<a href="index.php?stop_impersonate=1" class="nav-right" style="color:#f5b7b1;">Exit View</a>';
			$navItems .= '<span class="nav-right nav-user">' . $user . '</span>';
		} else {
			$navItems .= '<a href="index.php?page=logout" class="nav-right">Sign Out</a>';
			$navItems .= '<span class="nav-right nav-user">' . $user . '</span>';
		}
	}

?><!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title><?php echo h($title); ?> - Ryzom Core</title>
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
body { font-family: system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif; background: #0f1923; color: #c8d6e5; line-height: 1.6; min-height: 100vh; }
a { color: #5dade2; text-decoration: none; }
a:hover { color: #85c1e9; }

.navbar { background: #1a2634; border-bottom: 1px solid #2c3e50; padding: 0 1.5rem; display: flex; align-items: center; height: 3.5rem; }
.navbar .brand { font-weight: 700; font-size: 1.1rem; color: #ecf0f1; margin-right: 2rem; }
.navbar a { color: #8899a6; padding: 0.5rem 0.75rem; border-radius: 0.25rem; font-size: 0.9rem; }
.navbar a:hover, .navbar a.active { color: #ecf0f1; background: #253545; }
.navbar .nav-right { margin-left: auto; }
.navbar .nav-right + .nav-right { margin-left: 0; }
.navbar .nav-user { color: #5dade2; font-size: 0.85rem; padding: 0.5rem 0.75rem; }

.container { max-width: 64rem; margin: 2rem auto; padding: 0 1.5rem; }
.card { background: #1a2634; border: 1px solid #2c3e50; border-radius: 0.5rem; padding: 1.5rem; margin-bottom: 1.5rem; }
.card h2 { color: #ecf0f1; font-size: 1.1rem; margin-bottom: 1rem; padding-bottom: 0.5rem; border-bottom: 1px solid #2c3e50; }

.auth-container { max-width: 24rem; margin: 4rem auto; padding: 0 1.5rem; }
.auth-card { background: #1a2634; border: 1px solid #2c3e50; border-radius: 0.5rem; padding: 2rem; }
.auth-card h1 { color: #ecf0f1; font-size: 1.3rem; text-align: center; margin-bottom: 1.5rem; }
.auth-card .subtitle { text-align: center; color: #8899a6; margin-bottom: 1.5rem; font-size: 0.9rem; }

.form-group { margin-bottom: 1rem; }
.form-group label { display: block; font-size: 0.85rem; color: #8899a6; margin-bottom: 0.3rem; }
.form-group input[type="text"],
.form-group input[type="password"],
.form-group input[type="email"],
.form-group textarea,
.form-group select { width: 100%; padding: 0.5rem 0.75rem; background: #0f1923; border: 1px solid #2c3e50; border-radius: 0.25rem; color: #ecf0f1; font-size: 0.9rem; font-family: inherit; }
.form-group input:focus, .form-group textarea:focus, .form-group select:focus { outline: none; border-color: #5dade2; }
.form-group textarea { resize: vertical; min-height: 3rem; }

.btn { display: inline-block; padding: 0.5rem 1rem; border: none; border-radius: 0.25rem; font-size: 0.9rem; cursor: pointer; font-family: inherit; text-align: center; }
.btn-primary { background: #2e86c1; color: #fff; width: 100%; }
.btn-primary:hover { background: #2471a3; color: #fff; }
.btn-sm { padding: 0.3rem 0.75rem; font-size: 0.8rem; }
.btn-danger { background: #922b21; color: #fff; }
.btn-danger:hover { background: #7b241c; color: #fff; }
.btn-secondary { background: #2c3e50; color: #ecf0f1; }
.btn-secondary:hover { background: #34495e; color: #ecf0f1; }

.alert { padding: 0.75rem 1rem; border-radius: 0.25rem; margin-bottom: 1rem; font-size: 0.9rem; }
.alert-error { background: #641e16; border: 1px solid #922b21; color: #f5b7b1; }
.alert-success { background: #0e6251; border: 1px solid #148f77; color: #a3e4d7; }

table { width: 100%; border-collapse: collapse; font-size: 0.9rem; }
table th { text-align: left; color: #8899a6; font-weight: 600; padding: 0.5rem; border-bottom: 1px solid #2c3e50; font-size: 0.8rem; text-transform: uppercase; letter-spacing: 0.05em; }
table td { padding: 0.5rem; border-bottom: 1px solid #1e3040; }
table tr:last-child td { border-bottom: none; }

.badge { display: inline-block; padding: 0.15rem 0.5rem; border-radius: 1rem; font-size: 0.75rem; font-weight: 600; }
.badge-green { background: #0e6251; color: #a3e4d7; }
.badge-red { background: #641e16; color: #f5b7b1; }
.badge-blue { background: #1a5276; color: #85c1e9; }
.badge-yellow { background: #7d6608; color: #f9e79f; }
.badge-gray { background: #2c3e50; color: #8899a6; }

.grid { display: grid; gap: 1.5rem; }
.grid-2 { grid-template-columns: repeat(auto-fit, minmax(18rem, 1fr)); }
.grid-3 { grid-template-columns: repeat(auto-fit, minmax(14rem, 1fr)); }

.stat-value { font-size: 1.5rem; font-weight: 700; color: #5dade2; }
.stat-label { font-size: 0.8rem; color: #8899a6; }

.auth-links { text-align: center; margin-top: 1rem; font-size: 0.85rem; color: #8899a6; }

.empty-state { text-align: center; padding: 2rem; color: #8899a6; }
.empty-state p { margin-bottom: 0.5rem; }

.form-inline { display: flex; gap: 0.5rem; align-items: flex-end; flex-wrap: wrap; }
.form-inline .form-group { margin-bottom: 0; }

.impersonate-bar { background: #7d6608; color: #f9e79f; text-align: center; padding: 0.4rem 1rem; font-size: 0.85rem; }
.impersonate-bar a { color: #fff; font-weight: 700; text-decoration: underline; }
.impersonate-bar a:hover { color: #f5b7b1; }
</style>
</head>
<body>
<?php if ($loggedIn): ?>
<?php echo $impersonationBanner; ?>
<nav class="navbar">
	<span class="brand">Ryzom Core</span>
	<?php echo $navItems; ?>
</nav>
<?php endif; ?>
<?php echo $content; ?>
</body>
</html>
<?php
}

/* end of file */
