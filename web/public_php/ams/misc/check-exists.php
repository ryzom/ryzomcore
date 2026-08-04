<?php
/*
Uploadify
Copyright (c) 2012 Reactive Apps, Ronnie Garcia
Released under the MIT License <http://www.opensource.org/licenses/mit-license.php> 
*/

require_once(dirname(dirname(__DIR__)) . '/config.php');

require_once( $AMS_LIB . '/libinclude.php' );
session_start();

// this answers whether a path exists, so it is not for anonymous callers
if (!WebUsers::isLoggedIn()) {
	echo 0;
	return;
}

// Define a destination
$targetFolder = '/uploads'; // Relative to the root and should match the upload folder in the uploader script

// the name is posted by the browser: keep it to a file inside $targetFolder,
// or this reports on any path on the server
$filename = isset($_POST['filename']) ? basename(str_replace("\\", "/", (string)$_POST['filename'])) : '';

if ($filename !== '' && file_exists($_SERVER['DOCUMENT_ROOT'] . $targetFolder . '/' . $filename)) {
	echo 1;
} else {
	echo 0;
}
?>
