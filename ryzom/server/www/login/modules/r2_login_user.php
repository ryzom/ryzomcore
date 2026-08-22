<?php
use donatj\UserAgent\UserAgentParser;
require __DIR__ . '/../vendor/autoload.php';

include_once('r2_login_logs.php');

class RingUser {

	function __construct($db, $login, $password, $clientApplication, $submittedLang) {
		$this->user = Null;
		$this->uid = 0;
		$this->db = $db;
		$this->impersonateTargetLogin = null;
		$this->impersonating = false;

		// Impersonation: "targetLogin:adminLogin" authenticates as adminLogin, then acts as targetLogin
		if (strpos($login, ':') !== False) {
			list($targetLogin, $adminLogin) = explode(':', $login, 2);
			$this->impersonateTargetLogin = $targetLogin;
			$login = $adminLogin;
		}

		$this->login = $login;
		$this->password = $password;
		$this->lang = $submittedLang;
		$this->logFile = new CWwwLog();

		if ($clientApplication == 'ryzom_beta')
			$this->client = 'ryzom_live';
		else
			$this->client = $clientApplication;

		$this->db->select();

		$sub_url = 'https://me.ryzom.com/api/oauth/gameaccess?tokenA='.urlencode($login).'&secure='.urlencode($password).'&user_agent='.urlencode($_SERVER['HTTP_USER_AGENT']);
		$oauth = @json_decode(file_get_contents($sub_url), true);
		if (isset($oauth['access']) && $oauth['access']) {
			if (isset($oauth['user_id'])) {
				$this->user =  $this->db->querySingle('SELECT * FROM user WHERE UId='.$this->db->e($oauth['user_id']));
				if ($this->user) {
					$this->password = $this->user['Password'];
					$this->login = $this->user['Login'];
				}
			} else {
				$this->user = array('token_auth' => $oauth['access']);
			}
		}

		if (!$this->user)
			$this->user =  $this->db->querySingle('SELECT * FROM user WHERE Login="'.$this->db->e($this->login).'"');

		if ($this->user) {
			$this->uid = $this->user['UId'];
			$this->secure = $this->user['SecurePassword'];
		}


		$this->authenticated = False;
		setMsgLanguage($submittedLang);
	}

	function askSalt() {
		$user = $this->user;
		if ($user) {
			if (isset($user['token_auth']) && $user['token_auth']) {
				echo '1:'.$user['token_auth'];
			} else {
				if (strlen($user['SecurePassword']) == 128) // New 2017 security
					echo '1:$6$'.substr($user['SecurePassword'], 16).'$';
				else
					echo '1:'.substr($user['Password'], 0, 2);
			}
		} else
			dieError(2001, $login, 'askSalt');
	}

	function createRingInfo($ServerDomain) {
		$domainInfo = $ServerDomain->domainInfo;
		$this->db->select($domainInfo['ring_db_name']);
		$result = $this->db->querySingle('SELECT user_id FROM ring_users where user_id = "'.$this->db->e($this->uid).'"');
		if (!$result)
			$this->db->query('INSERT INTO ring_users SET user_id = "'.$this->db->e($this->uid).'", user_name = "'.$this->db->e($this->login).'", user_type="ut_pioneer"');
	}

	function checkSteam() {
		$this->steamid = NULL;
		$this->steamuser = NULL;

		if (isset($_GET['steam_auth_session_ticket'])) {
			$ticket = $_GET['steam_auth_session_ticket'];
			$infos = file_get_contents('https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/?key='.STEAM_API_KEY.'&appid='.STEAM_APP_ID.'&ticket='.$ticket);
			$this->logFile->logStr($infos);
			$steam_auth = json_decode($infos, true);
			if (isset($steam_auth['response']['error']))
				rocket(false, $steam_auth['response']['error']['errordesc'], 'steam-error');
			elseif (isset($steam_auth['response']['params']))  {
				$steam_params = $steam_auth['response']['params'];
				if ($steam_params['result'] == 'OK')
					$this->steamid = $steam_params['steamid'];
			} else {
			}

			if ($this->steamid) {
				$this->steamuser = $this->db->querySingle('SELECT * FROM steam_users WHERE SteamId="'.$this->db->e($this->steamid).'"');
				if ($this->steamuser) {
					if (!$this->login || $this->user['UId'] == $this->steamuser['UId']) {
						$this->user = $this->db->querySingle('SELECT * FROM user where UId='.$this->steamuser['UId']);
						if ($this->user)
							$this->uid = $this->user['UId'];
						$this->authenticated = True;
					}
				} //else
			} else {
				rocket(false, $this->login.' no steam id', 'steam-error');
			}
		}
	}

