{block name=content}

<h2>{$groupsname} Members List</h2>
<table>
    <thead>
	    <tr>
		    <th>ID</th>
		    <th>Name</th>
		    {if isset($isAdmin) && $isAdmin eq 'TRUE'}<th>Action</th>{/if}

	    </tr>
    </thead>   
    <tbody>
	{foreach from=$userlist item=user}
	  <tr>
		<td>{$user.tUserId}</td>
		<td><a href ="ams?page=show_user&id={$user.tUserId}">{$user.name}</a></td>
		{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a href="ams?page=show_sgroup&id={$target_id}&delete={$user.tUserId}"><font color="red"> Delete</font></a></td>{/if}
	  </tr>
	  {/foreach}

    </tbody>
</table>            

    {if isset($isAdmin) && $isAdmin eq 'TRUE'}
        <h2>Add user to '{$groupsname}'</h2>
	<table>
	    <tr>
		<td>
		    <form id="addSGroup" class="form-vertical" method="post" action="ams?page=show_sgroup&id={$target_id}">		    
		    <label>Username:</label>
		    <input type="text" maxlength="15"   id="Name" name="Name">		
		    <input type="hidden" name="function" value="add_user_to_sgroup">
		    <input type="hidden" name="target_id" value="{$target_id}">
		    <button type="submit" class="btn btn-primary" >Add</button>
		    </form>
		</td>
	    </tr>
	</table>
	

		
	{if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
	<font color="green">
		<p>{$add_to_group_success}</p>
	</font>
	{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "ALREADY_ADDED"}
	<font color="red">
		<p>{$user_already_added}</p>
	</font>
	{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "GROUP_NOT_EXISTING"}
	<font color="red">
		<p>{$group_not_existing}</p>
	</font>
	{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "USER_NOT_EXISTING"}
	<font color="red">
		<p>{$user_not_existing}</p>
	</font>
	{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NOT_MOD_OR_ADMIN"}
	<font color="red">
		<p>{$not_mod_or_admin}</p>
	</font>
	{/if}
	
		
        <h2>Modify Email Settings</h2>
        <form id="modifyMailSGroup" class="form-vertical" method="post" action="ams?page=show_sgroup&id={$target_id}">
	<table>
	    <tr>
		<td>
		    <label>Group Email</label>
		    <input type="text" id="GroupEmail" name="GroupEmail" value="{$groupemail}">
		</td>
		<td>
		    <label>IMAP Mail Server</label>
		    <input type="text" id="IMAP_MailServer" name="IMAP_MailServer" value="{$imap_mailserver}">
		</td>
	    </tr>
	    <tr>
		<td>
		    <label>IMAP Username</label>
		    <input type="text"  id="IMAP_Username" name="IMAP_Username" value="{$imap_username}">
		</td>
		<td>
		    <label>IMAP Password</label>
		    <input type="password"   id="IMAP_Password" name="IMAP_Password">
		</td>
	    </tr>
	    <tr>
		<td>			    
		    <input type="hidden" name="function" value="modify_email_of_sgroup">
		    <input type="hidden" name="target_id" value="{$target_id}">
		    <button type="submit" class="btn btn-primary" >Update</button>
		</td>
		<td></td>
	    </tr>
	</table>
	</form>   
		
	{if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "SUCCESS"}
	<font color="green">
		{$modify_mail_of_group_success}
	</font>
	{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "EMAIL_NOT_VALID"}
	<font color="red">
		{$email_not_valid}
	</font>
	{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "NO_PASSWORD"}
	<font color="red">
		{$no_password_given}
	</font>
	{/if}
		
	
    {/if}

{/block}
	
