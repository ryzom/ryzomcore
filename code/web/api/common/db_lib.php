<?php
/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

define('SQL_DEF_TEXT', 0);
define('SQL_DEF_BOOLEAN', 1);
define('SQL_DEF_INT', 2);
define('SQL_DEF_DATE', 3);


// Wrapper for SQL database interactions
class ServerDatabase
{
    var $hostname = '';
    var $username = '';
    var $password = '';
    var $database = '';

    var $_connection = Null;

    function ServerDatabase($host='', $user='', $passwd='', $dbname='')
    {
    	if (($host != '') && ($user != '') && ($dbname != ''))
    	{
	    	$this->hostname = $host;
    		$this->username = $user;
    		$this->password = $passwd;
    		$this->database = $dbname;
    	}

		if (($this->hostname != '') && ($this->username != '') && ($this->database != ''))
		{
        	$this->_connection = mysql_connect($this->hostname, $this->username, $this->password)
				or die("ERR1"); // ace . $this->get_error());
        	$this->select_db($this->database);
		}
    }

    function close()
    {
        @mysql_close($this->_connection);
    }

    function query($sql_statement)
    {
        $result = mysql_query($sql_statement, $this->_connection);
        return $result;
    }

	function select_db($dbname) {
		$this->database = $dbname;
		mysql_select_db($this->database, $this->_connection) or die("Database selection error : " . $this->get_error());
	}

	function num_rows($result)
	{
		return @mysql_num_rows($result);
	}

    function fetch_row($result, $result_type=MYSQL_BOTH)
    {
        return @mysql_fetch_array($result, $result_type);
    }
    
	function fetch_assoc($result)
    {
        return @mysql_fetch_array($result, MYSQL_ASSOC);
    }


    function query_single_row($sql_statement)
    {
        $result = $this->query($sql_statement);
        return @mysql_fetch_array($result);
    }

	function free_result($result)
	{
		@mysql_free_result($result);
	}

    function get_error()
    {
        return mysql_errno($this->_connection) .": ". mysql_error($this->_connection);
    }

    function last_insert_id()
    {
        return @mysql_insert_id();
    }

    function change_to($host,$user,$pass,$dbname)
    {
    	$this->close();
    	$this->hostname = $host;
    	$this->username = $user;
    	$this->password = $pass;
    	$this->database = $dbname;
    	$this->ServerDatabase();
    }
}




class ryDB {

	private static $_instances = array();
	private $db;
	private $defs = array();
	private $errors = '';
	
		
	private function __construct($db_name) {
		global $_RYZOM_API_CONFIG;

		$this->db = new ServerDatabase(RYAPI_WEBDB_HOST, RYAPI_WEBDB_LOGIN, RYAPI_WEBDB_PASS, $db_name);
		$this->db->query("SET NAMES utf8");
	}

	public static function getInstance($db_name) {
		if (!array_key_exists($db_name, self::$_instances))
			self::$_instances[$db_name] = new ryDB($db_name);

		self::$_instances[$db_name]->db->select_db($db_name);
		return self::$_instances[$db_name];
	}

	function setDbDefs($table, $defs) {
		$this->defs[$table] = $defs;
	}
	
	function getDefs($table) {
		if (!array_key_exists($table, $this->defs))
			die("Please add tables defs using setDbDefs('$table', \$defs)");
		return $this->defs[$table];
	}
	
	function getErrors() {
		return $this->db->get_error();
	}
	
	function now() {
		return date('Y-m-d H:i:s', time());
	}

	function toDate($timestamp) {
		return date('Y-m-d H:i:s', $timestamp);
	}

	function fromDate($string_date) {
		return strtotime($string_date);
	}

	function addDbTableProp($table, $props) {
		$this->props[$table] = $props;
	}
	
	/// DIRECT QUERY
	function sqlQuery($sql) {
		$result = $this->db->query($sql);
		$ret = array();
		while ($row = $this->db->fetch_row($result)) {
			$ret[] = $row;
		}
		return $ret;
	}
	
	
	/// QUERY ///
	function sqlSelect($table, $props, $values=array(), $extra='') {
		if ($table) {
			$sql = "SELECT\n\t";
			$params = array();
			$test = array();
			if (!$props)
				die("Bad Select on [$table] : missing props");
			
			foreach($props as $name => $type)
				$params[] = '`'.addslashes($name).'`';
				
			foreach($values as $name => $value) {
				if ($name[0] == '=')
					$test[] = '('.addslashes(substr($name, 1)).' LIKE '.var_export($value, true).')';
				else
					$test[] = '('.addslashes($name).' = '.var_export($value, true).')';
			}
			$sql .= implode(",\n\t", $params)."\nFROM\n\t".$table."\n";
			if ($test)
				$sql .= "WHERE\n\t".implode("\nAND\n\t", $test);
		}

		if ($extra)
			$sql .= "\n".$extra;
		return $sql.';';

	}
	
	function querySingle($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->db->query($sql);
		return $this->db->fetch_row($result);
	}
	
	function querySingleAssoc($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->db->query($sql);
		return $this->db->fetch_row($result, MYSQL_ASSOC);
	}
	
	function query($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->db->query($sql);
		$ret = array();
		while ($row = $this->db->fetch_row($result)) {
			$ret[] = $row;
		}
		return $ret;
	}
	
	function queryAssoc($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->db->query($sql);
		$ret = array();
		while ($row = $this->db->fetch_row($result, MYSQL_ASSOC)) {
			$ret[] = $row;
		}
		return $ret;
	}

