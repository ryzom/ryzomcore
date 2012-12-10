<?php
	/*
	 * Logging, logging, logging....
	 */

	class Logfile {
		private $logfile;

		function Logfile($f = false) {
			global $CONF,$MY_PATH;
			$this->logfile = false;

			if($f != false) {
				$this->logfile = fopen($MY_PATH.$CONF['logfile'].'.'.date("Ymd",time()).'.txt','a+');
				#echo "kk";
			}
		}

		function logf($t,$nl = true) {
			$this->write("[".date("H:i:s")."] ".$t);
			if($nl == true) {
				$this->write("\n");
			}
		}

		function logi($t,$nl = true) {
			#echo $t;
			$this->write("[".date("H:i:s")."]     > ".$t);
			if($nl == true) {
				$this->write("\n");
			}
		}

		function write($txt) {
			#echo $txt;
			fwrite($this->logfile,$txt);
		}

		function close() {
			fclose($this->logfile);
			#echo "ii";
		}
	}
?>