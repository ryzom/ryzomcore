<?php
class VotesController extends AppController {

	var $name = 'Votes';

	function index() {
		$this->Vote->recursive = 0;
//		var_dump($this->Vote->belongsTo);
//		var_dump($this->Vote->getAssociated());
//		$model = $this->{$this->modelClass};
//		$this->log($tree=$this->PathResolver->getAssociationsTree($model));
//		$this->log($this->PathResolver->getAssociationsGraph('User',$tree));
//		$this->log($this->PathResolver->printPath($model), 'info');
//		$this->log($this->PathResolver->node_path('Language', $tree));
		$this->set('votes', $this->paginate());
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
		if (empty($this->passedArgs['translation']))
		{
			$this->Session->setFlash(__('You need to choose translation for your vote', true));
			$this->redirect(array('controller' => 'translations', 'action' => 'index'));
		}
		else
		{
			$translation_id = $this->passedArgs['translation'];
			$translation = $this->Vote->Translation->read(null, $translation_id);
			if (!$translation)
			{
				$this->Session->setFlash(__("Translation doesn't exist.", true));
				$this->redirect(array('controller' => 'translations', 'action' => 'index'));
			}
			$vote = array("Vote" => array(
							'translation_id' => $translation_id,
							// TODO: authorized user
							'user_id' => 1,
						),
					);
			$this->Vote->create();
			$this->Vote->save($vote);
			$this->Session->setFlash(__('Vote added', true));
			$this->redirect($this->referer(array('controller' => 'translations', 'action' => 'index')));
//			$this->redirect(array('controller' => 'translations', 'action' => 'index'));
//			$this->data['Translation.identifier_id'] = $identifier_id;
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
