<?php
	class Logfile {
		private $logfile;
		private $buffer;

		function Logfile($f) {
			$this->logfile = $f;
			$this->buffer = "";
		}

		function append($t) {
			$this->buffer .= $t;
		}

		function write() {
			$f = fopen($this->logfile.'.'.date("Ymd",time()).'.txt','a');
			fwrite($f,$this->buffer);
			fclose($f);
			$this->buffer = "";
		}
	}
?>