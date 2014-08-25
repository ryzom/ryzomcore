<?php
	function dateTime_to_timestamp($dt) {
		#2012-05-12 00:26:40
		$tmp = explode(" ",$dt);
		$d = explode("-",$tmp[0]);
		$t = explode(":",$tmp[1]);

		return mktime($t[0],$t[1],$t[2],$d[1],$d[2],$d[0]);
	}

	function curl_get_file_contents($URL) { // http://developers.facebook.com/blog/post/2011/05/13/how-to--handle-expired-access-tokens/
		$c = curl_init();
		curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($c, CURLOPT_URL, $URL);
		$contents = curl_exec($c);
		#$err  = curl_getinfo($c,CURLINFO_HTTP_CODE);
		curl_close($c);
		if ($contents) return $contents;
		else return FALSE;
	}
?>