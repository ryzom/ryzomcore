{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
	<div class="well span5 center login-box">
		<div class="alert alert-info">
			 {$welcome_message}
		</div>
		<form id="signup" class="form-vertical" method="post" action="index.php">
			<legend>{$title}</legend>
			
			<div class="control-group {if isset($USERNAME_ERROR) and $USERNAME_ERROR eq "TRUE"}error{else if
			isset($USERNAME) and $USERNAME eq "success"}success{else}{/if}">
			<label class="control-label">{$username_tag}</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-user"></i></span>
						<input type="text" class="input-xlarge" id="Username" name="Username" placeholder="{$username_default}" {if isset($prevUsername)}value="{$prevUsername}"{/if} rel="popover" data-content="{$username_tooltip}" data-original-title="{$username_default}">
					</div>
				</div>
			</div>


			<div class="control-group {if isset($PASSWORD_ERROR) and $PASSWORD_ERROR eq "TRUE"}error{else if
			isset($PASSWORD) and $PASSWORD eq "success"}success{else}{/if}">
			<label class="control-label">{$password_tag}</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-lock"></i></span>
						<input type="Password" id="Password" class="input-xlarge" name="Password" placeholder="{$password_default}"  {if isset($prevPassword)}value="{$prevPassword}"{/if} rel="popover" data-content="{$password_tooltip}" data-original-title="{$password_default}">
					</div>
				</div>
			</div>
			
			<div class="control-group {if isset($CPASSWORD_ERROR) and $CPASSWORD_ERROR eq "TRUE"}error{else if
			isset($CPASSWORD) and $CPASSWORD eq "success"}success{else}{/if}">
			<label class="control-label">{$cpassword_tag}</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-lock"></i></span>
						<input type="Password" id="ConfirmPass" class="input-xlarge" name="ConfirmPass" placeholder="{$cpassword_default}"  {if isset($prevConfirmPass)}value="{$prevConfirmPass}"{/if} rel="popover" data-content="{$cpassword_tooltip}" data-original-title="{$cpassword_default}">
					</div>
				</div>
			</div>
			
			<div class="control-group {if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}error{else if
			isset($EMAIL) and $EMAIL eq "success"}success{else}{/if}">
			<label class="control-label">{$email_tag}</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-envelope"></i></span>
						<input type="text" class="input-xlarge" id="Email" name="Email" placeholder="{$email_default}" {if isset($prevEmail)}value="{$prevEmail}"{/if} rel="popover" data-content="{$email_tooltip}" data-original-title="{$email_default}">
					</div>
				</div>
			</div>
			
			<div class="control-group {if isset($TAC_ERROR) and $TAC_ERROR eq "TRUE"}error{else if
			isset($TAC) and $TAC eq "success"}success{else}{/if}">
				<div class="controls">
				    <div class="input-prepend">
						<input type="checkbox" class="input-xlarge" id="TaC" name="TaC" placeholder="Email">{$tac_tag1}<a href='{$TOS_URL}'>{$tac_tag2}</a>
						
					</div>
				</div>
			</div>
			
			{if isset($USERNAME_ERROR) and $USERNAME_ERROR eq "TRUE"}
			<div class="alert alert-error">
				<button type="button" class="close" data-dismiss="alert">×</button>
				<strong>Username Error</strong> {$USERNAME}
			</div>
			{/if}
			
			{if isset($PASSWORD_ERROR) and $PASSWORD_ERROR eq "TRUE"}
			<div class="alert alert-error">
				<button type="button" class="close" data-dismiss="alert">×</button>
				<strong>Password Error</strong> {$PASSWORD}
			</div>
			{/if}
			
			{if isset($CPASSWORD_ERROR) and $CPASSWORD_ERROR eq "TRUE" and $CPASSWORD != ""}
			<div class="alert alert-error">
				<button type="button" class="close" data-dismiss="alert">×</button>
				<strong>Confirmation Password Error</strong> {$CPASSWORD}
			</div>
			{/if}
			
			{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}
			<div class="alert alert-error">
				<button type="button" class="close" data-dismiss="alert">×</button>
				<strong>Email Error</strong> {$EMAIL}
			</div>
			{/if}
			
			{if isset($TAC_ERROR) and $TAC_ERROR eq "TRUE"}
			<div class="alert alert-error">
				<button type="button" class="close" data-dismiss="alert">×</button>
				<strong>Terms of Service Error</strong> {$tac_message}
			</div>
			{/if}
			
			<input type="hidden" name="function" value="add_user">
			<p class="center span5">
				<button type="submit" class="btn btn-primary" >Create My Account</button>
			</p>
	
		</form>


	</div><!--/span-->
</div><!--/row-->
{/block}

	
