<?php

include_once('utils.php');

// -------------------------------------
// add a new thread into user forum index
// -------------------------------------
function add_mail_to_index(&$from, &$to, &$subject, &$index)
{
	global $shard;
	$to_dir = get_user_dir($to, $shard);
	$to_index = $to_dir.'mail.index';

	use_index($to_index);

	$f = fopen($to_index, 'r+');
	read_next_index($f, $index);
	append_to_index($f, trim($from).'%%'.strtr(trim($subject), "\n", " ").' %%'.displayable_date().'%%1%%'.$index);
	update_next_index($f, $index+1);
	fclose($f);
}

// -------------------------------------
// remove mail
// -------------------------------------
function remove_mail($user, $mails)
{
	global $shard;
	$mail_dir = get_user_dir($user, $shard);

	read_index($mail_dir.'mail.index', $header, $array);

	sort($mails);
	$remove_mail = 0;

	for ($i=0; $i<count($array); ++$i)
		if ($remove_mail >= count($mails) || $array[$i][4] != $mails[$remove_mail])
			$newarray[] = $array[$i];
		else
			++$remove_mail;

	write_index($mail_dir.'mail.index', $header, $newarray);

	build_mail_page($user);
}

// -------------------------------------
// create new mail file
// -------------------------------------
function create_mail(&$from, &$to, &$subject, &$content, &$cleancontent, &$index)
{
	global $shard;
	$to_dir = get_user_dir($to, $shard);

	read_template('mail.html', $mail);
	
	$inst_mailbox = str_replace(array('%%FROM%%', '%%UCFROM%%',   '%%DATE%%',         '%%SUBJECT%%',     '%%CONTENT%%', '%%CLEANCONTENT%%', '%%MAIL%%'),
								array($from,      ucfirst($from), displayable_date(), ucfirst($subject), $content,      $cleancontent,      $index),
								$mail);

	// build mail message
	$f = fopen($to_dir."mail_$index.html", 'w');
	fwrite($f, $inst_mailbox);
	fclose($f);

	$f = fopen($to_dir.'new_mails', 'w');
	fwrite($f, '1');
	fclose($f);

	srand((float) microtime()*1000000);
	$fname = md5(rand());
	global $USERS_DIR;
	if (!@is_dir($USERS_DIR.'/incoming'))
		@mkdir($USERS_DIR.'/incoming');
	$f = @fopen($USERS_DIR.'/incoming/'.$fname, "w");
	if ($f)
	{
		fwrite($f, "shard=$shard to=$to from=$from\n$$$$");
		fclose($f);
	}
}


// -------------------------------------
// rebuild user mail box pages
// -------------------------------------
function build_mail_page($user)
{
	global $shard;

	$user_dir = get_user_dir($user, $shard);
	$user_index = $user_dir."mail.index";

	// open thread index
	read_index($user_index, $header, $mails);
	
	$num_mails = count($mails);
	$num_per_page = 10;
	$num_pages = (int)(($num_mails+$num_per_page-1) / $num_per_page);

	$mail = $num_mails-1;
	$page = 0;

	$altern_color = array("#333333", "#666666");
	$altern_index = 0;
	
	$links_str = '';

	read_template('mailbox_main.html', $mailbox_main);
	read_template('mailbox_mail.html', $mailbox_mail);

	read_template('browse_link.html', $browse_link);
	
	do
	{
		$num_in_page = 0;
		$inst_mail = '';

		while ($num_in_page < 10 && $mail >= 0)
		{
			$m = &$mails[$mail];
			
			$subject = ucfirst(displayable_string($m[1]));

			$inst_mail .= str_replace(array('%%FROM%%', '%%UCFROM%%',   '%%SUBJECT%%', '%%DATE%%', '%%USER%%', '%%MAIL%%', '%%COLOR%%'), 
									  array($m[0],      ucfirst($m[0]), $subject,      $m[2],      $user,      $m[4],      $altern_color[$altern_index]),
									  $mailbox_mail);

			// step to next message
			++$num_in_page;
			--$mail;
			$altern_index = 1-$altern_index;
		}

		$links_str = '';
		$link_previous = ($page == 0 ? "mailbox.php?page=".$page : "mailbox.php?page=".($page-1));
		for ($i=0; $i<$num_pages; ++$i)
		{
			$link = ($i == $page ? $i+1 : "<a href='mailbox.php?page=$i'>".($i+1)."</a>");
			$links_str .= str_replace(array('%%LINK%%'),
									  array($link),
									  $browse_link);
		}
		$link_next = (($page == $num_pages-1 || $num_pages <= 1) ? "mailbox.php?page=".$page : "mailbox.php?page=".($page+1));

		$inst_mailbox = str_replace(array('%%MAILS%%', '%%PREVIOUS%%', '%%LINKS%%', '%%NEXT%%'),
									array($inst_mail,  $link_previous, $links_str,  $link_next),
									$mailbox_main);

		$pagename = $user_dir."mailbox".($page==0 ? '' : '_'.$page).'.html';

		$f = fopen($pagename, 'w');
		fwrite($f, $inst_mailbox);
		fclose($f);

		++$page;
	}
	while ($mail >= 0);
}

?>
