<?php


///////////////////////////////////////////////////////
class CWwwLog {
	//function CWwwLog() {}

	/*
	 * Return the log directory. Create it if it does not exist, or return false if creation failed.
	 */
	function getSafeLogDir() {
		$logPath = SHARD_LOGS.'/'.date('Y').'/'.date('m');
		if (!is_dir($logPath))
		{
			$res = mkdir($logPath, 0700, true);
			return $res ? $logPath : false;
		}
		return $logPath;
	}

	function logStr($str) {
		$logPath = $this->getSafeLogDir();
		if ($logPath !== false) {
			$fp = fopen($logPath.'/r2_login_'.date('d').'.log', 'a');
			fwrite($fp, date('Y-m-d H:i:s').' ('.$_SERVER['REMOTE_ADDR'].':'.$_SERVER['REQUEST_URI']."): $str\n");
			fclose($fp);
		}
	}
}


///////////////////////////////////////////////////////
// see errorMsg
function errorMsgBlock($errNum=GENERIC_ERROR_NUM) // $mixedArgs
{
	$args = func_get_args();
	return '0:'.call_user_func_array('errorMsg', $args);
}


///////////////////////////////////////////////////////
// Callback called on end of output buffering
function ob_callback_r2login($buffer)
{
	// Log only in case of error or malformed result string
	$blockHd = substr($buffer, 0, 2);
	if ($blockHd != '1:')
	{
		$logFile = new CWwwLog();
		$logFile->logStr(str_replace("\n", '\n', $buffer));
	}
	return $buffer; // sent to output
}


///////////////////////////////////////////////////////
// Callback called on error
function err_callback($errno, $errmsg, $filename, $linenum, $vars)
{
	// don't log "PHP ERROR/2048 Only variables should be passed by reference"
	if($errno == 2048) return;

	$logFile = new CWwwLog();
	$logFile->logStr("PHP ERROR/$errno $errmsg ($filename:$linenum)");
	// Never die after an error
}

function dieError($errNum=GENERIC_ERROR_NUM) // $mixedArgs
{
	$args = func_get_args();
	die('0:'.call_user_func_array('errorMsg', $args));
}

///////////////////////////////////////////////////////
// Rocket
function rocket($important, $text, $channel='z-logins-ip') {
	$data = [
		'channel' => $channel,
		'username' => 'Splinter.Shell',
		'icon_emoji' => ':bow_and_arrow:',
		'text' => $text,
		'token' => NOTIFY_TOKEN
		];

	ob_start();
	$ch = curl_init(NOTIFY_URL);
	curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'POST');
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
	//curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
	$result = curl_exec($ch);
	curl_close($ch);
	ob_end_clean();
}

