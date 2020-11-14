<?php

include_once('utils.php');




// -------------------------------------
// add a new thread into user forum index
// -------------------------------------
function add_thread_to_forum_index(&$from, &$to, &$subject, &$index)
{
	global $shard;
	$to_dir = get_user_dir($to, $shard);
	$to_index = $to_dir.'forum.index';

	use_index($to_index);

	$f = fopen($to_index, 'r+');
	read_next_index($f, $index);
	append_to_index($f, trim($from).'%%'.strtr(trim($subject), "\n", " ").' %%".displayable_date()."%%1%%'.$index);
	update_next_index($f, $index+1);
	fclose($f);
}


// -------------------------------------
// create new thread index
// -------------------------------------
function create_thread(&$from, &$to, &$subject, &$index)
{
	global $shard;
	$to_dir = get_user_dir($to, $shard);

	// build mail message
	$array = array();
	write_index($to_dir."thread_$index.index", trim($from).'%%'.strtr(trim($subject), "\n", " ").' %%'.displayable_date(), $array);
}

// -------------------------------------
// remove thread
// -------------------------------------
function remove_thread($forum, $thread)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);

	read_index($forum_dir.'forum.index', $header, $array);

	for ($i=0; $i<count($array); ++$i)
		if (trim($array[$i][4]) != trim($thread))
			$newarray[] = $array[$i];

	write_index($forum_dir.'forum.index', $header, $newarray);

	$compare = 'thread_'.$thread;
	$len = strlen($compare);

	if ($dir = @opendir($forum_dir))
	{
		while($file = readdir($dir))
			if (!strncmp($file, $compare, $len))
				rename($forum_dir.$file, $forum_dir.'_'.$file);

		closedir($dir);
	}

	build_forum_page($forum);
}

// -------------------------------------
// add post to thread
// -------------------------------------
function add_post(&$from, &$to, &$content, &$index, &$last_date)
{
	global $shard;
	$to_dir = get_user_dir($to, $shard);

	// build mail message
	$last_date = displayable_date();
	$fname = $to_dir."thread_$index.index";
	$f = fopen($fname, "a+");
	append_to_index($f, trim($from).'%%'.trim($content).' %%'.$last_date);
	fclose($f);
}

// -------------------------------------
// remove post
// -------------------------------------
function remove_post($forum, $thread, $posts)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);

	read_index($forum_dir.'thread_'.$thread.'.index', $header, $array);

	sort($posts);
	$remove_post = 0;

	for ($i=0; $i<count($array); ++$i)
		if ($remove_post >= count($posts) || $i != $posts[$remove_post])
			$newarray[] = $array[$i];
		else
			++$remove_post;

	write_index($forum_dir.'thread_'.$thread.'.index', $header, $newarray);

	build_thread_page($forum, $thread, $num_posts);
	update_forum_index($forum, $thread, $num_posts, "");
	build_forum_page($forum);
}


// -------------------------------------
// update forum index after a thread is modified
// -------------------------------------
function update_forum_index($user, $thread, $num_posts, $last_date)
{
	global $shard;
	$user_dir = get_user_dir($user, $shard);
	$user_index = $user_dir.'forum.index';

	// read forum index
	read_index($user_index, $header, $threads);

	// search for thread line
	for ($i=0; $i<count($threads); ++$i)
	{
		if (trim($threads[$i][4]) == trim($thread))
		{
			$threads[$i][2] = $last_date;
			$threads[$i][3] = $num_posts;
		}
	}

	// write forum index
	write_index($user_index, $header, $threads);
}



// -------------------------------------
// rebuild whole forum index
// -------------------------------------
function build_forum_index($forum)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);
	$forum_index = $forum_dir.'forum.index';

	read_index($forum_index, $header, $threads);

	$threads = array();
	$browse_dir = opendir($forum_dir);
	while ($browse_dir && ($browse_file = readdir($browse_dir)))
	{
		if (ereg("^thread_([0-9]*)\.index", $browse_file, $regs))
		{
			echo "added $browse_file to forum<br>\n";
			$index = $regs[1];
			read_index($forum_dir.$browse_file, $fheader, $posts);

			$fhdr = explode('%%', $fheader);

			$threads[$index] = trim($fhdr[0]).'%%'.trim($fhdr[1]).'%%'.trim($fhdr[2]).'%%'.count($posts).'%%'.$index;
		}
	}

	$icontent = array();
	for ($i=0; $i<$header; ++$i)
	{
		if ($threads[$i] == '')
			continue;

		$icontent[] = array($threads[$i]);
	}

	write_index($forum_index, $header, $icontent);

	build_forum_page($forum);
}

