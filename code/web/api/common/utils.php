<?php

include_once('logger.php');

// Always use this function to get param because in game, the param can be pass by _GET or by _POST
function ryzom_get_param($var, $default='')
{
	if (isset($_POST[$var]))
		return $_POST[$var];
	else
		if (isset($_GET[$var]))
		return $_GET[$var];
	else
		return $default;
}

function parse_query($var)
{
	$var = parse_url($var);
	$arr = array();
	if (isset($var['query'])) {
		$var = html_entity_decode($var['query']);
		$var = explode('&', $var);

		foreach($var as $val) {
			$x = explode('=', $val);
			$arr[$x[0]] = urldecode($x[1]);
		}
		unset($val, $x, $var);
	}
	return $arr;
}

if (!function_exists('http_build_query')) {
	function http_build_query($data, $prefix='', $sep='', $key='') {
		$ret = array();
		foreach ((array)$data as $k => $v) {
			if (is_int($k) && $prefix != null) {
				$k = urlencode($prefix . $k);
			}
			if ((!empty($key)) || ($key === 0))  $k = $key.'['.urlencode($k).']';
			if (is_array($v) || is_object($v)) {
				array_push($ret, http_build_query($v, '', $sep, $k));
			} else {
				array_push($ret, $k.'='.urlencode($v));
			}
		}
		if (empty($sep)) $sep = ini_get('arg_separator.output');
		return implode($sep, $ret);
	}
}


/***
 *
 *  Translation utilities
 *
 * ***/

function translation_exists($id) {
	global $user, $ryzom_texts;
	return isset($ryzom_texts[$id]) && isset($ryzom_texts[$id][$user['lang']]);
}

// Translate the $id in the selected language
function get_translation($id, $lang, $args=array()) {
        global $ryzom_texts, $user;
        if(!isset($ryzom_texts[$id])) return '{'.$id.'}';
        if(!isset($ryzom_texts[$id][$lang])) return '{'.$id.'['.$lang.']}';
        if($ryzom_texts[$id][$lang] == '' && isset($ryzom_texts[$id]['en'])) return @vsprintf($ryzom_texts[$id]['en'], $args);
        return @vsprintf($ryzom_texts[$id][$lang], $args);
}


// Translate the $id in the user language
function _t($id, $args=array()) {
	global $ryzom_texts, $user;

	$a = '';
	if ($args) {
		if (is_array($args)) {
			$a = array();
			foreach ($args as $arg)
				$a[] = strval($arg);
			$a = ' '.implode(', ', $a);
		} else
			$a = ' '.strval($args);
	}
	if(!isset($ryzom_texts[$id])) return '{'.$id.$a.'}';
	if(!isset($ryzom_texts[$id][$user['lang']])) return '{'.$id.'['.$user['lang'].']'.$a.'}';
	if($ryzom_texts[$id][$user['lang']] == '' && isset($ryzom_texts[$id]['en']) && $ryzom_texts[$id]['en'] != '') return @vsprintf($ryzom_texts[$id]['en'], $args);
	if($ryzom_texts[$id][$user['lang']] == '' && isset($ryzom_texts[$id]['fr']) && $ryzom_texts[$id]['fr'] != '') return '{'.$id.$a.'}';
	if($ryzom_texts[$id][$user['lang']] == '' && isset($ryzom_texts[$id]['de']) && $ryzom_texts[$id]['de'] != '') return '{'.$id.$a.'}';
	return @vsprintf($ryzom_texts[$id][$user['lang']], $args);
}

/***
 *
 * Ryzom time
 *
 * ***/



// Get a human and translated readable time, for example "3 days ago"
function ryzom_relative_time($timestamp) {
	global $ryzom_periods, $user;
	$difference = time() - $timestamp;
	$lengths = array("60","60","24","7","4.35","12","10");

	if ($difference >= 0) { // this was in the past
		$ending = _t('ago');
	} else { // this was in the future
		$difference = -$difference;
		$ending = _t('to_go');
	}
	for($j = 0,$m=count($lengths); $j<$m && $difference >= $lengths[$j]; $j++)
		$difference /= $lengths[$j];
	// round hours as '1.2 hours to go'
	$difference = round($difference, ($j == 2) ? 1 : 0);

	$form = ($difference == 1) ? 'singular' : 'plural';

	// Handle exceptions
	// French uses singular form if difference = 0
	if ($user['lang'] == 'fr' && ($difference == 0)) {
		$form = 'singular';
	}
	// Russian has a different plural form for plurals of 2 through 4
	if ($user['lang'] == 'ru' && ($form == 'plural')) {
		if ($difference < 5) {
			$form = '2-4';
		}
	}

	$final = $ryzom_periods[$user['lang']][$form][$j];
	$text = _t('date_format', array($difference, $final, $ending));
	return $text;
}

// Get a human and translated absolute date
function ryzom_absolute_time($timestamp) {
	global $user, $ryzom_daysofweek, $ryzom_monthnames;
	$day_of_month = date("j", $timestamp);
	$dow = date("w", $timestamp);
	$month = date("n", $timestamp);
	$day_of_week = $ryzom_daysofweek[$user['lang']][$dow];
	$month_str = $ryzom_monthnames[$user['lang']][$month-1];
	$text = _t("absolute_date_format", array($day_of_month, $day_of_week, $month_str, $month, date("m", $timestamp), date("d", $timestamp)));
	return $text;
}

/***
 *
 * Debug tools
 *
 * ***/

function p($var, $value=NULL) {
	ob_start();
	debug_print_backtrace();
	$bt = ob_get_contents();
	ob_end_clean();
	$bt = explode("\n",$bt);
	$bt = explode('[', $bt[1]);
	ob_start();
	echo '<font color="#AAFFFF">'.substr($bt[count($bt)-1], 0, -1)."</font>\n";
	if ($value !== NULL) {
		echo '<font color="#FFFFFF">'.$var.' : </font>';
		$var = $value;
	}
	//if (is_array($var))
	echo '<pre>';
	print_r($var);
	echo '</pre>';
//	else
	//	var_dump($var);
	ryLogger::getInstance()->addPrint(ob_get_contents());
	ob_end_clean();
}

?>
