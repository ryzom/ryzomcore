<?php
	function nf($n) {
		return number_format($n, 0, '.', ',');
	}

	function ach_translate_cc($cc = 'c_neutral') {
		global $_USER;

		$t = array();
		$t['c_matis'] = 'Matis';
		$t['c_tryker'] = 'Tryker';
		$t['c_fyros'] = 'Fyros';
		$t['c_zorai'] = 'Zorai';
		$t['c_kami'] = 'Kami';
		$t['c_karavan'] = 'Karavan';
		$t['c_neutral'] = get_translation('ach_c_neutral',$_USER->getLang());

		return $t[$cc];
	}

	function ach_render_forbidden($ig) {
		if($ig) {
			return "This app is NOT available INGAME!";
		}
		else {
			return "This app is NOT available from the WEB!";
		}
	}
?>