<?php
class TranslationFilesController extends AppController {

	var $name = 'TranslationFiles';

	function index() {
		$this->TranslationFile->recursive = 0;

		$conditions = null;
		if (isset($this->passedArgs['language_id']) && $language_id = $this->passedArgs['language_id'])
			$conditions = array('TranslationFile.language_id' => $language_id);
		$this->set('translationFiles', $this->paginate($conditions));
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('translationFile', $this->TranslationFile->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->TranslationFile->create();
			if ($this->TranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->TranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->TranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->TranslationFile->read(null, $id);
		}
		$languages = $this->TranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation file', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->TranslationFile->delete($id)) {
			$this->Session->setFlash(__('Translation file deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation file was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->TranslationFile->recursive = 0;
		$this->set('translationFiles', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('translationFile', $this->TranslationFile->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->TranslationFile->create();
			if ($this->TranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->TranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation file', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->TranslationFile->save($this->data)) {
				$this->Session->setFlash(__('The translation file has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation file could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->TranslationFile->read(null, $id);
		}
		$languages = $this->TranslationFile->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation file', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->TranslationFile->delete($id)) {
			$this->Session->setFlash(__('Translation file deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation file was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
