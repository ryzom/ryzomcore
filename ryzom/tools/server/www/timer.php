<?php

	function getmicrotime()
	{
		list($usec, $sec) = explode(" ",microtime());
		return ((float)$usec + (float)$sec);
	}

	$__start = getmicrotime();

	function display_time()
	{
		global	$__start;
		$end = getmicrotime();
		echo (($end-$__start)*1000)." ms";
	}

?>