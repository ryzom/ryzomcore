<?php

class RingUser {

	function __construct($db, $login, $password, $clientApplication, $submittedLang) {
		global $AcceptUnknownUser;
		$this->db = $db;
		$this->login = $login;
		$this->password = $password;
		$this->lang = $submittedLang;
		$this->client = $clientApplication;

		$this->db->select();

		$this->user = $this->db->querySingle('SELECT * FROM user WHERE Login="'._e($this->login).'"');
		if ($this->user) {
			$this->uid = $this->user['UId'];
		} else {
			if ($AcceptUnknownUser && $this->password) {
				$this->db->query('INSERT INTO user SET Login = "'._e($this->login).'", Password = "'._e($this->password).'", ShardId=-1, Privilege=":DEV:"');
				$this->user = $this->db->querySingle('SELECT * FROM user WHERE Login="'._e($this->login).'"');
				$this->uid = $this->user['UId'];
			} else {
				$this->uid = 0;
				$this->user = array('Password' => 'AA');
			}
		}
		$this->authenticate = False;
		setMsgLanguage($submittedLang);
	}


	function askSalt() {
		$user = $this->user;
		if ($user)
			echo '1:'.substr($user['Password'], 0, 2);
		else
			dieError(2001, '', 'askSalt');
	}

	function createRingInfo($ServerDomain) {
		$domainInfo = $ServerDomain->domainInfo;
		$this->db->select($domainInfo['ring_db_name']);
		$result = $this->db->querySingle('SELECT user_id FROM ring_users where user_id = "'._e($this->uid).'"');
		if (!$result)
			$this->db->query('INSERT INTO ring_users SET user_id = "'._e($this->uid).'", user_name = "'._e($this->login).'", user_type="ut_pioneer"');
	}

	function checkAccess() {
	}

	function updatePrivs($privs) {
		$this->db->query('UPDATE user SET ExtendedPrivilege="'._e($privs).'" WHERE UId='._e($this->uid));
		$this->user['ExtendedPrivilege'] = $privs;
	}

	function addPriv($priv) {
		$eprivs = explode(':', $this->user['ExtendedPrivilege']);
		$privs = array();
		foreach ($eprivs as $p)
			$privs[$p] = $p;
		unset($privs['']);
		$privs[$priv] = $priv;
		$this->updatePrivs(':'.implode(':', array_keys($privs)).':');
	}

	function removePriv($priv) {
		$eprivs = explode(':', $this->user['ExtendedPrivilege']);
		$privs = array();
		foreach ($eprivs as $p)
			$privs[$p] = $p;
		unset($privs['']);
		unset($privs[$priv]);
		$this->updatePrivs(':'.implode(':', array_keys($privs)).':');
	}


	function checkPermission($ServerDomain) {
		$perm = $this->db->querySingle('SELECT * FROM permission WHERE UId="'._e($this->uid).'" AND ClientApplication="'._e($this->client).'"');
		if (!$perm)
			$this->db->query('INSERT INTO permission (UId, ClientApplication, ShardId, AccessPrivilege) VALUES ("'._e($this->uid).'", "'._e($this->client).'", "'.$ServerDomain->getShard('ShardId').'", "OPEN")');
	}

	function logConnection() {
	}

	function checkValidity($password, $ServerDomain) {
		$user = $this->user;

		if (!$user)
			dieError(3009, $this->login);

		if ($this->user['Password'] == $password) {
			$this->login = $this->user['Login']; // Correct case

			$this->checkAccess();
			$this->checkPermission($ServerDomain);
			$this->logConnection();

			$this->priv = $this->user['Privilege'];

			return True;
		} else
			dieError(2004);
		return False;
	}

}
