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

function ryzom_app_render($title, $content, $style='', $files=array(), $homeLink=false) {
	$c = '';
	
	// get Lua code
	$c .= ryLua::get(RYZOM_IG);
	$at_end = ryLua::getEnd(RYZOM_IG);
	
	// Render header
	$title_prefix = '';
	if (ON_IPHONE) {
		$title_prefix = 'Ryzom - ';
	}

	if (!RYZOM_IG)
		$c .= '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">'."\n";
	$c .= '<html><head>'."\n";
	$c .= '		<title>'.$title_prefix.(translation_exists($title)?_t($title):$title).'</title>'."\n";
	if (!RYZOM_IG)
		$c .= '		<meta HTTP-EQUIV="content-type" CONTENT="text/html; charset=UTF-8" />'."\n";

	$events = '';
	if (!RYZOM_IG) {
		$c .= ryzom_render_header();
		$c .= ryzom_render_header_www();
		if (function_exists('newrelic_get_browser_timing_header'))
			$c .= newrelic_get_browser_timing_header();
		if(ON_IPHONE) $events = 'onorientationchange="updateOrientation();" ';
	} else {
		#if (!$style)
			$style='bgcolor="#00000000"';
	}

	if (!RYZOM_IG) {
		// Javascript
		$extra_code = '';
		if (is_string($files))
			$files = array($files);
		foreach ($files as $file) {
			$sfile = explode('.', $file);
			if ($sfile[count($sfile)-1] == 'js')
				$extra_code .= '		<script type="text/javascript" src="'.$file.'"></script>'."\n";
			else if ($sfile[count($sfile)-1] == 'css')
				$extra_code .= '		<link rel="stylesheet" type="text/css" media="all" href="'.$file.'" />'."\n";
		}
		$c .= $extra_code;
	}

	$c .= '	</head>'."\n";
	$c .= '	<body '.$events.' '.$style.'>'."\n";

	if (!RYZOM_IG) {
		$c .= ryzom_render_www(ryzom_render_window($title, $content, $homeLink));
		$c .= '</body>';
		if (function_exists('newrelic_get_browser_timing_header'))
			$c .= newrelic_get_browser_timing_footer();
	} else {
		$c .= $content.'<br /><table bgcolor="#000000"><tr><td>'.ryLogger::getInstance()->getLogs().'</td></tr></table></body>';
	}

	$c .= '</html>'.$at_end;

	return $c;
}

function ryzom_render_header() {
	if (ON_IPHONE) {
        return '';
	} else {
		return '		<link type="text/css" href="'.RYAPI_URL.'data/css/ryzom_ui.css" rel="stylesheet" media="all" />';
	}
}

