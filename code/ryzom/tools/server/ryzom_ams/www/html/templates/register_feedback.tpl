{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
				<div class="well span5 center login-box">
					{if isset($status) and $status eq "ok"}
						<div class="alert alert-success">
							{$status_ok}
						</div>
					{else if isset($status) and $status eq "shardoffline"}
						<div class="alert alert-error">
							{$status_shardoffline}
						</div>
					{else if isset($status) and $status eq "liboffline"}
						<div class="alert alert-error">
							{$status_liboffline}
						</div>
					{/if}
					
					<div class="alert alert-info">
					<strong>{$login_title}</strong>
					<a href="index.php?page=register">{$login_text}</a>
					</div>
				</div><!--/span-->
			</div>
{/block}

