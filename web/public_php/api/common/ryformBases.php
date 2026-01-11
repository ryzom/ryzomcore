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

define('DEF_TYPE_UNKNOWN', 0);
define('DEF_TYPE_HIDDEN', 1);
define('DEF_TYPE_TEXT', 2);
define('DEF_TYPE_ID', 3);
define('DEF_TYPE_INT', 4);
define('DEF_TYPE_FLOAT', 5);
define('DEF_TYPE_BOOL', 6);
define('DEF_TYPE_OPTION', 7);
define('DEF_TYPE_TEXTAREA', 8);
define('DEF_TYPE_TRAD', 9);
define('DEF_TYPE_FORM', 10);
define('DEF_TYPE_ICON', 11);
define('DEF_TYPE_RYFORM', 12);
define('DEF_TYPE_RYFORMS_ARRAY', 13);
define('DEF_TYPE_BBCODE', 14);
define('DEF_TYPE_FUNCTION', 15);
define('DEF_TYPE_COMBO', 16);
define('DEF_TYPE_OPTION_FUNCTION', 17);
define('DEF_TYPE_NAMEID', 18);
define('DEF_TYPE_COMBO_FUNCTION', 19);
define('DEF_TYPE_DATE', 20);


define('DEF_TYPE_ICON_UNKNOWN', 0);
define('DEF_TYPE_ICON_SHARED', 1);
define('DEF_TYPE_ICON_RYZITEM', 2);
define('DEF_TYPE_ICON_URL', 3);

define('DATA_HTML_FORM', 0);
define('DATA_FORM_VALUES', 1);
define('DATA_FORM_VALID_VALUES', 2);
define('DATA_RYFORM_VALUE', 3);

function cleanFormName($name) {
	$final_name = '';
	for ($i=0; $i<strlen($name); $i++) {
		$c = substr($name, $i, 1);
		if  ( ((ord(strtolower($c)) >= ord('a')) && (ord(strtolower($c)) <= ord('z'))) ||
			  (in_array($c, array('-', '.', '_'))) ||
			  (ord(strtolower($c)) >= ord('0')) && (ord(strtolower($c)) <= ord('9')) )
			$final_name .= $c;
	}
	return $final_name;
}

function cleanNameID($name) {
	$final_name = '';
	for ($i=0; $i<strlen($name); $i++) {
		$c = substr($name, $i, 1);
		if ($c == ' ' || $c == '_')
			$final_name .= '_';
		else if  ( ((ord(strtolower($c)) >= ord('a')) && (ord(strtolower($c)) <= ord('z'))) ||
			  (ord($c) >= ord('0')) && (ord($c) <= ord('9')) )
			$final_name .= $c;
	}
	return $final_name;
}

function getNameId($name) {
	return str_replace('_', ' ', $name);
}

function getTrad($value) {
	if ($value[_user()->lang])
		$trad = $value[_user()->lang];
	foreach (array('en', 'fr', 'de', 'ru', 'es') as $lang) {
		if ($value[$lang]) {
			$trad = $value[$lang];
			break;
		}
	}
	if (substr($trad, 0, 2) == '//')
		$trad = strstr(str_replace("\r", '', $trad), "\n");
	return substr($trad, 1);
}

function setRyformSource($object, $src, $indent=0, $protecteds=array()) {
		$c = '';
		$src = str_replace("\r", '', $src);
		$ssrc = explode("\n", $src);
		$mode = 'var';
		$cache = '';
		$current_ryform = NULL;
		foreach ($ssrc as $line) {
			if (!$line)
				continue;
			if ($mode != 'ryform')
				$c .=  str_repeat("\t", $indent);
			switch ($mode) {
				case 'ryform':
					if ($line[0] == "\t") {
						$cache .= substr($line, 1)."\n";
						break;
					} else {
						$mode = 'array';
					}

				case 'array':
					if ($line == ')' || $line == '}') {
						if ($cache && $current_ryform) {
							$c .= $current_ryform->setSource($cache, $indent+1);
							$c .= 'SET SOURCE ';
							if ($line == ')')
								$array[] = $current_ryform;
							else
								$array = $current_ryform;
						}
						if (array_key_exists($var_name, $protecteds))
							call_user_func(array($object, $protecteds[$var_name]), $array);
						else
							$object->$var_name = $array;
						$mode = 'var';
						$cache = '';
						continue;
					} else if ($line[0] == '[') {
						if ($cache && $current_ryform) {
							$c .= $current_ryform->setSource($cache, $indent+1);
							$array[] = $current_ryform;
							$c .= 'SET SOURCE ';
						}
						$ryform_name = substr($line, 1, strlen($line)-2);
						$c .= 'New Ryform: '.$ryform_name."\n";
						$cache = '';
						$current_ryform = new $ryform_name();
						$mode = 'ryform';
						continue;
					}
				break;

				default:
					$sep = strpos($line, '=');
					if ($sep) {
						$var_name = substr($line, 0, $sep-1);
						$value = substr($line, $sep+2);
						if ($value == '(' || $value == '{') {
							$c .= $var_name.' is ARRAY'."\n";
							$mode = 'array';
							$array = array();
						} else {
							$c .= $var_name .' = '.$value."\n";
							if ($value[0] == '\'' && $value[strlen($value)-1] == '\'')
								$object->$var_name = str_replace('\n', "\n", substr($value, 1, -1));
							else {
								if (is_numeric($value))
									$object->$var_name = eval('return '.$value.';');
								else if ($value == 'false')
									$object->$var_name = false;
								else if ($value == 'true')
									$object->$var_name = true;
								else if ($value == 'NULL')
									$object->$var_name = NULL;
							}
						}
					}
				break;
			}
		}
		return $c;
	}

