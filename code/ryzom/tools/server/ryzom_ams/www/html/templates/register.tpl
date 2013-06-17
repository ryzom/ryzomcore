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
		<form id="signup" class="form-vertical" method="post" action="users::add_user()">
			<legend>Register Account</legend>
			
			<div class="control-group">
			<label class="control-label">Username</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-user"></i></span>
						<input type="text" class="input-xlarge" id="Username" name="Username" placeholder="Username">
					</div>
				</div>
			</div>


			<div class="control-group">
			<label class="control-label">Password</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-lock"></i></span>
						<input type="Password" id="Password" class="input-xlarge" name="Password" placeholder="Password">
					</div>
				</div>
			</div>
			<div class="control-group">
			<label class="control-label">Confirm Password</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-lock"></i></span>
						<input type="Password" id="ConfirmPass" class="input-xlarge" name="ConfirmPass" placeholder="Re-enter Password">
					</div>
				</div>
			</div>
			
			<div class="control-group">
			<label class="control-label">Email</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><i class="icon-envelope"></i></span>
						<input type="text" class="input-xlarge" id="Email" name="Email" placeholder="Email">
					</div>
				</div>
			</div>
			
			<div class="control-group">
				<div class="controls">
				    <div class="input-prepend">
						<input type="checkbox" class="input-xlarge" id="TaC" name="TaC" placeholder="Email">{$tac_tag}
					</div>
				</div>
			</div>
			
			<div class="control-group">
			<label class="control-label"></label>
		      <div class="controls">
		       <button type="submit" class="btn btn-large btn-primary" >Create My Account</button>
	
		      </div>
	
		</div>
	
		</form>


	</div><!--/span-->
</div><!--/row-->
{/block}

	
