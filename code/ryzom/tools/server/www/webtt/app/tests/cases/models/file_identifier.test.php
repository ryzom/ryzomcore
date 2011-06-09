<?php
/* FileIdentifier Test cases generated on: 2011-05-31 16:07:59 : 1306850879*/
App::import('Model', 'FileIdentifier');

class FileIdentifierTestCase extends CakeTestCase {
	var $fixtures = array('app.file_identifier', 'app.translation_file', 'app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote');

	function startTest() {
		$this->FileIdentifier =& ClassRegistry::init('FileIdentifier');
	}

	function endTest() {
		unset($this->FileIdentifier);
		ClassRegistry::flush();
	}

}
