<?php
class IdentifierColumnsController extends AppController {

	var $name = 'IdentifierColumns';

	function index() {
		$this->IdentifierColumn->recursive = 0;
		$this->set('identifierColumns', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid identifier column', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('identifierColumn', $this->IdentifierColumn->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->IdentifierColumn->create();
			if ($this->IdentifierColumn->save($this->data)) {
				$this->Session->setFlash(__('The identifier column has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier column could not be saved. Please, try again.', true));
			}
		}
		$identifiers = $this->IdentifierColumn->Identifier->find('list');
		$this->set(compact('identifiers'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid identifier column', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->IdentifierColumn->save($this->data)) {
				$this->Session->setFlash(__('The identifier column has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier column could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->IdentifierColumn->read(null, $id);
		}
		$identifiers = $this->IdentifierColumn->Identifier->find('list');
		$this->set(compact('identifiers'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for identifier column', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->IdentifierColumn->delete($id)) {
			$this->Session->setFlash(__('Identifier column deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Identifier column was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->IdentifierColumn->recursive = 0;
		$this->set('identifierColumns', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid identifier column', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('identifierColumn', $this->IdentifierColumn->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->IdentifierColumn->create();
			if ($this->IdentifierColumn->save($this->data)) {
				$this->Session->setFlash(__('The identifier column has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier column could not be saved. Please, try again.', true));
			}
		}
		$identifiers = $this->IdentifierColumn->Identifier->find('list');
		$this->set(compact('identifiers'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid identifier column', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->IdentifierColumn->save($this->data)) {
				$this->Session->setFlash(__('The identifier column has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The identifier column could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->IdentifierColumn->read(null, $id);
		}
		$identifiers = $this->IdentifierColumn->Identifier->find('list');
		$this->set(compact('identifiers'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for identifier column', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->IdentifierColumn->delete($id)) {
			$this->Session->setFlash(__('Identifier column deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Identifier column was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
