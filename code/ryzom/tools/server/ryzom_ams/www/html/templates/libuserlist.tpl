{block name=content}
	
	<div class="row-fluid">
		<div class="box span12">
			<div class="box-header well">
				<h2><i class="icon-info-sign"></i>The users in the libDB</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-round" onclick="javascript:show_help('intro');return false;"><i class="icon-info-sign"></i></a>
					<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<table border="1" cellpadding="5">
				<tr><td><strong>ID</strong></td><td><strong>type</strong></td><td><strong>user</strong></td><td><strong>email</strong></td><td><strong>remove</strong></td></tr>
				{foreach from=$liblist item=element}
					<tr><td>{$element.id}</td><td>{$element.type}</td><td>{$element.name}</td><td>{$element.mail}</td><td><a class="btn btn-danger" href="index.php?page=libuserlist&action=remove&id={$element.id}"><i class="icon-trash icon-white"></i>Delete</a></td></tr>
				{/foreach}
				</table>
				<div class="clearfix"></div>
			</div>
		</div>
	</div>
{/block}

