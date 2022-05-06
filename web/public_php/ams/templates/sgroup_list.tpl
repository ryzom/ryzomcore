{block name=content}


<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-9">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-list"></span> List
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>All support groups</legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Name</th>
				    <th>Tag</th>
				    <th>Email</th>
				    {if isset($isAdmin) && $isAdmin eq 'TRUE'}<th>Action</th>{/if}
			    </tr>
		    </thead>
		    <tbody>
			{foreach from=$grouplist item=group}
			  <tr>
				<td>{$group.sGroupId}</td>
				<td><a href ="index.php?page=show_sgroup&id={$group.sGroupId}">{$group.name}</a></td>
				<td class="center"><span class="label label-important" >{$group.tag}</span></td>
				<td class="center">{$group.groupemail}</td>
				{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a class="btn btn-danger" href="index.php?page=sgroup_list&delete={$group.sGroupId}"><span class="icon-trash icon-white"></span> Delete</a></td>{/if}
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
            <span class="icon-plus-sign"></span> Add
        </div>
        <div class="panel-body">
            <div class="row-fluid">

		<form id="addSGroup" class="form-vertical" method="post" action="index.php?page=sgroup_list">

		<legend>Add  a support group</legend>

		<div class="control-group">
		    <label class="control-label">Group name</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" maxlength="20"   id="Name" name="Name">
			</div>
		    </div>
		</div>

		<div class="control-group">
		    <label class="control-label">Group Tag</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" maxlength="4"  id="Tag" name="Tag">
			</div>
		    </div>
		</div>

		<div class="control-group">
		    <label class="control-label">Group EmailAddress</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text"  id="GroupEmail" name="GroupEmail">
			</div>
		    </div>
		</div>

		<div class="control-group">
		    <label class="control-label">IMAP MailServer IP</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text"  id="IMAP_MailServer" name="IMAP_MailServer">
			</div>
		    </div>
		</div>

		<div class="control-group">
		    <label class="control-label">IMAP Username</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text"  id="IMAP_Username" name="IMAP_Username">
			</div>
		    </div>
		</div>

		<div class="control-group">
		    <label class="control-label">IMAP Password</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="password"   id="IMAP_Password" name="IMAP_Password">
			</div>
		    </div>
		</div>

		<input type="hidden" name="function" value="add_sgroup">

		<div class="control-group">
		    <label class="control-label"></label>
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Add</button>
		    </div>
		</div>

		{if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
		<div class="alert alert-success">
			{$group_success}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NAME_TAKEN"}
		<div class="alert alert-error">
			{$group_name_taken}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "TAG_TAKEN"}
		<div class="alert alert-error">
			{$group_tag_taken}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SIZE_ERROR"}
		<div class="alert alert-error">
			{$group_size_error}
		</div>
		{/if}
		</form>

	    </div>
        </div>
        </div>
    </div><!--/span-->
    {/if}
</div><!--/row-->
{/block}

