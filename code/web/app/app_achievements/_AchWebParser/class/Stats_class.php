<?php
	class Stats {
		#private $user;

		function Stats() {
			#$this->user = $user;
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