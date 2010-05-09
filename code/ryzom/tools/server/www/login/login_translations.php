<?php

/*
 * Message tables.
 * Each msg can contain %0..%n arguments (see also errorMsg()).
 * %0 contains the error code passed to errorMsg(), even if the text is factorized in another message linked with 'lnk'.
 */

$ErrMsgs = array();

// Generic error message
define('GENERIC_ERROR_NUM', 1000);
$ErrMsgs[1000]['dbg'] = 'Generic login error';
$ErrMsgs[1000]['en'] = 'Error';
$ErrMsgs[1000]['fr'] = 'Erreur';
$ErrMsgs[1000]['de'] = 'Error';

// Common public error messages
$msgMisconfiguredClient['en'] = 'Your client might be misconfigured, please contact support';
$msgMisconfiguredClient['fr'] = 'Il se peut que votre client soit mal configuré, merci de contacter le service client';
$msgMisconfiguredClient['de'] = 'Dein Client ist möglicherweise falsch konfiguriert, bitte kontaktiere den Support';
$msgOutOfDateClient['en'] = 'This client software is out of date, please visit www.ryzom.com to download a new client';
$msgOutOfDateClient['fr'] = 'Ce logiciel client est trop ancien, merci de télécharger un nouveau client sur www.ryzom.com';
$msgOutOfDateClient['de'] = 'Die Software ist nicht mehr aktuell bitte geh auf www.ryzom.com und lade dir den neuen Client herunter';
$msgLSInMaintenance['en'] = 'The log-in server is in maintenance, please try later';
$msgLSInMaintenance['fr'] = 'Le serveur de connexion est en maintenance, merci de réessayer plus tard';
$msgLSInMaintenance['de'] = 'Der Login-Server ist wegen Wartungsarbeiten offline, bitte versuch es später nochmal';
$msgDBInMaintenance['en'] = 'The database server is in maintenance, please try later';
$msgDBInMaintenance['fr'] = 'Le serveur de base de données est en maintenance, merci de réessayer plus tard';
$msgDBInMaintenance['de'] = 'Der Datenbank-Server ist wegen Wartungsarbeiten offline, bitte versuch es später nochmal';
$msgGameServersClosed['en'] = 'Game servers are currently closed or restricted, please retry later';
$msgGameServersClosed['fr'] = 'Les serveurs de jeu sont actuellement fermés ou à accès restreint, merci de réessayer plus tard';
$msgGameServersClosed['de'] = 'Die Spiel-Server sind momentan geschlossen oder gesperrt, bitte versuch es später nochmal';

/*
 * Main account error messages:
 */
$ErrMsgs[2001]['dbg'] = '(in %2)';
$ErrMsgs[2001]['en'] = 'Invalid account: %1'; // ex 52 and 64
$ErrMsgs[2001]['fr'] = 'Compte erroné : %1';
$ErrMsgs[2001]['de'] = 'Ungültiger Account: %1';
$ErrMsgs[2001]['log'] = false;

$ErrMsgs[2002]['en'] = "Your account must be activated first. Please read the email sent to %1.";
$ErrMsgs[2002]['fr'] = "Votre compte doit d'abord être activé. Merci de lire l'e-mail envoyé à %1.";
$ErrMsgs[2002]['de'] = "Dein Account muss noch aktiviert werden. Bitte befolge die Anweisungen, die wir per Mail an %1 geschickt haben.";
$ErrMsgs[2002]['log'] = false;

$ErrMsgs[2003]['en'] = "Your account must be activated first. Please read the email that has been sent to you.";
$ErrMsgs[2003]['fr'] = "Votre compte doit d'abord être activé. Merci de lire l'e-mail que nous vous avons envoyé.";
$ErrMsgs[2003]['de'] = "Dein Account muss noch aktiviert werden. Bitte befolge die Anweisungen, die wir per Mail an Dich geschickt haben.";
$ErrMsgs[2003]['log'] = false;

$ErrMsgs[2004]['dbg'] = '(in %2)'; // user => ex 56; signup_data => ex 56B
$ErrMsgs[2004]['en'] = "Invalid password";
$ErrMsgs[2004]['fr'] = "Mot de passe erroné";
$ErrMsgs[2004]['de'] = "Falsches Passwort";
$ErrMsgs[2004]['log'] = false;

