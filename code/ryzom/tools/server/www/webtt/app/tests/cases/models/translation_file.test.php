<?php
/* TranslationFile Test cases generated on: 2011-05-29 19:13:14 : 1306689194*/
App::import('Model', 'TranslationFile');

class TranslationFileTestCase extends CakeTestCase {
	var $fixtures = array('app.translation_file', 'app.language', 'app.identifier');

	function startTest() {
		$this->TranslationFile =& ClassRegistry::init('TranslationFile');
	}

	function endTest() {
		unset($this->TranslationFile);
		ClassRegistry::flush();
	}

}
