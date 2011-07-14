<?php
/**
 * Comma Separated Values Datasource
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       datasources
 * @subpackage    datasources.models.datasources
 * @since         CakePHP Datasources v 0.3
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 * A CakePHP datasource for interacting with files using comma separated value storage.
 *
 * Create a datasource in your config/database.php
 *   public $csvfile = array(
 *     'datasource' => 'Datasources.CsvSource',
 *     'path' => '/path/to/file', // Path
 *     'extension' => 'csv', // File extension
 *     'readonly' => true, // Mark for read only access
 *     'recursive' => false // Only false is supported at the moment
 *   );
 */

if (!class_exists('Folder')) {
	App::import('Core', 'Folder');
}

if (!class_exists('File')) {
	App::import('Core', 'File');
}

/**
 * CSVSource Datasource
 *
 * @package datasources
 * @subpackage datasources.models.datasources
 */
class RawFilesSource extends DataSource {

/**
 * Description
 *
 * @var string
 */
	public $description = 'Directory Contents Data Source';

/**
 * File Handle
 *
 * @var mixed
 */
	public $handle = false;

/**
 * Default configuration.
 *
 * @var array
 */
	var $_baseConfig = array(
		'datasource' => 'Datasources.RawFilesSource',
		'path' => '.',
		'extension' => 'txt',
		'readonly' => true,
		'recursive' => false);

	var $_schema = array(
//		'files' => array(
			'filename' => array(
				'type' => 'string',
				'null' => false,
				'key' => 'primary',
				'length' => 255	
				),
			'size' => array(
				'type' => 'string',
				'null' => false,
				'length' => 40,
				),
			'modified' => array(
				'type' => 'string',
				'null' => false,
				'length' => 40,
				),
	//		)
		);

/**
 * Constructor
 *
 * @param string $config Configuration array
 * @param boolean $autoConnect Automatically connect to / open the file
 */
	public function __construct($config = null, $autoConnect = true) {
		$this->debug = Configure::read('debug') > 0;
		$this->fullDebug = Configure::read('debug') > 1;
		parent::__construct($config);
		if ($autoConnect) {
			$this->connect();
		}
	}

/**
 * Connects to the directory using options in the given configuration array.
 *
 * @return boolean True if the file could be opened.
 */
	public function connect() {
		$this->connected = false;

		if ($this->config['readonly']) {
			$create = false;
			$mode = 0;
		} else {
			$create = true;
			$mode = 0777;
		}

		$this->connection =& new Folder($this->config['path'], $create, $mode);
		if ($this->connection) {
			$this->handle = array();
			$this->connected = true;
		}

		return $this->connected;
	}

/**
 * List available sources
 *
 * @return array of available files
 */
 	public function listSources() {
		return array('raw_files');
	}

/**
 * Returns a Model description (metadata).
 *
 * @return mixed
 */
	public function describe($model) {
		return $this->_schema;
	}

/**
 * Close file handle
 *
 * @return null
 */
	public function close() {
		if ($this->connected) {
			if ($this->handle) {
				foreach($this->handle as $h) {
					@fclose($h);
				}
				$this->handle = false;
			}
			$this->connected = false;
		}
	}

/**
 * Private method to determine if file is in one of given directories
 *
 * @return boolean
 */
	private function fileInDir($filepath, $dirs)
	{
		foreach ($dirs as $dir)
		{
			$dir = $this->connection->realpath($this->config['path']) . DS . $dir;
			if ($dir . DS . basename($filepath) === $filepath)
				return true;
		}
		return false;
	}

/**
 * Read Data
 *
 * @param Model $model
 * @param array $queryData
 * @param integer $recursive Number of levels of association
 * @return mixed
 */
	public function read(&$model, $queryData = array(), $recursive = null) {
		if (isset($queryData["conditions"][$model->alias . ".extension"]))
			$extension = preg_quote($queryData["conditions"][$model->alias . ".extension"]);
		else
			$extension = $this->config['extension'];

		if (isset($queryData["conditions"][$model->alias . ".filename"]))
		{
			$filename = $queryData["conditions"][$model->alias .".filename"];
			$searchPattern = preg_quote($queryData["conditions"][$model->alias .".filename"], '/');
		}
		else
		{
//			$searchPattern = '.*' . '\.' . preg_quote($extension);
			$searchPattern = '.*' . '\.' . $extension;
		}

		if (isset($queryData["conditions"][$model->alias . ".dir"]))
		{
//			$dir = $this->connection->realpath($this->config['path']) . DS . $queryData["conditions"][$model->alias . ".dir"];
			$dir = is_array($dir = $queryData["conditions"][$model->alias . ".dir"]) ? $dir : array($dir);
		}

/*		var_dump($queryData);*/
//		var_dump($searchPattern);

		if ($this->config['recursive']) {
			$list = $this->connection->findRecursive($searchPattern, true);
/*			$this->log($list);
			echo "list#\n";
			var_dump($list);*/
			foreach($list as &$item) {
				$temp = $item;
				$item = array();
				$item["full"] = $temp;
				$item["short"] = mb_substr($temp, mb_strlen($this->connection->realpath($this->config['path']) . DS));
			}
			unset($item);
		} else {
			$list = $this->connection->find($searchPattern, true);
			foreach($list as &$item) {
				$temp = $item;
				$item = array();
				$item["full"] = $this->config['path'] . DS .$temp;
				$item["short"] = $temp;
			}
			unset($item);
		}

		$resultSet = array();
		foreach ($list as $item) {
/*			if (isset($dir) && isset($filename))
			{
				echo "dirconcat#\n";
				var_dump($dir . DS . $filename);
				echo "itemfull#\n";
				var_dump($item["full"]);
				if ($dir . DS . $filename === $item["full"])
					continue;
			}*/
			if (isset($dir))
				if (!$this->fileInDir($item["full"], $dir))
					continue;
			$file = new File($path = $this->config['path'] . DS . $item["short"], false);
//			var_dump($item);
//			$item = preg_replace('/' . $extPattern . '$/i', '', $item);
			$resultSet[] = array(
				$model->alias => array(
					'filename' => $item["short"],
					'size' => $file->size(),
					'modified' => $file->lastChange(),
					),
				);
		}
		if ($model->findQueryType === 'count') {
			return array(array(array('count' => count($resultSet))));
		}

		return $resultSet;
	}

/**
 * Calculate
 *
 * @param Model $model 
 * @param mixed $func 
 * @param array $params 
 * @return array with the field name with records count
 */
	public function calculate(&$model, $func, $params = array()) {
		return array('count');
	}
}
