<?php
/* Language Test cases generated on: 2011-05-31 15:48:43 : 1306849723*/
App::import('Model', 'Language');

class LanguageTestCase extends CakeTestCase {
	var $fixtures = array('app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote', 'app.translation_file');

	function startTest() {
		$this->Language =& ClassRegistry::init('Language');
	}

	function endTest() {
		unset($this->Language);
		ClassRegistry::flush();
	}

}
