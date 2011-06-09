<?php
class RawFile extends AppModel {
	var $name = 'RawFile';
	var $useDbConfig = 'raw_files';
	var $displayField = 'filename';
	var $useTable = false;
	var $primaryKey = 'filename';
	
	var $_parser;
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'filename',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
	);
/*	var $hasOne = array(
		'FileIdentifier' => array(
			'className' => 'FileIdentifier',
			'foreignKey' => 'translation_file_id',
			'dependent' => true,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		)
	);*/

	public function open($dir, $filename)
	{
		$ds = $this->getDataSource();
		$file = new File($filepath = $ds->config['path'] . DS . $dir . DS . $filename, false);
		if (!$file)
			return false;
		if (!$file->readable())
			return false;
//		var_dump($filename);
		$this->_currentFile = $file;
		return $file;
	}

	public function parseFile($file)
	{
		App::import("Vendor","UxtParser", array("file" => 'UxtParser.php'));
		$parser = $this->_parser = new UxtParser();
		if (!$this->_currentFile)
			return false;
		$entities = $parser->parseFile($this->_currentFile->read());
		return $entities;
	}
	public function getLanguageCode($filename)
	{
//		var_dump($filename);
		if (preg_match('|^([a-z]{2})_diff_[A-F0-9]{8}\.uxt$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^([a-z]{2})\.uxt$|', $filename, $matches))
			return $matches[1];
	}
}
