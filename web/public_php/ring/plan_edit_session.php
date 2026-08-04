<?php 
	// _once like every other ring page: a plain include here is one refactor
	// away from a function/class redeclare fatal (the exact break the
	// login/config.php charset helper caused across this directory once)
	include_once('../tools/validate_cookie.php');
	include_once('ring_session_manager_itf.php');
	include_once('../tools/domain_info.php');
	include_once('../login/config.php');

	// The schedule result callback lives on ScheduleSessionCb in
	// session_tools.php; a bare function of the same name here was never
	// called (this page only renders the form) and it tested the result
	// code backwards, so it is gone.

	///////////////////////////
	// Main code
	///////////////////////////
		

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "<h1>Invalid cookie, please relog<H1>";
		die;
	}

	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];
?>
	
<h1>Schedule an edit or animation session</h1>
<form action="send_plan_edit_session.php" method="post">
	Session title : <input type="text" name="title" value="enter a title"><br>
	Session description : <input type="text" name="description" value="enter a description"><br>
	<input type='hidden' name='session_type' value='st_edit'>
	<?php /* keep the character slot across the post: without it the session
	   is scheduled for slot 0, not the character that opened this page */ ?>
	<input type='hidden' name='charSlot' value='<?php echo getCharSlot(); ?>'>
	<input type="submit" name="button" value="Schedule edition">
</form>
<br>
<br>
<form action="send_plan_edit_session.php" method="post">
	Session title : <input type="text" name="title" value="enter a title"><br>
	Session description : <input type="text" name="description" value="enter a description"><br>
	<input type='hidden' name='session_type' value='st_anim'>
	<input type='hidden' name='charSlot' value='<?php echo getCharSlot(); ?>'>
	<input type="submit" name="button" value="Schedule animation">
</form>
<br>	
<br>	
<a href="web_start.php">Return to start menu</a>
