{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
	<div class="well span5 center login-box">
		<div class="alert alert-success">
			{$logout_message}
		</div>
		
		
		<div class="alert alert-info">
		<strong>{$login_title}</strong>
		<p>{$login_timer}<span id="seconds">5</span></p>
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

