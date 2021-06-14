<?php
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

	importParam('mail_from');
	importParam('mail_to');
	importParam('mail_subject');
	importParam('mail_content');

	// check mail is valid
	//if (!isset($mail_from) || $mail_from == "" || !isset($mail_to) || $mail_to == "" || !isset($mail_subject) || $mail_subject == "" || !isset($mail_content) || $mail_content == "" )
	//	die('Incomplete mail to send');

	// check recipient has an account

	if (trim($mail_content) != '')
	{
		if ($mail_to == '' || !is_dir($to_dir = get_user_dir($mail_to, $shard)))
		{
			$mail_subject = "<i>uiMFUndelivrableMail</i> '$mail_subject'";
			$mail_cleansubject = $mail_subject;
			$mail_content = "<i>uiMFUndelivrableMailTo</i> '$mail_to'.\n<i>uiMFUndelivrableMailCheck</i><br>\n<br>\n<i>uiMFMailContent</i><br>\n$mail_content";
			$mail_cleancontent = "<i>uiMFUndelivrableMailTo</i> '$mail_to'.\n<i>uiMFUndelivrableMailCheck</i>\n\n<i>uiMFMailContent</i>\n$mail_content";
			$mail_to = $mail_from;
			$mail_from = '<i>uiMFMailServer</i>';
		}
		else
		{
			$mail_cleansubject = $mail_subject;
			$mail_subject = displayable_string(clean_string($mail_subject));
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