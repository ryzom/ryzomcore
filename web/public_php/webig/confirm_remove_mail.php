<?php

// Ryzom Core - MMORPG Framework <http://ryzom.dev/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

	include_once('mail_utils.php');

	//
	// INPUT:
	//
	// $user_login			login of the user that consults the mailbox
	// $select_mail_%%		mails to be removed
	//

	read_template('confirm_delete_mail.html', $confirm_delete_mail);

	$mails = array();
	$selected_mails = '';
	foreach ($_POST as $var => $value)
	{
		if (matchParam($var, "select_mail_", $mail))
		{
			$mails[] = $mail;
			$selected_mails .= "<input type='hidden' name='select_mail_$mail' value='selected'>\n";
		}
	}

	//

	$instance = str_replace(array('%%MAIL%%', '%%SELECTED_MAILS%%'),
							array($mails[0],  $selected_mails),
							$confirm_delete_mail);

	echo $instance;
?>