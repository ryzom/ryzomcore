<?php


///////////////////////////////////////////////////////
class CWwwLog {
	//function CWwwLog() {}

	/*
	 * Return the log directory. Create it if it does not exist, or return false if creation failed.
	 */
	function getSafeLogDir() {
		$LogRelativePath = '/log';
		$pathInfo = pathinfo(__FILE__);
		$logPath = $pathInfo['dirname'].$LogRelativePath;
		if (!is_dir($logPath))
		{
			$res = mkdir($LogPath, 0700);
			return $res ? $logPath : false;
		}
		return $logPath;
	}

	function logStr($str) {
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
		$logFile->logStr(str_replace("\n",'\n',$buffer));
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
