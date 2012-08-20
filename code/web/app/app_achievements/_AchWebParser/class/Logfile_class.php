<?php
	/*
	 * Logging, logging, logging....
	 */

	class Logfile {
		private $logfile;

		function Logfile($f = false) {
			$this->logfile = false;

			if($f != false) {
				$this->logfile = fopen($this->logfile.'.'.$_REQUEST['file'].'.'.date("Ymd",time()).'.txt','a');;
			}
		}

		function logf($t,$nl = true) {
			$this->write($t);
			if($nl == true) {
				$this->write("\n");
			}
		}

		function logi($t,$nl = true) {
			$this->write("    > ".$t);
			if($nl == true) {
				$this->write("\n");
			}
		}

		function write($txt) {
			fwrite($this->logfile,$txt);
		}

		function close() {
			fclose($this->logfile);
		}
	}
?>