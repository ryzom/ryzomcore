<?php
class ImportedTranslationFilesController extends AppController {

	var $name = 'ImportedTranslationFiles';
//	var $layout = "default_debug";
	function index() {
		$this->ImportedTranslationFile->recursive = 0;
		$this->set('importedTranslationFiles', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('importedTranslationFile', $this->ImportedTranslationFile->read(null, $id));
//		var_dump($this->ImportedTranslationFile->RawFile);
	}

	function admin_import($filename = null) {
//		$this->view = "index";
		App::import("Vendor","UxtParser", array("file" => 'UxtParser.php'));
/*		if (!$filename) {
			$this->Session->setFlash(__('Invalid file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}*/
		$filename="diff/pl_diff_4DEC868A.uxt";
		$translationFile = $this->ImportedTranslationFile->find('first', array('conditions' => array('ImportedTranslationFile.filename' => $filename)));
		if ($translationFile)
		{
			$this->Session->setFlash(__('Translation file already imported', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
//		var_dump($file);
		$parser = new UxtParser();
		$parsedFile = $parser->parseFile($filename);
//		var_dump($parsedFile);
		$arr = explode("_", basename($filename, ".uxt"));
//		var_dump($arr);
		$language_id = 1;

		$this->ImportedTranslationFile->create();
		$data['ImportedTranslationFile']['language_id'] = $language_id;
		$data['ImportedTranslationFile']['filename'] = $filename;
		//$this->ImportedTranslationFile->save($data);
		foreach ($parsedFile as $ent)
		{
			$fi_data = array();
			if ($ent['type'] != "string")
				continue;


			$i_data['language_id'] = $language_id;
			$i_data['translation_index'] = $ent['index'];
			$i_data['reference_string'] = $ent['string'];
			unset($this->ImportedTranslationFile->Language->Identifier->id);
			$identifier = $this->ImportedTranslationFile->Language->Identifier->find('first',array('conditions' => array('Identifier.identifier' => $ent['identifier'], 'Identifier.language_id' => $language_id)));
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
			var_dump($i_data);
			$this->ImportedTranslationFile->Language->Identifier->save(array('Identifier' => $i_data));
			$identifier_id = $this->ImportedTranslationFile->Language->Identifier->id;
			var_dump($identifier_id);

			unset($this->ImportedTranslationFile->FileIdentifier->id);
			//TODO - set FileIdentifier['id'] if we import already imported file (imported imported file temporarly disabled)
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
		$this->ImportedTranslationFile->saveAll($data);
		$this->Session->setFlash(__('Translation file imported', true));
		$this->redirect(array('action' => 'view', $this->ImportedTranslationFile->id));
		$this->ImportedTranslationFile->recursive = 0;
		$this->set('importedTranslationFiles', $this->paginate());
//		$this->render('index');
	}


	function add() {
		if (!empty($this->data)) {
			$this->ImportedTranslationFile->create();
			if ($this->ImportedTranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->ImportedTranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->ImportedTranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->ImportedTranslationFile->read(null, $id);
		}
		$languages = $this->ImportedTranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation file', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->ImportedTranslationFile->delete($id)) {
			$this->Session->setFlash(__('Translation file deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation file was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->ImportedTranslationFile->recursive = 0;
//		FireCake::dump("??",$_SERVER);
		$this->set('importedTranslationFiles', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('importedTranslationFile', $this->ImportedTranslationFile->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->ImportedTranslationFile->create();
			if ($this->ImportedTranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->ImportedTranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->ImportedTranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->ImportedTranslationFile->read(null, $id);
		}
		$languages = $this->ImportedTranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation file', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->ImportedTranslationFile->delete($id)) {
			$this->Session->setFlash(__('Translation file deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation file was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
