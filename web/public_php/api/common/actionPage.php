<?php

/* Copyright (C) 2012 Winch Gate Property Limited
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
 * along with ryzom_app.  If not, see <http://www.gnu.org/licenses/>.
 */

class ryActionClass {
	public $classname;
	public $instance;
	public $args;
	public $requires;
	
	function __construct($classname, $instance, $args, $requires) {
		$this->classname = $classname;
		$this->instance = $instance;
		$this->args = $args;
		$this->requires = $requires;
	}
	
}

class ryActionPage {

	private static $classesArgs = array();
	private static $myClasses = array();
	private static $aliases = array();
	private static $messages;
	private static $haveMessage;
	protected static $id;
	
	public $instanceName;
	public $myMethods = array();

	function __construct() {
	}
	
	function addMethods($child_class) {
		if (is_array($child_class)) {
			foreach ($child_class as $c_class)
				$this->myMethods = array_merge($this->myMethods, get_class_methods($c_class));
		} else {
			$this->myMethods = get_class_methods($child_class);
		}
	}

	static function addClass($name, $classname, $args=array(), $requires=NULL) {
		self::$myClasses[$name] = new ryActionClass($classname, NULL, $args, $requires);
	}
	
	static function addAlias($aliasname, $name) {
		self::$aliases[$aliasname] = $name;
	}

	static function initInstance($listener) {
		$i = self::$myClasses[$listener];
		if (!$i->instance) {
			// requires
			if ($i->requires) {
				self::initInstance($i->requires);
			}
			if ($i->args)
				$i->instance = new $i->classname($listener, $i->args);
			else
				$i->instance = new $i->classname($listener);
			$i->instance->addMethods($i->classname);
			$i->instance->instanceName = $listener;
			
		}
		return $i->instance;
	}
	
	static function getInstance($listener) {
		return self::initInstance($listener);
	}
	
	static function _addMSG($type='OK', $message='') {
		self::$messages[] = array($type, $message);
		return '';
	}

	function addMSG($type='OK', $action='', $message='') {
		self::$messages[] = array($type, $message);
		$this->haveMessage = $action;
		return '';
	}

	static function getMSGs() {
		return self::$messages;
	}
	
	static function call($action, $url_params) {
		$action_params = explode('_', $action);

		if (count($action_params) != 2)
			return self::_addMSG('ERR', 'Action call error : bad params of ['.$action.']');
	
		list($listener, $call) = $action_params;
		if (array_key_exists($listener,self::$aliases))
			$listener = self::$aliases[$listener];
		
		if (!array_key_exists($listener, self::$myClasses))
			return self::_addMSG('ERR', 'Action call error : class ['. $listener .'] not found');
		
		$i = self::initInstance($listener);
		
		if (in_array('action'.$call, $i->myMethods)) {
			$i->haveMessage = NULL;
			$ret = call_user_func(array($i, 'action'.$call), $url_params);
			if (!isset($_SESSION['last_action']) or $action != $_SESSION['last_action'])
				$_SESSION['last_action'] = $action;
			$msg = $i->haveMessage;
			if ($msg and ($msg != $action)) {
				$ret = self::call($msg, $url_params);
				return self::_addMSG('OK', $ret);
			}
			return self::_addMSG('OK', $ret);
		} else
			return self::_addMSG('ERR', 'Action call error : action ['. $call .'] of ['. $listener .'] not found');
	}
}

function callAction($action) {
	$c = '';
	ryActionPage::call($action, ryzom_get_params());
	$msgs = ryActionPage::getMSGs();

	foreach ($msgs as $msg) {
		if ($msg[0] == 'HEADER')
			$c .= $msg[1];
	}

	foreach ($msgs as $msg) {
		if ($msg[0] == 'ERR')
			$c .= _s('message error', $msg[1]);
		else if ($msg[0] == 'MSG')
			$c .= _s('message', $msg[1]);
		else if ($msg[0] == 'WARNING')
			$c .= _s('message warning', $msg[1]);
		else if ($msg[0] != 'HEADER')
			$c .= $msg[1];
	}
	return $c;
}

?>
