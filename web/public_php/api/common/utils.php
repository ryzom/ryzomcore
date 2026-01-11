<?php

/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

include_once('logger.php');
include_once('dfm.php');

define('SERVER', 'http://'.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']);

function stripslashes_deep($value)
{
    $value = is_array($value) ?
                array_map('stripslashes_deep', $value) :
                stripslashes($value);

    return $value;
}

if (ini_get('magic_quotes_gpc') == 1) {
	$_POST = stripslashes_deep($_POST);
	$_GET = stripslashes_deep($_GET);
}

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
			if (count($x) > 1)
				$arr[$x[0]] = urldecode($x[1]);
			else
				$arr[$x[0]] = '';
		}
		unset($val, $x, $var);
	}
	return $arr;
}

function ryzom_get_params()
{
	if (!isset($GLOBALS['URL_PARAMS']))
		$GLOBALS['URL_PARAMS'] =  parse_query($_SERVER['REQUEST_URI']);
	return $GLOBALS['URL_PARAMS'];
}

function ryzom_unset_url_param($name) {
	if (!isset($GLOBALS['URL_PARAMS']))
		$GLOBALS['URL_PARAMS'] =  parse_query($_SERVER['REQUEST_URI']);
	unset($GLOBALS['URL_PARAMS'][$name]);
	return $GLOBALS['URL_PARAMS'];
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

if(!function_exists('_url')){
	function _url($base_params=null, $add_params=array()){
		if ($base_params !== null)
			return SERVER.'?'.http_build_query(array_merge($base_params, $add_params));
		else
			return SERVER;
	}
}


if(!function_exists('_h')){
	function _h($s){
		return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
	}
}

if(!function_exists('_i')){
	function _i($img, $alt=''){
		if (substr($img, strlen($img)-4) == '.tga') // img from client texture : ig only
			return $img;

		if (is_file(RYAPI_PATH.'/data/icons/'.$img.'.png'))
			$img = RYAPI_URL.'/data/icons/'.$img.'.png';
		else if (is_file(RYAPP_PATH.'/data/icons/'.$img.'.png'))
			$img = RYAPP_URL.'/data/icons/'.$img.'.png';
		else
			$img = 'view_remove';

		if ($alt)
			return '<img src="'.$img.'" title="'.$alt.'" alt="'.utf8_decode($alt).'" />';
		else
			return '<img src="'.$img.'" />';

	}
}

if(!function_exists('_l')){
	function _l($text, $base_params=array(), $add_params=array())
	{
		return '<a href="'.SERVER.'?'.http_build_query(array_merge($base_params, $add_params)).'">'.$text.'</a>';
	}
}

if(!function_exists('_b')){
	function _b($text, $base_params=array(), $add_params=array())
	{
		return '<a href="'.SERVER.'?'.http_build_query(array_merge($base_params, $add_params)).'" class="ryzom-ui-button">'.$text.'</a>';
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
		if(empty($ryzom_texts[$id][$lang])){
			if(isset($ryzom_texts[$id]['en'])) return @vsprintf($ryzom_texts[$id]['en'], $args);
			return '{'.$id.'['.$lang.']}';
		}
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
	if(empty($ryzom_texts[$id][$user['lang']])){
		if(!empty($ryzom_texts[$id]['en'])) return @vsprintf($ryzom_texts[$id]['en'], $args);
		if(!empty($ryzom_texts[$id]['fr'])) return '{'.$id.$a.'}';
		if(!empty($ryzom_texts[$id]['de'])) return '{'.$id.$a.'}';
		return '{'.$id.'['.$user['lang'].']'.$a.'}';
	}
	return @vsprintf($ryzom_texts[$id][$user['lang']], $args);
}

/***
 *
 * Ryzom time
 *
 * ***/

function ryzom_timer($timestamp) {
	$d = intval($timestamp / 86400);
	$timestamp = $timestamp % 86400;
	$h = intval($timestamp  / 3600);
	$timestamp = $timestamp % 3600;
	$m = intval($timestamp  / 60);
	$s = $timestamp % 60;
	if ($d>1)
		return sprintf('%d'._t('days').' %02d:%02d:%02d', $d, $h, $m, $s);
	else if ($d)
		return sprintf('%d'._t('day').' %02d:%02d:%02d', $d, $h, $m, $s);
	else
		return sprintf("%02d:%02d:%02d", $h, $m, $s);
}

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

	if(!empty($ryzom_periods[$user['lang']][$form][$j]))
		$final = $ryzom_periods[$user['lang']][$form][$j];
	else
		$final = $ryzom_periods['en'][$form][$j];
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
 * Ryzom utilities
 *
 *
 * ***/

function ryzom_generate_password($length=8, $level=2, $oneofeach=false) {
	$validchars[1] = "0123456789abcdfghjkmnpqrstvwxyz";
	$validchars[2] = "0123456789abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	$validchars[3] = "0123456789_!@#$%&*()-=+/abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_!@#$%&*()-=+/";

	$password  = "";
	$counter   = 0;

	while ($counter < $length) {
		$actChar = substr($validchars[$level], rand(0, strlen($validchars[$level])-1), 1);

		// if $oneofeach then All character must be different (slower)
		if (!$oneofeach || !strstr($password, $actChar)) {
			$password .= $actChar;
			$counter++;
		}
	}

	return $password;
}


function file_get_contents_cached($fn, $cache_time=300) {
	global $ryzom_bench_text;

	$lfn = 'tmp/'.strtr($fn, ':/.?&=', '____________');

    // get the new file from internet every $cache_time (default=5min)
	if (file_exists($lfn) && filesize($lfn) > 0 && time() < filemtime($lfn) + $cache_time) {
		$content = file_get_contents($lfn);
	} else {
		$content = file_get_contents($fn);
		if ($content != '') file_put_contents($lfn, $content);
	}
	return $content;
}

function ryzom_redirect($url, $group='', $extra_lua='') {
	global $user;
	$lua = $extra_lua."\n";
	if ($user['ig']) {
		if (!$group)
			$lua .= 'getUI(__CURRENT_WINDOW__):browse("'.str_replace('&', '&amp;', $url).'")';
		else {
			if (substr($group, 0, 3) == 'ui:')
				$lua .= 'getUI("'.$group.'"):browse("'.str_replace('&', '&amp;', $url).'")';
			else
				$lua .= 'getUI("ui:interface:'.$group.':content:html"):browse("'.str_replace('&', '&amp;', $url).'")';
		}
		echo '<lua>'.$lua.'</lua>';
		die();
	} else {
		header('Location: '.$url);
		die();
	}
}

/***
 *
 * Debug tools
 *
 * ***/

function alert($var, $value=NULL, $level=1) {
	p($var, $value, '#FF7777', $level);
}

define('pNULL', '§$£¤*µ%ù²&#!;,;:.?/?.<>');

function p($var, $value=pNULL, $color='#FFFF00', $level=0) {
	ob_start();
	debug_print_backtrace();
	$bt = ob_get_contents();
	ob_end_clean();
	$bt = explode("\n#",$bt);
	if (isset($bt[$level]))
		$bt1 = explode('[', $bt[$level]);
	else
		$bt1 = array('');

	if (isset($bt[$level+1]))
		$bt2 = explode('[', $bt[$level+1]);
	else
		$bt2 = array('');

	$c = '';
	if ($value !== pNULL) {
		$c .= '<font color="#FFFFFF">'.$var.' : </font>';
		$var = $value;
	}
	$c .= '<font color="#AAFFFF">'.substr(str_replace("\n", "", $bt2[count($bt2)-1]), 0, -1).' =&gt; '.substr(str_replace("\n", "", $bt1[count($bt1)-1]), 0, -1)."</font>   ";
	ryLogger::getInstance()->addPrint($c);
	ob_start();
	var_dump($var);
	ryLogger::getInstance()->addPrint(_h(ob_get_contents()), $color);
	ob_end_clean();
}


/***
 *
 * Lua tools
 *
 * ***/

 class ryLua {

	static private $lua = array();
	static private $luaend = array();
	static private $indent;
	static private $indentend;
	static private $linkTargetId = 0;

	static function add($code, $indent=NULL) {
		if ($indent !== NULL)
			self::$indent += $indent;
		$tabs = str_repeat("    ", self::$indent);
		$a = $tabs.str_replace("\n", "\n    ".$tabs, $code);
		self::$lua[] = $a;
	}

	static function addEnd($code, $indent=NULL) {
		if ($indent !== NULL)
			self::$indentend += $indent;
		$tabs = str_repeat("    ", self::$indentend);
		$a = $tabs.str_replace("\n", "\n    ".$tabs, $code);
		self::$luaend[] = $a;
	}


	static function get($ig) {
		ryLogger::getInstance()->addPrint(implode("\n", self::$lua), '#FF00FF');
		$ret =  ($ig)?"<lua>\n".implode("\n", self::$lua)."\n</lua>":'';
		self::$lua = array();
		return $ret;
	}

	static function getEnd($ig) {
		ryLogger::getInstance()->addPrint(implode("\n", self::$luaend), '#FF55FF');
		$ret =  ($ig)?"<lua>\n".implode("\n", self::$luaend)."\n</lua>":'';
		self::$luaend = array();
		return $ret;
	}

	static function text($text) {
		return str_replace('"', '\"', $text);
	}

	static function url($base_params=null, $add_params=array()) {
		return str_replace('&', '&amp;', _url($base_params, $add_params));
	}


	function openLink($text, $target='webig', $base_params=array(), $add_params=array(), $urllua='', $runlua='')
	{
		$url = self::url($base_params, $add_params);
		if ($target == "help_browser")
			$url .= "&amp;ignore=";
		$id = ryzom_generate_password(8).strval(time()).strval(self::$linkTargetId++);
		$lua = <<< END
function openLink{$id}()
	runAH(nil, "browse", "name=ui:interface:{$target}:content:html|url={$url}"{$urllua})
	{$runlua}
end
END;
		self::add($lua);
		if (RYZOM_IG)
			return '<a href="ah:lua&openLink'.$id.'()">'.$text.'</a>';
		return $text;
	}

	static function link($id, $luacode, $text) {
		$lua = <<<END
function runLua{$id}()
	{$luacode}
end
END;
		self::add($lua);
		if (RYZOM_IG)
			return '<a href="ah:lua&runLua'.$id.'()">'.$text.'</a>';
		return $text;
	}

}

?>
