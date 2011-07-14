<?php
class DATABASE_CONFIG {

	var $default = array(
		'driver' => 'mysqli',
		'persistent' => false,
		'host' => 'localhost',
		'login' => 'webtt',
		'password' => 'webtt77',
		'database' => 'webtt2',
		'encoding' => 'utf8'
	);
	var $raw_files = array(
		'datasource' => 'RawFilesSource',
		'path' => '/home/kaczorek/projects/webtt/distfiles/translation',
		'extension' => '(uxt|txt)',
		'readonly' => true,
		'recursive' => true,
	);
}
?>