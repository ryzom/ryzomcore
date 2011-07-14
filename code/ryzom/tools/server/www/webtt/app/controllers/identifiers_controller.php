<?php
class IdentifiersController extends AppController {

	var $name = 'Identifiers';
//	var $layout = "new";
	function index() {
		$this->Identifier->recursive = 0;
//		Router::connectNamed(array('language'));
		 
		$conditions = null;
		if (isset($this->passedArgs['language']) && $lang = $this->passedArgs['language'])
			$conditions = array('Identifier.language_id' => $lang);
		if (isset($this->passedArgs['translation_file_id']) && $translation_file_id = $this->passedArgs['translation_file_id'])
			$conditions = array('Identifier.translation_file_id' => $translation_file_id);
		$this->set('identifiers', $this->paginate($conditions));
	}

	function admin_withoutBestTranslation()
	{
		if (isset($this->passedArgs['imported_translation_file_id']) && $imported_translation_file_id = $this->passedArgs['imported_translation_file_id'])
		{
			$identifier_ids = $this->Identifier->withoutBestTranslation(array('ImportedTranslationFile.id' => $this->passedArgs['imported_translation_file_id']));
			if ($identifier_ids === false)
			{
				$this->Session->setFlash(__('Error: no conditions specified', true));
			}
			else
			{
				$conditions = array('Identifier.id' => $identifier_ids, 'BestTranslation.id' => NULL);
				$this->set('identifiers', $this->paginate($conditions));
			}
//			$this->log($this->Identifier->withoutBestTranslation(array('ImportedTranslationFile.id' => $this->passedArgs['imported_translation_file_id'])));
			// TOTHINK: try to achieve that with custom find with pagination
		}
		else
		{
			$this->Session->setFlash(__('No imported file specified', true));
			$this->redirect($this->referer());
		}
		$this->render('index');
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('identifier', $this->Identifier->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->Identifier->create();
			if ($this->Identifier->save($this->data)) {
				$this->Session->setFlash(__('The identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->Identifier->Language->find('list');
		$this->set(compact('languages'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Identifier->save($this->data)) {
				$this->Session->setFlash(__('The identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Identifier->read(null, $id);
		}
		$languages = $this->Identifier->Language->find('list');
		$this->set(compact('languages'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for identifier', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Identifier->delete($id)) {
			$this->Session->setFlash(__('Identifier deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Identifier was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->Identifier->recursive = 0;
		$this->set('identifiers', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('identifier', $this->Identifier->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->Identifier->create();
			if ($this->Identifier->save($this->data)) {
				$this->Session->setFlash(__('The identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier could not be saved. Please, try again.', true));
			}
		}
		$languages = $this->Identifier->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid identifier', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Identifier->save($this->data)) {
				$this->Session->setFlash(__('The identifier has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Identifier->read(null, $id);
		}
		$languages = $this->Identifier->Language->find('list');
		$this->set(compact('languages'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for identifier', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Identifier->delete($id)) {
			$this->Session->setFlash(__('Identifier deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Identifier was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
