<?php

class ServerDomain {

	function __construct($db, $clientApplication) {
		$this->db = $db;
		$this->db->select();
		$this->domainInfo = $this->db->querySingle('SELECT * FROM domain WHERE domain_name = "'._e($clientApplication).'"');
		if (!$this->domainInfo)
			die('Can\'t find row for domain '.$clientApplication);
		$this->id = $this->domainInfo['domain_id'];
		$this->shardInfo = $this->db->querySingle('SELECT * FROM shard WHERE domain_id = "'.$this->id.'"');
		if (!$this->shardInfo)
			die('Can\'t find row for shard '.$clientApplication);
		$this->domainInfo['shard'] = $this->shardInfo;
		return $this->domainInfo;
	}

	function get($info) {
		return $this->domainInfo[$info];
	}

	function getShard($info) {
		return $this->domainInfo['shard'][$info];
	}

	function checkStatus($status, $User) {
		$error = '';

		if ($status == 'ds_close')
			$error = '0:Reboot sequence...';
		else if ($status == 'ds_dev' && strstr($User->priv, ':DEV:') == false)
			$error = '0:You are not allowed to connect now, retry later';
		else if ($status == 'ds_restricted') {
			if (	strstr($User->priv, ':DEV:') == false
				&& 	strstr($User->priv, ':SGM:') == false
				&& 	strstr($User->priv, ':GM:') == false
				&& 	strstr($User->priv, ':EM:') == false
				&& 	strstr($User->priv, ':EG:') == false
				&& 	strstr($User->priv, ':TEST:') == false
				) {
				$error = '0: Reboot sequence...'."\n\n";
				$error .= 'The server has been rebooted. The Ryzom Team are carefully inspecting the shard to get it ready for you.'."\n\n";
				$error .= 'Le serveur a été redémarré. L\'équipe de Ryzom est en train de le vérifier méticuleusement et le prépare pour vous.'."\n\n";
				$error .= 'Der Server wurde neu gestartet. Das Ryzom-Team führt eine ausführliche Überprüfung des Servers durch, um ihn für Dich vorzubereiten!'."\n\n";
				$error .= 'El servidor va a reiniciar. El equipo de Ryzom está revisando suavemente el servidor y preparándolo para ti!'."\n\n";
			}
		}

		if ($error) {
			echo iconv('UTF-8', 'ISO-8859-1', $error);
			die();
		}
	}

}
