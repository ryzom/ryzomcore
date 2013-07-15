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
		
		<form id="addSGroup" class="form-vertical" method="post" action="index.php">
		    
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
		<div class="alert alert-warning">
			{$group_name_taken}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "TAG_TAKEN"}
		<div class="alert alert-warning">
			{$group_tag_taken}
		</div>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SIZE_ERROR"}
		<div class="alert alert-warning">
			{$group_size_error}
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
		
			    </tr>
		    </thead>   
		    <tbody>
			{foreach from=$grouplist item=group}
			  <tr>
				<td>{$group.sGroupId}</td>
				<td><a href ="index.php?page=show_group&id={$group.sGroupId}">{$group.name}</a></td>
				<td class="center"><span class="label label-important" >{$group.tag}</span></td>
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
	
