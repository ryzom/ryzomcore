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
		$this->set('fileIdentifier', $fileIdentifier_data = $this->FileIdentifier->read(null, $id));
		if (empty($this->data)) {
			$this->data = $fileIdentifier_data;
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
