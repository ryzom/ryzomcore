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

function ryzom_app_render($title, $content, $ig=false, $bgcolor='', $javascript=array(), $homeLink=false) {
	$c = '';
	// Render header
	$title_prefix = '';
	if (ON_IPHONE) {
		$title_prefix = 'Ryzom - ';
	}

	if (!$bgcolor)
		$bgcolor = '#000000'.($ig?'00':'');

	if (!$ig) {
		$c .= '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">'."\n";
		$c .= '<html>
		<head>'."\n";
		$c .= '		<title>'.$title_prefix.(translation_exists($title)?_t($title):$title).'</title>'."\n";
		$c .= '		<meta HTTP-EQUIV="content-type" CONTENT="text/html; charset=UTF-8" />'."\n";
		$c .= ryzom_render_header();
		$c .= ryzom_render_header_www();
		$events = ON_IPHONE ? 'onorientationchange="updateOrientation();" ' : '';
		$c .= '	</head>'."\n";
		$c .= '	<body '.$events.'bgcolor="'.$bgcolor.'">'."\n";
		// Javascript
		$js_code = '';
		foreach ($javascript as $js)
			$js_code .= '<script type="text/javascript" src="'.$js.'"></script>';
		$c .= $js_code;

		$c .= ryzom_render_www(ryzom_render_window($title, $content, $homeLink));
		$c .= '</body></html>';
	} else {
		$c .= '<html><body>';
		$c .= $content;
		$debug = ryLogger::getInstance()->getLogs();
		if ($debug)
			$c .= '<table width="100%"><tr bgcolor="#002200"><td>'.$debug.'</td></tr></table>';
		$c .= '</body></html>';
	}
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
	return '</div>
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

function ryzom_render_login_form($char, $aligned=true) {
	$c = '';
	if ($aligned) {
		$c .= '<form action="" method="post"><table>';
		$c .= '<tr><td>'._t('enter_char').'</td></tr>';
		$c .= '<tr><td><input style="width: 200px" type="text" name="char" value="'.$char.'"/></td></tr>';
		$c .= '<tr><td>'._t('enter_password').'</td></tr>';
		$c .= '<tr><td><input style="width: 200px" type="password" name="password" /></td></tr>';
		$c .= '<tr><td><input type="submit" value="'._t('submit').'" /></td></tr>';
	} else {
		$c .= '<form action="" method="post"><table width="100%">';
		$c .= '<tr><td align="center">'._t('login').'</td></tr>';
		$c .= '<tr><td align="center"><input style="width: 200px" type="text" name="char" value="'.$char.'"/></td></tr>';
		$c .= '<tr><td align="center">'._t('password').'</td></tr>';
		$c .= '<tr><td align="center"><input style="width: 200px" type="password" name="password" /></td></tr>';
		$c .= '<tr><td align="center"><input type="submit" value="'._t('submit').'" /></td></tr>';
	}
	$c .= '</table></form>';
	return $c;
}

?>
