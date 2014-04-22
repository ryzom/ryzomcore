<?php
/*
	Ryzom Core Web-Based Translation Tool
	Copyright (C) 2011 Piotr Kaczmarek <p.kaczmarek@openlink.pl>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
?>
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
		$this->set('identifierColumn', $identifierColumn_data = $this->IdentifierColumn->read(null, $id));
		if (empty($this->data)) {
			$this->data = $identifierColumn_data;
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
