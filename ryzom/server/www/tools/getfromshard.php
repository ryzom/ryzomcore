<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');

$args = array_slice($argv, 1);
$ret = queryShard('egs', implode(" ", $args));

if ($ret['status']) {
	$final = explode("\n", $ret['raw'][0]);
	array_shift($final);
	array_shift($final);
	echo implode("\n", $final);
}

