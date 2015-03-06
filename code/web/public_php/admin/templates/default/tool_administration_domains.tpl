
{include file="page_header.tpl"}

<table width="100%" cellpadding="2" cellspacing="0" border="0">
	<tr>
		<td align="left" valign="center"><span class="alert">{$tool_alert_message}</span></td>
		<td align="right">
			<table cellpadding="1" cellspacing="5" border="0">
				<tr>
					{section name=onemenu loop=$tool_menu}
					<td height="22" class="boxed"><a href="{$tool_menu[onemenu].uri}">{$tool_menu[onemenu].title}</a></td>
					{/section}
				</tr>
			</table>
		</td>
	</tr>
</table>

<br>

<table width="100%" border="0" cellpadding="0">
<tr>
	<td align="left" valign="top" width="70%">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="12">Domains</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Name</b></td>
			<td><b>Application</b></td>
			<td><b>AS Host</b></td>
			<td><b>AS Port</b></td>
			<td><b>MFS Web</b></td>
			<td><b>RRD Path</b></td>
			<td><b>LAS Admin Path</b></td>
			<td><b>LAS Local Path</b></td>
			<td><b>Ring DB</b></td>
			<td><b>CS DB</b></td>
			<td><b>HD Check</b></td>
		</tr>
{section name=domain loop=$tool_domain_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_domain_list[domain].domain_id}</td>
			<td><a href="tool_administration.php?toolmode=domains&toolaction=edit&domain_id={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
			<td>{$tool_domain_list[domain].domain_application}</td>
			<td>{$tool_domain_list[domain].domain_as_host}</td>
			<td>{$tool_domain_list[domain].domain_as_port}</td>
			<td>{$tool_domain_list[domain].domain_mfs_web}</td>
			<td>{$tool_domain_list[domain].domain_rrd_path}</td>
			<td>{$tool_domain_list[domain].domain_las_admin_path}</td>
			<td>{$tool_domain_list[domain].domain_las_local_path}</td>
			<td>{if $tool_domain_list[domain].domain_sql_string != ''}True{else}False{/if}</td>
			<td>{if $tool_domain_list[domain].domain_cs_sql_string != ''}True{else}False{/if}</td>
			<td>{if $tool_domain_list[domain].domain_hd_check == 1}Yes{else}No{/if}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=domains" method="post">
		<tr>
			<th colspan="10">Domain Details</th>
		</tr>
{if $tool_domain_edit_data.domain_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_domain_id" value="{$tool_domain_edit_data.domain_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_domain_name" value="{$tool_domain_edit_data.domain_name}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">Application :</td>
			<td><input type="text" name="tool_form_domain_application" value="{$tool_domain_edit_data.domain_application}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">AS Host :</td>
			<td><input type="text" name="tool_form_domain_as_host" value="{$tool_domain_edit_data.domain_as_host}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">AS Port :</td>
			<td><input type="text" name="tool_form_domain_as_port" value="{$tool_domain_edit_data.domain_as_port}" maxlength="5" size="30"></td>
		</tr>
		<tr>
			<td align="right">MFS Web :</td>
			<td><input type="text" name="tool_form_domain_mfs_web" value="{$tool_domain_edit_data.domain_mfs_web}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">RRD Path :</td>
			<td><input type="text" name="tool_form_domain_rrd_path" value="{$tool_domain_edit_data.domain_rrd_path}" maxlength="255" size="30"></td>
		</tr>
		<tr>
			<td align="right">LAS Admin Path :</td>
			<td><input type="text" name="tool_form_domain_las_admin_path" value="{$tool_domain_edit_data.domain_las_admin_path}" maxlength="255" size="30"></td>
		</tr>
		<tr>
			<td align="right">LAS Local Path :</td>
			<td><input type="text" name="tool_form_domain_las_local_path" value="{$tool_domain_edit_data.domain_las_local_path}" maxlength="255" size="30"></td>
		</tr>
		<tr>
			<td align="right">Ring DB String :</td>
			<td><input type="text" name="tool_form_domain_sql_string" value="{$tool_domain_edit_data.domain_sql_string}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">CS DB String :</td>
			<td><input type="text" name="tool_form_domain_cs_sql_string" value="{$tool_domain_edit_data.domain_cs_sql_string}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td width="40%" align="right">HD Check :</td>
			<td>
				<select name="tool_form_domain_hd_check">
					<option value="1" {if $tool_domain_edit_data.domain_hd_check == 1}selected{/if}>Yes</option>
					<option value="0" {if $tool_domain_edit_data.domain_hd_check == 0}selected{/if}>No</option>
				</select>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_domain_edit_data.domain_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE the domain &lt; {$tool_domain_edit_data.domain_name} &gt; ?')) return true; return false;">
{else}
				<input type="submit" name="toolaction" value="create">

{/if}

			</td>
		</tr>
		</form>
		</table>

{if $tool_domain_nel_data}
		<br>
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=domains" method="post">
		<tr>
			<th colspan="10">Domain Data</th>
		</tr>
		<tr>
			<td align="right">ID :</td>
			<td><input type="text" name="tool_form_domain_nel_id" value="{$tool_domain_nel_data.domain_id}" maxlength="128" size="30" readonly ></td>
		</tr>
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_domain_nel_name" value="{$tool_domain_nel_data.domain_name}" maxlength="128" size="30" readonly ></td>
		</tr>
		<!--<tr>
			<td align="right">Version :</td>
			<td><input type="text" name="tool_form_domain_nel_version" value="{$tool_domain_nel_data.patch_version}" maxlength="128" size="30"></td>
		</tr>-->
		<tr>
			<td align="right">Status :</td>
			<td><select name="tool_form_domain_nel_status" style="width:150px;">
{section name=status loop=$tool_domain_nel_status}
					<option value="{$tool_domain_nel_status[status]}" {if $tool_domain_nel_data.status == $tool_domain_nel_status[status]}selected{/if}>{$tool_domain_nel_status[status]}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>
				<input type="submit" name="toolaction" value="update_nel">
				<input type="hidden" name="tool_form_domain_id" value="{$tool_domain_edit_data.domain_id}">
			</td>
		</tr>
		</form>
		</table>
{/if}

	</td>

</tr>
</table>

{include file="page_footer.tpl"}
