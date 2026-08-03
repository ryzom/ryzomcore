<?php

	// js_html_entity_decode() used to live here. Nothing called it, and it ran
	// the matched text back through the interpreter with the preg_replace /e
	// modifier, which php removed in 7.0. html_entity_decode() does the job.

	function tool_notes_get_list($user_id, $active=null)
	{
		global $db;

		$data = array();

		if ($active === null)	// edit note list
			$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_user_id=". intval($user_id) ." ORDER BY note_active DESC, note_date DESC";
		else					// view note list
			$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE (note_user_id=". intval($user_id) ." OR note_global=1) AND note_active='". intval($active) ."' ORDER BY note_global DESC, note_title ASC";

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
					// tool_notes_add/update filter this on the way in, but rows
					// written before they did are still in the table, and the
					// template hands the value to window.open(): escaping it for
					// the javascript string does nothing about a javascript: url.
					if (isset($row['note_popup_uri']))
						$row['note_popup_uri'] = tool_notes_safe_popup_uri($row['note_popup_uri']);
					$data[] = $row;
				}

			}
		}

		return $data;
	}

	/*
	 * note_popup_uri is opened via openWindow() in a javascript string.
	 * Only relative paths and http(s) are useful; reject javascript: and
	 * data: schemes even though the template also escapes the value.
	 */
	function tool_notes_safe_popup_uri($uri)
	{
		$uri = trim((string)$uri);
		if ($uri === '')
			return '';
		if (strlen($uri) > 255)
			$uri = substr($uri, 0, 255);
		if (preg_match('#^(https?://|/)#i', $uri))
			return $uri;
		// relative path without a scheme
		if (!preg_match('#^[a-z][a-z0-9+.-]*:#i', $uri))
			return $uri;
		return '';
	}

	function tool_notes_add($user_id, $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction)
	{
		global $db;

		$note_title	= trim(stripslashes($note_title));
		$note_data	= trim(stripslashes($note_data));
		$note_uri	= tool_notes_safe_popup_uri($note_uri);

		if ($note_title == '')	return "/!\ Error: note title is empty!";
		//if ($note_data == '')	return "/!\ Error: note data is empty!";

		if ($note_mode == 'text')	$note_mode = 0;
		else						$note_mode = 1;

		$sql  = "INSERT INTO ". NELDB_NOTE_TABLE ." (`note_user_id`,`note_title`,`note_data`,`note_date`,`note_active`,`note_global`,`note_mode`,`note_popup_uri`,`note_popup_restriction`) VALUES ";
		// htmlentities() is what keeps the note readable when the templates
		// print it, but it is not an sql escape: it leaves a backslash alone,
		// and a title ending in one closes the literal and lets the note body
		// continue the statement. Escape after encoding, like the two fields
		// below already do.
		$sql .= " ('". intval($user_id) ."','". $db->sql_escape_string(htmlentities($note_title, ENT_QUOTES)) ."','". $db->sql_escape_string(htmlentities($note_data, ENT_QUOTES)) ."','". time() ."',". intval($note_active) .",". intval($note_global) .",". intval($note_mode) .",'". $db->sql_escape_string($note_uri) ."','". $db->sql_escape_string($note_restriction) ."')";

		$db->sql_query($sql);

		return "";
	}

	function tool_notes_get_id($user_id, $note_id)
	{
		global $db;

		$data = array();

		$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". intval($note_id) ." AND note_user_id=". intval($user_id);
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

		$sql = "DELETE FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". intval($note_id) ." AND note_user_id=". intval($user_id);
		$db->sql_query($sql);
	}

	function tool_notes_update($user_id, $note_id, $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction)
	{
		global $db;

		if ($note_mode == 'text')	$note_mode = 0;
		else						$note_mode = 1;

		$note_uri = tool_notes_safe_popup_uri($note_uri);

		$sql = "SELECT * FROM ". NELDB_NOTE_TABLE ." WHERE note_id=". intval($note_id) ." AND note_user_id=". intval($user_id);
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
//				$sql = "UPDATE ". NELDB_NOTE_TABLE ." SET note_title='". htmlentities($note_title, ENT_QUOTES) ."',note_data='". htmlentities($note_data, ENT_QUOTES) ."',note_date='". time() ."',note_active='". $note_active ."',note_global='". $note_global ."',note_mode=". $note_mode .",note_popup_uri='". $note_uri ."',note_popup_restriction='". $note_restriction ."'  WHERE note_id=". $note_id;
				// same as in tool_notes_add(): htmlentities() is for the page,
				// sql_escape_string() is what keeps the value inside the literal.
				// popup_uri was never written on update; store the same safe
				// shape as on create.
				$sql = "UPDATE ". NELDB_NOTE_TABLE ." SET note_title='". $db->sql_escape_string(htmlentities($note_title, ENT_QUOTES)) ."',note_data='". $db->sql_escape_string(htmlentities($note_data, ENT_QUOTES)) ."',note_date='". time() ."',note_active='". intval($note_active) ."',note_global='". intval($note_global) ."',note_popup_uri='". $db->sql_escape_string($note_uri) ."',note_popup_restriction='". $db->sql_escape_string($note_restriction) ."'  WHERE note_id=". intval($note_id);
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