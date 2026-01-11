<?php
	class mySQL {
		var $DBc;
		var $DBstats;
		var $cached;

		function mre($in) {
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

		function mySQL($err=false) {
			$this->DBstats = array();
			$this->DBc = false;
			if($err === "DIE" || $err === "PRINT" || $err === "ALERT" || $err === "HIDE" || $err === "LOG") {
				$this->DBerror = $err;
			}
			else {
				$this->DBerror = "HIDE";
			}
			$this->resetStats();
			$this->cached = false;
		}

		function connect($ip,$user,$pass,$db=false) {
			$this->DBc = mysql_pconnect($ip,$user,$pass) or $this->error(mysql_error());
			if($this->DBc && $db) {
				$this->database($db);
			}
			$this->resetStats();
		}

		function database($db) {
			if(!$this->DBc) {
				return false;
			}
			mysql_select_db($db,$this->DBc) or $this->error(mysql_error());

			return true;
		}

		function resetStats() {
			$this->DBstats['query'] = 0;
			$this->DBstats['error'] = 0;
		}

		function getStats() {
			return $this->DBstats;
		}

		function sqlQuery($query) {
			return $this->sendSQL($query,"ARRAY");
		}

		function sendSQL($query,$handling="PLAIN",$buffer=false) { // can be INSERT, DELETE, UPDATE, ARRAY, NONE, PLAIN
			#if($this->cached !== false)  {
				#$this->unlinkSql($this->cached);
			#}
			if(!$this->DBc) {
				return false;
			}

			if($buffer === false && $handling !== "PLAIN") {
				$res = mysql_unbuffered_query($query,$this->DBc) or $this->error(mysql_error(),$query);
			}
			else {
				$res = mysql_query($query,$this->DBc) or $this->error(mysql_error(),$query);
			}

			#$this->cached = $res;

			$this->DBstats['query']++;

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
					$tmp = $this->parseSql($res);
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
				//mysql_free_result($res);
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

		private function error($error,$query = false) {
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
					logf("MySQL ERROR: ".$error);
					break;
				default:
					flush();
					break;
			}
		}
	}
?>