	/// INSERT ///
	function sqlInsert($table, $props, $vals) {
		$sql = 'INSERT INTO '.$table.' ';
		$params = array();
		$values = array();
		foreach($props as $name => $type) {
			if (!isset($vals[$name]))
				continue;
			$params[] = $name;
			switch ($type) {
				case SQL_DEF_BOOLEAN:
					$values[] = $vals[$name]?1:0;
					break;
				case SQL_DEF_INT:
					$values[] = $vals[$name];
					break;
				case SQL_DEF_DATE: // date
					if (is_string($vals[$name]))
						$values[] = "'".addslashes($vals[$name])."'";
					else
						$values[] = "'".$this->toDate($vals[$name])."'";
					break;
				default:
					$values[] = "'".addslashes($vals[$name])."'";
					break;
			}
		}
		$sql .= "(\n\t".implode(",\n\t", $params)."\n) VALUES (\n\t".implode(",\n\t", $values)."\n);";
		return $sql;
	}

	function insert($table, $values) {
		$sql = $this->sqlInsert($table, $this->getDefs($table), $values);
		$this->db->query($sql);
		return $this->db->last_insert_id();
	}

	/// DELETE ///
	function sqlDelete($table, $values=array(), $where='') {
		$sql = "DELETE FROM\n\t".$table."\n";
		$test = array();
		foreach($values as $name => $value)
			$test[] = '('.addslashes($name).' = '.var_export($value, true).')';
		
		if ($test or $where)
			$sql .= "WHERE\n\t";
		if ($test)
			$sql .= implode("\nAND\n\t", $test);
		if ($where)
			$sql .= "\n".$where;
		return $sql.';';
	}
	
	function delete($table, $values=array(), $where='') {
		$sql = $this->sqlDelete($table, $values, $where);
		$result = $this->db->query($sql);
		return $result;
	}

	/// UPDATE ///
	function sqlUpdate($table, $props, $vals, $tests, $extra) {
		$sql = 'UPDATE '.$table.' SET ';
		$params = array();
		$test = array();
		$values = array();
		foreach($props as $name => $type) {
			if (!array_key_exists($name, $vals))
				continue;
			switch ($type) {
				case SQL_DEF_BOOLEAN:
					$values[] = '`'.$name.'` = '.($vals[$name]?'1':'0');
					break;
				case SQL_DEF_DATE:
					if (is_string($vals[$name]))
						$values[] = '`'.$name.'` = \''.addslashes($vals[$name]).'\'';
					else
						$values[] = '`'.$name.'` = \''.$this->toDate($vals[$name]).'\'';
					break;
				default:
					$values[] = '`'.$name.'` = \''.addslashes($vals[$name]).'\'';
					break;
			}
		}
		$sql .= "\n\t".implode(",\n\t", $values)."\n";

		foreach($tests as $name => $value) {
			$test[] = '('.addslashes($name).' = '.var_export($value, true).')';
		}
		if ($test)
			$sql .= "WHERE\n\t".implode("\nAND\n\t", $test);
		
		$sql .= "\n".$extra;

		return $sql;
	}


	function update($table, $values=array(), $test=array(), $extra='') {
		$sql = $this->sqlUpdate($table, $this->getDefs($table), $values, $test, $extra);
		$result = $this->db->query($sql);
		return $result;
	}

	function sqlInsertOrUpdate($table, $props, $vals) {
		$sql = $this->sqlInsert($table, $props, $vals);
		$sql = substr($sql, 0, strlen($sql)-1);
		$params = array();
		$test = array();
		$values = array();
		foreach($props as $prop) {
			if (!array_key_exists($prop[2], $vals))
			continue;
			$type = $prop[0];
			$check = $prop[1];
			$name = $prop[2];
			if ($type{0} == '#')
			$type = substr($type, 1);
			if (($type{0} == '>') or ($type == 'id'))
			continue;
			switch ($type) {
			case 'trad':
				$values[] = '`'.$name."` = '".addslashes($vals[$name])."'";
				break;
			case 'textarea':
			case 'string':
			case 'option':
				$values[] =  '`'.$name."` = '".addslashes($vals[$name])."'";
				break;
			case 'id':
			case 'int':
			case 'float':
				$values[] =  '`'.$name.'` = '.addslashes($vals[$name]);
				break;
			case 'bool':
				$values[] = '`'.$name.'` = '.($vals[$name]?'1':'0');
				break;
			}
		}
		$sql .= "\nON DUPLICATE KEY UPDATE\n\t".implode(",\n\t", $values)."\n";
		return $sql;
	}

	function insertOrUpdate($table, $values) {
		$sql = $this->sqlInsertOrUpdate($table, $this->getDefs($table), $values);
		return $result;
	}

	
	/// Display
	function getTableHtml($name, $params, $values, $order_by='')
	{
		$ret = '<table cellpadding="0" cellspacing="0" width="100%">';
		$tr_header = '<td align="left" height="32px">&nbsp;';
		$tr_header .= implode('</td><td align="left">&nbsp;', array_keys($params)).'</td>';
		$ret .= _s('t header', $tr_header);
		$i = 0;
		if (!$values)
			return '';
		$current_section = '';
		foreach ($values as $rows) {
			if ($order_by && $rows[$order_by] != $current_section) {
				$current_section = $rows[$order_by];
				if ($current_section != '0')
					$ret .= _s('t row ', '<td>'._s('section', $current_section).'</td>'.str_repeat('<td>'._s('section', '&nbsp;').'</td>', count($params)-1));
			}
			$td = '';
			foreach ($params as $test => $param)
				$td .= '<td align="left" height="22px">&nbsp;'.$rows[$param].'</td>';
			$ret .= _s('t row '.strval($i % 2), $td);
			$i++;
		}
		$ret .= '</table>';
		return $ret;
	}

}

?>
