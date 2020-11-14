<?php
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

	read_template('new_mail.html', $new_mail);

	$instance = str_replace(array('%%FROM%%',  '%%UCFROM%%',         '%%TO%%', '%%UCTO%%',        '%%SUBJECT%%',                              '%%CONTENT%%'),
							array($user_login, ucfirst($user_login), $mail_to, ucfirst($mail_to), ucfirst(displayable_string($mail_subject)), displayable_content($mail_content)),
							$new_mail);

	echo $instance;
?>