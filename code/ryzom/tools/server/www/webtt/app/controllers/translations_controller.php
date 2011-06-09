<?php
class TranslationsController extends AppController {

	var $name = 'Translations';

	function index() {
		$this->Translation->recursive = 0;
		$this->set('translations', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
//		$this->recursive=2;
		$this->set('translation', $bumz = $this->Translation->read(null, $id));
//		var_dump($bumz);
	}

	function add() {
		if (!empty($this->data)) {
			$this->Translation->create();
			if ($this->Translation->save($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		$identifiers = $this->Translation->Identifier->find('list');
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Translation->save($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Translation->read(null, $id);
		}
		$identifiers = $this->Translation->Identifier->find('list');
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Translation->delete($id)) {
			$this->Session->setFlash(__('Translation deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->Translation->recursive = 0;
		$this->set('translations', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('translation', $this->Translation->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->Translation->create();
			if ($this->Translation->save($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		$identifiers = $this->Translation->Identifier->find('list');
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Translation->save($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Translation->read(null, $id);
		}
		$identifiers = $this->Translation->Identifier->find('list');
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Translation->delete($id)) {
			$this->Session->setFlash(__('Translation deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Translation was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