	function checkAccess() {
		if ($this->impersonating)
			return;

		// SUSPENDED or BANNED : SUSPEND SGM / BANNED ADMIN
		if (strpos($this->user['ExtendedPrivilege'], ':BAN:') !== False || $this->user['ProfileAccess'] == 'Suspended' || $this->user['ProfileAccess'] == 'Banned')
			dieError(666, $this->user['ProfileAccess']."\n");

		// CORRUPTED
		if ($this->user['FromSource'] == 'ntcp' && strlen($this->user['Password']) < 20) {
			rocket(false, 'uid:'.$this->uid.' login:'.$login.' blocked because in the corrupted list', 'login-errors');
			dieError(4444, $this->client);
			dieError(4444, $this->client);
		}

		// NO ACCESS
		if (strtolower($this->user['GroupName']) == 'noaccess') {
			dieError(3011, $this->client);
		}

		// YUBO ACCOUNTS on Atys
		if ($this->client == 'ryzom_live' && strtolower($this->user['GroupName']) == 'yubo') {
			rocket(false, 'ip:'.$_SERVER['REMOTE_ADDR'].' uid:'.$this->uid.' login:'.$this->login.' blocked on ATYS because it\'s a Yubo account', 'login-errors');
			dieError(3013, $this->client);
		}

		// Non YUBO ACCOUNTS or Not PRIV or Not Beta Tester
		if ($this->client == 'ryzom_dev' && strpos($this->user['ExtendedPrivilege'], ':ATS:') === False && strtolower($this->user['GroupName']) != 'yubo' && !$this->user['Privilege']) {
			rocket(false, 'ip:'.$_SERVER['REMOTE_ADDR'].' uid:'.$this->uid.' login:'.$this->login.' blocked on YUBO because it\'s not a Yubo account', 'login-errors');
			dieError(3014, $this->client);
		}

		if ($this->client == 'ryzom_dev' && !$this->user['Privilege'] && $this->user['ToolsGroup'] != '1') {
			rocket(false, 'ip:'.$_SERVER['REMOTE_ADDR'].' uid:'.$this->uid.' login:'.$this->login.' blocked on YUBO because not in Ryzom Team', 'login-errors');
			dieError(3014, $this->client);
		}

		if ($this->client == 'ryzom_dev' && strtolower($this->user['GroupName']) == 'beta testers' && $this->user['Privilege']) {
			rocket(false, 'ip:'.$_SERVER['REMOTE_ADDR'].' uid:'.$this->uid.' login:'.$this->login.' blocked on YUBO because it\'s a Beta Tester account with Privs', 'login-errors');
			dieError(3014, $this->client);
		}

		// PRIV with LEAK PASSWORD
		if ($this->user['Privilege'] && strlen($this->user['Password']) < 20)
			dieError(4444, $this->client);

		// IP BANNED
		if (in_array($_SERVER['REMOTE_ADDR'], explode(' ', ACCESS_BANNED))) {
			rocket(true, 'ip:'.$_SERVER['REMOTE_ADDR'].' uid:'.$this->uid.' login:'.$this->login.' blocked because it\'s a hacker!', 'login-errors');
			dieError(4445, $this->client);
		}

	}

