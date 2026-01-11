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

 function getDirLinks($url_params, $path, $getvar, $home) {
	$ret = '';
	$dirs = explode('/', $path);
	$dirpath = '';
	$ret .= _l($home, $url_params, array($getvar => ''));
	foreach($dirs as $dirname) {
		if ($dirname)  {
			$ret .= ' &raquo; '._l($dirname, $url_params, array($getvar => '/'.$dirpath.$dirname));
			$dirpath .= $dirname.'/';
		}
	}
	return $ret;
}

function isEmptyDir($dir)
{
	if (($files = scandir($dir)) && count($files) <= 2) {
		return true;
	}
	return false;
}
 
class ryDataFileManager {

	public $id;
	public $log_dir;
	public $data_dir;
	public $user_dir;
	public $app_name;
	
	function __construct($id, $app_name=APP_NAME) {
		$this->app_name = $app_name;
		$id = (strlen($id) == 0?'0':'').$id;
		$id = (strlen($id) == 1?'0':'').$id;
		$this->id = $id;
		$this->log_dir = RYAPP_PATH.$app_name.'/data/logs/';
		$this->data_dir = RYAPP_PATH.$app_name.'/data/app/';
		$this->user_dir =  RYAPP_PATH.$app_name.'/data/chars/'.$id[0].'/'.$id[1].'/'.$id.'/';
		
		if (!is_dir($this->user_dir))
			@mkdir($this->user_dir, 0777, true);

		if (!is_dir($this->log_dir))  {
			@mkdir($this->log_dir, 0777, true);
			@mkdir($this->data_dir, 0777, true);
		}
	}	
	
	/*** Generic datafiles access methods ***/

	function getData($name, $default=null) {
		if (file_exists($name))
			return unserialize(file_get_contents($name));
		if ($default !== NULL) {
			@file_put_contents($name, serialize($default));
			return $default;
		}
		return NULL;
	}

	function saveData($name, $datas, $create_folders=true) {
		if ($create_folders) {
			if (!is_dir(dirname($name)))
				@mkdir(dirname($name), 0777, true);
		}
		if ($datas !== NULL)
			@file_put_contents($name, serialize($datas));
		else
			@unlink($name);
	}
	
	function listDataFiles($dir) {
		$ret = array();
		if ($handle = @opendir($dir)) {
			while (false !== ($file = readdir($handle))) {
				if ($file != '.' && $file != '..' && $file[0] != '.')
					$ret[] = $file;
			}
		}
		return $ret;
	}

	/*** App Datas ***/

	function loadAppData($name, $default=null) {
		return $this->getData($this->data_dir.$name, $default);
	}

	function saveAppData($name, $datas, $create_folders=true) {
		return $this->saveData($this->data_dir.$name, $datas, $create_folders);
	}

	function listAppDataFiles($basedir='') {
		return $this->listDataFiles($this->data_dir.$basedir);
	}


	/*** User Datas ***/
	
	function loadUserData($name, $default=null) {
		return $this->getData($this->user_dir.$name, $default);
	}

	function saveUserData($name, $datas, $create_folders=true) {
		return $this->saveData($this->user_dir.$name, $datas, $create_folders);
	}

	function listUserDataFiles($basedir='') {
		return $this->listDataFiles($this->user_dir.$basedir);
	}

	function loadUserDataFromApp($name, $app, $default=null) {
		$id = $this->id;
		$file = RYAPP_PATH.$app.'/data/chars/'.$id[0].'/'.$id[1].'/'.$id.'/'.$name;
		if (file_exists($file))
			return unserialize(file_get_contents($file));
		if ($default !== null)
			return $default;
		return null;
	}

	function saveUserDataFromApp($name, $app, $datas) {
		$id = $this->id;
		$dir = RYAPP_PATH.$app.'/data/chars/'.$id[0].'/'.$id[1].'/'.$id.'/';
		if (!is_dir($dir))
			@mkdir($dir, 0777, true);
		file_put_contents($dir.$name, serialize($datas));
	}
}


?>
