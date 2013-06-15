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
		

		  <form name="Page1"method="post">
		    <table>
		      <tr>
			<td width="33%" {if isset($USERNAME_ERROR) && $USERNAME_ERROR eq "TRUE"}class="error"{/if} id="caption-Username">{$username_tag} </td>

			<td width="25%">
			  <input type="text"
			       name="Username"
			       value="{if isset($Username)}{$Username}{/if}"
			       maxlength="12"
			       onfocus=
			       "javascript:showTooltip('{$username_tooltip}', this);" />
			</td>
		
			<td id="comment-Username" {if isset($USERNAME_ERROR) && $USERNAME_ERROR eq "TRUE"}class="error"{/if} width="42%">{if isset($Username)}{$Username}{/if}</td>
		      </tr>

		      <tr>
			<td width="33%" {if isset($PASSWORD_ERROR) && $PASSWORD_ERROR eq "TRUE"}class="error"{/if} id="caption-Password">{$password_tag}</td>

			<td width="25%">
			  <input type="password"
			       name="Password"
			       value=""
			       maxlength="20"
			       onkeyup=
			       "testPassword(document.Page1.Password.value, 'comment-Password')"
			       onfocus="javascript:showTooltip('{$password_message}', this);" />
			</td>

			<td id="comment-Password" {if isset($PASSWORD_ERROR) && $PASSWORD_ERROR eq "TRUE"}class="error"{/if} width="42%">{if isset($PASSWORD_ERROR) && $PASSWORD_ERROR eq "TRUE"}{$Password}{/if}</td>
		      </tr>

		      <tr>
			<td width="33%" {if isset($CPASSWORD_ERROR) && $CPASSWORD_ERROR eq "TRUE"}class="error"{/if} id="caption-ConfirmPass">{$cpassword_tag}</td>

			<td width="25%"><input type="password"
			       name="ConfirmPass"
			       value=""
			       maxlength="20"
			       onfocus="javascript:showTooltip('{$cpassword_message}', this);" />
			</td>

			<td id="comment-ConfirmPass" {if isset($CPASSWORD_ERROR) && $CPASSWORD_ERROR eq "TRUE"}class="error"{/if} width="42%">{if isset($CPASSWORD_ERROR) && $CPASSWORD_ERROR eq "TRUE"}{$ConfirmPass}{/if}</td>
		      </tr>

		      <tr>
			<td width="33%" {if isset($CPASSWORD_ERROR) && $CPASSWORD_ERROR eq "TRUE"}class="error"{/if} id="caption-Email">{$email_tag}</td>

			<td width="25%">
			  <input type="text"
			       name="Email"
			       value=""
			       maxlength="255"
			       onfocus=
			       "javascript:showTooltip('{$email_message}', this);" />
			</td>

			<td id="comment-Email" {if isset($EMAIL_ERROR) && $EMAIL_ERROR eq "TRUE"}class="error"{/if} width="42%">{if isset($EMAIL_ERROR) && $EMAIL_ERROR eq "TRUE"}{$Email}{/if}</td>
		      </tr>

		      <tr>
			<td width=
			"33%" {if isset($TAC_ERROR) && $TAC_ERROR eq "TRUE"}class="error"{/if}
			    colspan="2"><input type="checkbox"
			       name="TaC"
			       value="1"
			       onfocus="javascript:showTooltip('', this);" /><span id=
			       "caption-TaC">{$tac_tag}</span></td>
				                <td id="comment-TaC" {if isset($TAC_ERROR) && $TAC_ERROR eq "TRUE"}class="error"{/if} width="42%"><div class="alert alert-danger">{$tac_message}</div></td>

		      </tr>
		    </table>

		    <div class="c1">
		      <input type="submit"
			   name="Submit"
			   value="Continue" />
		    </div>
		    <input type="hidden" name="function" value="add_user">
		  </form>

		  <div id="signupTooltip"
		       class="c2"
		       inset=""></div>

		  <div id="tooltip-Username">
			<div class="alert alert-danger">
		    		{$username_tooltip}
		  	</div>
		  </div>

		  <div id="tooltip-Password">
		  	<div class="alert alert-danger">
		    		{$password_message}
			</div>
		  </div>

		  <div id="tooltip-ConfirmPass">
		     	<div class="alert alert-danger">
		    		{$cpassword_message}
			</div>
		  </div>

		  <div id="tooltip-Email">
			<div class="alert alert-danger">
		    		{$email_message}
			</div>
		  </div>

		
	</div><!--/span-->
</div><!--/row-->
{/block}
