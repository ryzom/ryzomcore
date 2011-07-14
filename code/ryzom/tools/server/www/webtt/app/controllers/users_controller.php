<?php
class UsersController extends AppController {

	var $name = 'Users';

	var $components = array('Email');

	function index() {
		$this->User->recursive = 0;
		$this->set('users', $this->paginate());
	}

	function view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid user', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('user', $this->User->read(null, $id));
	}

	function add() {
		if (!empty($this->data)) {
			$this->User->create();
			if ($this->User->save($this->data)) {
				$this->Session->setFlash(__('The user has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The user could not be saved. Please, try again.', true));
			}
		}
	}

	function edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid user', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->User->save($this->data)) {
				$this->Session->setFlash(__('The user has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The user could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->User->read(null, $id);
		}
	}

	function delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for user', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->User->delete($id)) {
			$this->Session->setFlash(__('User deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('User was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	function admin_index() {
		$this->User->recursive = 0;
		$this->set('users', $this->paginate());
	}

	function admin_view($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid user', true));
			$this->redirect(array('action' => 'index'));
		}
		$this->set('user', $this->User->read(null, $id));
	}

	function admin_add() {
		if (!empty($this->data)) {
			$this->User->create();
			if ($this->User->save($this->data)) {
				$this->Session->setFlash(__('The user has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The user could not be saved. Please, try again.', true));
			}
		}
	}

	function admin_edit($id = null) {
		if (!$id && empty($this->data)) {
			$this->Session->setFlash(__('Invalid user', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!empty($this->data)) {
			if ($this->User->save($this->data)) {
				$this->Session->setFlash(__('The user has been saved', true));
				$this->redirect(array('action' => 'index'));
			} else {
				$this->Session->setFlash(__('The user could not be saved. Please, try again.', true));
			}
		}
		if (empty($this->data)) {
			$this->data = $this->User->read(null, $id);
		}
	}

	function admin_delete($id = null) {
		if (!$id) {
			$this->Session->setFlash(__('Invalid id for user', true));
			$this->redirect(array('action'=>'index'));
		}
		if ($this->User->delete($id)) {
			$this->Session->setFlash(__('User deleted', true));
			$this->redirect(array('action'=>'index'));
		}
		$this->Session->setFlash(__('User was not deleted', true));
		$this->redirect(array('action' => 'index'));
	}
	
	function login() {
	}

	function logout() {
		   $this->redirect($this->Auth->logout());
	}
	
	function beforeFilter() {
		parent::beforeFilter();
		$this->Auth->allow(array('register', 'login', 'logout', 'confirm'));
	}

	function register() {
		if(!empty($this->data)) {
			$this->User->create();
/*			$assigned_password = 'newpass';
			$this->data['User']['password'] = $this->Auth->password($assigned_password);*/
			$this->data['User']['password'] = $this->Auth->password($this->data['User']['passwd']);
			$this->data['User']['confirm_hash'] = $this->Auth->password($this->data['User']['name'] . time());
			if($user = $this->User->save($this->data)) {
				// send signup email containing password to the user
//				$this->Session->setFlash('your password is ' . $assigned_password);
//				$this->Session->setFlash('your password is ' . var_export($this->data['User']['password'], true));
//				$this->Auth->login($this->data);
//				$this->Email->delivery = 'debug';
				$this->Email->from = 'webtt-noreply@openlink.pl';
				$this->Email->to = $user['User']['email'];
				$this->Email->subject = 'WebTT registration';
				$this->Email->sendAs = 'text';
				$this->Email->template = 'registration';
				$this->set('user', $this->data);
				$this->set('serverName', $_SERVER['SERVER_NAME']);
				$this->params['url']['ext'] = 'no_debug';
//				var_dump($this->helpers);
				unset($this->helpers['DebugKit.Toolbar']);
				$this->Email->send();
				$this->Session->setFlash('Thank you for registreation. Please use confirm link from email to finalize registration.');
				$this->redirect('/');
			}
		}
	}
	
	function confirm($confirm_hash)
	{
		$user = $this->User->find('first', array('conditions' => array('User.confirm_hash' => $confirm_hash)));
		if (!$user)
		{
			$this->Session->setFlash('No user found. Please register again.');
			$this->redirect('/');
		}
		$this->User->id = $user['User']['id'];
		$this->User->save(array('confirm_hash' => null));
		$this->Session->setFlash('Thank you for registreation. You can now log in.');
		$this->redirect('/');
	}
}