// Translated Login Service error messages
define('BASE_TRANSLATED_LS_ERROR_NUM', 2100);

$ErrMsgs[2101]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2101]['en'] = 'Invalid account'; // ex 1?: 'invalid user'
$ErrMsgs[2101]['fr'] = 'Compte erroné';
$ErrMsgs[2101]['de'] = 'Ungültiger Account';

$ErrMsgs[2102]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2102]['en'] = 'Your account is already in online state, please retry in a few seconds'; // ex 2: 'User already online, please relog'
$ErrMsgs[2102]['fr'] = "Votre compte est encore dans l'état en ligne, merci de réessayer dans quelques secondes";
$ErrMsgs[2102]['de'] = 'Dein Account ist schon als online gekennzeichnet, bitte versuche es in ein paar Sekunden nochmals';
$ErrMsgs[2102]['log'] = false;

$ErrMsgs[2103]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2103]['en'] = 'Dual logging with a Privileged account is not permitted; this action has been logged and Gameforge CS has been notified';
$ErrMsgs[2103]['fr'] = 'TODO';
$ErrMsgs[2103]['de'] = 'TODO';
$ErrMsgs[2103]['mail'] = array('duallog@ryzom.com', "Dual Logging with GM account detected",
	"UserId %3 attempted to log-in while related account (linked through GMId) was in online state (error code %0).\n".
	"Message from server: %2\n".
	"This could be a false positive if the user just disconnected without waiting for 30 s, and tried to log-in with their other account.");

$ErrMsgs[2104]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2104]['lnk'] = $ErrMsgs[2103];

$ErrMsgs[2105]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2105]['lnk'] = $ErrMsgs[2101];

$ErrMsgs[2106]['dbg'] = '(LS error %1: %2 for userId %3)';
$ErrMsgs[2106]['lnk'] = $msgDBInMaintenance;


/*
 * Technical error messages:
 * "visible" error messages (server down, etc.) are localized,
 * unlike errors that should never occur (ex: bad parameters from client)
 */
define('BASE_TECHNICAL_ERROR_NUM', 3000);

$ErrMsgs[3001]['dbg'] = 'Failed to find a ring domain record for domainId: %1';

$ErrMsgs[3002]['dbg'] = 'Missing cmd';

$ErrMsgs[3003]['dbg'] = 'No response from Shard Unifier';
$ErrMsgs[3003]['lnk'] = $msgLSInMaintenance;

$ErrMsgs[3004]['dbg'] = "Can't connect to the %1 db server host:%2 user:%3";
$ErrMsgs[3004]['lnk'] = $msgDBInMaintenance;

$ErrMsgs[3005]['dbg'] = "Can't access the %1 database db:%2 host:%3 user:%4 (check privileges)";
$ErrMsgs[3005]['lnk'] = $msgDBInMaintenance;

$ErrMsgs[3006]['dbg'] = "Can't execute query '%1' on the %2 database db:%3 host:%4 user:%5 error:%6";
$ErrMsgs[3006]['lnk'] = $msgDBInMaintenance;

$ErrMsgs[3007]['dbg'] = "Can't find domain: %1"; // ex 'x'
$ErrMsgs[3007]['lnk'] = $msgMisconfiguredClient;
$ErrMsgs[3007]['add'] = 'dbg';

$ErrMsgs[3008]['dbg'] = "Login '%1' was created because it was not found in database"; // ex 50

$ErrMsgs[3009]['dbg'] = "Can't fetch login '%1' after insertion"; // ex 51

$ErrMsgs[3010]['dbg'] = "No permission found, but I need to accept Unknown user, so permission created, please RELOG";

$ErrMsgs[3011]['dbg'] = "(client application: %1 domain: %2)"; // ex 53
$ErrMsgs[3011]['en'] = "Your account needs a proper subscription to connect";
$ErrMsgs[3011]['fr'] = "Votre compte doit avoir un abonnement actif pour se connecter";
$ErrMsgs[3011]['de'] = 'Du kannst dich nicht ohne abgeschlossenes Abonemment in deinen Account einloggen';
$ErrMsgs[3011]['add'] = 'dbg';
$ErrMsgs[3011]['log'] = false;

