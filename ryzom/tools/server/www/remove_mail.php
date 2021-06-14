<?php
	include_once('mail_utils.php');

	//
	// INPUT:
	//
	// $user_login			login of the user that consults the mailbox
	// $select_mail_%%		mails to be removed
	//

	$mails = array();

	foreach ($_POST as $var => $value)
		if (matchParam($var, "select_mail_", $mail))
			$mails[] = $mail;

	if (count($mails) > 0)
		remove_mail($user_login, $mails);

	// redirect browser to new mailbox page
	//redirect("mailbox.php");
	include_once('mailbox.php');
?>