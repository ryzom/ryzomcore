<?php
/* Translations Test cases generated on: 2011-05-29 21:10:36 : 1306696236*/
App::import('Controller', 'Translations');

class TestTranslationsController extends TranslationsController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class TranslationsControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.translation', 'app.identifier', 'app.translation_file', 'app.language', 'app.user', 'app.vote');

	function startTest() {
		$this->Translations =& new TestTranslationsController();
		$this->Translations->constructClasses();
	}

	function endTest() {
		unset($this->Translations);
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