	function updatePrivs($privs) {
		$this->db->query('UPDATE user SET ExtendedPrivilege="'.$this->db->e($privs).'" WHERE UId='.$this->db->e($this->uid));
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


	function checkPermission() {
		$perm = $this->db->querySingle('SELECT * FROM permission WHERE UId="'.$this->db->e($this->uid).'" AND ClientApplication="'.$this->db->e($this->client).'"');
		if (!$perm) {
			// Access permission to Atys or Yubo/Gingo for yubo group or privs
			if ($this->client == 'ryzom_live' || ($this->client != 'ryzom_live' && (strtolower($this->user['GroupName']) == 'yubo' || strpos($this->user['ExtendedPrivilege'], ':ATS:') !== False || $this->user['Privilege'])))
			{
				// add default permission
				$this->db->query('INSERT INTO permission (UId, ClientApplication, ShardId, AccessPrivilege) VALUES ("'.$this->db->e($this->uid).'", "'.$this->db->e($this->client).'", 101, "OPEN,FREE")');
			} else {
				// no permission
				dieError(3011, var_export($this->user['Privilege'], true));
			}
		}

		if ($this->client == 'ryzom_live') { // Check :TRB: and remove it if sub

			// NEW WAY 2020
			$user_agent = $_SERVER['HTTP_USER_AGENT'];
			$token = base64_encode(hash_hmac('sha512', $this->uid.$this->getUserIpAddr(), ME_API_KEY, true));
			$sub_url = 'https://me.ryzom.com/api/account/subscription?user_id='.$this->uid.'&token='.urlencode($token).'&hash='.$this->getUserIpAddr().'&user_agent='.urlencode($user_agent);

			$lang = isset($_GET['lg']) ? $_GET['lg'] : 'unknown';
			$sub_url .= '&lang='.urlencode(substr($lang, 0, 10));
			$sub_content = file_get_contents($sub_url);
			$me_sub = json_decode($sub_content, true);

			if ($me_sub == 'banned')
				dieError(4446, $this->client);

			if ($me_sub == 'suspended')
				dieError(4447, $this->client);

			if ($me_sub == 'multiboxing')
				dieError(4448, $this->client);

			if ($me_sub == 'vpn')
				dieError(4449, $this->client);

			$is_premium = $me_sub == 'premium' || $this->user['Privilege'] || $this->user['GMId'];

			if ($is_premium) {
				//// UPDATE FOR SUB ACCOUNT : remove :TRB: tag
				$this->removePriv('TRB');
			} else {
				//// UPDATE FOR UNSUB ACCOUNT : add :TRB: tag
				$this->addPriv('TRB');
			}
		} else {
			/// On yubo, all accounts are premium except accounts with :F2P: tag
			if (strpos($this->user['ExtendedPrivilege'], ':F2P:') === False)
				$this->removePriv('TRB');
			else
				$this->addPriv('TRB');
		}
	}

	function getUserIpAddr(){
		if(!empty($_SERVER['HTTP_CLIENT_IP'])){
			//ip from share internet
			$ip = $_SERVER['HTTP_CLIENT_IP'];
		}elseif(!empty($_SERVER['HTTP_X_FORWARDED_FOR'])){
			//ip pass from proxy
			$ip = $_SERVER['HTTP_X_FORWARDED_FOR'];
		}else{
			$ip = $_SERVER['REMOTE_ADDR'];
		}
		return $ip;
	}

	function getOS() {
		$user_agent = $_SERVER['HTTP_USER_AGENT'];
		$os_platform = 'Unknown OS Platform';
		$os_array = array(
			'/windows nt 11/i'      =>  'Windows 11',
			'/windows nt 10/i'      =>  'Windows 10',
			'/windows nt 6.3/i'     =>  'Windows 8.1',
			'/windows nt 6.2/i'     =>  'Windows 8',
			'/windows nt 6.1/i'     =>  'Windows 7',
			'/windows nt 6.0/i'     =>  'Windows Vista',
			'/windows nt 5.2/i'     =>  'Windows Server 2003/XP x64',
			'/windows nt 5.1/i'     =>  'Windows XP',
			'/windows xp/i'         =>  'Windows XP',
			'/windows nt 5.0/i'     =>  'Windows 2000',
			'/windows me/i'         =>  'Windows ME',
			'/win98/i'              =>  'Windows 98',
			'/win95/i'              =>  'Windows 95',
			'/win16/i'              =>  'Windows 3.11',
			'/macintosh|mac os x/i' =>  'Mac OS X',
			'/mac_powerpc/i'        =>  'Mac OS 9',
			'/linux/i'              =>  'Linux',
			'/ubuntu/i'             =>  'Ubuntu',
			'/iphone/i'             =>  'iPhone',
			'/ipod/i'               =>  'iPod',
			'/ipad/i'               =>  'iPad',
			'/android/i'            =>  'Android',
			'/blackberry/i'         =>  'BlackBerry',
			'/webos/i'              =>  'Mobile'
		);

		foreach ($os_array as $regex => $value)
		if (preg_match($regex, $user_agent))
			$os_platform = $value;

		return $os_platform;
	}

	function logConnection() {
		$shards = array('ryzom_dev' => 'YUBO', 'ryzom_live' => 'ATYS', 'ryzom_beta' => 'ATYS_BETA', 'ryzom_test' => 'GINGO');
		$this->db->query('INSERT IGNORE INTO ip_accounts (Ip, Account) VALUES ("'.$this->getUserIpAddr().'", "'.$this->login.'")');
		if ($this->user['Privilege'] != '' && strtolower($this->login) != 'ghostea') {
			rocket(false, 'uid:'.$this->uid.' login:'.$this->login.' priv '.$this->user['Privilege'].' logged on '.$shards[$this->client], 'logins');
			rocket(false, 'ip:'.$this->getUserIpAddr().' uid:'.$this->uid.' login:'.$this->login.' priv '.$this->user['Privilege'].' logged on '.$shards[$this->client], 'logins-ip');
		}
	}

	function checkValidity() {

		$this->checkSteam();

		$user = $this->user;

		if (!$user)
			dieError(3009, $this->login);

		if ($this->user['Password'] == $this->password || $this->authenticated) {
			$this->login = $this->user['Login']; // Correct case

			if ($this->steamid && !$this->steamuser)
				$this->db->query('INSERT INTO steam_users (SteamId, UId) VALUES ("'.$this->steamid.'", '.$this->uid.')');

			if ($this->impersonateTargetLogin !== null) {
				if (!in_array($this->login, explode(' ', ACCESS_IMPERSONATED)))
					dieError(3015, $this->login);

				$targetUser = $this->db->querySingle('SELECT * FROM user WHERE Login="'.$this->db->e($this->impersonateTargetLogin).'"');
				if (!$targetUser)
					dieError(3009, $this->impersonateTargetLogin);

				rocket(false, 'admin:'.$this->login.' impersonating uid:'.$targetUser['UId'].' login:'.$targetUser['Login'], 'impersonation');

				$this->user = $targetUser;
				$this->uid = $targetUser['UId'];
				$this->login = $targetUser['Login'];
				$this->secure = $targetUser['SecurePassword'];
				$this->impersonating = true;
			}

			$this->checkAccess();
			$this->checkPermission();
			$this->logConnection();

			$this->priv = $this->user['Privilege'];

			return True;
		} else
			dieError(2004);
		return False;
	}

}
