<?php
class RawFilesController extends AppController {

	var $name = 'RawFiles';
	var $helpers = array('Paginator', 'Time', 'Session');
	var $components = array('Session');

	function index() {
		$this->RawFile->recursive = 1;
//		var_dump($this->RawFile->find('count'));
//		$db =& ConnectionManager::getDataSource($this->RawFile->useDbConfig);
//		var_dump($db->calculate($this->RawFile, 'count'));
		$this->set('rawFiles', $this->paginate());
//		var_dump($this->paginate());
	}

	function admin_index() {
		$this->index();
	}

	function listdir($ext = null) {
		$this->RawFile->recursive = 0;
		$this->set('rawFiles', $this->paginate(array("ext" => $ext)));
//		var_dump($this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid raw file', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('rawFile', $this->RawFiles->read(null, $id));
	}

	function admin_import($dir = null, $filename = null) {
//		$this->view = "index";
//		App::import("Vendor","UxtParser", array("file" => 'UxtParser.php'));
		if (!$filename) {
			$this->Session->setFlash(__('Invalid file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		if (!$this->RawFile->open($dir, $filename))
		{
			$this->Session->setFlash(__('Can\'t open file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		$importedTranslationFileModel = $this->RawFile->ImportedTranslationFile;
		$languageModel = $importedTranslationFileModel->Language;
		$identifierModel = $languageModel->Identifier;
		$fileIdentifierModel = $importedTranslationFileModel->FileIdentifier;

//		$filename="diff/pl_diff_4DEC868A.uxt";
		$importedTranslationFile = $importedTranslationFileModel->find('first', array('conditions' => array('ImportedTranslationFile.filename' => $dir . DS . $filename)));
/*		var_dump($translationFile);
		return 0;*/
		if ($importedTranslationFile)
		{
			$this->Session->setFlash(__('Translation file already imported', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
//		var_dump($file);
//		$parser = new UxtParser();
		$parsedFile = $this->RawFile->parseFile($filename);
//		var_dump($parsedFile);
//		$arr = explode("_", basename($filename, ".uxt"));
//		var_dump($arr);

//		$language_id = 1;
		$languageCode = $this->RawFile->getLanguageCode($filename);
		if (!$languageCode)
		{
			$this->Session->setFlash(__('Can\'t identify language', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		$language = $importedTranslationFileModel->Language->find('first', array('conditions' => array('code' => $languageCode)));
		$language_id = $language['Language']['id'];
		if (!$language_id)
		{
			$this->Session->setFlash(__('Can\'t find language in database', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		else
		{
//			var_dump($language_id);
		}

//		return 0;

		$importedTranslationFileModel->create();
		$data['ImportedTranslationFile']['language_id'] = $language_id;
		$data['ImportedTranslationFile']['filename'] = $dir . DS . $filename;
		//$this->ImportedTranslationFile->save($data);
		foreach ($parsedFile as $ent)
		{
			$fi_data = array();
			if ($ent['type'] != "string")
				continue;

			$i_data['language_id'] = $language_id;
			$i_data['translation_index'] = $ent['index'];
			$i_data['reference_string'] = $ent['string'];

			unset($identifierModel->id);
			$identifier = $identifierModel->find('first',array('conditions' => array('Identifier.identifier' => $ent['identifier'], 'Identifier.language_id' => $language_id)));
			if ($identifier)
			{
//				var_dump($identifier);
				$i_data['id']=$identifier['Identifier']['id'];
			}
			else
			{
				$i_data['identifier'] = $ent['identifier'];
				$i_data['translated'] = false;
			}
//			var_dump($i_data);
			$identifierModel->save(array('Identifier' => $i_data));
			$identifier_id = $identifierModel->id;
//			var_dump($identifier_id);

			unset($fileIdentifierModel->id);
			//TODO - set FileIdentifier['id'] if we import already imported file (importing imported file temporarly disabled)
//			$identifier = $this->ImportedTranslationFile->FileIdentifier->find('first',array('conditions' => array('FileIdentifier.identifier' => $ent['identifier'], 'FileIdentifier.translation_file_id' => $)));
//			$data['FileIdentifier']['translation_file_id'] = $this->ImportedTranslationFile->id;
			if ($ent['diff'])
				$fi_data['command'] = "DIFF " . mb_strtoupper($ent['diff']);
			$fi_data['translation_index'] = $ent['index'];
//			$data['FileIdentifier']['identifier_id'] = ;
			$fi_data['reference_string'] = $ent['string'];
			$fi_data['identifier_id'] = $identifier_id;

//			$this->ImportedTranslationFile->FileIdentifier->create();
//			$this->ImportedTranslationFile->FileIdentifier->save($data);
			$data['FileIdentifier'][] = $fi_data;
//			$l_data['Language']['id'] = $language_id;
//			$l_data['Identifier'][] = $i_data;
//			$data['Identifier'][] = $i_data;
		}
//		var_dump($data);
//		$this->ImportedTranslationFile->Language->saveAll($l_data);
		$importedTranslationFileModel->saveAll($data);
		$this->Session->setFlash(__('Translation file imported', true));
		$this->redirect(array('controller' => 'imported_translation_files', 'action' => 'view', $importedTranslationFileModel->id));
//		$this->ImportedTranslationFile->recursive = 0;
//		$this->set('importedTranslationFiles', $this->paginate());
//		$this->render('index');
	}
	
	
}
