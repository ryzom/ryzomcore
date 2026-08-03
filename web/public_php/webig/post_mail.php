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
	// $user_login			login of the user that posts a mail
	// $shard				shard from which the client connects in
	// $mail_from			login of the user that posts a mail
	// $mail_to				login of the user that receives the mail
	// $mail_subject		subject of the mail
	// $mail_content		content of the mail
	//

	importParam('mail_to');
	importParam('mail_subject');
	importParam('mail_content');
	global $mail_from;
	global $mail_to;
	global $mail_subject;
	global $mail_content;

	// The sender is whoever the session cookie authenticated, not whatever
	// name came in with the request -- mail_from used to be read from the
	// query string, so a player could send in game mail as any character.
	$mail_from = $user_login;

	// check mail is valid
	//if (!isset($mail_from) || $mail_from == "" || !isset($mail_to) || $mail_to == "" || !isset($mail_subject) || $mail_subject == "" || !isset($mail_content) || $mail_content == "" )
	//	die('Incomplete mail to send');

	// check recipient has an account

	if (trim($mail_content) != '')
	{
		if ($mail_to == '' || !is_dir($to_dir = get_user_dir($mail_to, $shard)))
		{
			// the bounce message is built as html, so escape the parts that
			// came in with the request before they are pasted into it
			$safe_to = displayable_string($mail_to);
			$safe_subject = displayable_string($mail_subject);
			$safe_content = displayable_string($mail_content);
			$mail_subject = "<i>uiMFUndelivrableMail</i> '$safe_subject'";
			$mail_cleansubject = clean_string($mail_subject);
			$mail_content = "<i>uiMFUndelivrableMailTo</i> '$safe_to'.\n<i>uiMFUndelivrableMailCheck</i><br>\n<br>\n<i>uiMFMailContent</i><br>\n$safe_content";
			$mail_cleancontent = "<i>uiMFUndelivrableMailTo</i> '$safe_to'.\n<i>uiMFUndelivrableMailCheck</i>\n\n<i>uiMFMailContent</i>\n$safe_content";
			$mail_to = $mail_from;
			$mail_from = '<i>uiMFMailServer</i>';
		}
		else
		{
			// the index file is %%-separated: the subject has to go through
			// clean_string() like the thread index does, or a subject with %%
			// in it shifts the mailbox columns
			$mail_cleansubject = clean_string($mail_subject);
			$mail_subject = displayable_string($mail_cleansubject);
			$mail_cleancontent = displayable_content($mail_content);
			$mail_content = displayable_string($mail_content);
			$mail_from = displayable_string(clean_string($mail_from));
			$mail_to = displayable_string(clean_string($mail_to));
		}
	
		$to_dir = build_user_dir($mail_to, $shard);
		$to_index = $to_dir.'mail.index';

		//
		// send mail to recipient
		//
	
		// create new mail index
		add_mail_to_index($mail_from, $mail_to, $mail_cleansubject, $mail_index);
	
		// create mail file
		create_mail($mail_from, $mail_to, $mail_subject, $mail_content, $mail_cleancontent, $mail_index);
	
		// rebuild recipient mailbox page
		build_mail_page($mail_to);
	}


	// redirect browser to new forum page
	//redirect("mailbox.php");
	include_once('mailbox.php');

?>