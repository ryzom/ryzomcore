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
 
require_once(RYAPI_PATH.'data/ryzom/sbrick_db.php');
require_once(RYAPI_PATH.'data/ryzom/items_db.php');

$ryzom_item_icon_path = RYAPI_PATH.'data/ryzom/interface/';

// return TRUE if $bg was modified, FALSE if not
// ryzom_item_icon_colorize if needed, apply mask if needed and mask color is set
function ryzom_item_icon_load_image($bg, $fileName, $color=-1, $mColor=-1){
	if(!file_exists($fileName) || is_dir($fileName)){
		// requested fileName not found or it points to directory
		return false;
	}
	$im = @imagecreatefrompng($fileName);
	if($im===false){
		return false;
	}
	imagesavealpha($im, true);

	$im_w=imagesx($im);
	$im_h=imagesy($im);

	$bg_w=imagesx($bg);
	$bg_h=imagesy($bg);

	if($color != -1) {
		$im=ryzom_item_icon_colorize($im, ($color >> 24) & 255, $color&255, ($color>>8)&255, ($color >> 16)&255);
	}elseif(/*$rzColor*/1 != -1){
		$mask_file=str_replace('.png', '_mask.png', $fileName);
		if(file_exists($mask_file)){
			$mask = imagecreatefrompng($mask_file);
			if($mask!==false){
				imagesavealpha($mask, true);
				$mask=ryzom_item_icon_colorize($mask, ($mColor >> 24) & 255, $mColor&255, ($mColor>>8)&255, ($mColor >> 16)&255);
				imagecopy($im, $mask, 0, 0, 0, 0, $im_w, $im_h);
				imagedestroy($mask);
			}
		}
	}
	if($bg_w<$im_w){
		// resize source image to background image (make it smaller)
		imagecopyresized($bg, $im, 0, 0, 0, 0, $bg_w, $bg_h, $im_w, $im_h);
	}else{
		// center to background, but do not resize
		$pad_x=($bg_w-$im_w)/2;
		$pad_y=($bg_h-$im_h)/2;
		imagecopy($bg, $im, $pad_x, $pad_y, 0, 0, $im_w, $im_h);
	}
	imagedestroy($im);

	return true;
}

function ryzom_item_icon_colorize($im, $ca, $cr, $cg, $cb){
	$w=imagesx($im);
	$h=imagesy($im);

	$out = imagecreatetruecolor($w, $h);

	imagealphablending($out, false);

	$black=imagecolorallocate($out, 0, 0, 0);
	imagefill($out, 0, 0, $black);

	for($y=0;$y<$h;$y++){
		for($x=0;$x<$w;$x++){
			$rgba= imagecolorat($im, $x, $y);
			$sa = ($rgba>>24)&255;
			$sr = ($rgba>>16)&255;
			$sg = ($rgba>>8)&255;
			$sb = ($rgba>>0)&255;

			$r=$cr*$sr/255;
			$g=$cg*$sg/255;
			$b=$cb*$sb/255;
			$c=imagecolorallocatealpha($out, $r, $g, $b, $sa);
			imagesetpixel($out, $x, $y, $c);
		}
	}

	return $out;
}

function ryzom_item_icon_image_typo_width($txt){
	global $ryzom_item_icon_path;
	$x=0;
	for($i=0;$i<strlen($txt);$i++){
		if($txt{$i}=='?') $t='question'; else $t=strtolower(substr($txt, $i, 1));
		
		$typo_file=$ryzom_item_icon_path.'typo/typo_'.$t.'.png';

		if(file_exists($typo_file)){
			$wh=getimagesize($typo_file);
			if($wh!==false){
				$x+=$wh[0];
			}
		}
	}//for
	return $x;
}

function ryzom_item_icon_image_typo($bg, $txt, $x, $y, $use_numbers=true) {
	global $ryzom_item_icon_path;

	for($i=0;$i<strlen($txt);$i++) {
		if($txt{$i}=='?') $t='question'; else $t=strtolower(substr($txt, $i, 1));
		// use_numbers is used for sap/stack/quality display 
		// sbrick uses numbers from type_X.png files
		if($use_numbers && is_numeric($t)){
			$typo_file=$ryzom_item_icon_path.'typo/numbers_'.$t.'.png';
		}else{
			$typo_file=$ryzom_item_icon_path.'typo/typo_'.$t.'.png';
		}
		if(file_exists($typo_file)){
			$im=imagecreatefrompng($typo_file);
			$w=imagesx($im);
			$h=imagesy($im);
			imagecopy($bg, $im, $x, $y+(8-$h)/2, 0, 0, $w, $h); // center 'char' to 5x8
			$x += $w;
		}
	}
}

function split_fname($fname){
	// get image names and possible colors
	if(strpos($fname, '|')===false){
		return array($fname, -1);
	}else{
		return explode('|', $fname);
	}
}

