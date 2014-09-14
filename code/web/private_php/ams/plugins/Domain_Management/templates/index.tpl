{block name=content}

{if $permission eq 3}
	{if isset($smarty.get.edit_domain)}
<div class="row-fluid sortable ui-sortable">
	<div class="box col-md-9">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-list"></span> Domain: {$hook_info['Domain_Management']['domains']['0']['domain_name']}
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<legend>Users with permissions in domain </legend>
			<table class="table table-striped table-bordered bootstrap-datatable datatable">
				<thead>
					<tr>
						<th>User ID</th>
						<th>Username</th>
						<th>Permissions</th>
						<th></th>
					</tr>
				</thead>
				<tbody>
				{assign var=val value=0}
				{foreach from=$hook_info['Domain_Management']['userlist'] item=element name=loop}
				  <tr>
					<td>{$element.id}</td>
					<td class="center"><a href="index.php?page=show_user&id={$element.id}">{$element.username}</a></td>
					<td class="center">
						{if isset($hook_info['Domain_Management']['permissions'][{$val}]['UId']) and $smarty.foreach.loop.iteration eq $hook_info['Domain_Management']['permissions'][{$val}]['UId']}
							{if $hook_info['Domain_Management']['permissions'][{$val}]['AccessPrivilege'] eq ''}
								BANNED!
							{else}
								{$hook_info['Domain_Management']['permissions'][{$val}]['AccessPrivilege']}
							{/if}
							{assign var=val value=$val+1}
						{else}
							NO PERMISSIONS
						{/if}
					</td>
					<td class="center">
						<div class="btn-group" style="display: inline-block;">
							<a class="btn btn-info" href="index.php?page=settings&id={$element.id}"><span class=" icon-pencil icon-white"></span> Edit User Permissions</a>
						</div>
					</td>
				  </tr>
				  {/foreach}
				</tbody>
			</table>
			</div>
		</div>
		</div>
		</div><!--/span-->

		<div class="box col-md-3">
		<div class="panel panel-default">
		<div class="panel-heading" data-original-title="">
				<span class="icon-pencil"></span> Modify Domain Settings
			</div>
		 <div class="panel-body">
				<div class="row-fluid">

			<form id="modifyDomain" class="form-vertical" method="post" action="index.php?page=layout_plugin&name=Domain_Management&edit_domain={$hook_info['Domain_Management']['domains']['0']['domain_id']}&ModifyDomain=1">

			<legend style="margin:0"> Domain Settings of '{$hook_info['Domain_Management']['domains']['0']['domain_name']}'</legend>

			<div class="control-group disabled" style="display: inline-block; ">
				<label class="control-label">Domain ID</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="domain_id" name="domain_id" value="{$hook_info['Domain_Management']['domains']['0']['domain_id']}" disabled>
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Domain Name</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="domain_name" name="domain_name" value="{$hook_info['Domain_Management']['domains']['0']['domain_name']}">
					</div>
				</div>
			</div>
			
			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Domain Status</label>		
				<div class="controls">
					<div class="input-prepend">
						<select id="status" multiple class="form-control"  form="modifyDomain">
							<option value="ds_close" {if {$hook_info['Domain_Management']['domains']['0']['status']} eq "ds_close"}selected{/if}>ds_close</option>
							<option value="ds_dev" {if {$hook_info['Domain_Management']['domains']['0']['status']} eq "ds_dev"}selected{/if}>ds_dev</option>
							<option value="ds_restricted" {if {$hook_info['Domain_Management']['domains']['0']['status']} eq "ds_restricted"}selected{/if}>ds_restricted</option>
							<option value="ds_open" {if {$hook_info['Domain_Management']['domains']['0']['status']} eq "ds_open"}selected{/if}>ds_open</option>
						</select>
					</div>
				</div>
			</div>
			
			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Patch Version</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="patch_version" name="patch_version" value="{$hook_info['Domain_Management']['domains']['0']['patch_version']}">
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Backup Patch URL</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="backup_patch_url" name="backup_patch_url" value="{$hook_info['Domain_Management']['domains']['0']['backup_patch_url']}">
					</div>
				</div>
			</div>
			
			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Patch URLs</label>
				<div class="controls">
					<div class="input-prepend">
						<textarea id="patch_urls" name="patch_urls" >{$hook_info['Domain_Management']['domains']['0']['patch_urls']}</textarea>
					</div>
				</div>
			</div>
			
			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Login Address</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="login_address" name="login_address" value="{$hook_info['Domain_Management']['domains']['0']['login_address']}">
					</div>
				</div>
			</div>
			
			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Session Manager Address</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="session_manager_address" name="session_manager_address" value="{$hook_info['Domain_Management']['domains']['0']['session_manager_address']}">
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Ring DB Name</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="ring_db_name" name="ring_db_name" value="{$hook_info['Domain_Management']['domains']['0']['ring_db_name']}">
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Web Host</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="web_host" name="web_host" value="{$hook_info['Domain_Management']['domains']['0']['web_host']}">
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Web Host PHP</label>
				<div class="controls">
					<div class="input-prepend">
						<input type="text" id="web_host_php" name="web_host_php" value="{$hook_info['Domain_Management']['domains']['0']['web_host_php']}">
					</div>
				</div>
			</div>

			<div class="control-group" style="display: inline-block; ">
				<label class="control-label">Description</label>
				<div class="controls">
					<div class="input-prepend">
						<textarea id="description" name="description" >{$hook_info['Domain_Management']['domains']['0']['description']}</textarea>
					</div>
				</div>
			</div>

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
			{/if}

			</form>
			</div>
		 </div>
		 </div>
	</div>

	</div><!--/row-->
	{else}
	
	<div class="row-fluid">
<div class="box col-md-12">
<div class="panel panel-default">
<div class="panel-heading" data-original-title="">

<span class="icon-user"></span>
Domains

</div>
<div class="panel-body">
	<table class="table table-striped table-bordered">
					<thead>
						<tr>
							<th>Domain ID</th>
							<th>Domain Name</th>
							<th>Status</th>
							<th>Patch Version</th>
							<th>Description</th>			   
						</tr>
					</thead>
				<tbody>
				{if isset($hook_info['Domain_Management']['domains'][0])}
					{foreach from=$hook_info['Domain_Management']['domains'] item=array}
						<tr>
							<td>{$array['domain_id']}</td>
							<td><a href="{$hook_info['Domain_Management']['path']}?page=layout_plugin&name=Domain_Management&edit_domain={$array['domain_id']}">{$array['domain_name']}</a></td>
							<td>{$array['status']}</td>
							<td>{$array['patch_version']}</td>
							<td>{$array['description']|truncate}</td>	
						</tr>
					{/foreach}
				{/if}
		</tbody>
	 </table>
		 </div>
		 </div>
	</div>

	</div><!--/row-->
	{/if}	
{/if}	
{/block}
