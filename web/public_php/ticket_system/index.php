<?php

// Customer support page of the in-game support window (cs_browser, the SUPPORT entry of the game menu).
//
// A placeholder until there is a ticket system: it offers the categories of the 2005 support window and refuses
// every submission, saving nothing. The client loads /ticket_system/index.php?mode=load from the web server of the
// shard and adds its debug and system information (UserId, ShardId, Player Name, UserPosition, ...) to every
// request; a ticket system would take the player from there.

header('Content-Type: text/html; charset=utf-8');

// The categories and their texts as the 2005 support window had them, with "the game" for "SoR".
$categories = array(
	'suggestion' => array('Suggestion',
		"You have a suggestion to make regarding the game."),
	'name' => array('Name change',
		"You have found someone with a name you believe to be in violation of the Rules of Conduct. Please be specific on the offending name."),
	'world' => array('World design',
		"You found a problem with the world: a missing texture, a hole in the terrain, you are able to reach a seemingly restricted area, etc. Please be specific with the location."),
	'bug' => array('Bug',
		"You have discovered a bug in the game. Please provide as much information as possible. What were you doing when the bug happened? Where were you? If possible, what are the steps to recreating the bug? Any information you feel would help us discover the cause of the bug."),
	'harassment' => array('Harassment/Profanity',
		"Someone is violating the harassment section of the Rules of Conduct. Please be clear on the type of harassment."),
	'question' => array('Question',
		"You have a question regarding the game's game play."),
	'technical' => array('Technical Problem',
		"You have a problem running the game (e.g. sound, graphics, or connection)."),
	'quest' => array('Quest',
		"You have a problem with a mission: an NPC doesn't assign a mission to you, or there is a problem in one of the mission steps (e.g. it won't finish, or it won't go to the next step). Please be specific."),
	'exploit' => array('Exploit',
		"You have discovered a means of exploiting a bug for personal gain, or you wish to report someone for doing so. Please be specific on the type of exploit, and if possible how it was created."),
);

function h($s)
{
	return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

$mode = (isset($_GET['mode']) && is_string($_GET['mode'])) ? $_GET['mode'] : 'load';
$category = (isset($_REQUEST['category']) && is_string($_REQUEST['category'])) ? $_REQUEST['category'] : '';
if (!isset($categories[$category]))
	$category = '';

echo '<html><head><title>Customer Support</title></head><body>';
echo '<h1>Customer Support..</h1>';

if ($mode === 'submit' && $category !== '')
{
	echo '<h3>' . h($categories[$category][0]) . '</h3>';
	echo '<p>Your ticket could not be submitted: this shard has no support ticket system yet. Nothing was sent or saved.</p>';
	echo '<p><a href="index.php?mode=load">Back to the categories</a></p>';
}
else if ($mode === 'category' && $category !== '')
{
	echo '<h3>' . h($categories[$category][0]) . '</h3>';
	echo '<p>' . h($categories[$category][1]) . '</p>';
	echo '<form method="post" action="index.php?mode=submit">';
	echo '<input type="hidden" name="category" value="' . h($category) . '">';
	echo '<p>Describe your problem below :</p>';
	// the client sizes the box from cols alone, so it fits the default width of the support window
	echo '<textarea name="problem" rows="10" cols="18"></textarea>';
	echo '<p><input type="submit" value="Submit"></p>';
	echo '</form>';
	echo '<p><a href="index.php?mode=load">Back to the categories</a></p>';
}
else
{
	// two columns, as in the 2005 window
	$keys = array_keys($categories);
	$half = (int)ceil(count($keys) / 2);
	echo '<p>Choose a category :</p>';
	echo '<table width="100%" cellpadding="4"><tr>';
	foreach (array(array_slice($keys, 0, $half), array_slice($keys, $half)) as $column)
	{
		echo '<td valign="top" width="50%">';
		foreach ($column as $key)
		{
			echo '<p><a href="index.php?mode=category&amp;category=' . h($key) . '">' . h($categories[$key][0]) . '</a><br>';
			echo h($categories[$key][1]) . '</p>';
		}
		echo '</td>';
	}
	echo '</tr></table>';
}

echo '</body></html>';
