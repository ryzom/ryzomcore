<?php
/* TranslationFiles Test cases generated on: 2011-07-04 15:19:54 : 1309785594*/
App::import('Controller', 'TranslationFiles');

class TestTranslationFilesController extends TranslationFilesController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class TranslationFilesControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.translation_file', 'app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote', 'app.comment', 'app.file_identifier', 'app.imported_translation_file', 'app.raw_file');

	function startTest() {
		$this->TranslationFiles =& new TestTranslationFilesController();
		$this->TranslationFiles->constructClasses();
	}

	function endTest() {
		unset($this->TranslationFiles);
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
