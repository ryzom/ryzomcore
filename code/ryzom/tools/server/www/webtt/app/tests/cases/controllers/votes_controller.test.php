<?php
/* Votes Test cases generated on: 2011-05-29 21:12:06 : 1306696326*/
App::import('Controller', 'Votes');

class TestVotesController extends VotesController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class VotesControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.vote', 'app.translation', 'app.identifier', 'app.translation_file', 'app.language', 'app.user');

	function startTest() {
		$this->Votes =& new TestVotesController();
		$this->Votes->constructClasses();
	}

	function endTest() {
		unset($this->Votes);
		ClassRegistry::flush();
	}

	function testIndex() {

	}

	function testView() {

	}

	function testAdd() {

	}

	function testEdit() {

	}

	function testDelete() {

	}

	function testAdminIndex() {

	}

	function testAdminView() {

	}

	function testAdminAdd() {

	}

	function testAdminEdit() {

	}

	function testAdminDelete() {

	}

}
