<?php
	function logf($txt,$nl = true) {
		global $logfile;
		if($logfile) {
			if($nl) {
				$txt .= "\n";
			}
			fwrite($logfile,"[".date('H:i:s',time())."] ".$txt);
		}
	}

	function logi($txt,$i = 1) {
		$tmp = "";
		for($v=0;$v<$i;$v++) {
			$tmp .= "   ";
		}
		return $tmp."> ".$txt;
	}
?>