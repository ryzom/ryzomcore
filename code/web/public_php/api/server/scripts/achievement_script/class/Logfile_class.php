<?php
	/*
	 * Logging, logging, logging....
	 */

	class Logfile {
		private $logfile;

		function Logfile($f = false,$logto = "std") {
			global $CONF,$MY_PATH;
			$this->logfile = false;

			if($f != false) {


				$ldir = $MY_PATH.$CONF['logfile'].date("Y-m-d",time());

				#$tmp = fopen($MY_PATH."/testlog.txt",'a+');
				#fwrite($tmp, $ldir.'/'.date("H",time()).".txt\n");
				#fclose($tmp);

				if(!is_dir($ldir)) {
					mkdir($ldir,0777,true);
				}
				$this->logfile = fopen($ldir.'/'.date("H",time()).'_'.$logto.'.txt','a+');
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
			if($this->logfile != false) {
				fwrite($this->logfile,$txt);
			}
		}

		function close() {
			if($this->logfile != false) {
				fclose($this->logfile);
			}
			#echo "ii";
		}
	}
?>