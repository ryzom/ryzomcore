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

	function __construct($name, $title) {
	}
	
	function getForm($url_params) {
		$form = new ryForm($this->formName, $this->tools);
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
	
	function preSerialization() {
		unset($this->tools);
	}
			
	function postSerialization($vars=array()) {
	}
	
	function getTemplate() {
		return '';
	}
}

?>