interface iRyForm {	
	function getForm($url_params);
	function setFormParams($params);
	function getHtmlRepr();
	function getFormDefs();
	function getFormDefsExtraValues();
	function preSerialization();
	function postSerialization($vars=array());
	function getTemplate();
}

class ryFormDef {
	
	public $name = '';
	public $type = DEF_TYPE_UNKNOWN;
	public $params = array();
	public $infos = '';
	public $defaultValue = NULL;
	public $value = NULL;
	public $extraValues = array();
	public $hidden = false;
	public $optional = false;
	public $admin = false;
	public $superAdmin = false;
	public $prefixTrad = '';
	
	function __construct($name, $type, $params=array(), $defaultValue=NULL, $optional=false, $infos='') {
		$this->name = $name;
		$this->type = $type;
		$this->params = $params;
		$this->defaultValue = $defaultValue;
		$this->optional = $optional;
		$this->infos = $infos;
	}

}

class basicRyForm implements iRyForm {
	public $formName = ''; // Used by Form
	public $id = 0;

	function __construct($name='', $title='') {
	}
	
	function getForm($url_params) {
		$form = new ryForm($this->formName);
		$form_defs = $this->getFormDefs();
		foreach ($form_defs as $def)
			$form->addDefine($def);
		return $form->getForm(_s('section', $this->formName.' ('.get_class($this).')'));
	}
	
	function setFormParams($params) {
		foreach ($params as $name => $value) {
			if (property_exists($this, $name))
				$this->$name = $value;
		}
	}
	
	function getHtmlRepr() {
		return  $this->formName.' ('.get_class($this).')';
	}
	
	function getFormDefs() {
		return array();
	}
	
	function getFormDefsExtraValues() {
		return array();
	}
	
	/*function preSerialization() {
		unset($this->tools);
	}*/
	
	function preSerialization() {
		$all_defs = array('class_name');
		$this->author = _user()->id;
		foreach ($this->getFormDefs() as $def)
			$all_defs[] = $def->name;
		
		foreach (get_object_vars($this) as $name => $value) {
			if (!in_array($name, $all_defs))
				unset($this->$name);
		}
	}
	
	function postSerialization($vars=array()) {
	}
	
	function getTemplate() {
		return '';
	}
	
	function getSource($indent=0) {
		$attrs = $this->getFormDefs();
		$c = str_repeat("\t", $indent-1).'['.get_class($this).']'."\n";
		foreach ($attrs as $attr) {
			$c .= str_repeat("\t", $indent).$attr->name.' ';
			$var = $this->{$attr->name};
			if (is_object($var)) {
				$c .= "= {\n".substr($var->getSource($indent+1), 0, -1)."\n".str_repeat("\t", $indent).'}';
			} else if (is_array($var)) {
				$c .= '= ('."\n";
				foreach ($var as $element) {
					if (is_object($element)) 
						$c .= $element->getSource($indent+1);
					else if ($element)
						$c .= '#'.str_replace("\r", '', str_replace("\n", '\\\\n', var_export($element, true)));
				}
				$c .= str_repeat("\t", $indent).")";
			} else
				$c .= '= '.str_replace("\r", '', str_replace("\n", '\\\\n', var_export($var, true)));
			$c .= "\n";
		}
		return $c;
	}
	
	function setSource($src, $indent=0) {
		$this->preSerialization();
		return setRyformSource($this, $src, $indent);
	}
}

?>
