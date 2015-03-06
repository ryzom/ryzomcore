{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<a href="?"><img src="img/mainlogo.png"/></a>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
	<div class="well span5 center login-box">
		<div class="alert alert-info">
			{$login_info}
		</div>
		
		<form method="post" action="index.php{if isset($getstring)}{$getstring}{/if}" class="form-horizontal">
			<fieldset>
				<div data-rel="tooltip" class="input-prepend" data-original-title="Username/Email">
					<span class="add-on"><span class="icon-user"></span></span><input type="text" value="" id="LoginValue" name="LoginValue" class="input-large span10" placeholder="Username or Email">
				</div>
				<div class="clearfix"></div>

				<div data-rel="tooltip" class="input-prepend" data-original-title="Password">
					<span class="add-on"><span class="icon-lock"></span></span><input type="password" value="" id="Password" name="Password" class="input-large span10" placeholder="Password">
				</div>
				<div class="clearfix"></div>

				<div class="input-prepend">
				<label for="remember" class="remember "><div class="checkbox" id="uniform-remember"><span><input type="checkbox" id="remember" ></span></div>Remember me</label>
				</div>
				<div class="clearfix"></div>

				<p class="center span5">
				<input type="hidden" name="function" value="login">
				<button class="btn btn-primary" type="submit">Login</button>
				</p>
			</fieldset>
		</form>

		{if isset($login_error) and $login_error eq "TRUE"}
		<div class="alert alert-error">
			<button type="button" class="close" data-dismiss="alert">×</button>
			<strong>{$login_error_message}</strong>
		</div>
		{/if}
		
		
		<div class="alert alert-info">
		{if $userRegistration == '0'|| $userRegistration == '2'}<a href="?page=register">{$login_register_message}</a>.<br>{/if}<a href="?page=forgot_password">{$login_forgot_password_message}</a>
		</div>
		
		
	</div><!--/span-->
</div>
{/block}

