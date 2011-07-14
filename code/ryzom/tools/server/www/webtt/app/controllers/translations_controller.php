<?php
class TranslationsController extends AppController {

	var $name = 'Translations';

	function index() {
		$this->Translation->recursive = 0;
		$conditions = null;
		if (isset($this->passedArgs['identifier_id']) && $identifier = $this->passedArgs['identifier_id'])
			$conditions = array('Translation.identifier_id' => $identifier);
		$this->set('translations', $this->paginate($conditions));
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
//		$this->recursive=2;
		$this->set('translation', $translation = $this->Translation->read(null, $id));
		$this->set('identifier', $identifier = $this->Translation->Identifier->read(null, $translation['Translation']['identifier_id']));
//		var_dump($translation);
//		var_dump($identifier);
	}

	function add() {
		if (!empty($this->data)) {
			$this->Translation->create();
			if ($res = $this->Translation->save($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('action' => 'index', 'identifier_id' => $res['Translation']['identifier_id']));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->passedArgs['identifier_id']))
		{
			$this->Session->setFlash(__('You need to choose identifier for translation', true));
			$this->redirect(array('controller' => 'identifiers', 'action' => 'index'));
		}
		else
		{
			$identifier_id = $this->passedArgs['identifier_id'];
			$this->set('identifier', $identifier = $this->Translation->Identifier->read(null, $identifier_id));
//			$this->data['Translation.identifier_id'] = $identifier_id;
		}
//		$identifiers = $this->Translation->Identifier->find('list', array('recursive' => -1));
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
//		var_dump($this->data);
		$identifier = $this->Translation->Identifier->read(null, $this->data['Translation']['identifier_id']);
		$identifiers = $this->Translation->Identifier->find('list');
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users', 'identifier'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Translation->delete($id)) {
			$this->Session->setFlash(__('Translation deleted', true));
//			$this->redirect(array('action'=>'index'));
			$this->redirect($this->referer());
		}
		$this->Session->setFlash(__('Translation was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		return $this->index();
	}

	function admin_setBest($id) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for translation', true));
			$this->redirect($this->referer());
		}
		if ($this->Translation->setBest($id))
			$this->Session->setFlash(__('Set successful', true));
		else
			$this->Session->setFlash(__('Set error', true));
		$this->redirect($this->referer());
//		$this->index();
//		$this->render('index');
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
