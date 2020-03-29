<?php
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