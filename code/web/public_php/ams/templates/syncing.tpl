{block name=content}

	<div class="row-fluid">
		<div class="box col-md-12">
		<div class="box-inner">
			<div class="box-header well">
				<h2><i class="icon-info-sign"></i>{$syncing_title}</h2>
			</div>
			<div class="box-content">
				<center>
				<p>{$syncing_info}</p>
				{if $shard eq "online"}
				<div class="alert alert-success">
					<i class="icon-refresh icon-white"></i>{$shard_online}<a href="#" id="sync" onclick="sync()">{$syncing_sync}</a>
					<script>
						function sync(){
							xmlhttp=new XMLHttpRequest();
							xmlhttp.open("POST","../../../ams_lib/cron/sync_cron.php",true);
							xmlhttp.send();
						}
					</script>
				</div>
				{else}
				<div class="alert alert-error">
					<strong><i class="icon-refresh icon-white"></i></strong> {$shard_offline}
				</div>
				{/if}
				</center>
				<div class="clearfix"></div>
			</div>
			</div>
		</div>
	</div>
			<div class="row-fluid sortable">
				<div class="box col-md-12">
				<div class="box-inner">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> {$members}</h2>
					</div>
					<div class="box-content">
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>{$id}</th>
								  <th>{$type}</th>
							  </tr>
						  </thead>
						  <tbody>
							{foreach from=$liblist item=element}
							<tr>
								<td>{$element.id}</td>
								<td class="center">{$element.type}</td>


							</tr>
							{/foreach}

						  </tbody>
					  </table>
						<div style="width: 300px; margin:0px auto;">
							<ul class="pagination">
								<li><a href="index.php?page=syncing&pagenum=1">&laquo;</a></li>
								{foreach from=$links item=link}
								<li {if $link == $currentPage}class="active"{/if}><a href="index.php?page=syncing&pagenum={$link}">{$link}</a></li>
								{/foreach}
								<li><a href="index.php?page=syncing&pagenum={$lastPage}">&raquo;</a></li>
							</ul>
						</div>
					</div>
					</div>
				</div><!--/span-->

			</div><!--/row-->
{/block}

