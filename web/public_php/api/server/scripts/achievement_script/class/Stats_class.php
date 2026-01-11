<?php
	class Stats {
		#private $user;
		private $data;

		function Stats() {
			global $cdata,$DBc;
			#$this->user = $user;

			$DBc->sendSQL("INSERT IGNORE INTO stat_players (sp_char) VALUES ('".$cdata['cid']."')","NONE");

			$this->data = array();
		}

		function setValue($k,$v) {
			global $DBc;

			$this->data[] = $k."='".$DBc->mre($v)."'";
		}

		function writeData() {
			global $DBc,$cdata;

			$DBc->sendSQL("UPDATE stat_players SET ".implode(',',$this->data)." WHERE sp_char='".$cdata['cid']."'","NONE");
		}

		function register() { // register the stats code

			include_once("script/statsdb.php");

			return null;
		}

		function registerValue($name,$func) { // register to listen for a value
			global $_DISPATCHER;

			$tmp = new Callback($this,$func);
			$_DISPATCHER->registerValue($name,$tmp);
		}

		function unregisterValue($name,$callback) { // unregister listening
			global $_DISPATCHER;

			$_DISPATCHER->unregisterValue($name,$callback);
		}

		function registerEntity($name,$func) { // register to listen for an entity
			global $_DISPATCHER;

			$tmp = new Callback($this,$func);
			$_DISPATCHER->registerEntity($name,$tmp);
		}

		function unregisterEntity($name,$callback) { // unregister
			global $_DISPATCHER;

			$_DISPATCHER->unregisterEntity($name,$callback);
		}
	}
?>