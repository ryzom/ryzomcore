{block name=content}
<div class="row-fluid">	
				<div class="box span12">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> {$plugin_title}</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					{if isset($smarty.get.result) and $smarty.get.result eq "1"}<div class="alert alert-error"><p>{$ip_success}</p></div>{/if}
					<div class="box-content">
						<center><p>{$plugin_info}</p></center>
						<center><a href="index.php?page=plugins&action=deletePlugins"><button class="btn btn-primary btn-large">Delete</button></a>
                <a href="index.php?page=plugins&action=activatePlugins"><button class="btn btn-primary btn-large dropdown-toggle">Activate</button></a>
                <a href="index.php?page=plugins&action=deactivatePlugins"><button class="btn btn-primary btn-large dropdown-toggle">Deactivate</button></a>
				<a href="index.php?page=install_plugin"><button class="btn btn-primary btn-large dropdown-toggle">Add</button></a>
				<a href="index.php?page=plugins&action=updatePlugins"><button class="btn btn-primary btn-large dropdown-toggle">Check for updates</button></a>
				</center>
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>{$plugin_status}</th>	
								  <th width="150">{$plugin_name}</th>
								  <th>{$plugin_version}</th>
								  <th width="400">{$plugin_description}</th>
								  <th>{$plugin_type}</th>
								  <th>{$plugin_permission}</th>
							  </tr>
						  </thead>   
						  <tbody>
							{foreach from=$plug item=element}
							<tr>
								<td><input type="checkbox" name ="{$element.id}"{if ($element.plugin_status) eq "1"}checked{/if}/></td>
								<td class="center">{$element.plugin_name}</td>
								<td class="center">{$element.plugin_info->Version}</td>
								<td class="center">{$element.plugin_info->Description}</td>
								<td class="center">{$element.plugin_type}</td>
								<td class="center">{$element.plugin_permission}</td>
							</tr>
							{/foreach}
					
						  </tbody>
					  </table>
					  <div style="width: 300px; margin:0px auto;">
						<ul class="pagination">
							<li><a href="index.php?page=plugins&pagenum=1">&laquo;</a></li>
							{foreach from=$links item=link}
							<li {if $link == $currentPage}class="active"{/if}><a href="index.php?page=plugins&pagenum={$link}">{$link}</a></li>
							{/foreach}
							<li><a href="index.php?page=plugins&pagenum={$lastPage}">&raquo;</a></li>
						</ul>
					  </div>
					</div>
					
				</div><!--/span-->
			</div><!--/row-->
{/block}

