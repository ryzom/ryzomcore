<?php
/* Vote Test cases generated on: 2011-05-29 21:11:49 : 1306696309*/
App::import('Model', 'Vote');

class VoteTestCase extends CakeTestCase {
	var $fixtures = array('app.vote', 'app.translation', 'app.identifier', 'app.translation_file', 'app.language', 'app.user');

	function startTest() {
		$this->Vote =& ClassRegistry::init('Vote');
	}

	function endTest() {
		unset($this->Vote);
		ClassRegistry::flush();
	}

}