$ErrMsgs[3012]['dbg'] = "No access privilege found for %1, but I need to accept Unknown user, so permission created, RELOG";

$ErrMsgs[3013]['dbg'] = "(client application: %1 domain: %2 reqPriv: %3)";
$ErrMsgs[3013]['en'] = "You don't have sufficient privilege to connect now, please try later";
$ErrMsgs[3013]['fr'] = "Vous n'avez pas les privilèges nécessaires pour vous connecter maintenant, veuillez essayer plus tard";
$ErrMsgs[3013]['de'] = 'Du hast nicht die nötigen Rechte um dich jetzt zu verbinden, bitte versuch es später nochmal';
$ErrMsgs[3013]['log'] = false;

// Translated Ring Session Manager (joinSession) error messages
define('BASE_TRANSLATED_RSM_ERROR_NUM', 4000);

$ErrMsgs[4001]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4002]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4003]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4004]['dbg'] = 'joinSession error %1: %2 for userId %3';

$ErrMsgs[4005]['dbg'] = '(joinSession error %1: %2 for userId %3)';
$ErrMsgs[4005]['lnk'] = $msgGameServersClosed;

$ErrMsgs[4006]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4007]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4008]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4009]['dbg'] = 'joinSession error %1: %2 for userId %3';

$ErrMsgs[4010]['dbg'] = '(joinSession error %1: %2 for userId %3)';
$ErrMsgs[4010]['lnk'] = $msgGameServersClosed;

$ErrMsgs[4011]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4012]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4013]['dbg'] = 'joinSession error %1: %2 for userId %3';

$ErrMsgs[4014]['dbg'] = '(joinSession error %1: %2 for userId %3)';
$ErrMsgs[4014]['lnk'] = $msgGameServersClosed;

$ErrMsgs[4015]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4016]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4017]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4018]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4019]['dbg'] = 'joinSession error %1: %2 for userId %3';
$ErrMsgs[4020]['dbg'] = 'joinSession error %1: %2 for userId %3';


$MsgLanguages = array('en');
$DisplayDbg = false;

/**
 * Whenever a language information is known, use this method to refine the language used for error messages.
 * Accepted values:
 * 'en', 'fr', 'de', array of this values, or 'all' ('all' will display all versions)
 */
function setMsgLanguage($languages='en')
{
	global $MsgLanguages;
	if ($languages == 'all')
	{
		$MsgLanguages = $languages;
		return;
	}
	if (!is_array($languages))
		$languages = array($languages);
	foreach ($languages as $index => $lg)
	{
		switch ($lg)
		{
			case 'fr':
			case 'de':;
				break;
			default:
				$languages[$index] = 'en';
		}
	}
	$MsgLanguages = $languages;
}

/**
 * Find the specified error message, and return the first found with the following precedence:
 * 1. Current language(s) set by setMsgLangage()
 * 2. English message
 * 3. Debug message
 * 4. Generic error
 * Each language can be found through ['lnk'] if needed
 * Tags (%1..%n) in the message are replaced by mixed arguments specified after $errNum.
 * Ex: errorMsg(55, $domainName)
 * if 'dbg' is found in ['add'] or $DisplayDbg is true, the 'dbg' version is appended to the found version.
 */
