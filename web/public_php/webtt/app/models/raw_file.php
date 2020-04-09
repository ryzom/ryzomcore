<?php
/*
	Ryzom Core Web-Based Translation Tool
	Copyright (C) 2011 Piotr Kaczmarek <p.kaczmarek@openlink.pl>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
?>
<?php
class RawFile extends AppModel {
	var $name = 'RawFile';
	var $useDbConfig = 'raw_files';
	var $displayField = 'filename';
//	var $useTable = false;
	var $primaryKey = 'filename';
	
	var $_parser;

	var $belongsTo = array(
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'filename',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
	);

	public function open($dir, $filename, $write = false)
	{
		$this->_currentFile = null;
		$this->_currentFileLastModified = null;

		$ds = $this->getDataSource();
		$file = new File($filepath = $ds->config['path'] . DS . $dir . DS . $filename, false);
		if (!$file)
			return false;
		if (!$file->readable())
			return false;
		if ($write && !$file->writable())
			return false;
		$this->_currentFile = $file;
		$this->_currentFileLastChange = $file->lastChange();
		return $file;
	}

	public function parseFile()
	{
		if (!$this->_currentFile)
			return false;

		// TODO: file types array with filenames regex
		if (
			preg_match('|^([a-z]{2})_diff_[A-F0-9]{8}\.uxt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^([a-z]{2})\.uxt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^r2_([a-z]{2})\.uxt$|', $this->_currentFile->name, $matches)
		)
		{
			App::import("Vendor","StringParser", array("file" => 'StringParser.php'));
			$parser = $this->_parser = new StringParser();
		}
		else if (
			preg_match('|^phrase_([a-z]{2})_diff_[A-F0-9]{8}\.txt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^phrase_([a-z]{2})\.txt$|', $this->_currentFile->name, $matches)
		)
		{
			App::import("Vendor","PhraseParser", array("file" => 'PhraseParser.php'));
			$parser = $this->_parser = new PhraseParser();
		}
		else if (preg_match('|^.*_words_([a-z]{2})_diff_[A-F0-9]{8}\..*$|', $this->_currentFile->name, $matches)
			|| preg_match('|^.*_words_([a-z]{2})\..*$|', $this->_currentFile->name, $matches))
		{
			App::import("Vendor","SheetParser", array("file" => 'SheetParser.php'));
			$parser = $this->_parser = new SheetParser();
		}
		else
		{
			return false;
		}
		$entities = $parser->parseFile($this->_currentFile->read());
		return $entities;
	}

	public function buildFile($entities)
	{
		if (!$this->_currentFile)
			return false;

		// TODO: file types array with filenames regex
		if (
			preg_match('|^([a-z]{2})_diff_[A-F0-9]{8}\.uxt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^([a-z]{2})\.uxt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^r2_([a-z]{2})\.uxt$|', $this->_currentFile->name, $matches)
		)
		{
			App::import("Vendor","StringParser", array("file" => 'StringParser.php'));
			$parser = $this->_parser = new StringParser();
		}
		else if (
			preg_match('|^phrase_([a-z]{2})_diff_[A-F0-9]{8}\.txt$|', $this->_currentFile->name, $matches)
			|| preg_match('|^phrase_([a-z]{2})\.txt$|', $this->_currentFile->name, $matches)
		)
		{
			App::import("Vendor","PhraseParser", array("file" => 'PhraseParser.php'));
			$parser = $this->_parser = new PhraseParser();
		}
		else if (preg_match('|^.*_words_([a-z]{2})_diff_[A-F0-9]{8}\..*$|', $this->_currentFile->name, $matches)
			|| preg_match('|^.*_words_([a-z]{2})\..*$|', $this->_currentFile->name, $matches))
		{
			App::import("Vendor","SheetParser", array("file" => 'SheetParser.php'));
			$parser = $this->_parser = new SheetParser();
		}
		else
		{
			return false;
		}
		$content = $parser->buildFile($entities);
		$ret = $this->_currentFile->write($content);
		return $ret;
	}


	public function getLanguageCode($filename)
	{
		if (preg_match('|^([a-z]{2})_diff_[A-F0-9]{8}\.uxt$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^([a-z]{2})\.uxt$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^r2_([a-z]{2})\.uxt$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^phrase_([a-z]{2})_diff_[A-F0-9]{8}\..*$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^phrase_([a-z]{2})\..*$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^.*_words_([a-z]{2})_diff_[A-F0-9]{8}\..*$|', $filename, $matches))
			return $matches[1];
		else if (preg_match('|^.*_words_([a-z]{2})\..*$|', $filename, $matches))
			return $matches[1];
	}
}
