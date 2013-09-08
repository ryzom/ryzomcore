{block name=content}
<tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="{$second_menu_bg_color}" valign="middle">
	  <td>
	  <table>
	    <tr>
	      <td>
		<table cellspacing="0" cellpadding="4">
		  <tr>		    
		    <td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_user&id={$target_id}"><h7>Browse User</h7></a></td>
		    <td valign="middle" nowrap><a href="{$ingame_webpath}?page=createticket&user_id={$target_id}"><h7>Send Ticket</h7></a></td>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=2"><h7>Make Moderator</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 2 }
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 3 }
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=2"><h7>Demote to Moderator</h7></a></td>
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
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Change Settings of {$target_username}</h1></td>
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
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						
					<p><h3>Change Password</h3></p>
					
					<form id="changePassword" method="post" action="{$ingame_webpath}?page=settings&id={$target_id}">
						<table cellpadding="1">
						{if !isset($changesOther) or $changesOther eq "FALSE"}
						<tr>
							<td valign="middle">
							Current Password:
							</td>
							<td>
							<input type="text" id="CurrentPass" name="CurrentPass" {if isset($prevCurrentPass)}value="{$prevCurrentPass}"{/if}>
							{if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}<font color="red">The password is incorrect</font>{/if}
							</td>
							</p>
						</tr>
						{/if}
						<tr><td valign="middle">
						New Password:
						</td><td>
						<input type="text" class="input-xlarge" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
						{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<font color="red">{$newpass_error_message}</font>{/if}
						</td></tr>
							
						<tr><td valign="middle">
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
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<p><h3>Change Email</h3></p>
						
						<form id="changeEmail" class="form-vertical" method="post" action="{$ingame_webpath}?page=settings&id={$target_id}">
						<table>
							<tr><td valign="middle">
							New Email:
							</td><td>
							<input type="text" class="input-xlarge" id="NewEmail" size="200" name="NewEmail" placeholder="Your new email" {if isset($prevNewEmail)}value="{$prevNewEmail}"{else if isset($current_mail)}value="{$current_mail}"{/if}>
							{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}<font color="red">{$EMAIL}</font>{/if}
							</td></tr>
						</table>
						{if isset($SUCCESS_MAIL) and $SUCCESS_MAIL eq "OK"}
						<p>
							<font color="green">The email has been changed!</green>
						</p>
						{/if}
						
						<input type="hidden" name="function" value="change_mail">
						<input type="hidden" name="target_id" value="{$target_id}">
						<p>
						<input type="submit" value="Change Email"/>
						</p>
					</form>
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
	
{/block}
	
