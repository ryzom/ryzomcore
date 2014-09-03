{block name=content}
<div class="row-fluid">
				<div class="box col-md-12">
				<div class="box-inner">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> {$plugin_title}</h2>
					</div>
					{if isset($smarty.get.result) and $smarty.get.result eq "1"}<div class="alert alert-error"><p>{$ip_success}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "0"}<div class="alert alert-error"><p>{$dp_error}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "2"}<div class="alert alert-error"><p>{$dp_success}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "3"}<div class="alert alert-error"><p>{$ac_success}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "4"}<div class="alert alert-error"><p>{$ac_error}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "5"}<div class="alert alert-error"><p>{$dc_success}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "6"}<div class="alert alert-error"><p>{$dc_error}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "7"}<div class="alert alert-error"><p>{$up_success}</p></div>{/if}
					{if isset($smarty.get.result) and $smarty.get.result eq "8"}<div class="alert alert-error"><p>{$up_install_success}</p></div>{/if}
					<div class="box-content">
						<center><p>{$plugin_info}</p></center>
						<center>
						<a href="index.php?page=install_plugin"><button class="btn btn-primary btn-large dropdown-toggle">Install New Plugin</button></a>
						<a href="index.php?page=plugins_update"><button class="btn btn-primary btn-large dropdown-toggle">Check for updates</button></a>
						</center>
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>{$plugin_status}</th>
								  <th width="100">{$plugin_name}</th>
								  <th>{$plugin_version}</th>
								  <th width="350">{$plugin_description}</th>
								  <th width="80">{$plugin_type}</th>
								  <th>{$plugin_permission}</th>
								  <th>{$plugin_actions}</th>
							  </tr>
						  </thead>
						  <tbody>
							{foreach from=$plug item=element}
							<tr>
								<td>{if ($element.plugin_status) eq "1"}<i class="glyphicon glyphicon-ok green"></i>{else}<i class="glyphicon glyphicon-remove red"></i>{/if}</td>
								<td class="center">{$element.plugin_name}</td>
								<td class="center">{$element.plugin_info->Version}</td>
								<td class="center">{$element.plugin_info->Description}</td>
								<td class="center">{$element.plugin_type}</td>
								<td class="center">{$element.plugin_permission}</td>
								<td>
                {if ($element.plugin_status) eq "0"}
                <a href="index.php?page=plugins&action=delete_plugin&id={$element.id}"><button class="btn btn-primary btn-large">Delete</button></a>
                <a href="index.php?page=plugins&action=activate_plugin&id={$element.id}"><button class="btn btn-primary btn-large dropdown-toggle">Activate</button></a>{/if}
                {if ($element.plugin_status) eq "1"}<a href="index.php?page=plugins&action=deactivate_plugin&id={$element.id}"><button class="btn btn-primary btn-large dropdown-toggle">Deactivate</button></a>{/if}</td>
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
					</div>

				</div><!--/span-->
			</div><!--/row-->
{/block}
