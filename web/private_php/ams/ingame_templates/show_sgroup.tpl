{block name=content}

  <tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Support Group: {$groupsname}</h1></td>
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
						<p><h3>Add user to the list</h3></p>
						 {if isset($isAdmin) && $isAdmin eq 'TRUE'}	    
						    <form id="addSGroup" class="form-vertical" method="post" action="{$ingame_webpath}?page=show_sgroup&id={$target_id}">
						    <table>
							<tr>
							    <td valign="middle">Username: </td>
							    <td><input type="text" maxlength="15"   id="Name" name="Name"></td>
							</tr>
						    </table>
						    <input type="hidden" name="function" value="add_user_to_sgroup">
						    <input type="hidden" name="target_id" value="{$target_id}">
							
						    <p>
							<input type="submit" value="Add user" />
						    </p>
						    
						    {if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
						    <p>
							    <font color="green">{$add_to_group_success}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "ALREADY_ADDED"}
						    <p>
							    <font color="red">{$user_already_added}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "GROUP_NOT_EXISTING"}
						    <p>
							    <font color="red">{$group_not_existing}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "USER_NOT_EXISTING"}
						    <p>
							    <font color="red">{$user_not_existing}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NOT_MOD_OR_ADMIN"}
						    <p>
							    <font color="red">{$not_mod_or_admin}</font>
						    </p>
						    {/if}
						    </form>
						{/if}
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
						<p><h3>All members</h3></p>
						<table cellpadding="4">
						    <tr bgcolor="{$table_header_tr_color}">
							    <td>ID</td>
							    <td>Name</td>
							    {if isset($isAdmin) && $isAdmin eq 'TRUE'}<td>Action</td>{/if}
					
						    </tr>
						{foreach from=$userlist item=user}
						  <tr>
							<td>{$user.tUserId}</td>
							<td><a href ="{$ingame_webpath}?page=show_user&id={$user.tUserId}">{$user.name}</a></td>
							{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a href="{$ingame_webpath}?page=show_sgroup&id={$target_id}&delete={$user.tUserId}"><font color="red">Delete</font></a></td>{/if}
						  </tr>
						  {/foreach}
						</table>            
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
						<p><h3>Mail settings</h3></p>
						<form id="modifyMailSGroup" class="form-vertical" method="post" action="{$ingame_webpath}?page=show_sgroup&id={$target_id}">
						 <table>
						    
						    <tr>
							<td valign="middle">Group Email: </td>
							<td><input type="text" id="GroupEmail" name="GroupEmail" size="220" value="{$groupemail}"></td>
						    </tr>
						    
						    <tr>
							<td valign="middle">IMAP Mail Server: </td>
							<td><input type="text" id="IMAP_MailServer" name="IMAP_MailServer" size="220" value="{$imap_mailserver}"></td>
						    </tr>
						    
						    <tr>
							<td valign="middle">IMAP Username: </td>
							<td><input type="text"  id="IMAP_Username" name="IMAP_Username" size="220" value="{$imap_username}"></td>
						    </tr>
						    
						    <tr>
							<td valign="middle">IMAP Password: </td>
							<td><input type="text"   id="IMAP_Password" size="220" name="IMAP_Password"></td>
						    </tr>
						     
						</table>
						
						<input type="hidden" name="function" value="modify_email_of_sgroup">
						<input type="hidden" name="target_id" value="{$target_id}">
						    
						<p>
							<input type="submit" value="Update"/>
						</p>
						
						{if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "SUCCESS"}
						<p>
							<font color="green">{$modify_mail_of_group_success}</font>
						</p>
						{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "EMAIL_NOT_VALID"}
						<p>
							<font color="red">{$email_not_valid}</font>
						</p>
						{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "NO_PASSWORD"}
						<p>
							<font color="yellow">{$no_password_given}</font>
						</p>
						{/if}
						
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
	
