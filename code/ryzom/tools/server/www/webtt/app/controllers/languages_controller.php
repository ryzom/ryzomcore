<?php
class LanguagesController extends AppController {

	var $name = 'Languages';
	var $layout="new";

	function index() {
/*		App::import('Vendor', 'DebugKit.FireCake');
		FireCake::enable();
		firecake('testestestes');
		FireCake::dump('test','testsss');*/
		$this->Language->recursive = 0;
		$this->set('languages', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid language', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('language', $this->Language->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->Language->create();
			if ($this->Language->save($this->data)) {
				$this->Session->setFlash(__('The language has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The language could not be saved. Please, try again.', true));
			}
		}
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid language', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Language->save($this->data)) {
				$this->Session->setFlash(__('The language has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The language could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Language->read(null, $id);
		}
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for language', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Language->delete($id)) {
			$this->Session->setFlash(__('Language deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Language was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->Language->recursive = 0;
		$this->set('languages', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid language', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('language', $this->Language->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->Language->create();
			if ($this->Language->save($this->data)) {
				$this->Session->setFlash(__('The language has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The language could not be saved. Please, try again.', true));
			}
		}
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid language', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Language->save($this->data)) {
				$this->Session->setFlash(__('The language has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The language could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Language->read(null, $id);
		}
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for language', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Language->delete($id)) {
			$this->Session->setFlash(__('Language deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Language was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
