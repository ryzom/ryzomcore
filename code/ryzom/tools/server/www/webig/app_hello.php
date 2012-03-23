<?php

require_once 'lib/functions.php';

$user = app_authenticate();
if(empty($user)){
	// redirect to login page
	$url = 'http://'.$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
	header('Location: index.php?redirect='.urlencode($url));
	exit;
}

// get more info about character - race, civlization, cult, guild, etc
$character = webig_load_character($user['cid']);

?>
<html>
<head>
	<title>App Hello World!</title>
</head>
<body>
	<h1>APP Hello World!</h1>

	<a href="index.php">index</a><?=(!isWEBIG ? '| <a href="?logout">logout</a>' : '')?>

	<h2>Character</h2>
	<pre><?php print_r($character);?></pre>
</body>
</html>
