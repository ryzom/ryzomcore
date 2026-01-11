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
		$this->set('translation', $translation = $this->Translation->read(null, $id));
		if (isset($translation['Translation']['identifier_id']))
		{
			$this->set('identifier', $identifier = $this->Translation->Identifier->read(null, $translation['Translation']['identifier_id']));
			$this->set('columnTranslations', $columnTranslations = $this->Translation->find('all', array('conditions' => array('Translation.parent_id' => $translation['Translation']['id']), 'order' => 'Translation.id')));
		}
		if ($identifier_id = $translation['Translation']['identifier_id'])
			$this->set('identifierNeighbours', $this->Translation->Identifier->getNeighbours($identifier_id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->Translation->create();
			if ($this->data['ChildTranslation'])
			{
				unset($translationText);
				foreach ($this->data['ChildTranslation'] as $num => $childTranslation)
				{
					$ent['columns'][] = $childTranslation['translation_text'];
					$translationText .= $childTranslation['translation_text'] . "\t";
				}
				$this->data['Translation']['translation_text'] = substr($translationText, 0, -1);
			}
			else
				$ent['string'] = $this->data['Translation']['translation_text'];
			sort($ent['columns']);
			$this->data['Translation']['translation_hash'] = $this->Translation->makeHash($ent);
			$this->data['Identifier']['id'] = $this->data['Translation']['identifier_id'];
			$this->data['Identifier']['translated'] = 1;
			$res = $this->Translation->saveAll($this->data);
			$this->log($this->data);
			if ($res) {
				$this->Session->setFlash(__('The translation has been saved', true));
				if ($this->params['form']['Next'])
				{
					$identifier_id = $this->data['Translation']['identifier_id'];
					$identifier = $this->Translation->Identifier->read(null, $identifier_id);
					$identifierNeighbors = $this->Translation->Identifier->find('neighbors', array('field' => 'id', 'value' => $identifier_id, 'conditions' => array('translation_file_id' => $identifier['Identifier']['translation_file_id'])));
					if ($nextIdentifier = $identifierNeighbors['next'])
						$this->redirect(array('action' => 'add', 'identifier_id' => $nextIdentifier['Identifier']['id']));
					else
						$this->redirect(array('controller' => 'identifiers', 'action' => 'index', 'translation_file_id' => $identifier['Identifier']['translation_file_id']));
				}
				else
					$this->redirect(array('controller' => 'identifiers', 'action' => 'view', $this->data['Translation']['identifier_id']));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->passedArgs['identifier_id']))
		{
			$this->Session->setFlash(__('You need to choose identifier for translation', true));
			$this->redirect($this->referer());
		}
		else
		{
			$identifier_id = $this->passedArgs['identifier_id'];
			$this->set('identifier', $identifier = $this->Translation->Identifier->read(null, $identifier_id));
			$this->set('identifierNeighbours', $this->Translation->Identifier->getNeighbours($identifier_id));
			$this->set('identifierColumns', $identifierColumns = $this->Translation->IdentifierColumn->find('list', array('conditions' => array('identifier_id' => $identifier_id), 'order' => 'IdentifierColumn.id')));
			if ($identifierColumns)
				$this->set('identifierColumnsDetails', Set::combine($this->Translation->IdentifierColumn->find('all', array('conditions' => array('identifier_id' => $identifier_id), 'order' => 'IdentifierColumn.id')), '{n}.IdentifierColumn.id', '{n}.IdentifierColumn'));
		}
		$users = $this->Translation->User->find('list');
		$this->set(compact('identifiers', 'users'));
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid translation', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->data['ChildTranslation'])
			{
				unset($translationText);
				foreach ($this->data['ChildTranslation'] as $num => $childTranslation)
				{
					$ent['columns'][] = $childTranslation['translation_text'];
					$translationText .= $childTranslation['translation_text'] . "\t";
				}
				$this->data['Translation']['translation_text'] = substr($translationText, 0, -1);
			}
			else
				$ent['string'] = $this->data['Translation']['translation_text'];
			$this->data['Translation']['translation_hash'] = $this->Translation->makeHash($ent);
			$this->data['Identifier']['id'] = $this->data['Translation']['identifier_id'];
			$this->data['Identifier']['translated'] = 1;
			if ($this->Translation->saveAll($this->data)) {
				$this->Session->setFlash(__('The translation has been saved', true));
				$this->redirect(array('controller' => 'identifiers', 'action' => 'view', $this->data['Translation']['identifier_id']));
			} else {
				$this->Session->setFlash(__('The translation could not be saved. Please, try again.', true));
			}
		}
		$this->set('translation', $translation_data = $this->Translation->read(null, $id));
		if (empty($this->data)) {
			$this->data = $translation_data;
		}
		$identifier_id= $translation_data['Translation']['identifier_id'];
		$this->set('identifierNeighbours', $this->Translation->Identifier->getNeighbours($identifier_id));
		$this->set('identifierColumns', $identifierColumns = $this->Translation->IdentifierColumn->find('list', array('conditions' => array('identifier_id' => $identifier_id), 'order' => 'IdentifierColumn.id')));
		if ($identifierColumns)
		{
			$contain = array('Translation' => array(
				'conditions' => array('Translation.parent_id' => $translation_data['Translation']['id']),
			));
			$identifierColumnsAll = $this->Translation->IdentifierColumn->find('all', array('conditions' => array('identifier_id' => $identifier_id), 'order' => 'IdentifierColumn.id', 'contain' => $contain));
			foreach ($translation_data['ChildTranslation'] as $childTranslationKey => $childTranslation)
			{
				$mapChildTranslationsColumns[$childTranslation['identifier_column_id']] = $childTranslationKey;
				
			}
			$this->set(compact('mapChildTranslationsColumns'));
			$this->set('identifierColumnsDetails', Set::combine($identifierColumnsAll, '{n}.IdentifierColumn.id', '{n}.IdentifierColumn'));
			$this->set('identifierColumnTranslations', Set::combine($translation_data['ChildTranslation'], '{n}.identifier_column_id'));//, '{n}.identifier_column_id'));
		}
		$identifier = $this->Translation->Identifier->read(null, $this->data['Translation']['identifier_id']);
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
	}

	function admin_view($id = null) {
		return $this->view($id);
	}

	function admin_add() {
		return $this->add();
	}

	function admin_edit($id = null) {
		return $this->edit($id);
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
