<?php

require_once('common/config.php');
require_once('common/time.php');
require_once('common/xml_utils.php');

if (RYAPI_MODE == 'server') {
	require_once('server/config.php');
	require_once('server/time.php');
} else {
	require_once('client/time.php');
}

$format = 'raw';
if (isset($_GET['format']))	$format = trim($_GET['format']);

// Compute

$tick = ryzom_time_tick();
switch($format) {
	case 'raw':
		echo $tick;
		break;
	case 'txt':
		$rytime = ryzom_time_array($tick);
		echo ryzom_time_txt($rytime);
		break;
	case 'xml':
		$rytime = ryzom_time_array($tick);
		ryzom_display_xml_header();
		echo ryzom_time_xml($rytime)->asXML();
		break;
}

?>