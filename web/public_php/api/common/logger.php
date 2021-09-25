<?php

/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

class ryLogger {

	public $enable = false;
	private $logs = array();
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
		if ($this->logs && $this->enable) {
			$ret = '<b>Debug</b><br /><br /><pre style="overflow:auto; width:100%">'. implode('<br />', $this->logs).'</pre>';
			$this->logs = array();
		}
		return $ret;
	}
}

function _log() {
	return ryLogger::getInstance();
}

?>
