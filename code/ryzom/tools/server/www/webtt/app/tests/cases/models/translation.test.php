<?php
/* Translation Test cases generated on: 2011-05-29 19:19:42 : 1306689582*/
App::import('Model', 'Translation');

class TranslationTestCase extends CakeTestCase {
	var $fixtures = array('app.translation', 'app.identifier', 'app.translation_file', 'app.language', 'app.user', 'app.vote');

	function startTest() {
		$this->Translation =& ClassRegistry::init('Translation');
	}

	function endTest() {
		unset($this->Translation);
		ClassRegistry::flush();
	}

}
