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
class IdentifiersController extends AppController {
	var $name = 'Identifiers';
	function index() {
		$this->Identifier->recursive = 0;
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
		$this->set('identifier', $identifier = $this->Identifier->read(null, $id));
		if ($identifier)
			$this->set('identifierNeighbours', $this->Identifier->getNeighbours($id));
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
		$this->set('identifier', $identifier = $this->Identifier->read(null, $id));
		if ($identifier)
			$this->set('identifierNeighbours', $this->Identifier->getNeighbours($id));
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
		$this->set('identifier', $identifier_data = $this->Identifier->read(null, $id));
		if (empty($this->data)) {
			$this->data = $identifier_data;
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
