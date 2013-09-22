{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
	<div class="well span5 center login-box">
		{if isset($SUCCESS_PASS) and $SUCCESS_PASS eq "OK"}
		<div class="alert alert-success">
			The password has been changed!
		</div>
		{/if}
		
		{if isset($SUCCESS_PASS) and $SUCCESS_PASS eq "SHARDOFF"}
		<div class="alert alert-warning">
			The password has been changed, though the shard seems offline, it may take some time to see the change on the shard.
		</div>
		{/if}
		
		<div class="alert alert-info">
		<strong>{$reset_success_title}</strong>
		<p>{$reset_success_timer}<span id="seconds">5</span></p>
		<p><a href="index.php">{$login_text}</a></p>
		</div>
		
		<script>
		var seconds = 5;
		setInterval(
		  function(){
		    if (seconds <= 1) {
		      window.location = 'index.php';
		    }
		    else {
		      document.getElementById('seconds').innerHTML = --seconds;
		    }
		  },
		  1000
		);
	      </script>
	</div><!--/span-->
</div>
{/block}

