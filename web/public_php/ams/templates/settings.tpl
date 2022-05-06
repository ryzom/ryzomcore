{block name=content}
	<div class="row-fluid js-masonry" data-masonry-options='{ "itemSelector": ".js-masonry-item", "transitionDuration": 0 }'>
		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> Change Password
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="changePassword" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Password</legend>

						{if !isset($changesOther) or $changesOther eq "FALSE"}
							<div class="control-group {if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}error{else if
				isset($match_error_message) and $match_error_message neq "fail"}success{else}{/if}">
							<label class="control-label">Current Password</label>
								<div class="controls">
								    <div class="input-prepend">
									<span class="add-on" style="margin-left:5px;"><span class="icon-lock"></span></span>
										<input type="password" class="setup-width" id="CurrentPass" name="CurrentPass" placeholder="Your current password" {if isset($prevCurrentPass)}value="{$prevCurrentPass}"{/if}>
										{if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}<span class="help-inline">The password is incorrect</span>{/if}
								    </div>
								</div>
							</div>
						{/if}
							<div class="control-group {if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($newpass_error_message) and $newpass_error_message eq "success"}success{else}{/if}">
						<label class="control-label">New Password</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><span class="icon-tag"></span></span>
									<input type="password" class="setup-width" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
									{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$newpass_error_message}</span>{/if}
							   </div>
							</div>
						</div>

						<div class="control-group {if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}error{else if
			isset($confirmnewpass_error_message) and $confirmnewpass_error_message eq "success"}success{else}{/if}">
						<label class="control-label">Confirm New Password</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><span class="icon-tags"></span></span>
									<input type="password" class="setup-width" id="ConfirmNewPass" name="ConfirmNewPass" placeholder="Re-enter the new password"  {if isset($prevConfirmNewPass)}value="{$prevConfirmNewPass}"{/if}>
									{if isset($CNEWPASSWORD_ERROR) and $CNEWPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$confirmnewpass_error_message}</span>{/if}
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

						<input type="hidden" name="function" value="change_password">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Change Password</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div><!--/span-->

		{if $permission != '1'}
		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> Add User
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="addUser" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Add User</legend>

						<div class="control-group {if isset($USERNAME_ERROR) and $USERNAME_ERROR eq "TRUE"}error{/if}">
							<label class="control-label">Username</label>
								<div class="controls">
									<div class="input-prepend">
									<span style="margin-left:5px;" class="add-on"><span class="icon-user"></span></span>
										<input type="text" placeholder="Username" name="Username" id="Username" class="setup-width" {if isset($prevUsername)}value="{$prevUsername}"{/if}>
										{if isset($USERNAME_ERROR) and $USERNAME_ERROR eq "TRUE"}<span class="help-inline">{$USERNAME}</span>{/if}
									</div>
								</div>
						</div>

						<div class="control-group {if isset($PASSWORD_ERROR) and $PASSWORD_ERROR eq "TRUE"}error{/if}">
							<label class="control-label">Password</label>
								<div class="controls">
								    <div class="input-prepend">
										<span style="margin-left:5px;" class="add-on"><span class="icon-lock"></span></span>
										<input type="password" placeholder="Password" name="Password" id="Password" class="setup-width">
										{if isset($PASSWORD_ERROR) and $PASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$PASSWORD}</span>{/if}
									 </div>
								</div>
						</div>

						<div class="control-group {if isset($CPASSWORD_ERROR) and $CPASSWORD_ERROR eq "TRUE"}error{/if}">
							<label class="control-label">Confirm Password</label>
								<div class="controls">
								    <div class="input-prepend">
										<span style="margin-left:5px;" class="add-on"><span class="icon-lock"></span></span>
										<input type="password" placeholder="Confirm Password" name="ConfirmPass" id="ConfirmPass" class="setup-width">
										{if isset($CPASSWORD_ERROR) and $CPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$CPASSWORD}</span>{/if}
									 </div>
								</div>
						</div>

						<div class="control-group {if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE" and isset($do)}error{/if}">
						<label class="control-label">Email</label>
							<div class="controls">
							    <div class="input-prepend">
								<span style="margin-left:5px;" class="add-on"><span class="icon-envelope"></span></span>
									<input type="text" class="setup-width" id="Email" name="Email" placeholder="Email" {if isset($prevEmail)}value="{$prevEmail}"{/if}>
									{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE" and isset($do)}<span class="help-inline">{$EMAIL}</span>{/if}
							    </div>
							</div>
						</div>


						{if isset($SUCCESS_ADD) and $SUCCESS_ADD eq "ok"}
						<div class="alert alert-success">
							The user has been created!
						</div>
						{/if}

						{if isset($SUCCESS_ADD) and $SUCCESS_ADD eq "shardoffline"}
						<div class="alert alert-warning">
							The user is created, though the shard seems offline, it may take some time to see the change on the shard.
						</div>
						{/if}

						<input type="hidden" name="function" value="add_user">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Create User</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div><!--/span-->
		{/if}

		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> Change Email
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Email</legend>
						<div class="control-group {if isset($CEMAIL_ERROR) and $CEMAIL_ERROR eq "TRUE"}error{/if}">
						<label class="control-label">New Email</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><span class="icon-envelope"></span></span>
									<input type="text" class="setup-width" id="NewEmail" name="NewEmail" placeholder="Your new email" {if isset($prevNewEmail)}value="{$prevNewEmail}"{else if isset($current_mail)}value="{$current_mail}"{/if}>
									{if isset($CEMAIL_ERROR) and $CEMAIL_ERROR eq "TRUE"}<span class="help-inline">{$EMAIL}</span>{/if}

							    </div>
							</div>
						</div>


						{if isset($SUCCESS_MAIL) and $SUCCESS_MAIL eq "OK"}
						<div class="alert alert-success">
							The email has been changed!
						</div>
						{/if}

						{if isset($SUCCESS_MAIL) and $SUCCESS_MAIL eq "SHARDOFF"}
						<div class="alert alert-warning">
							The email has been changed, though the shard seems offline, it may take some time to see the change on the shard.
						</div>
						{/if}

						<input type="hidden" name="function" value="change_mail">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Change Email</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div>

		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-envelope"></span> Ticket updates
			</div>


			<div class="panel-body">
				<div class="row-fluid">
					<form id="changeReceiveMail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Ticket-Update Mail Settings</legend>

						<div class="control-group">
							<label class="control-label">Receive ticket updates</label>
							<div class="controls">
								 <select name="ReceiveMail">
									<option value="1" {if isset($ReceiveMail) and $ReceiveMail eq 1}selected="selected"{/if}>Yes</option>
									<option value="0" {if isset($ReceiveMail) and $ReceiveMail eq 0}selected="selected"{/if}>No</option>
								</select>
							</div>

						</div>

						<input type="hidden" name="function" value="change_receivemail">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Change Updates</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div><!--/span-->

		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> Change Info
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Info</legend>

						<div class="control-group">
						<label class="control-label">Firstname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><span class="icon-user"></span></span>
									<input type="text" class="setup-width" id="FirstName" name="FirstName" placeholder="Your firstname" {if isset($FirstName) and $FirstName neq ""}value="{$FirstName}"{/if}>
								</div>
							</div>
						</div>

						<div class="control-group">
						<label class="control-label">Lastname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><span class="icon-user"></span></span>
									<input type="text" class="setup-width" id="LastName" name="LastName" placeholder="Your lastname" {if isset($LastName) and $LastName neq ""}value="{$LastName}"{/if}>
								</div>
							</div>
						</div>

						<div class="control-group">
						<label class="control-label">Country</label>
							<div class="controls">
								 <select name="Country">
									{foreach from=$country_array key=k item=v}
										<option value="{$k}" {if isset($Country) and $Country eq $k}selected="selected"{/if}>{$v}</option>
									{/foreach}
								</select>
							</div>
						</div>

						<div class="control-group gender-list">
							<label class="control-label">Gender</label>
							<div class="controls">
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($Gender) and $Gender eq 0}checked{/if}"><input name="Gender" id="optionsRadios0" value="0" {if isset($Gender) and $Gender eq 0}checked=""{/if} type="radio"></span></div>
								      Secret
								</label>
								<div style="clear:both"></div>
								<label class="radio">
									<div id="uniform-optionsRadios1" class="radio"><span class="{if isset($Gender) and $Gender eq 1}checked{/if}"><input name="Gender" id="optionsRadios1" value="1" {if isset($Gender) and $Gender eq 1}checked=""{/if} type="radio"></span></div>
									Male
								</label>
								<div style="clear:both"></div>
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($Gender) and $Gender eq 2}checked{/if}"><input name="Gender" id="optionsRadios2" value="2" {if isset($Gender) and $Gender eq 2}checked=""{/if} type="radio"></span></div>
								      Female
								</label>
							</div>
						</div>

						{if isset($info_updated) and $info_updated eq "OK"}
						<div class="alert alert-success">
							The Info has been updated!
						</div>
						{/if}

						<input type="hidden" name="function" value="change_info">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Change Info</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div><!--/span-->
		
		{if $permission == '3'}
		<div class="col-sm-4 js-masonry-item">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> User Registration
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="userRegistration" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>User Registration</legend>

						<div class="control-group gender-list">
							<div class="controls">
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($userRegistration) and $userRegistration eq 0}checked{/if}"><input name="userRegistration" id="optionsRadios0" value="0" {if isset($userRegistration) and $userRegistration eq 0}checked=""{/if} type="radio"></span></div>
								      In-Game and Web Registration
								</label>
								<div style="clear:both"></div>
								<label class="radio">
									<div id="uniform-optionsRadios1" class="radio"><span class="{if isset($userRegistration) and $userRegistration eq 1}checked{/if}"><input name="userRegistration" id="optionsRadios1" value="1" {if isset($userRegistration) and $userRegistration eq 1}checked=""{/if} type="radio"></span></div>
									In-Game Registration Only
								</label>
								<div style="clear:both"></div>
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($userRegistration) and $userRegistration eq 2}checked{/if}"><input name="userRegistration" id="optionsRadios2" value="2" {if isset($userRegistration) and $userRegistration eq 2}checked=""{/if} type="radio"></span></div>
								      Web Registration Only
								</label>
								<div style="clear:both"></div>
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class="{if isset($userRegistration) and $userRegistration eq 3}checked{/if}"><input name="userRegistration" id="optionsRadios2" value="3" {if isset($userRegistration) and $userRegistration eq 3}checked=""{/if} type="radio"></span></div>
								      No Registration
								</label>
							</div>
						</div>

						<input type="hidden" name="function" value="userRegistration">
						<input type="hidden" name="target_id" value="{$target_id}">
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px;">Change Setting</button>
							</div>
						</div>
					</form>
				</div>
			</div>
			</div>
		</div><!--/span-->
		{/if}
		
	</div><!--/row-->

	<script src="js/masonry.pkgd.min.js"></script>

{/block}

