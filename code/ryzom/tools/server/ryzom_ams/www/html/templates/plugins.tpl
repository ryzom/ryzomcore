 
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
					<div class="box-content">
						<center><p>{$plugin_info}</p></center>

						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>{$plugin_id}</th>
								  <th>{$plugin_permission}</th>
								  <th>{$plugin_name}</th>
								  <th>{$plugin_version}</th>
								  <th>{$plugin_is_active}</th>
							  </tr>
						  </thead>   
						  <tbody>
							{foreach from=$plug item=element}
							<tr>
								<td>{$element.id}</td>
								<td class="center">{$element.plugin_permission}</td>
								<td class="center">{$element.plugin_name}</td>
								<td class="center">{$element.plugin_version}</td>
								<td class="center">{$element.plugin_isactive}</td>
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
				<div class="box span3">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i>Actions</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
		<div class="btn-group">
                <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
                <ul class="dropdown-menu">
		    <li class="divider"></li>
		    <li><a href="">Edit Plugins</a></li>
		    <li><a href="">Add Plugin</a></li>
		    <li class="divider"></li>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Make Moderator</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 2 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 3 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Demote to Moderator</a></li>
			{/if}
			<li class="divider"></li>
		    {/if}
		    
                </ul>
              </div>
            </div>                   
        </div>
    </div><!--/span-->
			
			</div><!--/row-->
{/block}