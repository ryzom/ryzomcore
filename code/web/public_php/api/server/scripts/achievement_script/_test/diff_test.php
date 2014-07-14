<?php
	require_once("diff_class.php");

	$microstart = explode(' ',microtime());
	$start_time = $microstart[0] + $microstart[1];

	echo Diff::toString(Diff::compareFiles('old_char_346.xml', 'char_346.xml', false));

	$microstop = explode(' ',microtime());
	$stop_time = $microstop[0] + $microstop[1];

	echo "Expired time: ".($stop_time - $start_time)."<br>";

	echo "Memory load: ".memory_get_usage()." bytes";
?>