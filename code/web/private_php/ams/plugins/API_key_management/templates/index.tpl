{block name=content}

{if isset($smarty.get.plugin_action) and $smarty.get.plugin_action eq 'generate_key'}
<div class="row-fluid">	
				<div class="box col-md-12">
				<div class="box-inner">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> API KEY management</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					<div class="box col-md-4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Generate Access Key</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="generateKey" class="form-vertical" method="post" action="index.php?page=layout_plugin&&name={$arrkey}&&plugin_action=generate_key">
						<legend>Generate Key</legend>
						
						<div class="control-group">
						<label class="control-label">Name:</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-user"></i></span>
									<input type="text" class="input-xlarge" id="sp_name" name="sp_name" placeholder="Your friendly name">
							    </div>
							</div>
						</div>
						
					<div class="control-group">
							<label class="control-label">Type:</label>
							<div class="controls">
								 <select name="api_type">	
									<option value="Character">Character</option>
									<option value="Corporation">Corporation</option>
								</select>	
							</div>
						</div>						
					<div class="control-group">
							<label class="control-label">Character:</label>
							<div class="controls">
								 <select name="character_name">
								 	{foreach from=$hook_info.API_key_management.characters item=element}
									<option value="{$element}">{$element}</option>
									{/foreach}
								</select>	
							</div>
						</div>
										
						<div class="control-group ">
						<label class="control-label">Expirey:</label>
							<div class="controls">
							    <div class="input-prepend">
								<span style="margin-left:5px;" class="add-on"><i class="icon-time"></i></span>
									<input type="text" placeholder="Expiry Date" name="expDate" id="expDate" class="input-xlarge">		
							    </div>
							</div>
						</div>
						
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" name="gen_key" value="true" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Generate Key</button>
							</div>
						</div>
					</form>		
				</div>                   
			</div>
		</div><!--/span-->
				</div><!--/span-->
				</div><!--/span-->
			</div><!--/row-->
{else}
<div class="row-fluid">	
				<div class="box col-md-12">
				<div class="box-inner">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> API KEY management</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					
			
			{if isset($hook_info.API_key_management['gen_key_validate']) and $hook_info.API_key_management['gen_key_validate'] eq 'false' }<div class="alert alert-error"><p>Please enter all the fields</p></div>{/if}
					{if isset($smarty.get.success) and $smarty.get.success eq '1'}<div class="alert alert-error"><p>Key added successfully</p></div>{/if}
					{if isset($smarty.get.success) and $smarty.get.success eq '2'}<div class="alert alert-error"><p>Key deleted successfully</p></div>{/if}
					<center>
						<a href="index.php?page=layout_plugin&&name=API_key_management&&plugin_action=generate_key"><button class="btn btn-primary btn-large dropdown-toggle">Generate key</button></a>
						</center>
			<div class="box-content">
				<div class="row-fluid">
					<center><p>All the keys you have generated will be shown and you can customize from here.</p></center>
						
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>Name</th>	
								  <th>Type</th>
								  <th>Character</th>
								  <th>Access Key</th>
								  <th>Expires</th>
								  <th>Actions</th>
							  </tr>
						  </thead>   
						  						  <tbody>
							{foreach from=$hook_info.API_key_management.api_keys item=element}
							<tr>
								<td class="center">{$element.FrName}</td>
								<td class="center">{$element.UserType}</td>
								<td class="center">{$element.UserCharacter}</td>
								<td class="center">{$element.AccessToken}</td>
								<td class="center">{$element.ExpiryDate}</td>
								<td><a href="index.php?page=layout_plugin&&name={$arrkey}&&delete_id={$element.SNo}"><button class="btn btn-primary btn-large">Delete</button></a>
                			</tr>
							{/foreach}
					
						  </tbody>
							
					  </table>		                   
			</div>
		</div><!--/span-->
				</div><!--/span-->
				</div><!--/span-->
			</div><!--/row-->
			{/if}
{/block}