function errorMsg($errNum=GENERIC_ERROR_NUM) // $mixedArgs
{
	// Find specified message using precedence rules
	global $MsgLanguages;
	$precedence = array(
		array($errNum, ($MsgLanguages == 'all') ? array('en', 'fr', 'de') : $MsgLanguages),
		array($errNum, array('en')),
		array($errNum, array('dbg')),
		array(GENERIC_ERROR_NUM, $MsgLanguages),
		array(GENERIC_ERROR_NUM, array('en')));
	global $ErrMsgs;
	$args = func_get_args();
	$msg = '';
	foreach ($precedence as $rule)
	{
		// Find message
		list($actualErrNum, $languages) = $rule;
		foreach ($languages as $lg)
		{
			if (isset($ErrMsgs[$actualErrNum][$lg]) && ($ErrMsgs[$actualErrNum][$lg] != 'TODO'))
				appendToMsg($msg, $errNum, $ErrMsgs[$actualErrNum][$lg], $args);
			else if (isset($ErrMsgs[$actualErrNum]['lnk']) && isset($ErrMsgs[$actualErrNum]['lnk'][$lg]) && ($ErrMsgs[$actualErrNum]['lnk'][$lg] != 'TODO'))
				appendToMsg($msg, $errNum, $ErrMsgs[$actualErrNum]['lnk'][$lg], $args);
		}
		
		// Try next rule only if not found
		if (!empty($msg))
			break;
	}

	// Add debug version if needed
	global $DisplayDbg;
	$msgHasDebug = in_array('dbg', $languages);
	$logExtMsg = '';
	if ((!$msgHasDebug) && isset($ErrMsgs[$actualErrNum]['dbg']))
	{
		if ($DisplayDbg || (isset($ErrMsgs[$actualErrNum]['add']) && ($ErrMsgs[$actualErrNum]['add'] == 'dbg')))
			$msg .= '['.$ErrMsgs[$actualErrNum]['dbg']."]\n"; // to result/screen message
		else
			$logExtMsg .= '['.$ErrMsgs[$actualErrNum]['dbg'].']'; // to log message
	}

	// Get mail data if specified
	$mailData = (isset($ErrMsgs[$actualErrNum]['mail']) ? $ErrMsgs[$actualErrNum]['mail'] :
		   (isset($ErrMsgs[$actualErrNum]['lnk']['mail']) ? $ErrMsgs[$actualErrNum]['lnk']['mail'] :
		   	array()));

	// Apply params if applicable
	$numArgs = func_num_args();
	if ($numArgs > 1)
	{
		for ($i=0; $i!=$numArgs; ++$i) // include $errNum (%0)
		{
			$msg = str_replace("%$i", $args[$i], $msg);
			$logExtMsg = str_replace("%$i", $args[$i], $logExtMsg);
			$mIdx = 0;
			foreach ($mailData as $field)
			{
				$mailData[$mIdx] = str_replace("%$i", $args[$i], $field);
				++$mIdx;	
			}
		}
	}
	
	// Log technical errors if possible
	$logMode = (isset($ErrMsgs[$actualErrNum]['log']) ? $ErrMsgs[$actualErrNum]['log'] :
			    (isset($ErrMsgs[$actualErrNum]['lnk']['log']) ? $ErrMsgs[$actualErrNum]['lnk']['log'] :
			     true));
	if ($logMode && class_exists('CWwwLog'))
	{
		$logFile = new CWwwLog();
		$logFile->logStr(/*$msg.*/$logExtMsg); // message is already logged by ob_callback_r2login() 
	}
	
	// Send email if specified
	if (!empty($mailData) && (count($mailData) >= 3))
	{
		include_once('email/htmlMimeMail.php');
		$mail = new htmlMimeMail();
		$mail->setFrom('noreply@ryzom.com');
		$mail->setSubject($mailData[1]);
		$mail->setText('Application: '.$_GET['clientApplication'].' - Login: '.$_GET['login']."\n". // display as much info as possible
			$mailData[2]);
		$result = $mail->send(array($mailData[0]));
	}
	
	return $msg;
}

// Helper for errorMsg()
function appendToMsg(&$msg, $errNum, $str, &$args)
{
	$msg .= "$str ($errNum)";
	if (($errNum == GENERIC_ERROR_NUM) && !empty($args))
	{
		foreach ($args as $arg)
		{
			$msg .= " [$arg]"; // display all passed args if returning the default generic error
		}
	}
	$msg .= "\n";
}

/*
if (isset($_GET['export']))
	exportErrMsgsToTSV();
// Utility
function exportErrMsgsToTSV()
{
	global $ErrMsgs;
	echo "Num\ten\tfr\tde\tdbg\t\n";
	foreach ($ErrMsgs as $num => $txtArray)
	{
		echo $num."\t";
		foreach (array('en','fr','de','dbg') as $lg)
		{
			if (isset($txtArray[$lg]))
				echo $txtArray[$lg];
			else if (isset($txtArray['lnk'][$lg]))
				echo $txtArray['lnk'][$lg];
			echo "\t";
		}
		echo "\n";
	}
}
*/

?>