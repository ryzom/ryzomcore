<?php
/* Identifiers Test cases generated on: 2011-05-31 16:28:53 : 1306852133*/
App::import('Controller', 'Identifiers');

class TestIdentifiersController extends IdentifiersController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class IdentifiersControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.identifier', 'app.language', 'app.translation_file', 'app.file_identifier', 'app.translation', 'app.user', 'app.vote');

	function startTest() {
		$this->Identifiers =& new TestIdentifiersController();
		$this->Identifiers->constructClasses();
	}

	function endTest() {
		unset($this->Identifiers);
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
