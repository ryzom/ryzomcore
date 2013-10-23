<?php
/**
 * Application level Controller
 *
 * This file is application-wide controller file. You can put all
 * application-wide controller-related methods here.
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       cake
 * @subpackage    cake.cake.libs.controller
 * @since         CakePHP(tm) v 0.2.9
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 */

/**
 * This is a placeholder class.
 * Create the same file in app/app_controller.php
 *
 * Add your application-wide methods in the class below, your controllers
 * will inherit them.
 *
 * @package       cake
 * @subpackage    cake.cake.libs.controller
 * @link http://book.cakephp.org/view/957/The-App-Controller
 */
class AppController extends Controller {
	var $components = array('DebugKit.Toolbar' => array(
//			'panels' => array('variables'=>false)
		), 'Session', 'PathResolver', 'Auth');
	var $layout = "new";
	
	function beforeFilter() {
		parent::beforeFilter();
		$this->Auth->autoRedirect = false;
		$this->Auth->authorize = 'controller';
		$this->Auth->userScope = array('User.activated' => true, 'User.confirm_hash' => null);
		$this->Auth->loginAction = array('admin' => false, 'controller' => 'users', 'action' => 'login');

		if ($this->Auth->user('role') == "admin")
			$this->Auth->allow("*");
		else if ($this->Auth->user())
		{
			// $this->Auth->allow('index', 'view', 'add', 'delete', 'edit');
			foreach ($this->methods as $method)
				if (mb_strpos($method, 'admin_') !== 0)
					$this->Auth->allow($method);
		}
	}

	function isAuthorized() {
/*		if (isset($this->params['prefix']) && $this->params['prefix'] == "admin" && $this->Auth->user('role') != "admin")
		{
			return false;
		}

		return true;*/
		$action = $this->params['action'];
		$allowedActions = array_map('strtolower', $this->Auth->allowedActions);
		$isAllowed = (
			$this->Auth->allowedActions == array('*') ||
			in_array($action, $allowedActions)
		);
//		$this->log($isAllowed);
		return $isAllowed;
	}
}
