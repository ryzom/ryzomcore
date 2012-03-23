<?php

if(strstr($_SERVER['HTTP_USER_AGENT'], 'Ryzom')){
	define('isWEBIG', true);
}else{
	define('isWEBIG', false);
}


require_once 'config.php';
require_once 'pdr_util.php';
require_once 'pdr_util_character.php';
require_once 'pdr_util_guild.php';

/**
 * Singleton wrapper class for PDO database connection
 */
class DB {
	private $_pdo = null;

	private function __construct(){
		$this->_pdo = new PDO('mysql:host='.$GLOBALS['DBHost'].';dbname='.$GLOBALS['DBName'].';charset=utf-8', $GLOBALS['DBUserName'], $GLOBALS['DBPassword'], array(PDO::MYSQL_ATTR_INIT_COMMAND => 'set names utf8'));
		$this->_pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	}

	public function getInstance(){
		static $instance = null;
		if($instance === null){
			$instance = new DB();
		}
		return $instance;
	}

	/**
	 * @param string $sql
	 * @param array $params (optional)
	 * @return PDOStatement
	 */
	function query($sql, $params=array()){
		if(empty($params)){
			$stmt = $this->_pdo->query($sql);
		}else{
			$stmt = $this->_pdo->prepare($sql);
			$stmt->execute($params);
		}
		return $stmt;
	}
}

/**
 * Verify and log-in user
 *
 * @return mixed user info array or boolean FALSE when user was not verified
 */
function app_authenticate(){
	// mask possible double session_start() warning
	@session_start();

	if(isWEBIG){
		// ingame login

		// gather user from $_GET or $_POST variables
		$user = webig_user();

		// verify it against database
		$user = webig_auth($user);
	}else{
		// outgame login

		if(isset($_POST['login'])){
			// login request
			$shardid = is($_POST['login']['shardid'], '');
			$name    = is($_POST['login']['name'], '');
			$passwd  = is($_POST['login']['passwd'], '');

			// verify character and password against database and populate $_GET with user info
			$user = login_auth($shardid, $name, $passwd);
			$_SESSION['login']['error'] = ($user === false);
		}elseif(isset($_GET['logout'])){
			// logout request
			unset($_SESSION['user']);
			unset($_SESSION['authkey']);

			// redirect to self without URL parameters
			header('Location: '.$_SERVER['PHP_SELF']);
			exit;
		}else{
			// continue session
			$user = is($_SESSION['user'], false);

			// verify user in session against database (e.g. user might be deleted)
			$user = load_user($user['shardid'], null, $user['cid']);
		}
	}

	// auth failed?
	if(empty($user)){
		return false;
	}

	// remove values we do not need to keep in session
	unset($user['password']);
	unset($user['cookie']);

	// return user info array on success
	$_SESSION['user'] = $user;
	return $user;
}

// get user info that WebIG sends us
function webig_user(){
	$user = array();

	// shard id (302)
	$user['shardid'] = ryzom_get_param('shardid');

	// character name (User)
	$user['name']    = ryzom_get_param('name');

	// character id (16), user id is calculated as 'uid = cid >> 4';
	$user['cid']     = ryzom_get_param('cid');

	// language
	$user['lang']    = ryzom_get_param('lang');

	$user['authkey'] = ryzom_get_param('authkey');

	return $user;
}

/**
 * Verify character using info from ig browser
 *
 * @param  array $user
 * @return bool  return user info array on success and FALSE on error
 */
function webig_auth($user){
	// find user by shard and character id (name might be temporarily changed in game)
	$result = load_user($user['shardid'], null, $user['cid']);
	if(empty($result)){
		// should not happen, but user was not found
		return false;
	}

	// Create auth key by using cookie from DB and user info from user
	$authkey = webig_create_authkey($user, $result['cookie']);
	if($user['authkey'] !== $authkey){
		// something is out of sync - either user info or cookie
		return false;
	}

	// return result from DB
	return $result;
}

/**
 * Verify character
 *
 * @param  int    $shardid character shard id
 * @param  string $name    character name
 * @param  string $passwd  plain text password
 * @return mixed  return user info array on success or boolean false on error
 */
function login_auth($shardid, $name, $passwd){
	// get character from db
	$user = load_user($shardid, $name);
	if(empty($user)){
		// user was not found
		return false;
	}

	$passwd = crypt($passwd, substr($user['password'], 0, 2));
	if($passwd !== $user['password']){
		// password failed
		return false;
	}

	return $user;
}

/**
 * Fetch user info from db
 *
 * If name is NULL, then $cid is used
 *
 * @param int    $shardid
 * @param string $name
 * @param int    $cid
 * @return array
 */
function load_user($shardid, $name, $cid = null){
	// `nel`.`user` has password
	// `ring_open`.`ring_users` has cookie
	// `ring_open`.`characters` has char_id, char_name, home_mainland_session_id(==shardid)

	$sql = 'SELECT c.`char_id` cid, c.`char_name` name, c.`home_mainland_session_id` shardid, n.`password`, u.`cookie`
		FROM `ring_open`.`characters` c
		JOIN `ring_open`.`ring_users` u on u.`user_id` = c.`user_id`
		JOIN `nel`.`user`             n on n.`uid` = c.`user_id`
		WHERE c.`home_mainland_session_id` = :shardid';
	$params = array('shardid' => $shardid);
	if($name !== null){
		$sql .= ' AND c.`char_name` = :name';
		$params['name'] = $name;
	}elseif($cid !== null){
		$sql .= ' AND c.`char_id` = :cid';
		$params['cid'] = $cid;
	}else{
		// $name or $cid both empty
		return false;
	}

	$result = DB::getInstance()->query($sql, $params)->fetch(PDO::FETCH_ASSOC);
	return $result;
}

