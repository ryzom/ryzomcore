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
class VotesController extends AppController {

	var $name = 'Votes';

	function index() {
		$this->Vote->recursive = 0;
		$conditions = null;
		if (isset($this->passedArgs['translation_id']) && $translation_id = $this->passedArgs['translation_id'])
			$conditions = array('Vote.translation_id' => $translation_id);
		$this->set('votes', $this->paginate($conditions));
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid vote', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('vote', $this->Vote->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->Vote->create();
			if ($this->Vote->save($this->data)) {
				$this->Session->setFlash(__('The vote has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The vote could not be saved. Please, try again.', true));
			}
		}
		$translations = $this->Vote->Translation->find('list');
		$users = $this->Vote->User->find('list');
		$this->set(compact('translations', 'users'));
	}

	function vote() {
		if (empty($this->passedArgs['translation_id']))
		{
			$this->Session->setFlash(__('You need to choose translation for your vote', true));
			$this->redirect(array('controller' => 'translations', 'action' => 'index'));
		}
		else
		{
			$translation_id = $this->passedArgs['translation_id'];
			$translation = $this->Vote->Translation->read(null, $translation_id);
			if (!$translation)
			{
				$this->Session->setFlash(__("Translation doesn't exist.", true));
				$this->redirect(array('controller' => 'translations', 'action' => 'index'));
			}
			$vote = array("Vote" => array(
							'translation_id' => $translation_id,
							'user_id' => $this->Auth->user('id'),
						),
					);
			$this->Vote->create();
			$this->Vote->save($vote);
			$this->Session->setFlash(__('Vote added', true));
			$this->redirect($this->referer());
		}
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid vote', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Vote->save($this->data)) {
				$this->Session->setFlash(__('The vote has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The vote could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Vote->read(null, $id);
		}
		$translations = $this->Vote->Translation->find('list');
		$users = $this->Vote->User->find('list');
		$this->set(compact('translations', 'users'));
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for vote', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Vote->delete($id)) {
			$this->Session->setFlash(__('Vote deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Vote was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->Vote->recursive = 0;
		$this->set('votes', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid vote', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('vote', $this->Vote->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->Vote->create();
			if ($this->Vote->save($this->data)) {
				$this->Session->setFlash(__('The vote has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The vote could not be saved. Please, try again.', true));
			}
		}
		$translations = $this->Vote->Translation->find('list');
		$users = $this->Vote->User->find('list');
		$this->set(compact('translations', 'users'));
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid vote', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->Vote->save($this->data)) {
				$this->Session->setFlash(__('The vote has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The vote could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->Vote->read(null, $id);
		}
		$translations = $this->Vote->Translation->find('list');
		$users = $this->Vote->User->find('list');
		$this->set(compact('translations', 'users'));
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for vote', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->Vote->delete($id)) {
			$this->Session->setFlash(__('Vote deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('Vote was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
}
