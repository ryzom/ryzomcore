<?php

/* Copyright (C) 2012 Winch Gate Property Limited
 * 
 * This file is part of ryzom_app.
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

/***
 * List of web egs commands 
 * 
 *  - give_item sheet quality quantity inventory
 *  - recv_item sheet quality quantity inventory
 *  - check_item sheet quality quantity inventory
 *  - check_position min_x min_y max_x max_y
 *  - check_fame faction_name "below"|"above" value
 *  - check_target "sheet"|"bot_name"|"player_name" name
 *  - check_brick brick_name
 *  - set_brick "add"|"del" brick_name
 *  - check_outpost name "attacker"|"defender"|"attack"|"defend" (attacker/defender : check if guild of player is attacker/defender, attack/defend : check if OP is in state atttack/defend)
 *  - create_group number sheet [ dispersion "self"|"random"|orientation bot_name x y ]
 *  - group_script bot_name command1 [ command2 ... ]
 *  - change_vpx player_name property value
 *  - set_title title
 * 
 ***/

include_once(RYAPI_PATH.'/common/dfm.php');
 
class ryHmagic {

	function __construct() {
		$this->dfm = new ryDataFileManager(_user()->id);
	}
	
	function getWebCommand($web_app_url, $command, $is_next=false, $need_callback=false, $old_sep=false, $replace_space=true) {
		$command = str_replace('#player', ryzom_get_param('player_eid'), $command);
		if ($replace_space)
			$command = str_replace(' ', chr(160), $command);
		
		$last_connected_date = strtotime($_SESSION['last_played_date']);
		$index_infos = $this->dfm->loadUserDataFromApp('hmagic.index', 'app_profile');
		if ($index_infos == null) {
			$index_infos['last_played_date'] = $last_connected_date;
			$index_infos['time'] = array();
			$index_infos['index'] = 0;
		}
		
		if ($index_infos['last_played_date'] != $last_connected_date) {
			$index_infos['last_played_date'] = $last_connected_date;
			$index_infos['time'] = array();
			$index_infos['index'] = 0;
		}
		if (!is_array($index_infos['time']))
			$index_infos['time'] = array();
		
		$index_infos['index']++;
		$index_infos['time'][strval($index_infos['index'])] = strval($index_infos['index']).'_'.strval(time());
		$index_infos['url'][strval($index_infos['index'])] = $web_app_url;
		$tid = 'tid='.$index_infos['time'][strval($index_infos['index'])];
		$this->dfm->saveUserDataFromApp('hmagic.index', 'app_profile', $index_infos);
		$eid = ryzom_get_param('datasetid');
		$checksum = $web_app_url.'&'.$tid . $last_connected_date . $index_infos['index'] . $command . $eid;
		$hmac =  strtoupper(hash_hmac('sha1', $checksum, RYAPI_EGS_SALT));
		return '	local command = \''.str_replace("'", '\\\'',str_replace('&', '&amp;', $command)).'\''."\n\t".
		(RYAPI_HMAGIC_DEBUG?'runCommand("a","webExecCommand","debug", "1", command, "hmac", "2","'.($is_next?'1':'0').'","'.($need_callback?'1':'0').'")':'runCommand("a","webExecCommand","'.str_replace('&', '&amp;', $web_app_url).'&amp;'.$tid.'","'.$index_infos['index'].'",command,"'.$hmac.'","'.($old_sep?'1':'3').'","'.($is_next?'1':'0').'","'.($need_callback?'1':'0').'")');
	}

	function validateCallback() {
		$index_infos = $this->dfm->loadUserDataFromApp('hmagic.index', 'app_profile');
		$playerEid = ryzom_get_param('player_eid');
		$event = ryzom_get_param('event');
		$desc_error = ryzom_get_param('desc');
		$desc = '';
		if ($desc_error)
			$desc = '&desc='.$desc_error;
		list($index, $tid) = explode('_', ryzom_get_param('tid'));
		$web_app_url = $index_infos['url'][$index];

		if ($index.'_'.$tid != $index_infos['time'][$index])
			return false;
		$trans_id = '';
		if ($tid)
			$trans_id = '&tid='.$index.'_'.$tid;
		$hmac = ryzom_get_param('hmac');		
		$checksum = $web_app_url.$trans_id.'&player_eid='.$playerEid.'&event='.$event.$desc;
		$real_hmac = strtoupper(hash_hmac('sha1', $checksum, RYAPI_EGS_SALT));
		if ($real_hmac != $hmac)
			return false;
		else
			return true;
	}	
}

?>
