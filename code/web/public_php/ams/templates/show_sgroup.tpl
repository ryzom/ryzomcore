{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-9">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-list"></span>{$groupsname} List
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>{$groupsname} Support Group Members</legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
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
				<td><a href ="index.php?page=show_user&id={$user.tUserId}">{$user.name}</a></td>
				{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a class="btn btn-danger" href="index.php?page=show_sgroup&id={$target_id}&delete={$user.tUserId}"><span class="icon-trash icon-white"></span> Delete</a></td>{/if}
			  </tr>
			  {/foreach}

		    </tbody>
	    </table>
	    </div>
	</div>
	</div>
    </div><!--/span-->

    {if isset($isAdmin) && $isAdmin eq 'TRUE'}
    <div class="box col-md-3">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-plus-sign"></span> Add User
        </div>
        <div class="panel-body">
            <div class="row-fluid">

		<form id="addSGroup" class="form-vertical" method="post" action="index.php?page=show_sgroup&id={$target_id}">

		<legend style="margin:0">Add user to '{$groupsname}'</legend>

		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">username</label>
		    <div class="controls">
		    <select style="width: 140px;" name="Name">
			{if isset($users)}
				{foreach from=$users item=member}
					<option value="{$member.name}" >{$member.name}</option>
				{/foreach}
			{/if}
		    </select>
		    </div>
		</div>

		<input type="hidden" name="function" value="add_user_to_sgroup">
		<input type="hidden" name="target_id" value="{$target_id}">

		<div class="control-group">
		    <label class="control-label"></label>
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Add</button>
		    </div>
		</div>

		{if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
		<div class="alert alert-success">
			{$add_to_group_success}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "ALREADY_ADDED"}
		<div class="alert alert-error">
			{$user_already_added}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "GROUP_NOT_EXISTING"}
		<div class="alert alert-error">
			{$group_not_existing}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "USER_NOT_EXISTING"}
		<div class="alert alert-error">
			{$user_not_existing}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NOT_MOD_OR_ADMIN"}
		<div class="alert alert-error">
			{$not_mod_or_admin}
		</div>
		{/if}
		</form>

	    </div>
        </div>
	</div>
    </div><!--/span-->
	<div class="box col-md-3">
	<div class="panel panel-default">
	<div class="panel-heading" data-original-title="">
            <span class="icon-pencil"></span> Modify Email Settings
        </div>
	 <div class="panel-body">
            <div class="row-fluid">

		<form id="modifyMailSGroup" class="form-vertical" method="post" action="index.php?page=show_sgroup&id={$target_id}">

		<legend style="margin:0">Mail settings of '{$groupsname}'</legend>

		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">Group Email</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" id="GroupEmail" name="GroupEmail" value="{$groupemail}">
			</div>
		    </div>
		</div>

		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">IMAP Mail Server</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" id="IMAP_MailServer" name="IMAP_MailServer" value="{$imap_mailserver}">
			</div>
		    </div>
		</div>

		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">IMAP Username</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text"  id="IMAP_Username" name="IMAP_Username" value="{$imap_username}">
			</div>
		    </div>
		</div>

		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">IMAP Password</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="password"   id="IMAP_Password" name="IMAP_Password">
			</div>
		    </div>
		</div>

		<input type="hidden" name="function" value="modify_email_of_sgroup">
		<input type="hidden" name="target_id" value="{$target_id}">

		<div class="control-group">
		    <label class="control-label"></label>
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Update</button>
		    </div>
		</div>

		{if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "SUCCESS"}
		<div class="alert alert-success">
			{$modify_mail_of_group_success}
		</div>
		{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "EMAIL_NOT_VALID"}
		<div class="alert alert-error">
			{$email_not_valid}
		</div>
		{else if isset($RESULT_OF_MODIFYING) and $RESULT_OF_MODIFYING eq "NO_PASSWORD"}
		<div class="alert alert-warning">
			{$no_password_given}
		</div>
		{/if}

		</form>
	    </div>
	 </div>
	 </div>
</div>

    {/if}
</div><!--/row-->

{/block}

