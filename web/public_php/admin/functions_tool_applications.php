<?php

	function tool_applications_menu_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_APPLICATION_TABLE ." WHERE application_visible=1 AND application_uri<>'' ORDER BY application_order ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

?>