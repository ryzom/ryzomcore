{block name=content}
<div class="row-fluid">
				<div class="box span12">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> {$up_title}</h2>
					</div>
					<div class="box-content">
						<center><p>{$up_info}</p></center>
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th width="100">{$plugin_name}</th>
								  <th>{$plugin_version}</th>
								  <th>{$up_updated_version}</th>
								  <th width="500">{$up_description}</th>
								  <th>{$up_actions}</th>
							  </tr>
						  </thead>
						  <tbody>
							{foreach from=$plug item=element}
							<tr>
								<td class="center">{$element.plugin_name}</td>
								<td class="center">{$element.plugin_info->Version}</td>
								<td class="center">{$element.update_info->Version}</td>
								<td class="center">{$element.update_info->UpdateInfo}</td>
								<td><a href="index.php?page=plugins&action=update_plugins&id={$element.id}"><button class="btn btn-primary btn-large">Update</button></a>
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
