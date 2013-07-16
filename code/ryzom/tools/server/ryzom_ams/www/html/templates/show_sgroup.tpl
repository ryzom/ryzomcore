{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span12">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-plus-sign"></i> Add User</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
		
		<form id="addSGroup" class="form-vertical" method="post" action="index.php?page=show_sgroup&id={$target_id}">
		    
		<legend>Add  a user to the group '{$groupsname}'</legend>
		
		<div class="control-group" style="display: inline-block; ">
		    <label class="control-label">username</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" maxlength="15"   id="Name" name="Name">
			</div>
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
		{/if}
		</form>
		
	    </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->

<div class="row-fluid sortable ui-sortable">
    <div class="box span12">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-list"></i>{$groupsname} List</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>All support groups</legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Name</th>
				    <th>Action</th>
		
			    </tr>
		    </thead>   
		    <tbody>
			{foreach from=$userlist item=user}
			  <tr>
				<td>{$user.tUserId}</td>
				<td><a href ="index.php?page=show_user&id={$user.tUserId}">{$user.name}</a></td>
				<td class="center"><a class="btn btn-danger" href="#"><i class="icon-trash icon-white"></i> Delete</a></td>  
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	    </div>
	</div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
