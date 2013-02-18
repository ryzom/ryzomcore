<?php
function item_grade($item) {
	global $DBc;

	#echo $item;

	$res = $DBc->sendSQL("SELECT grade FROM ryzom_nimetu_item_data WHERE sheetid='".str_replace(".sitem","",$item)."'","ARRAY");

	#echo $res[0]['grade'];

	return $res[0]['grade'];
}
?>