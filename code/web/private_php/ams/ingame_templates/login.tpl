		<p>&nbsp;</p>
		<table cellpadding = "10"><tr><td>
		<div class="alert alert-info">
			<h3>{$login_info}</h3>
		</div>
		<form method="post" action="{$ingame_webpath}?page=login" class="form-horizontal">
			<br/>
			<p>
				Username:
				<input type="text" value="" id="Username" name="Username" class="input-large span10" placeholder="Username">
			</p>

			<p>
				Password:
				<input type="text" value="" id="Password" name="Password" class="input-large span10" placeholder="Password">
			</p>
			
			<p>
				Remember me:
				<label for="remember" class="remember"><div class="checker" id="uniform-remember"><span><input type="checkbox" id="remember" style="opacity: 0;"></span></div>Remember me</label>
			</p>
		
			<p>
			<input type="hidden" name="function" value="login"/>
			<input type="submit" value="Login"/>
			</p>

		</form>
		
		{if isset($login_error) and $login_error eq "TRUE"}
		<p>
			<strong><font color="red">{$login_error_message}</font></strong>
		</p>
		{/if}
		<p>
		<font color="green">{$login_register_message} <a href="{$ingame_webpath}?page=register">{$login_register_message_here}</a></font>!
		</p>
		</td></tr>
		</table>
