<?php
/* FileIdentifiers Test cases generated on: 2011-05-31 16:08:10 : 1306850890*/
App::import('Controller', 'FileIdentifiers');

class TestFileIdentifiersController extends FileIdentifiersController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class FileIdentifiersControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.file_identifier', 'app.translation_file', 'app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote');

	function startTest() {
		$this->FileIdentifiers =& new TestFileIdentifiersController();
		$this->FileIdentifiers->constructClasses();
	}

	function endTest() {
		unset($this->FileIdentifiers);
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
