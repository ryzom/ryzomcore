<?php


class LoginCb extends CLoginServiceWeb {

	function init($db, $ServerDomain) {
		$this->db = $db;
		$this->domainInfo = $ServerDomain->domainInfo;
		$this->domainId = $ServerDomain->id;
	}


	// receive the login result sent back by the LS
	function loginResult($userId, $cookie, $resultCode, $errorString) {
		if ($resultCode == 0 && $cookie != '') {
			// gather the domain informations (server version, patch urls and backup patch url

			$RingWebHost = $this->domainInfo['web_host'];
			$RingWebHostPHP = $this->domainInfo['web_host_php'];

			// set the cookie
			setcookie('ryzomId' , $cookie, 0, '/');
			$_COOKIE['ryzomId'] = $cookie; // make it available immediately

			// Auto-join an available mainland shard
			global $FSHostLuaMode, $FSHostResultStr;
			$FSHostLuaMode = false;
			$res = joinMainland($userId, $this->domainId, $this->domainInfo['domain_name']);

			if ($res) {
				// return the cookie to the user, il will then be used as an auth to browse the site and to connect to the shard
				//echo "1#".$cookie."#http://".$RingWebHost."/ring/web_start.php\n";
// use this line to use woopra stats
//                  echo "1#".$cookie."#".$FSHostResultStr."#http://".$RingWebHost."/ring/web_start.php#http://".$RingWebHostPHP."/ring/#1\n";
				echo '1#'.$cookie.'#'.$FSHostResultStr.'#http://'.$RingWebHost.'/ring/web_start.php#http://'.$RingWebHostPHP.'/ring/'."\n";
				// return the ring domain information
				echo $this->domainInfo['patch_version'].'#'.$this->domainInfo['backup_patch_url'].'#'.$this->domainInfo['patch_urls'];

			} else {
				global $JoinSessionResultCode, $JoinSessionResultMsg;
				echo errorMsgBlock(BASE_TRANSLATED_RSM_ERROR_NUM + $JoinSessionResultCode, $JoinSessionResultCode, $JoinSessionResultMsg, $userId);
			}
		} else {
			// empty cookie, this mean the user id can't be validated by the LS
			echo errorMsgBlock(BASE_TRANSLATED_LS_ERROR_NUM + $resultCode, $resultCode, $errorString, $userId);
		}
	}
}
