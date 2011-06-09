<?php
/* TranslationFiles Test cases generated on: 2011-05-29 20:05:35 : 1306692335*/
App::import('Controller', 'TranslationFiles');

class TestTranslationFilesController extends TranslationFilesController {
	var $autoRender = false;

	function redirect($url, $status = null, $exit = true) {
		$this->redirectUrl = $url;
	}
}

class TranslationFilesControllerTestCase extends CakeTestCase {
	var $fixtures = array('app.translation_file', 'app.language', 'app.identifier', 'app.translation', 'app.user', 'app.vote');

	function startTest() {
		$this->TranslationFiles =& new TestTranslationFilesController();
		$this->TranslationFiles->constructClasses();
	}

	function endTest() {
		unset($this->TranslationFiles);
		ClassRegistry::flush();
	}

}
