{block name=content}
<tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="#00000040" valign="middle">
	  <td>
	  <table>
	    <tr>
	      <td>
		<table cellspacing="0" cellpadding="4">
		  <tr>		    
		    <td valign="middle" nowrap><a href="index.php?page=settings&id={$target_id}"><h7>Edit User</h7></a></td>
		    <td valign="middle" nowrap><a href="index.php?page=createticket&user_id={$target_id}"><h7>Send Ticket</h7></a></td>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=2"><h7>Make Moderator</h7></a></td>
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 2 }
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 3 }
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="index.php?page=change_permission&user_id={$target_id}&value=2"><h7>Demote to Moderator</h7></a></td>
			{/if}
		    {/if}
		  </tr>
		</table>
	      </td>
	    </tr>
	  </table>
	  </td>
	</tr>
	<tr>
		<td height="3" bgcolor="#000000"></td>
	</tr>
      </table>
  </td></tr>

    <tr><td>
	<table width="100%" bgcolor="#303030" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Change Settings</h1></td>
	</tr>
	<tr>
	  <td height="5"></td><td></td>
	</tr>
	</table>
  </td></tr>
  <tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="#000000" valign="middle">
	  <td>
	    <table>
	      <tr><td height="8"></td></tr>
	    </table>
	  </td>
	</tr>
	<tr><td height="2"></td></tr>
	<tr><td height="1" bgcolor="#000000"></td></tr>
	<tr><td height="10"></td></tr>
	<tr valign="middle">
	  <td>
	    <table width="100%" height="100%" cellpadding="10">
	      <tr><td>
		<table width="100%" bgcolor="#00000030" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						
					<p><h3>Change Password</h3></p>
					
					<form id="changePassword" method="post" action="index.php?page=settings&id={$target_id}">
						<table cellpadding="1">
						<tr><td height="3"></td></tr>
						{if !isset($changesOther) or $changesOther eq "FALSE"}
						<tr>
							<td>
							Current Password:
							</td>
							<td>
							<input type="text" id="CurrentPass" name="CurrentPass" {if isset($prevCurrentPass)}value="{$prevCurrentPass}"{/if}>
							{if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}<font color="red">The password is incorrect</font>{/if}
							</td>
							</p>
						</tr>
						{/if}
						<tr><td>
						New Password:
						</td><td>
						<input type="text" class="input-xlarge" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
						{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<font color="red">{$newpass_error_message}</font>{/if}
						</td></tr>
							
						<tr><td>
						Confirm New Password:
						</td><td>
						<input type="text" class="input-xlarge" id="ConfirmNewPass" name="ConfirmNewPass" placeholder="Re-enter the new password"  {if isset($prevConfirmNewPass)}value="{$prevConfirmNewPass}"{/if}>
						{if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}<font color="red">{$confirmnewpass_error_message}</font>{/if}
						</td></tr>  
						</table>
						{if isset($SUCCESS_PASS) and $SUCCESS_PASS eq "OK"}
						<p><font color="green">
							The password has been changed!
						</font></p>
						{/if}
						
						
						<input type="hidden" name="function" value="change_password">
						<input type="hidden" name="target_id" value="{$target_id}">

						<p><input type="submit" value="Change Password"/></p>

					</form>		
								
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
	      <tr><td>
		<table width="100%" bgcolor="#00000030" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<p><h3>Change Email</h3></p>
						
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
		<tr><td>
		<table width="100%" bgcolor="#00000030" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<p><h3>Change Info</h3></p>
						
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
	    </table>
	  </td>
	</tr>
      </table>
      
  </td></tr>


































	<div class="row-fluid sortable ui-sortable">
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Password</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changePassword" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Password</legend>
						
						{if !isset($changesOther) or $changesOther eq "FALSE"}
							<div class="control-group {if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}error{else if
				isset($match_error_message) and $match_error_message neq "fail"}success{else}{/if}">
							<label class="control-label">Current Password</label>
								<div class="controls">
								    <div class="input-prepend">
									<span class="add-on" style="margin-left:5px;"><i class="icon-lock"></i></span>
										<input type="password" class="input-xlarge" id="CurrentPass" name="CurrentPass" placeholder="Your current password" {if isset($prevCurrentPass)}value="{$prevCurrentPass}"{/if}>
										{if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}<span class="help-inline">The password is incorrect</span>{/if}
								    </div>
								</div>
							</div>
						{/if}
							<div class="control-group {if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($newpass_error_message) and $newpass_error_message eq "success"}success{else}{/if}">
						<label class="control-label">New Password</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-tag"></i></span>
									<input type="password" class="input-xlarge" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
									{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$newpass_error_message}</span>{/if}
							   </div>
							</div>
						</div>
							
						<div class="control-group {if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($confirmnewpass_error_message) and $confirmnewpass_error_message eq "success"}success{else}{/if}">
						<label class="control-label">Confirm New Password</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-tags"></i></span>
									<input type="password" class="input-xlarge" id="ConfirmNewPass" name="ConfirmNewPass" placeholder="Re-enter the new password"  {if isset($prevConfirmNewPass)}value="{$prevConfirmNewPass}"{/if}>
									{if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$confirmnewpass_error_message}</span>{/if}
							    </div>
							</div>
						</div>
						
						
					
						{if isset($SUCCESS_PASS) and $SUCCESS_PASS eq "OK"}
						<div class="alert alert-success">
							The password has been changed!
						</div>
						{/if}
						
						{if isset($SUCCESS_PASS) and $SUCCESS_PASS eq "SHARDOFF"}
						<div class="alert alert-warning">
							The password has been changed, though the shard seems offline, it may take some time to see the change on the shard.
						</div>
						{/if}
						
						<input type="hidden" name="function" value="change_password">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Password</button>
							</div>
						</div>
					</form>		
				</div>                   
			</div>
		</div><!--/span-->
				
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Email</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Email</legend>
						<div class="control-group {if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}error{/if}">
						<label class="control-label">New Email</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-envelope"></i></span>
									<input type="text" class="input-xlarge" id="NewEmail" name="NewEmail" placeholder="Your new email" {if isset($prevNewEmail)}value="{$prevNewEmail}"{else if isset($current_mail)}value="{$current_mail}"{/if}>
									{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}<span class="help-inline">{$EMAIL}</span>{/if}
						
							    </div>
							</div>
						</div>
						
						{if isset($SUCCESS_MAIL) and $SUCCESS_MAIL eq "OK"}
						<div class="alert alert-success">
							The email has been changed!
						</div>
						{/if}
						
						{if isset($SUCCESS_MAIL) and $SUCCESS_MAIL eq "SHARDOFF"}
						<div class="alert alert-warning">
							The email has been changed, though the shard seems offline, it may take some time to see the change on the shard.
						</div>
						{/if}
						
						<input type="hidden" name="function" value="change_mail">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Email</button>
							</div>
						</div>
					</form>
				</div>                   
			</div>
		</div><!--/span-->
				
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Info</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Info</legend>
						
						<div class="control-group">
						<label class="control-label">Firstname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-user"></i></span>
									<input type="text" class="input-xlarge" id="FirstName" name="FirstName" placeholder="Your firstname" {if isset($FirstName) and $FirstName neq ""}value="{$FirstName}"{/if}>
								</div>
							</div>
						</div>
						
						<div class="control-group">
						<label class="control-label">Lastname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-user"></i></span>
									<input type="text" class="input-xlarge" id="LastName" name="LastName" placeholder="Your lastname" {if isset($LastName) and $LastName neq ""}value="{$LastName}"{/if}>
								</div>
							</div>
						</div>
						
						<div class="control-group">
						<label class="control-label">Country</label>
							<div class="controls">
								 <select name="Country">
									{foreach from=$country_array key=k item=v}
										<option value="{$k}" {if isset($Country) and $Country eq $k}selected="selected"{/if}>{$v}</option>
									{/foreach}
								</select>	
							</div>
						</div>
						
						<div class="control-group">
							<label class="control-label">Gender</label>
							<div class="controls">							
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($Gender) and $Gender eq 0}checked{/if}"><input style="opacity: 0;" name="Gender" id="optionsRadios0" value="0" {if isset($Gender) and $Gender eq 0}checked=""{/if} type="radio"></span></div>
								      Secret
								</label>
								<div style="clear:both"></div>
								<label class="radio">
									<div id="uniform-optionsRadios1" class="radio"><span class="{if isset($Gender) and $Gender eq 1}checked{/if}"><input style="opacity: 0;" name="Gender" id="optionsRadios1" value="1" {if isset($Gender) and $Gender eq 1}checked=""{/if} type="radio"></span></div>
									Male
								</label>
								<div style="clear:both"></div>
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($Gender) and $Gender eq 2}checked{/if}"><input style="opacity: 0;" name="Gender" id="optionsRadios2" value="2" {if isset($Gender) and $Gender eq 2}checked=""{/if} type="radio"></span></div>
								      Female
								</label>
							</div>
						</div>
										
						{if isset($info_updated) and $info_updated eq "OK"}
						<div class="alert alert-success">
							The Info has been updated!
						</div>
						{/if}
						
						<input type="hidden" name="function" value="change_info">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Info</button>
							</div>
						</div>
					</form>
				</div>                   
			</div>
		</div><!--/span-->
	</div><!--/row-->
			
			
	
{/block}
	
