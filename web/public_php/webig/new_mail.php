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

	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that write the mail
	// $mail_to			user to send mail to (optional)
	// $mail_subject	subject of the mail (optional)
	// $mail_content	content of the mail (optional)
	//

	importParam('mail_to');
	importParam('mail_subject');
	importParam('mail_content');
	global $mail_to;
	global $mail_subject;
	global $mail_content;

	read_template('new_mail.html', $new_mail);

	// the sender and the recipient arrive with the request too, so they need
	// the same escaping the subject and the content already get; every field
	// but the login is optional (a fresh compose form sends none of them)
	$e_login = htmlspecialchars((string)$user_login, ENT_QUOTES);
	$e_to    = htmlspecialchars((string)$mail_to, ENT_QUOTES);

	$instance = str_replace(array('%%FROM%%', '%%UCFROM%%',      '%%TO%%', '%%UCTO%%',     '%%SUBJECT%%',                              '%%CONTENT%%'),
							array($e_login,   ucfirst($e_login), $e_to,    ucfirst($e_to), ucfirst(displayable_string($mail_subject)), displayable_content($mail_content)),
							$new_mail);

	echo $instance;
?>