<?php

function _e($value) {
	global $RingDb;
	return mysqli_real_escape_string($RingDb->db, $value);
}

class LoginDB {

	function __construct($DBHost, $DBUserName, $DBPassword, $DefaultDBName) {
		$this->host = $DBHost;
		$this->user = $DBUserName;
		$this->pass = $DBPassword;
		$this->default_name = $DefaultDBName;
		return $this->connect();
	}

	function connect() {
		$this->db = mysqli_connect($this->host, $this->user, $this->pass) or die(errorMsgBlock(3004, 'DB', $this->host, $this->user));
		return $this->db;
	}

	function select($db='') {
		if ($db)
			$this->name = $db;
		else
			$this->name = $this->default_name;
		mysqli_select_db($this->db, $this->name) or die(errorMsgBlock(3005, 'DB', $this->name, $this->host, $this->user));
	}

	function query($query) {
		$result = mysqli_query($this->db, $query) or die(errorMsgBlock(3006, $query, 'DB', $this->name, $this->host, $this->user, mysqli_error($this->db)));
		$ret = array();
		if (!is_bool($result)) {
			while ($row = mysqli_fetch_array($result))
				$ret[] = $row;
		}
		return $ret;
	}

	function querySingle($query) {
		$result = mysqli_query($this->db, $query) or die(errorMsgBlock(3006, $query, 'DB', $this->name, $this->host, $this->user, mysqli_error($this->db)));
		if (mysqli_num_rows($result))
			return mysqli_fetch_array($result);
		return array();
	}

	function getLastId() {
		return mysqli_insert_id($this->db);
	}

}
