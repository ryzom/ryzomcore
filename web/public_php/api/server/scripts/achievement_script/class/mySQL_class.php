<?php
	/*
	 * MySQL connection class
	 */
	class mySQL {
		var $DBc;
		var $DBstats;
		var $cached;
		var $sqltime;
		var $sqltime_post;
		var $longQuery;

		function mre($in) { // shorter than "mysql_real_escape_string"
			if(is_array($in)) {
				foreach($in as $key=>$elem) {
					$in[$key] = mysql_real_escape_string(stripslashes($elem));
				}
			}
			else {
				$in = mysql_real_escape_string(stripslashes($in));
			}
			return $in;
		}

		function mySQL($err=false) { // constructor
			$this->DBstats = array();
			$this->DBc = false;
			//set error handling
			if($err === "DIE" || $err === "PRINT" || $err === "ALERT" || $err === "HIDE" || $err === "LOG") {
				$this->DBerror = $err;
			}
			else {
				$this->DBerror = "HIDE";
			}
			$this->resetStats(); // reset stats counter
			$this->cached = false;
			$this->sqltime = 0;
			$this->sqltime_post = 0;
			$this->longQuery = array();
		}

		function connect($ip,$user,$pass,$db=false) { // connect
			$this->DBc = mysql_pconnect($ip,$user,$pass) or $this->error(mysql_error());
			if($this->DBc && $db) {
				$this->database($db);
			}
			$this->resetStats();
		}

		function database($db) { // set database
			if(!$this->DBc) {
				return false;
			}
			mysql_select_db($db,$this->DBc) or $this->error(mysql_error());
		}

		function resetStats() {
			$this->DBstats['query'] = 0;
			$this->DBstats['error'] = 0;
		}

		function getStats() { // return stats
			return $this->DBstats;
		}

		function sendSQL($query,$handling="PLAIN",$buffer=false) { // can be INSERT, DELETE, UPDATE, ARRAY, NONE, PLAIN
			if(!$this->DBc) {
				return false;
			}

			$microstart = explode(' ',microtime());
			$start_time = $microstart[0] + $microstart[1];

			if($buffer === false && $handling !== "PLAIN") {
				$res = mysql_unbuffered_query($query,$this->DBc) or $this->error(mysql_error(),$query);
			}
			else {
				$res = mysql_query($query,$this->DBc) or $this->error(mysql_error(),$query);
			}

			$this->DBstats['query']++;

			$microstop = explode(' ',microtime());
			$stop_time = $microstop[0] + $microstop[1];

			if(($stop_time - $start_time) > 0.5) {
				$this->longQuery[] = array(($stop_time - $start_time),$query);
			}

			$this->sqltime += ($stop_time - $start_time);

			if($res) {
				if($handling === "INSERT") {
					$tmp = mysql_insert_id($this->DBc) or $this->error(mysql_error());;
					$this->unlinkSql($res);
					return $tmp;
				}
				elseif($handling === "DELETE" || $handling === "UPDATE") {
					$tmp = mysql_affected_rows($this->DBc) or $this->error(mysql_error());
					$this->unlinkSql($res);
					return $tmp;
				}
				elseif($handling === "ARRAY") {
					$microstart = explode(' ',microtime());
					$start_time = $microstart[0] + $microstart[1];

					$tmp = $this->parseSql($res);

					$microstop = explode(' ',microtime());
					$stop_time = $microstop[0] + $microstop[1];

					$this->sqltime_post += ($stop_time - $start_time);

					$this->unlinkSql($res);
					return $tmp;
				}
				elseif($handling === "NONE") {
					$this->unlinkSql($res);
					return true;
				}
				else {
					return $res;
				}
			}
			else {
				return false;
			}
		}

		function unlinkSql($res) {
			@mysql_free_result($res);
		}

		private function parseSql($res) {
			$data = array();
			$k = 0;
			while($tmp = mysql_fetch_array($res,MYSQL_ASSOC)) {
				$data[$k] = $tmp;
				$k++;
			}

			return $data;
		}

		function getNext($res) {
			if($res) {
				if($tmp = mysql_fetch_array($res,MYSQL_ASSOC)) {
					return $tmp;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}

		private function error($error,$query = false) { // error handler
			global $log;

			$this->DBstats['error']++;

			if($query != false) {
				$error .= " -->|".$query."|<--";
			}

			switch($this->DBerror) {
				case 'DIE':
					die($error);
					break;
				case 'PRINT':
					echo "<br><b>".$error."</b><br>";
					break;
				case 'ALERT':
					echo "<script language='javascript'>\n<!--\nalert(\"database error:\\n".mysql_real_escape_string($error)."\");\n// -->\n</script>";
					break;
				case 'LOG':
					$log->logf("MySQL ERROR: ".$error);
					break;
				default:
					flush();
					break;
			}
		}
	}
?>