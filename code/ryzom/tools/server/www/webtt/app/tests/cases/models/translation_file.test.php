<?php
/* TranslationFile Test cases generated on: 2011-07-04 13:04:17 : 1309777457*/
App::import('Model', 'TranslationFile');

class TranslationFileTestCase extends CakeTestCase {
	var $fixtures = array('app.translation_file', 'app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote', 'app.comment', 'app.file_identifier', 'app.imported_translation_file', 'app.raw_file');

	function startTest() {
		$this->TranslationFile =& ClassRegistry::init('TranslationFile');
	}

	function endTest() {
		unset($this->TranslationFile);
		ClassRegistry::flush();
	}

}
