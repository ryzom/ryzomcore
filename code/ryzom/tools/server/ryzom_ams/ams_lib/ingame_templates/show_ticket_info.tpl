{block name=content}
<tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="{$second_menu_bg_color}" valign="middle">
	  <td>
	  <table>
	    <tr>
	      <td>
		<table cellspacing="0" cellpadding="4">
		  <tr>		    
			{if isset($isMod) and $isMod eq "TRUE"}<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket_log&id={$ticket_id}">Show Ticket Log</a></td>{/if}
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket&id={$ticket_id}">Show Ticket</a></td>
		  </tr>
		</table>
	      </td>
	    </tr>
	  </table>
	  </td>
	</tr>
	<tr>
		<td height="3" bgcolor="#000000"></td>
	</tr>
      </table>
  </td></tr>

  <tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Additional Info For Ticket <a href="{$ingame_webpath}?page=show_ticket&id={$ticket_id}">[#{$ticket_id}]</a></h1></td>
	</tr>
	<tr>
	  <td height="5"></td><td></td>
	</tr>
	</table>
  </td></tr>
  <tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="#000000" valign="middle">
	  <td>
	    <table>
	      <tr><td height="8"></td></tr>
	    </table>
	  </td>
	</tr>
	<tr><td height="2"></td></tr>
	<tr><td height="1" bgcolor="#000000"></td></tr>
	<tr><td height="10"></td></tr>
	<tr valign="middle">
	  <td>
	    <table width="100%" height="100%" cellpadding="10">
	      <tr><td>
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10" width="100%">
					<tr><td>
					    <table cellpadding="5" width="100%">
						    <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">
							    <tr><td>
							    <p><h3>Ingame related</h3></p>
							    <table cellpadding="3" width="100%">
								<tr><td></td><td></td></tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/shard.png"/></td>
								    <td><font color="{$info_color}">Shard ID: </font>{$shard_id}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/user.png"/></td>
								    <td><font color="{$info_color}">User_Id:  </font>{$user_id}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/position.png"/></td>
								    <td><font color="{$info_color}">User Position: </font>{$user_position}</td> 
							        </tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/view.png"/></td>
								    <td><font color="{$info_color}">View Position: </font>{$view_position}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/client.png"/></td>
								    <td><font color="{$info_color}">Client_Version: </font>{$client_version}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/patch.png"/></td>
								    <td><font color="{$info_color}">Patch_Version:  </font>{$patch_version}</td>                  
								</tr>
								<tr>
								    <td><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/server.png"/></td>
								    <td><font color="{$info_color}"> Server_Tick:  </font>{$server_tick}</td>
								</tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
						    <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">
							    <tr><td>
							    <p><h3>Hardware & Software related</h3></p>
							    <table cellpadding="3" width="100%">
								<tr><td></td><td></td></tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/memory.png"/></td>
								    <td><font color="{$info_color}">Memory: </font>{$memory}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/processor.png"/></td>
								    <td><font color="{$info_color}">Processor:  </font>{$processor}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/cpuid.png"/></td>
								    <td><font color="{$info_color}">Cpu_Id: </font>{$cpu_id}</td> 
							        </tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/mask.png"/></td>
								    <td><font color="{$info_color}">Cpu_Mask: </font>{$cpu_mask}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/ht.png"/></td>
								    <td><font color="{$info_color}">HT: </font>{$ht}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/os.png"/></td>
								    <td><font color="{$info_color}">OS: </font>{$os}</td>                  
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/nel.png"/></td>
								    <td><font color="{$info_color}">NeL3D: </font>{$nel3d}</td>
								</tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
						    <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">
							    <tr><td>
							    <p><h3>Network related</h3></p>
							    <table cellpadding="3" width="100%">
								<tr><td></td><td></td></tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/connect.png"/></td>
								    <td><font color="{$info_color}">Connect_State: </font>{$connect_state}</td>
								</tr>
								<tr>
								    <td width=30><img src="{$IMAGELOC_WEBPATH}/ams_lib/img/info/local.png"/></td>
								    <td><font color="{$info_color}">Local_Address:  </font>{$local_address}</td>
								</tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
					    </table>
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
	    </table>
	  </td>
	</tr>
      </table>
      
  </td></tr>

{/block}
	