// -------------------------------------
// recover all deleted threads
// -------------------------------------
function recover_thread($forum)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);

	$browse_dir = opendir($forum_dir);
	while ($browse_dir && ($browse_file = readdir($browse_dir)))
	{
		if (ereg("^_thread_([0-9]*)\.(index|html)", $browse_file))
		{
			echo "recover file $browse_file<br>\n";
			rename($forum_dir.'/'.$browse_file, $forum_dir.'/'.substr($browse_file, 1));
		}
	}

	build_forum_index($forum);
}

// -------------------------------------
// recover all deleted threads
// -------------------------------------
function recover_one_thread($forum, $thread)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);

	if (file_exists($forum_dir.$thread))
	{
		rename($forum_dir.$thread, $forum_dir.substr($thread, 1));
		build_forum_index($forum);
	}
	else if (file_exists($forum_dir.'_thread_'.$thread.'.index'))
	{
        $file = 'thread_'.$thread.'.index';
        rename($forum_dir.'_'.$file, $forum_dir.$file);

        if (file_exists($forum_dir.'_thread_'.$thread.'.html'))
        {
                $file = 'thread_'.$thread.'.html';
                rename($forum_dir.'_'.$file, $forum_dir.$file);
        }

        build_forum_index(nameToFile($forum));
	}
}

// -------------------------------------
// rename forum
// -------------------------------------
function rename_forum($forum, $into)
{
	global $shard;

	$olddir = build_user_dir($forum);
	$newdir = build_user_dir($into);

	$dir = opendir($olddir);
	while ($dir && ($file = readdir($dir)))
		$files[] = $file;

	foreach ($files as $file)
	{
		copy($olddir.'/'.$file, $newdir.'/'.$file);
		delete($olddir.'/'.$file);
	}

	build_forum_page($into);
}



// -------------------------------------
// rebuild user mail box pages
// -------------------------------------
function build_forum_page($forum)
{
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);
	$forum_index = $forum_dir.'forum.index';

	// open forum index
	read_index($forum_index, $header, $threads);

	$num_threads = count($threads);
	$num_per_page = 10;
	$num_pages = (int)(($num_threads+$num_per_page-1) / $num_per_page);

	$thread = $num_threads-1;
	$num_in_page = 0;
	$page = 0;

	$altern_color = array("#333333", "#666666");

	$links_str = '';

	read_template('forum_main.html', $forum_main);
	read_template('forum_topic.html', $forum_topic);

	read_template('browse_link.html', $browse_link);

	do
	{
		$inst_topic = '';
		$num_in_page = 0;
		$altern_index = 0;

		while ($num_in_page < 10 && $thread >= 0)
		{
			$t = &$threads[$thread];

			// replace in topic
			$subject = ucfirst(displayable_string($t[1]));

			$inst_topic .= str_replace(array('%%SUBJECT%%',     '%%SENDER%%', '%%UCSENDER%%', '%%NUMPOSTS%%', '%%DATE%%', '%%FORUM%%', 			'%%UCFORUM%%',              '%%THREAD%%', '%%COLOR%%'),
									   array(ucfirst($subject), $t[0],        ucfirst($t[0]), $t[3],          $t[2],      nameToURL($forum),	convert_forum_name($forum), $t[4],        $altern_color[$altern_index]),
									   $forum_topic);

			// step to next thread
			++$num_in_page;
			--$thread;
			$altern_index = 1-$altern_index;
		}

		$links_str = '';
		$link_previous = ($page == 0 ? "forum.php?page=".$page : "forum.php?page=".($page-1));
		for ($i=0; $i<$num_pages; ++$i)
		{
			$link = ($i == $page ? $i+1 : "<a href='forum.php?page=$i'>".($i+1)."</a>");
			$links_str .= str_replace(array('%%LINK%%'),
									  array($link),
									  $browse_link);
		}
		$link_next = (($page == $num_pages-1 || $num_pages <= 1) ? "forum.php?page=".$page : "forum.php?page=".($page+1));

		// replace in forum
		$inst_forum = str_replace(array('%%TOPICS%%', '%%FORUM_POST%%', '%%FORUM%%', 		'%%UCFORUM%%',              '%%PREVIOUS%%', '%%LINKS%%', '%%NEXT%%'),
							   	  array($inst_topic,  $forum, 			nameToURL($forum),	convert_forum_name($forum), $link_previous, $links_str,  $link_next),
							   	  $forum_main);

		$pagename = $forum_dir.'forum'.($page==0 ? '' : '_'.$page).'.html';

		$f = fopen($pagename, 'w');
		fwrite($f, $inst_forum);
		fclose($f);

		++$page;

	}
	while ($thread >= 0);
}