/**
 * Verify user info that ig browser sent us using cookie from database
 *
 * @param  array  $user   user info array
 * @param  string $cookie User login cookie from database
 * @return string         md5 hash
 */
function webig_create_authkey($user, $cookie){
	return md5($user['shardid'].$user['name'].$user['cid'].'\''.$cookie.'\'');
}

/**
 * Return user privileges from DB
 *
 * @param int $uid user id (uid = cid >> 4)
 * @return mixed array of user privileges or boolean FALSE when user was not found
 */
function webig_get_privileges($uid){
	$sql = 'select `privilege` from `nel`.`user` where `uid` = :id';
	$params = array('id' => $uid);

	$result = DB::getInstance()->query($sql, $params)->fetchColumn(0);

	if($result !== false){
		$result = explode(':', $result);
		$ret = array();
		foreach($result as $k=>$v){
			if($v != ''){
				$ret[]=$v;
			}
		}
		$result = $ret;
	}

	return $result;
}

/**
 * Test user privileges
 *
 * @param  int   $uid  user id
 * @param  array $priv array of privileges, like array('DEV', 'GM')
 * @return bool
 */
function webig_has_privileges($uid, $priv){
	$userpriv = webig_get_privileges($uid);
	$result   = array_intersect($priv, $userpriv);
	return !empty($result);
}

/**
 * Test user privileges against (DEV, SGM, GM)
 *
 * @param int $uid user id
 * @return bool
 */
function webig_is_admin($uid){
	// entities_game_service/player_manager/player_manager.cpp defines order
	// DEV > SGM > EM > GM > EG > VG > SG > G > OBSERVER > PR
	return webig_has_privileges($uid, array('DEV', 'SGM', 'EM', 'GM'));
}

/**
 * Load character from shard save binary file
 *
 * @param int $cid
 * @return mixed array with character info or boolean FALSE on error
 */
function webig_load_character($cid){
	$pdr  = CharacterPdr::createDefault();
	$char = $pdr->load($cid);
	if(empty($char)){
		return false;
	}

	$result = array(
		'id'     => (int)    $cid,
		'name'   => (string) $char->EntityBase->_Name['value'],
		'title'  => (string) $char->_Title['value'],
		'race'   => (string) $char->EntityBase->_Race['value'],
		'gender' => (int)    $char->EntityBase->_Gender['value'] == '0' ? 'male' : 'female',
		'cult'   => (string) $char->DeclaredCult['value'],
		'civ'    => (string) $char->DeclaredCiv['value'],
		'guild'  => false,
	);

	$guild_id = (int) $char->_GuildId['value'];
	if($guild_id>0){
		// if char is in guild, then also get guild info
		$result['guild'] = webig_load_guild($guild_id);

		// get guild rank (also from guild file)
		$result['guild_membership'] = webig_load_guild_membership($guild_id, $cid);
	}
	unset($char);

	return $result;
}

/**
 * Load basic guild info (name, description, motd, culv, civ)
 *
 * @param int $guild_id
 * @return mixed array with guild info or boolean FALSE on error
 */
function webig_load_guild($guild_id){
	$pdr   = GuildPdr::createDefault();
	$guild = $pdr->load($guild_id);
	if(empty($guild)){
		return false;
	}

	$result = array(
		'id'          => (int) $guild_id,
		'icon'        => (string) $guild->Icon['value'],
		'name'        => (string) $guild->_Name['value'],
		'description' => (string) $guild->_Description['value'],
		'motd'        => (string) $guild->_MessageOfTheDay['value'],
		'cult'        => (string) $guild->DeclaredCult['value'],
		'civ'         => (string) $guild->DeclaredCiv['value'],
	);
	unset($guild);

	return $result;
}

/**
 * Load guild member info
 *
 * @param int $guild_id
 * @param int $char_id
 * @return mixed array with guild member info or boolean FALSE if guild or character not found
 */
function webig_load_guild_membership($guild_id, $char_id){
	$pdr   = GuildPdr::createDefault();
	$guild = $pdr->load($guild_id);
	if(empty($guild)){
		return false;
	}

	$result = false;

	// test for 'id' and type (CHAR == 0), ignore creator (should be 0x00) and dynamic
	// 0x0000000013:00:00:87
	$eid = sprintf('0x%010x:00:', $char_id);
	$i = 0;
	while(isset($guild->Members->__Key__[$i])){
		$key = $guild->Members->__Key__[$i];
		$pos = strpos((string)$key['value'], $eid);
		if($pos === 1){
			$val = $guild->Members->__Val__[$i];
			$result = array(
				'grade'  => (string) $val->Members->Grade['value'],
				'joined' => (int) $val->Members->EnterTime['value'],
			);
			break;
		}
		$i++;
	}
	unset($guild);

	return $result;
}

// shortcut for 'isset() ? .. : ..'
function is(&$var, $default = null){
	return isset($var) ? $var : $default;
}

// escape string so it's safe for HTML
function h($str){
	return htmlspecialchars($str, ENT_QUOTES, 'UTF-8');
}

// return $_GET[var] or $_POST[var] or $default
function ryzom_get_param($var, $default=''){
	return is($_GET[$var], is($_POST[$var], $default));
}