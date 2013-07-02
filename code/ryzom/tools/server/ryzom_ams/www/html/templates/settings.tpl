{block name=content}
	<div class="row-fluid sortable ui-sortable">
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Password</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changePassword" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Password</legend>
						
						{if !isset($isAdmin) or $isAdmin eq "FALSE"}
							<div class="control-group {if isset($MATCH_ERROR) and $MATCH_ERROR eq "TRUE"}error{else if
				isset($match_error_message) and $match_error_message neq "fail"}success{else}{/if}">
							<label class="control-label">Current Password</label>
								<div class="controls">
								    <div class="input-prepend">
									<span class="add-on" style="margin-left:5px;"><i class="icon-lock"></i></span>
										<input type="password" class="input-xlarge" id="CurrentPass" name="CurrentPass" placeholder="Your current password" {if isset($prevCurrentPass)}value="{$prevCurrentPass}"{/if}>
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
								<span class="add-on" style="margin-left:5px;"><i class="icon-tag"></i></span>
									<input type="password" class="input-xlarge" id="NewPass" name="NewPass" placeholder="Your new password"  {if isset($prevNewPass)}value="{$prevNewPass}"{/if}>
									{if isset($NEWPASSWORD_ERROR) and $NEWPASSWORD_ERROR eq "TRUE"}<span class="help-inline">{$newpass_error_message}</span>{/if}
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
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Password</button>
							</div>
						</div>
					</form>		
				</div>                   
			</div>
		</div><!--/span-->
				
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Email</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php?page=settings&id={$target_id}">
						<legend>Change Email</legend>
						<div class="control-group {if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}error{/if}">
						<label class="control-label">New Email</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-envelope"></i></span>
									<input type="text" class="input-xlarge" id="NewEmail" name="NewEmail" placeholder="Your new email" {if isset($prevNewEmail)}value="{$prevNewEmail}"{else if isset($current_mail)}value="{$current_mail}"{/if}>
									{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}<span class="help-inline">{$EMAIL}</span>{/if}
						
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
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Email</button>
							</div>
						</div>
					</form>
				</div>                   
			</div>
		</div><!--/span-->
				
		<div class="box span4">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Change Info</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="changeEmail" class="form-vertical" method="post" action="index.php">
						<legend>Change Info</legend>
						
						<div class="control-group">
						<label class="control-label">Firstname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-user"></i></span>
									<input type="text" class="input-xlarge" id="Firstname" name="Firstname" placeholder="Your firstname">
								</div>
							</div>
						</div>
						
						<div class="control-group">
						<label class="control-label">Lastname</label>
							<div class="controls">
							    <div class="input-prepend">
								<span class="add-on" style="margin-left:5px;"><i class="icon-user"></i></span>
									<input type="text" class="input-xlarge" id="Lastname" name="Lastname" placeholder="Your lastname">
								</div>
							</div>
						</div>
						
						<div class="control-group">
						<label class="control-label">Country</label>
							<div class="controls">
								 <select>
									<option value="AA" selected="selected">Select one</option>
									<option value="AF">Afghanistan</option>
									<option value="AX">Åland Islands</option>
									<option value="AL">Albania</option>
									<option value="DZ">Algeria</option>
									<option value="AS">American Samoa</option>
									<option value="AD">Andorra</option>
									<option value="AO">Angola</option>
									<option value="AI">Anguilla</option>
									<option value="AQ">Antarctica</option>
									<option value="AG">Antigua and Barbuda</option>
									<option value="AR">Argentina</option>
									<option value="AM">Armenia</option>
									<option value="AW">Aruba</option>
									<option value="AU">Australia</option>
									<option value="AT">Austria</option>
									<option value="AZ">Azerbaijan</option>
									<option value="BS">Bahamas</option>
									<option value="BH">Bahrain</option>
									<option value="BD">Bangladesh</option>
									<option value="BB">Barbados</option>
									<option value="BY">Belarus</option>
									<option value="BE">Belgium</option>
									<option value="BZ">Belize</option>
									<option value="BJ">Benin</option>
									<option value="BM">Bermuda</option>
									<option value="BT">Bhutan</option>
									<option value="BO">Bolivia, Plurinational State of</option>
									<option value="BQ">Bonaire, Sint Eustatius and Saba</option>
									<option value="BA">Bosnia and Herzegovina</option>
									<option value="BW">Botswana</option>
									<option value="BV">Bouvet Island</option>
									<option value="BR">Brazil</option>
									<option value="IO">British Indian Ocean Territory</option>
									<option value="BN">Brunei Darussalam</option>
									<option value="BG">Bulgaria</option>
									<option value="BF">Burkina Faso</option>
									<option value="BI">Burundi</option>
									<option value="KH">Cambodia</option>
									<option value="CM">Cameroon</option>
									<option value="CA">Canada</option>
									<option value="CV">Cape Verde</option>
									<option value="KY">Cayman Islands</option>
									<option value="CF">Central African Republic</option>
									<option value="TD">Chad</option>
									<option value="CL">Chile</option>
									<option value="CN">China</option>
									<option value="CX">Christmas Island</option>
									<option value="CC">Cocos (Keeling) Islands</option>
									<option value="CO">Colombia</option>
									<option value="KM">Comoros</option>
									<option value="CG">Congo</option>
									<option value="CD">Congo, the Democratic Republic of the</option>
									<option value="CK">Cook Islands</option>
									<option value="CR">Costa Rica</option>
									<option value="CI">Côte d'Ivoire</option>
									<option value="HR">Croatia</option>
									<option value="CU">Cuba</option>
									<option value="CW">Curaçao</option>
									<option value="CY">Cyprus</option>
									<option value="CZ">Czech Republic</option>
									<option value="DK">Denmark</option>
									<option value="DJ">Djibouti</option>
									<option value="DM">Dominica</option>
									<option value="DO">Dominican Republic</option>
									<option value="EC">Ecuador</option>
									<option value="EG">Egypt</option>
									<option value="SV">El Salvador</option>
									<option value="GQ">Equatorial Guinea</option>
									<option value="ER">Eritrea</option>
									<option value="EE">Estonia</option>
									<option value="ET">Ethiopia</option>
									<option value="FK">Falkland Islands (Malvinas)</option>
									<option value="FO">Faroe Islands</option>
									<option value="FJ">Fiji</option>
									<option value="FI">Finland</option>
									<option value="FR">France</option>
									<option value="GF">French Guiana</option>
									<option value="PF">French Polynesia</option>
									<option value="TF">French Southern Territories</option>
									<option value="GA">Gabon</option>
									<option value="GM">Gambia</option>
									<option value="GE">Georgia</option>
									<option value="DE">Germany</option>
									<option value="GH">Ghana</option>
									<option value="GI">Gibraltar</option>
									<option value="GR">Greece</option>
									<option value="GL">Greenland</option>
									<option value="GD">Grenada</option>
									<option value="GP">Guadeloupe</option>
									<option value="GU">Guam</option>
									<option value="GT">Guatemala</option>
									<option value="GG">Guernsey</option>
									<option value="GN">Guinea</option>
									<option value="GW">Guinea-Bissau</option>
									<option value="GY">Guyana</option>
									<option value="HT">Haiti</option>
									<option value="HM">Heard Island and McDonald Islands</option>
									<option value="VA">Holy See (Vatican City State)</option>
									<option value="HN">Honduras</option>
									<option value="HK">Hong Kong</option>
									<option value="HU">Hungary</option>
									<option value="IS">Iceland</option>
									<option value="IN">India</option>
									<option value="ID">Indonesia</option>
									<option value="IR">Iran, Islamic Republic of</option>
									<option value="IQ">Iraq</option>
									<option value="IE">Ireland</option>
									<option value="IM">Isle of Man</option>
									<option value="IL">Israel</option>
									<option value="IT">Italy</option>
									<option value="JM">Jamaica</option>
									<option value="JP">Japan</option>
									<option value="JE">Jersey</option>
									<option value="JO">Jordan</option>
									<option value="KZ">Kazakhstan</option>
									<option value="KE">Kenya</option>
									<option value="KI">Kiribati</option>
									<option value="KP">Korea, Democratic People's Republic of</option>
									<option value="KR">Korea, Republic of</option>
									<option value="KW">Kuwait</option>
									<option value="KG">Kyrgyzstan</option>
									<option value="LA">Lao People's Democratic Republic</option>
									<option value="LV">Latvia</option>
									<option value="LB">Lebanon</option>
									<option value="LS">Lesotho</option>
									<option value="LR">Liberia</option>
									<option value="LY">Libya</option>
									<option value="LI">Liechtenstein</option>
									<option value="LT">Lithuania</option>
									<option value="LU">Luxembourg</option>
									<option value="MO">Macao</option>
									<option value="MK">Macedonia, the former Yugoslav Republic of</option>
									<option value="MG">Madagascar</option>
									<option value="MW">Malawi</option>
									<option value="MY">Malaysia</option>
									<option value="MV">Maldives</option>
									<option value="ML">Mali</option>
									<option value="MT">Malta</option>
									<option value="MH">Marshall Islands</option>
									<option value="MQ">Martinique</option>
									<option value="MR">Mauritania</option>
									<option value="MU">Mauritius</option>
									<option value="YT">Mayotte</option>
									<option value="MX">Mexico</option>
									<option value="FM">Micronesia, Federated States of</option>
									<option value="MD">Moldova, Republic of</option>
									<option value="MC">Monaco</option>
									<option value="MN">Mongolia</option>
									<option value="ME">Montenegro</option>
									<option value="MS">Montserrat</option>
									<option value="MA">Morocco</option>
									<option value="MZ">Mozambique</option>
									<option value="MM">Myanmar</option>
									<option value="NA">Namibia</option>
									<option value="NR">Nauru</option>
									<option value="NP">Nepal</option>
									<option value="NL">Netherlands</option>
									<option value="NC">New Caledonia</option>
									<option value="NZ">New Zealand</option>
									<option value="NI">Nicaragua</option>
									<option value="NE">Niger</option>
									<option value="NG">Nigeria</option>
									<option value="NU">Niue</option>
									<option value="NF">Norfolk Island</option>
									<option value="MP">Northern Mariana Islands</option>
									<option value="NO">Norway</option>
									<option value="OM">Oman</option>
									<option value="PK">Pakistan</option>
									<option value="PW">Palau</option>
									<option value="PS">Palestinian Territory, Occupied</option>
									<option value="PA">Panama</option>
									<option value="PG">Papua New Guinea</option>
									<option value="PY">Paraguay</option>
									<option value="PE">Peru</option>
									<option value="PH">Philippines</option>
									<option value="PN">Pitcairn</option>
									<option value="PL">Poland</option>
									<option value="PT">Portugal</option>
									<option value="PR">Puerto Rico</option>
									<option value="QA">Qatar</option>
									<option value="RE">Réunion</option>
									<option value="RO">Romania</option>
									<option value="RU">Russian Federation</option>
									<option value="RW">Rwanda</option>
									<option value="BL">Saint Barthélemy</option>
									<option value="SH">Saint Helena, Ascension and Tristan da Cunha</option>
									<option value="KN">Saint Kitts and Nevis</option>
									<option value="LC">Saint Lucia</option>
									<option value="MF">Saint Martin (French part)</option>
									<option value="PM">Saint Pierre and Miquelon</option>
									<option value="VC">Saint Vincent and the Grenadines</option>
									<option value="WS">Samoa</option>
									<option value="SM">San Marino</option>
									<option value="ST">Sao Tome and Principe</option>
									<option value="SA">Saudi Arabia</option>
									<option value="SN">Senegal</option>
									<option value="RS">Serbia</option>
									<option value="SC">Seychelles</option>
									<option value="SL">Sierra Leone</option>
									<option value="SG">Singapore</option>
									<option value="SX">Sint Maarten (Dutch part)</option>
									<option value="SK">Slovakia</option>
									<option value="SI">Slovenia</option>
									<option value="SB">Solomon Islands</option>
									<option value="SO">Somalia</option>
									<option value="ZA">South Africa</option>
									<option value="GS">South Georgia and the South Sandwich Islands</option>
									<option value="SS">South Sudan</option>
									<option value="ES">Spain</option>
									<option value="LK">Sri Lanka</option>
									<option value="SD">Sudan</option>
									<option value="SR">Suriname</option>
									<option value="SJ">Svalbard and Jan Mayen</option>
									<option value="SZ">Swaziland</option>
									<option value="SE">Sweden</option>
									<option value="CH">Switzerland</option>
									<option value="SY">Syrian Arab Republic</option>
									<option value="TW">Taiwan, Province of China</option>
									<option value="TJ">Tajikistan</option>
									<option value="TZ">Tanzania, United Republic of</option>
									<option value="TH">Thailand</option>
									<option value="TL">Timor-Leste</option>
									<option value="TG">Togo</option>
									<option value="TK">Tokelau</option>
									<option value="TO">Tonga</option>
									<option value="TT">Trinidad and Tobago</option>
									<option value="TN">Tunisia</option>
									<option value="TR">Turkey</option>
									<option value="TM">Turkmenistan</option>
									<option value="TC">Turks and Caicos Islands</option>
									<option value="TV">Tuvalu</option>
									<option value="UG">Uganda</option>
									<option value="UA">Ukraine</option>
									<option value="AE">United Arab Emirates</option>
									<option value="GB">United Kingdom</option>
									<option value="US">United States</option>
									<option value="UM">United States Minor Outlying Islands</option>
									<option value="UY">Uruguay</option>
									<option value="UZ">Uzbekistan</option>
									<option value="VU">Vanuatu</option>
									<option value="VE">Venezuela, Bolivarian Republic of</option>
									<option value="VN">Viet Nam</option>
									<option value="VG">Virgin Islands, British</option>
									<option value="VI">Virgin Islands, U.S.</option>
									<option value="WF">Wallis and Futuna</option>
									<option value="EH">Western Sahara</option>
									<option value="YE">Yemen</option>
									<option value="ZM">Zambia</option>
									<option value="ZW">Zimbabwe</option>
								</select>	
							</div>
						</div>
						
						<div class="control-group">
							<label class="control-label">Gender</label>
							<div class="controls">							
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class=""><input style="opacity: 0;" name="optionsRadios" id="optionsRadios2" value="option2" checked="" type="radio"></span></div>
								      Secret
								</label>
								<div style="clear:both"></div>
								<label class="radio">
									<div id="uniform-optionsRadios1" class="radio"><span class="checked"><input style="opacity: 0;" name="optionsRadios" id="optionsRadios1" value="option1"  type="radio"></span></div>
									Male
								</label>
								<div style="clear:both"></div>
								<label class="radio">
								      <div id="uniform-optionsRadios2" class="radio"><span class=""><input style="opacity: 0;" name="optionsRadios" id="optionsRadios2" value="option2" type="radio"></span></div>
								      Female
								</label>
							</div>
						</div>
										
												
						<input type="hidden" name="function" value="change_info">	
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Change Info</button>
							</div>
						</div>
					</form>
				</div>                   
			</div>
		</div><!--/span-->
	</div><!--/row-->
			
			
	
{/block}
	