function ryzom_item_icon_generate($filename, $sheetid, $c, $q, $s, $sap, $destroyed, $label) {
	global $ryzom_item_icon_path, $itemsList, $sbrickList;

	$item = array();
	
	if(preg_match('/\.sbrick$/', $sheetid)){
		// variables with different meaning
		// $q - TRUE - show sbrick level
		// $s - custom text on bottom-left corner
		$item = $sbrickList[$sheetid];
		$w=24; $h=24;
	}else{
		if (array_key_exists($sheetid, $itemsList))
			$item = $itemsList[$sheetid];
		else
			$item = $itemsList['test_scroll.sitem'];
			
		$w=40; $h=40;
	}

	$color_array = array(
		0 => array(233,22,0,255),
		1 => array(220,140,50,255),
		2 => array(170,250,0,255),
		3 => array(0,215,120,255),
		4 => array(50,100,255,255),
		5 => array(170,55,110,255),
		6 => array(250,250,250,255),
		7 => array(80,80,120,255),
	);
	$mask_color = -1;
	if(isset($color_array[$c])) {
		$mask_color = $color_array[$c][0] + ($color_array[$c][1] << 8) + ($color_array[$c][2] << 16);
	}

	list($bgFile, $bgColor) = split_fname($item['bg']);
	list($fgFile, $fgColor) = split_fname($item['fg']);
	list($i1File, $i1Color) = split_fname($item['i1']);
	list($i2File, $i2Color) = split_fname($item['i2']);

	// background image
	$bg=imagecreatetruecolor($w, $h);
	$transp=imagecolorallocatealpha($bg, 0, 0, 0, 127);
	imagefill($bg, 0, 0, $transp);
	imagesavealpha($bg, true);

	// stack item images on-top of background
	if(ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.$bgFile, $bgColor, $mask_color)){
		// draw background twice making semi-transparent background darker,
		// but still allowing icons with full transparent backgrounds like ixpca01.sitem (XP crystals)
		ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.$bgFile, $bgColor, $mask_color);
	}
	ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.$fgFile, $fgColor, $mask_color);
	ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.$i1File, $i1Color, $mask_color);
	ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.$i2File, $i2Color);

	if($w==24){// sbrick 24x24
		if($q===true){ // sbrick level, bottom-right corner
			$txt_w=ryzom_item_icon_image_typo_width($item['lvl']);
			ryzom_item_icon_image_typo($bg, $item['lvl'], $w-$txt_w-1, $h-8, false);
		}
		if(!empty($s)){
			ryzom_item_icon_image_typo($bg, $s, 1, $h-7, false); // 1px lower than lvl number
		}
	}else{ // item, 40x40
		// put item name, quality, stack size and sap loar to final image
		if($label && $item['txt']!=''){ // top right
			ryzom_item_icon_image_typo($bg, $item['txt'], 1, 0, false);
		}
		if($q>0){ // to bottom-right corner
			ryzom_item_icon_image_typo($bg, $q, 40-strlen($q)*5-1, 32-1, true);// 40-8
		}
		if($s>0){ // bottom-left corner
			ryzom_item_icon_image_typo($bg, 'x'.$s, 1, 32-1, true);// 40-8
		}
		if($sap>=0){ // top-left corner
			$im=ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.'sapload.png');
			if($im!==false && $sap>=1){
				ryzom_item_icon_image_typo($bg, $sap-1, 1, 2, true);
			}
		}
		if($destroyed){
			ryzom_item_icon_load_image($bg, $ryzom_item_icon_path.'ico_task_failed.png');
		}
	}

	imagepng($bg, $filename);
	return $filename;
}

function ryzom_item_icon_path($sheetid, $c, $q, $s, $sap, $destroyed, $label) {
	return RYAPI_PATH.'data/cache/item_icons/'.md5($sheetid.$c.$q.$s.$sap.$destroyed.$label).'.png';
}

function ryzom_item_icon($sheetid, $c, $q, $s, $sap, $destroyed, $label) {
	global $use_cache;

	$filename = ryzom_item_icon_path($sheetid, $c, $q, $s, $sap, $destroyed, $label);

	// always use cache
	if(/*!$use_cache ||*/ !file_exists($filename)) {
		ryzom_item_icon_generate($filename, $sheetid, $c, $q, $s, $sap, $destroyed, $label);
	}
	return file_get_contents($filename);
}

function ryzom_item_icon_url($sheetid, $c=1, $q=0, $s=0, $sap=-1, $destroyed=false, $label='') {
	$filename = ryzom_item_icon_path($sheetid, $c, $q, $s, $sap, $destroyed, $label);
	if (!file_exists($filename))
		ryzom_item_icon_generate($filename, $sheetid, $c, $q, $s, $sap, $destroyed, $label);
	return RYAPI_URL.'data/cache/item_icons/'.md5($sheetid.$c.$q.$s.$sap.$destroyed.$label).'.png';
}



?>