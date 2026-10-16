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
		$this->set('user', $user_data = $this->User->read(null, $id));
		if (empty($this->data)) {
			$this->data = $user_data;
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
		if (!empty($this->data))
		{
			$user = $this->User->find('first', array('conditions' => array('User.username' => $this->data['User']['username'])));
			$this->log($user);
			if ($user['User']['confirm_hash'])
			{
				$this->Session->delete('Message.auth');
				$this->Session->setFlash('This account is not yet confirmed. Please use confirmation link from email to finalize registration.');
				$this->redirect($this->referer());
			}
			if (!$user['User']['activated'])
			{
				$this->Session->delete('Message.auth');
				$this->Session->setFlash('This account is not yet activated. Please wait until administrator activates your account.');
				$this->redirect($this->referer());
			}

		}
		if (!(empty($this->data)) && $this->Auth->user())
		{
			$this->log('a');
			$this->User->id = $this->Auth->user('id');
			$this->User->saveField('last_login', date('Y-m-d H:i:s'));
			$this->redirect($this->Auth->redirect());
		}
					$this->log('b');
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
			$this->data['User']['password'] = $this->Auth->password($this->data['User']['passwd']);
			$this->data['User']['confirm_hash'] = $this->Auth->password($this->data['User']['name'] . time());
			if($user = $this->User->save($this->data)) {
				// send signup email containing password to the user
				$this->Email->from = 'webtt-noreply@openlink.pl';
				$this->Email->to = $user['User']['email'];
				$this->Email->subject = 'WebTT registration';
				$this->Email->sendAs = 'text';
				$this->Email->template = 'registration';
				$this->set('user', $this->data);
				$this->set('serverName', $_SERVER['SERVER_NAME']);
				$this->params['url']['ext'] = 'no_debug';
				unset($this->helpers['DebugKit.Toolbar']);
				$this->Email->send();
				$this->Session->setFlash('Thank you for registrating. Please use confirmation link from email to finalize registration.');
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
		$this->Session->setFlash('Thank you for registrating. You will be able to log in after your account is activated by administrator.');
		$this->redirect('/');
	}
}
