<?php
	function logf($txt,$nl = true) {
		global $logfile;
		if($logfile) {
			if($nl) {
				$txt .= "\n";
			}
			$logfile->append("[".date('H:i:s',time())."] ".$txt);
		}
	}

	function logi($txt,$i = 1) {
		$tmp = "";
		for($v=0;$v<$i;$v++) {
			$tmp .= "   ";
		}
		return $tmp."> ".$txt;
	}

	function dateTime_to_timestamp($dt) {
		#2012-05-12 00:26:40
		$tmp = explode(" ",$dt);
		$d = explode("-",$tmp[0]);
		$t = explode(":",$tmp[1]);

		return mktime($t[0],$t[1],$t[2],$d[1],$d[2],$d[0]);
	}

	function received_char($res, $url, $ch, $argv) {
		logf("character tracking returned: ".$res);
	}
?>