// -------------------------------------
// rebuild user mail box pages
// -------------------------------------
function build_thread_page($forum, $thread, &$num_posts)
{
	global $shard;
	$thread_dir = get_user_dir($forum, $shard);
	$thread_index = $thread_dir."thread_$thread.index";

	// open thread index
	read_index($thread_index, $header, $posts);

	$header = explode('%%', $header);

	$thread_subject = $header[1];

	$num_posts = count($posts);
	$num_per_page = 10;
	$num_pages = (int)(($num_posts+$num_per_page-1) / $num_per_page);

	$post = 0;
	$page = 0;

	$altern_color = array("#333333", "#666666");
	$altern_index = 0;

	$links_str = '';

	read_template('topic_main.html', $topic_main);
	read_template('topic_post.html', $topic_post);

	read_template('browse_link.html', $browse_link);

	do
	{
		$num_in_page = 0;
		$inst_post = '';

		while ($num_in_page < 10 && $post < $num_posts)
		{
			$p = &$posts[$post];

			$content = nl2br(displayable_content($p[1]));

			$inst_post .= str_replace(array('%%FORUM%%', 		'%%UCFORUM%%',              '%%SENDER%%', '%%UCSENDER%%', '%%DATE%%', '%%CONTENT%%', '%%POST%%', '%%COLOR%%'),
									  array(nameToURL($forum),  convert_forum_name($forum), $p[0],        ucfirst($p[0]), $p[2],      $content,      $post,      $altern_color[$altern_index]),
									  $topic_post);

			// step to next post
			++$num_in_page;
			++$post;
			$altern_index = 1-$altern_index;
		}

		$links_str = '';
		$link_previous = ($page == 0 ? "thread.php?thread=$thread&page=".$page : "thread.php?thread=$thread&page=".($page-1));
		for ($i=0; $i<$num_pages; ++$i)
		{
			$link = ($i == $page ? $i+1 : "<a href='thread.php?thread=$thread&page=$i'>".($i+1)."</a>");
			$links_str .= str_replace(array('%%LINK%%'),
									  array($link),
									  $browse_link);
		}
		$link_next = (($page == $num_pages-1 || $num_pages <= 1) ? "thread.php?thread=$thread&page=".$page : "thread.php?thread=$thread&page=".($page+1));

		$inst_topic = str_replace(array('%%POSTS%%', '%%FORUM_POST%%', 	'%%FORUM%%', 		'%%UCFORUM%%', 				'%%THREAD%%', 	'%%PREVIOUS%%', '%%LINKS%%', '%%NEXT%%', '%%SUBJECT%%'),
							   	  array($inst_post,  $forum, 			nameToURL($forum), 	convert_forum_name($forum), $thread, 		$link_previous, $links_str,  $link_next, ucfirst($thread_subject)),
							   	  $topic_main);

		$pagename = $thread_dir."thread_$thread".($page==0 ? '' : '_'.$page).'.html';

		$f = fopen($pagename, 'w');
		fwrite($f, $inst_topic);
		fclose($f);

		++$page;
	}
	while ($post < $num_posts);
}


?>