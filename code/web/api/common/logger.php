<?php

class ryLogger {

	public $enable = false;
	private $logs;
	private static $_instance = NULL;
	
	public static function getInstance() {
		if (self::$_instance === NULL)
			self::$_instance = new ryLogger();
		return self::$_instance;
	}

	function addLog($log, $indent=NULL) {
		if ($indent !== NULL)
		$this->log_indent += $indent;
		if ($log) {
			$tabs = str_repeat("    ", $this->log_indent);
			$a = $tabs.str_replace("\n", "\n    ".$tabs, $log);
			$this->logs[] = '<font color="#00FF00">'.$a.'</font>';
		}
	}

	function addPrint($log, $color='#FFFF00') {
		$this->logs[] = '<font color="'.$color.'">'.$log.'</font>';
	}

	function addError($log) {
		$this->logs[] = '<font color="#FF5500"> ERROR: '.$log.'</font>';
	}

	function getLogs() {
		$ret = '';
		if ($this->logs && $this->enable)
			$ret = "<b>Debug</b>\n".implode("\n", $this->logs);
		$this->logs = array();
		return $ret;
	}
}


?>
