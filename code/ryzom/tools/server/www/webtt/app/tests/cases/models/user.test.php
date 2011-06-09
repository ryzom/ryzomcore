<?php
/* User Test cases generated on: 2011-05-29 19:20:07 : 1306689607*/
App::import('Model', 'User');

class UserTestCase extends CakeTestCase {
	var $fixtures = array('app.user', 'app.translation', 'app.identifier', 'app.translation_file', 'app.language', 'app.vote');

	function startTest() {
		$this->User =& ClassRegistry::init('User');
	}

	function endTest() {
		unset($this->User);
		ClassRegistry::flush();
	}

}
