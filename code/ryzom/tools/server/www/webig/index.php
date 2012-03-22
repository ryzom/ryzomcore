<?php

$__start = microtime(true);
require_once 'lib/functions.php';

session_start();

// do user login
$user = app_authenticate();

if(empty($user)){
	// user not verified
	//
	$error = is($_SESSION['login']['error'], '');
	if(!empty($error)){
		$error = '<p style="color: red;">login error</p>';
	}
	echo '
	<html>
	<head>
		<title>WebIG - Login</title>
	</head>
	<body
		<h1>Login</h1>
		<form method="post" action="">
			<input type="hidden" name="login[shardid]" value="302" />
			Char name <input type="text" name="login[name]" value="" /><br />
			Password <input type="password" name="login[passwd]" value="" /><br />
			<input type="submit" name="login[submit]" value="Login" />
		</form>
	</body>
	</html>';
	exit;
}

// if this was login request from app, then redirect back there
$redirect = is($_GET['redirect'], '');
if(!empty($redirect)){
	header('Location: '.$redirect);
	exit;
}

// check user privileges
$is_admin = webig_is_admin($user['cid']>>4);

// get more info about character - race, civlization, cult, guild, etc
$character = webig_load_character($user['cid']);

// user is verified
?>
<html>
<head>
	<title>App Index</title>
</head>
<body>
	<h1>Hello "<?=h($user['name'])?>"!</h1>

	<a href="index.php">index</a> | <a href="app_hello.php">Hello APP</a><?=(!isWEBIG ? '| <a href="?logout">logout</a>' : '')?>

<?php
	if($is_admin){
		if(isWEBIG){
			display_teleport_list();
		}
	}

echo '<h2>User info</h2>';
echo 'USER:'.dump_array($user);
echo 'CHARACTER:'.dump_array($character);

	$__end = microtime(true);
	echo "<pre>\n---\npage created ".sprintf("%.5fsec", $__end - $__start).'</pre>';

?>
</body>
</html>
<?php

function dump_array($array){
	ob_start();

	echo '
	<table cellspacing="2" cellpadding="0" bgcolor="#4f4f4f">
	<tr><td>
	<table cellspacing="0" cellpadding="0">
	';
	$c=0;
	foreach($array as $k => $v){
		if(is_array($v)){
			$v = dump_array($v);
		}else{
			// make value safe for html
			$v = h($v);
		}
		echo '
		<tr valign="middle" bgcolor="'.($c%2 ? '#9f9f9f' : '#909090').'">
			<td height="20">'.h($k).'</td><td>'.$v.'</td>
		</tr>
		';
		$c++;
	}
	echo '
		</table>
	</td></tr>
	</table>
	';

	return ob_get_clean();
}

function display_teleport_list(){
	$places = array(
		'Ranger Camp'  => array(10314,-11734),
		'Shining Lake' => array(9056, -10822),
	);
?>
	<h2>Teleport destinations</h2>
	<table cellspacing="2" cellpadding="0" bgcolor="#4f4f4f">
	<tr><td>
		<table cellspacing="0" cellpadding="0">
		<?php $c=0; foreach($places as $txt => $xyz){
			echo '
			<tr valign="middle" bgcolor="'.($c%2 ? '#3f3f3f' : '#303030').'">
				<td height="20">'.h($txt).'</td><td><a href="ah:command&a&Position&'.join(',', $xyz).'">'.join(',', $xyz).'</a></td>
			</tr>
			';
			$c++;
		} ?>
		</table>
	</td></tr>
	</table>
<?php
}