// Call this inside the <head> part if you want to use ryzom_render_www
function ryzom_render_header_www() {
	if (ON_IPHONE) {
		return '
		<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.8;">
		<link rel="apple-touch-icon" href="data/img/ryzom_icon.png"/>
		<link type="text/css" href="data/css/ryzom_iphone.css" rel="stylesheet" media="all" />
		<script type="text/javascript">
			window.addEventListener("load", function() { setTimeout(loaded, 100) }, false);
			window.onload = function initialLoad() { updateOrientation(); }
			iPhone.DomLoad(updateOrientation);
			setInterval(updateOrientation, 500);

			function loaded() {
				document.getElementById("main").style.visibility = "visible";
				window.scrollTo(0, 1); // hide the location bar
			}
			function updateOrientation(){
				switch (window.orientation) {
					case 0:
					case 180:
						contentType = "show_normal";
					break;
					case -90:
					case 90:
					default:
						contentType = "show_wide";
						break;
				}
				document.getElementById("main").setAttribute("class", contentType);
			}
			function sizeTextarea(t) {
				a = t.value.split(\'\n\');
				b=1;
				for (x=0;x < a.length; x++) {
				 if (a[x].length >= t.cols) b+= Math.floor(a[x].length/t.cols);
				 }
				b+= a.length;
				if (b > t.rows) t.rows = b;
			}
		</script>
			';
	} else {
    	return '
		<style type="text/css">
			body{background-image:url('.RYAPI_URL.'data/img/bg.jpg);background-repeat:no-repeat;color:white;background-color:black;font-family:arial;font-size:12px}
			#main{width:95%;height:300px;margin-left:auto;margin-right:auto;text-align:left}
			a, a:visited{color:orange;font-size:12px}
			td{font-size:12px}
			a:hover{color:orange}
			.error{padding:.5em;background:#ff5555;color:white;font-weight:bold}
			img{border:0px}
			textarea{background-color:black;color:white;font-family:arial;font-size:12px}
			pre{overflow:auto;width:800px}
		</style>
';
    }
}

// Render a Ryzom style window
function ryzom_render_window($title, $content, $homeLink=false) {
	return ryzom_render_window_begin($title, $homeLink) . $content . ryzom_render_window_end();
}

function ryzom_render_window_begin($title, $homeLink=false) {

	if ($homeLink === false)
		$homeLink = '<span style="float:right;margin-right:12px;"><a href="'.RYAPP_URL.'/index.php" class="ryzom-ui-text-button">'._t('home').'</a></span>';

	return '
		<div class="ryzom-ui ryzom-ui-header">
			<div class="ryzom-ui-tl"><div class="ryzom-ui-tr">
				<div class="ryzom-ui-t">'.(translation_exists($title)?_t($title):$title).$homeLink.'</div>
			</div>
		</div>
		<div class="ryzom-ui-l"><div class="ryzom-ui-r"><div class="ryzom-ui-m">
			<div class="ryzom-ui-body">
';
}

function ryzom_render_window_end() {
	global $user;
	return '</div>
		<div>P_'.(isset($user['id'])?$user['id']:'GUEST').':'.(isset( $user['groups'])?implode(':', $user['groups']):'').'</div>
		<div style="background-color: #000000">'.ryLogger::getInstance()->getLogs().'</div></div></div></div>
		<div class="ryzom-ui-bl"><div class="ryzom-ui-br"><div class="ryzom-ui-b"></div></div></div><p class="ryzom-ui-notice">powered by <a class="ryzom-ui-notice" href="http://dev.ryzom.com/projects/ryzom-api/wiki">ryzom-api</a></p>
		</div>
	';
}

// Render a webpage using www.ryzom.com style
function ryzom_render_www($content) {
	return ryzom_render_www_begin() . $content . ryzom_render_www_end();
}

function ryzom_render_www_begin($url='') {
	$style1 = 'position: relative; padding-top: 20px; padding-right: 30px; margin-bottom: -3px';
	$style2 = 'position: absolute; bottom: 0; right: 0; ';
	if (ON_IPHONE) {
		$style1 = 'position: relative; padding-top: 30px; padding-right: 30px; ';
		$style2 = 'position: fixed; top: 0; right: 0; padding-right: 0px;';
		$marginBottom = '';
	}
	if (!$url) {
		$url_params = parse_query($_SERVER['REQUEST_URI']);
		unset($url_params['lang']);

		$url = 'http://'.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'].'?'.http_build_query($url_params);
	}
	return '
		<br />
		<div id="main">
				<div style="'.$style1.'">
					<a href="'.$url.'&lang=en"><img hspace="5" border="0" src="'.RYAPI_URL.'data/img/lang/en.png" alt="English" /></a>
					<a href="'.$url.'&lang=fr"><img hspace="5" border="0" src="'.RYAPI_URL.'data/img/lang/fr.png" alt="French" /></a>
					<a href="'.$url.'&lang=de"><img hspace="5" border="0" src="'.RYAPI_URL.'data/img/lang/de.png" alt="German" /></a>
					<a href="'.$url.'&lang=es"><img hspace="5" border="0" src="'.RYAPI_URL.'data/img/lang/es.png" alt="Spanish" /></a>
					<a href="'.$url.'&lang=ru"><img hspace="5" border="0" src="'.RYAPI_URL.'data/img/lang/ru.png" alt="Russian" /></a>
					<div style="'.$style2.'">
						<a href="http://www.ryzom.com/"><img border="0" src="'.RYAPI_URL.'data/img/logo.gif" alt=""/></a>
					</div>
				</div>
';
}

function ryzom_render_www_end() {
	return '</div>';
}


function _s($tag, $text) {
	global $ryzom_render_styles, $ryzom_render_tmpls;
	if (!array_key_exists($tag,  $ryzom_render_tmpls))
		return $text;
	if (is_array($text))
		return $p = $text;
	else
		$p[0] = $text;
	$p['color1'] = $ryzom_render_styles[$tag][0];
	$p['color2'] = $ryzom_render_styles[$tag][1];
	$code = '$c = "'.str_replace('"', '\"', $ryzom_render_tmpls[$tag]).'";';
	eval($code);
	return $c;
}

function ryzom_get_color_style($tag, $color=0) {
	global $ryzom_render_styles;
	if (!array_key_exists($tag,  $ryzom_render_styles))
		return '000000';
	return $ryzom_render_styles[$tag][$color];
}

function ryzom_set_title($title) {
	$GLOBALS['ryzom_render_title'] = $title;
}

function ryzom_get_title() {
	return $GLOBALS['ryzom_render_title'];
}

function ryzom_font($text, $color="", $size="") {
	if (RYZOM_IG) {
		$color = $color?'color="'.$color.'"':'';
		$size = $size?'size="'.$size.'"':'';
	} else {
		$color = $color?'color:'.$color.';':'';
		$size = $size?'font-size:'.$size.'pt':'';
	}
	return (RYZOM_IG?"<font $color $size>":"<font style=\"$color $size\">").$text.'</font>';
}

function ryzom_render_login_form($char, $aligned=true, $action="") {
	$c = '';
	if ($aligned) {
		$c .= '<form action="'.$action.'" method="post"><table>';
		$c .= '<tr><td>'._t('enter_char').'</td></tr>';
		$c .= '<tr><td><input cols="80" style="width: 200px" type="text" name="char" value="'.$char.'"/></td></tr>';
		$c .= '<tr><td>'._t('enter_password').'</td></tr>';
		if (RYZOM_IG)
			$c .= '<tr><td><input cols="80" style="width: 200px" type="text" name="password" /></td></tr>';
		else
			$c .= '<tr><td><input style="width: 200px" type="password" name="password" /></td></tr>';
		$c .= '<tr><td><input type="submit" value="'._t('submit').'" /></td></tr>';
	} else {
		$c .= '<form action="'.$action.'" method="post"><table width="100%">';
		$c .= '<tr><td align="center">'._t('login').'</td></tr>';
		$c .= '<tr><td align="center"><input cols="80" style="width: 200px" type="text" name="char" value="'.$char.'"/></td></tr>';
		$c .= '<tr><td align="center">'._t('password').'</td></tr>';
		if (RYZOM_IG)
			$c .= '<tr><td align="center"><input cols="80" style="width: 200px" type="text" name="password" /></td></tr>';
		else
			$c .= '<tr><td align="center"><input style="width: 200px" type="password" name="password" /></td></tr>';
		$c .= '<tr><td align="center"><input type="submit" value="'._t('submit').'" /></td></tr>';
	}
	$c .= '</table></form>';
	return $c;
}

function ryzom_dialog_yes_no($desc, $action, $name) { // will append ryzom_dialog=yes|no to url

	return '<table width="100%"><tr bgcolor="#333333"><td height="35px" align="center" valign="middle">'.(RYZOM_IG?'<font color="#DDAA33" size="11">':'<font style="color:#DDAA33; font-size:11pt">').
	$desc.'<form action="'.$action.'" method="POST">
		<table width="100%">
			<tr><td align="center"><select name="'.$name.'"><option value="no">'._t('no').'</option><option value="yes">'._t('yes').'</option></select></td></tr>
			<tr><td align="center"><input type="submit" value="'._t('submit').'"/></td></tr>
		</table>
	</form></font></td></tr></table>';
}

$GLOBALS['ryzom_render_title'] = defined('APP_NAME')?APP_NAME:'Ryzom';

$ig =  (isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Ryzom')) || ryzom_get_param('ig'); // need be set using url param because auth is not done
$transparency = $ig?'':'';
$ryzom_render_styles = array();
$ryzom_render_tmpls = array();

$ryzom_render_styles['main title'] = array('#222222'.$transparency, '#FFFFFF');
$ryzom_render_tmpls['main title'] = '<table width="100%" cellpadding="0" cellspacing="0"><tr bgcolor="${p[\'color1\']}"><td height="42px" valign="middle"><font '.($ig?'color="${p[\'color2\']}" size="14"':'style="color:${p[\'color2\']};font-size:16pt; font-weight: bold"').'>&nbsp;${p[0]}</font></td></tr></table>'."\n";

$ryzom_render_styles['section'] = array('#555555'.$transparency, '#FFFFFF');
$ryzom_render_tmpls['section'] = '<table width="100%" cellpadding="0" cellspacing="0"><tr bgcolor="${p[\'color1\']}"><td height="40px" align="left" valign="middle"><font '.($ig?'color="${p[\'color2\']}" size="12"':'style="color:${p[\'color2\']}; font-size:10pt; font-weight: bold"').'>&nbsp;${p[0]}</font></td></tr></table>'."\n";

$ryzom_render_styles['color'] = array('', '');
$ryzom_render_tmpls['color'] = ($ig?'<font color="${p[0]}">':'<font style="color:${p[0]}">').'${p[0]}</font>';

$ryzom_render_styles['link'] = array('#111111', '');
$ryzom_render_tmpls['link'] = '<table width="100%" cellpadding="0" cellspacing="0"><tr bgcolor="${p[\'color1\']}" ><td height="24px" valign="middle">&nbsp;${p[0]}</td></tr></table>'."\n";

$ryzom_render_styles['button'] = array('#000000', '');
$ryzom_render_tmpls['button'] = '<table cellpadding="5" cellspacing="0"><tr bgcolor="${p[\'color1\']}" ><td height="20px" align="center" valign="middle">&nbsp;${p[0]}</td></tr></table>'."\n";

$ryzom_render_styles['links'] = array('#111111'.$transparency, '');
$ryzom_render_tmpls['links'] = '<table width="100%" cellpadding="0" cellspacing="0"><tr bgcolor="${p[\'color1\']}"><td height="28px" valign="middle">&nbsp;${p[0]}</td></tr></table>'."\n";

$ryzom_render_styles['back'] = array('#000000'.$transparency, '');
$ryzom_render_tmpls['back'] = '<table cellpadding="0" cellspacing="0" width="100%"><tr valign="middle" bgcolor="${p[\'color1\']}"><td align="left" height="25px" nowrap>&nbsp;<b><a href="${p[0]}">&laquo;'.'main'.'</a></b></td></tr><tr><td>&nbsp;</td></tr></table>';

$ryzom_render_styles['highlight'] = array('#55ff55'.$transparency, '');
$ryzom_render_tmpls['highlight'] = '<font color="${p[\'color1\']}">${p[0]}</font>';

$ryzom_render_styles['backlight'] = array('#272727'.$transparency, '');
$ryzom_render_tmpls['backlight'] = '<table width="100%"><tr bgcolor="${p[\'color1\']}"><td height="35px" valign="middle">${p[0]}</td></tr></table>'."\n";

$ryzom_render_styles['actionbar'] = array('#555555'.$transparency, '');
$ryzom_render_tmpls['actionbar'] = '<tr bgcolor="${p[\'color1\']}" valign="middle">${p[0]}</tr>'."\n";

$ryzom_render_styles['table'] = array('#050505'.$transparency, '#FFFFFF');
$ryzom_render_tmpls['table'] = '<table width="100%" cellpadding="0" cellspacing="0">${p[0]}</table>'."\n";

$ryzom_render_styles['t header'] = array('#111111'.$transparency, '#FFFFFF');
$ryzom_render_tmpls['t header'] = '<tr style="color: #FFFFFF" bgcolor="${p[\'color1\']}" valign="middle">${p[0]}</tr>'."\n";

$ryzom_render_styles['t row 0'] = array('#353535'.$transparency, '');
$ryzom_render_tmpls['t row 0'] = '<tr bgcolor="${p[\'color1\']}" valign="middle">${p[0]}</tr>'."\n";

$ryzom_render_styles['t row 1'] = array('#252525'.$transparency, '');
$ryzom_render_tmpls['t row 1'] = '<tr bgcolor="${p[\'color1\']}" valign="middle">${p[0]}</tr>'."\n";

$ryzom_render_styles['t element'] = array('#FFFFFF'.$transparency, '');
$ryzom_render_tmpls['t element'] = '<font color="${p[\'color1\']}">${p[0]}</font>';

$ryzom_render_styles['log'] = array('#001100'.$transparency, '');
$ryzom_render_tmpls['log'] = '<div style="background-color: ${p[\'color1\']}"><pre style="width: auto">${p[0]}</pre></div>'."\n";

$ryzom_render_styles['message'] = array('#445566'.$transparency, '#FFDDAA');
$ryzom_render_tmpls['message'] = '<table width="100%" cellspacing="0" cellpadding="0"><tr bgcolor="${p[\'color1\']}"><td height="5px"></td></tr><tr bgcolor="${p[\'color1\']}"><td align="center" valign="middle"><font '.($ig?'color="${p[\'color2\']}" size="16"':'style="color:${p[\'color2\']};font-size:12pt; font-weight: bold"').'>${p[0]}</font></td></tr><tr bgcolor="${p[\'color1\']}"><td height="5px"></td></tr></table>'."\n";

$ryzom_render_styles['message warning'] = array('#AA3300'.$transparency, '');
$ryzom_render_tmpls['message warning'] = '<table width="100%"><tr bgcolor="${p[\'color1\']}"><td align="center" valign="middle"><h3>&nbsp;${p[0]}</h3></td></tr></table>'."\n";

$ryzom_render_styles['message window'] = array('#5555ff'.$transparency, '#7799ff');
$ryzom_render_tmpls['message window'] = '<table width="100%" cellspacing="0" cellpadding="0"><tr><td bgcolor="${p[\'color2\']}" width="3px"></td><td height="3px" bgcolor="${p[\'color2\']}"></td><td bgcolor="${p[\'color2\']}"></td><td bgcolor="${p[\'color2\']}" width="3px"></td></tr>'.'<tr><td bgcolor="${p[\'color2\']}" width="3px"></td><td bgcolor="${p[\'color1\']}" align="center" valign="middle">${p[0]}</td><td bgcolor="${p[\'color1\']}" valign="top" align="right">${p[0]}</td><td width="3px" bgcolor="${p[\'color2\']}"></td>'.
					'<tr bgcolor="${p[\'color1\']}"><td bgcolor="${p[\'color2\']}" width="3px"></td><td height="3px" bgcolor="${p[\'color2\']}"></td><td bgcolor="${p[\'color2\']}" width="3px"></td><td bgcolor="${p[\'color2\']}"></td></tr></table>'."\n";

$ryzom_render_styles['message ask'] = array('#333333'.$transparency, '');
$ryzom_render_tmpls['message ask'] = '<table width="100%"><tr bgcolor="${p[\'color1\']}"><td valign="middle">'.($ig?'<font color="#DDAA33" size="11">':'<font style="color:#DDAA33; font-size:11pt">').'${p[0]}</font></td></tr></table>'."\n";

$ryzom_render_styles['message error'] = array('#AA2222'.$transparency, '');
$ryzom_render_tmpls['message error'] = '<table width="100%"><tr bgcolor="${p[\'color1\']}"><td height="30px" align="center" valign="middle"><h3>&nbsp;${p[0]}</h3></td></tr></table>'."\n";

$ryzom_render_styles['message debug'] = array('#FFAA22'.$transparency, '');
$ryzom_render_tmpls['message debug'] = '<table width="100%"><tr bgcolor="${p[\'color1\']}"><td height="30px" valign="middle"><font color="#000">${p[0]}</font></td></tr></table>'."\n";

$ryzom_render_styles['progress bar'] = array('#FF0000'.$transparency, '#000000');
$ryzom_render_tmpls['progress bar'] = '<table width="100%"><tr><td bgcolor="${p[\'color1\']}" height="30px" width="${p[0]}%" align="center" valign="middle">&nbsp;</td>${p[0]}<td bgcolor="${p[\'color2\']}" height="30px" valign="middle" align="center">${p[0]}&nbsp;</td></tr></table>'."\n";

?>
