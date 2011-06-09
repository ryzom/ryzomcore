<?php
/* Identifier Test cases generated on: 2011-05-29 19:17:37 : 1306689457*/
App::import('Model', 'Identifier');

class IdentifierTestCase extends CakeTestCase {
	var $fixtures = array('app.identifier', 'app.translation_file', 'app.language', 'app.translation');

	function startTest() {
		$this->Identifier =& ClassRegistry::init('Identifier');
	}

	function endTest() {
		unset($this->Identifier);
		ClassRegistry::flush();
	}

}
