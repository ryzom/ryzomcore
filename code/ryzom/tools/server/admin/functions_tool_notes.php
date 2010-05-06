<?php

	function js_html_entity_decode($string)
	{
	   // replace numeric entities
	   $string = preg_replace('~&#x([0-9a-f]+);~ei', 'chr(hexdec("\\1"))', $string);
	   $string = preg_replace('~&#([0-9]+);~e', 'chr(\\1)', $string);
	   // replace literal entities
	   $trans_tbl = get_html_translation_table(HTML_ENTITIES);
	   $trans_tbl = array_flip($trans_tbl);
	   return strtr($string, $trans_tbl);
	}

	function tool_notes_get_list($user_id, $active=null)
	{
		global $db;

		$data = array();

		if ($active === null)	// edit note list
			$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_user_id=". $user_id ." ORDER BY note_active DESC, note_date DESC";
		else					// view note list
			$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE (note_user_id=". $user_id ." OR note_global=1) AND note_active='". $active ."' ORDER BY note_global DESC, note_title ASC";

		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				while ($row = $db->sql_fetchrow($result))
				{
					if ($active)
					{
						$row['note_data'] = addslashes(htmlentities(html_entity_decode(str_replace("\r\n","<br>",$row['note_data']), ENT_QUOTES), ENT_COMPAT));
						$row['note_title2'] = addslashes(htmlentities(html_entity_decode($row['note_title'], ENT_QUOTES), ENT_COMPAT));
					}
					$data[] = $row;
				}

			}
		}

		return $data;
	}

	function tool_notes_add($user_id, $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction)
	{
		global $db;

		$note_title	= trim(stripslashes($note_title));
		$note_data	= trim(stripslashes($note_data));

		if ($note_title == '')	return "/!\ Error: note title is empty!";
		//if ($note_data == '')	return "/!\ Error: note data is empty!";

		if ($note_mode == 'text')	$note_mode = 0;
		else						$note_mode = 1;

		$sql  = "INSERT INTO ". NELDB_NOTE_TABLE ." (`note_user_id`,`note_title`,`note_data`,`note_date`,`note_active`,`note_global`,`note_mode`,`note_popup_uri`,`note_popup_restriction`) VALUES ";
		$sql .= " ('". $user_id ."','". htmlentities($note_title, ENT_QUOTES) ."','". htmlentities($note_data, ENT_QUOTES) ."','". time() ."',". $note_active .",". $note_global .",". $note_mode .",'". $note_uri ."','". $note_restriction ."')";

		$db->sql_query($sql);

		return "";
	}

	function tool_notes_get_id($user_id, $note_id)
	{
		global $db;

		$data = array();

		$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". $note_id ." AND note_user_id=". $user_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
				$data['note_title'] = $data['note_title'];
				$data['note_data'] = $data['note_data'];
			}
		}

		return $data;
	}

	function tool_notes_del($user_id, $note_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". $note_id ." AND note_user_id=". $user_id;
		$db->sql_query($sql);
	}

	function tool_notes_update($user_id, $note_id, $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction)
	{
		global $db;

		if ($note_mode == 'text')	$note_mode = 0;
		else						$note_mode = 1;

		$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". $note_id ." AND note_user_id=". $user_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
//				$sql = "UPDATE ". NELDB_NOTE_TABLE ." SET note_title='". htmlentities($note_title, ENT_QUOTES) ."',note_data='". htmlentities($note_data, ENT_QUOTES) ."',note_date='". time() ."',note_active='". $note_active ."',note_global='". $note_global ."',note_mode=". $note_mode .",note_popup_uri='". $note_uri ."',note_popup_restriction='". $note_restriction ."'  WHERE note_id=". $note_id;
				$sql = "UPDATE ". NELDB_NOTE_TABLE ." SET note_title='". htmlentities($note_title, ENT_QUOTES) ."',note_data='". htmlentities($note_data, ENT_QUOTES) ."',note_date='". time() ."',note_active='". $note_active ."',note_global='". $note_global ."'  WHERE note_id=". $note_id;
				$db->sql_query($sql);
			}
			else
			{
				return "/!\ Error: no such note for this user!";
			}
		}

		return "";
	}

?>