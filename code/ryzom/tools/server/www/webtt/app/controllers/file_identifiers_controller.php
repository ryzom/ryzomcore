<?php
class FileIdentifiersController extends AppController {

	var $name = 'FileIdentifiers';

	function index() {
		$this->FileIdentifier->recursive = 0;
		$this->set('fileIdentifiers', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid file identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('fileIdentifier', $this->FileIdentifier->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->FileIdentifier->create();
			if ($this->FileIdentifier->save($this->data)) {
				$this->Session->setFlash(__('The file identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The file identifier could not be saved. Please, try again.', true));
			}
		}
		$importedTranslationFiles = $this->FileIdentifier->ImportedTranslationFile->find('list');
		$identifiers = $this->FileIdentifier->Identifier->find('list');
		$this->set(compact('importedTranslationFiles', 'identifiers'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid file identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->FileIdentifier->save($this->data)) {
				$this->Session->setFlash(__('The file identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The file identifier could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->FileIdentifier->read(null, $id);
		}
		$importedTranslationFiles = $this->FileIdentifier->ImportedTranslationFile->find('list');
		$identifiers = $this->FileIdentifier->Identifier->find('list');
		$this->set(compact('importedTranslationFiles', 'identifiers'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for file identifier', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->FileIdentifier->delete($id)) {
			$this->Session->setFlash(__('File identifier deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('File identifier was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->FileIdentifier->recursive = 0;
		$this->set('fileIdentifiers', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid file identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('fileIdentifier', $this->FileIdentifier->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->FileIdentifier->create();
			if ($this->FileIdentifier->save($this->data)) {
				$this->Session->setFlash(__('The file identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The file identifier could not be saved. Please, try again.', true));
			}
		}
		$importedTranslationFiles = $this->FileIdentifier->ImportedTranslationFile->find('list');
		$identifiers = $this->FileIdentifier->Identifier->find('list');
		$this->set(compact('importedTranslationFiles', 'identifiers'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid file identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->FileIdentifier->save($this->data)) {
				$this->Session->setFlash(__('The file identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The file identifier could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->FileIdentifier->read(null, $id);
		}
		$importedTranslationFiles = $this->FileIdentifier->ImportedTranslationFile->find('list');
		$identifiers = $this->FileIdentifier->Identifier->find('list');
		$this->set(compact('importedTranslationFiles', 'identifiers'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for file identifier', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->FileIdentifier->delete($id)) {
			$this->Session->setFlash(__('File identifier deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('File identifier was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
