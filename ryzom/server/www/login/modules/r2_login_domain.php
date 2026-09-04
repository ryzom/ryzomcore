<?php

class ServerDomain {

	function __construct($db, $clientApplication) {
		$this->db = $db;

		if ($clientApplication == 'ryzom_beta') {
			$domainInfo = $this->db->querySingle('SELECT * FROM domain WHERE domain_name = "'.$this->db->e($clientApplication).'"');
			$this->isBeta = $domainInfo;
			$clientApplication = 'ryzom_live';
		} else
			$this->isBeta = Null;

		$this->db->select();
		$this->domainInfo = $this->db->querySingle('SELECT * FROM domain WHERE domain_name = "'.$this->db->e($clientApplication).'"');
		if (!$this->domainInfo)
			die('Can\'t find row for domain '.$clientApplication);
		$this->id = $this->domainInfo['domain_id'];
		$this->domainInfo['isBeta'] = $this->isBeta;
		return $this->domainInfo;
	}

	function get($info) {
		return $this->domainInfo[$info];
	}

	function checkStatus($status, $User) {
		$error = '';

		$lang = $_GET['lg'];
		if ($status == 'ds_close')
			$error = '0:Reboot sequence...';
		else if ($status == 'ds_dev' && strstr($User->priv, ':DEV:') == false) {
			$error = '0: Reboot sequence...'."\n";
			if ($lang == 'en')
				$error .= 'The server has been rebooted. The Ryzom Team are carefully inspecting the shard to get it ready for you.'."\n";
			if ($lang == 'fr')
				$error .= 'Le serveur a été redémarré. L\'équipe de Ryzom est en train de le vérifier méticuleusement et le prépare pour vous.'."\n";
			if ($lang == 'de')
				$error .= 'Der Server wurde neu gestartet. Das Ryzom-Team führt eine ausführliche Überprüfung des Servers durch, um ihn für Dich vorzubereiten!'."\n";
			if ($lang == 'es')
				$error .= 'El servidor va a reiniciar. El equipo de Ryzom está revisando cuidadosamente el servidor y preparándolo para ti!'."\n";
		} else if ($status == 'ds_restricted') {
			if (
				!in_array($User->uid, explode(' ', ACCESS_WHEN_LOCK_USERS)) &&
				strstr($User->priv, ':DEV:') == false
				&& 	strstr($User->priv, ':SGM:') == false
				&& 	strstr($User->priv, ':VG:') == false
				&& 	strstr($User->priv, ':GM:') == false
				&& 	strstr($User->priv, ':EM:') == false
				&& 	strstr($User->priv, ':EG:') == false
				&& 	strstr($User->priv, ':TEST:') == false
				) {
				$error = '0: Reboot sequence...'."\n";
			if ($lang == 'en')
				$error .= 'The server has been rebooted. The Ryzom Team are carefully inspecting the shard to get it ready for you.'."\n";
			if ($lang == 'fr')
				$error .= 'Le serveur a été redémarré. L\'équipe de Ryzom est en train de le vérifier méticuleusement et le prépare pour vous.'."\n";
			if ($lang == 'de')
				$error .= 'Der Server wurde neu gestartet. Das Ryzom-Team führt eine ausführliche Überprüfung des Servers durch, um ihn für Dich vorzubereiten!'."\n";
			if ($lang == 'es')
				$error .= 'El servidor va a reiniciar. El equipo de Ryzom está revisando cuidadosamente el servidor y preparándolo para ti!'."\n";
			}
		}

		if ($error) {
			die($error);
		}
	}

}
