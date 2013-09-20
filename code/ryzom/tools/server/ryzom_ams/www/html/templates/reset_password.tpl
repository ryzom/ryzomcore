{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
	<div class="well span5 center login-box">

		<form id="signup" class="form-vertical" method="post" action="index.php{$getstring}">
			<legend>{$title}</legend>

				<div class="control-group {if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($newpass_error_message) and $newpass_error_message eq "success"}success{else}{/if}">
				<label class="control-label">New Password</label>
					<div class="controls">
					    <div class="input-prepend">
						<span class="add-on" style="margin-left:5px;"><i class="icon-tag"></i></span>
							<input type="password" class="input-xlarge" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
							{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<br/><span class="help-inline">{$newpass_error_message}</span>{/if}
					   </div>
					</div>
				</div>
					
				<div class="control-group {if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($confirmnewpass_error_message) and $confirmnewpass_error_message eq "success"}success{else}{/if}">
				<label class="control-label">Confirm New Password</label>
					<div class="controls">
					    <div class="input-prepend">
						<span class="add-on" style="margin-left:5px;"><i class="icon-tags"></i></span>
							<input type="password" class="input-xlarge" id="ConfirmNewPass" name="ConfirmNewPass" placeholder="Re-enter the new password"  {if isset($prevConfirmNewPass)}value="{$prevConfirmNewPass}"{/if}>
							{if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}<br/><span class="help-inline">{$confirmnewpass_error_message}</span>{/if}
					    </div>
					</div>
				</div>
				
				
			
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
			
			<input type="hidden" name="function" value="reset_password">
			<div class="control-group">
				<label class="control-label"></label>
				<div class="controls">
					<button type="submit" class="btn btn-large btn-primary" >Reset the password!</button>
	
				</div>
			</div>
	
		</form>


	</div><!--/span-->
</div><!--/row-->
{/block}

	
