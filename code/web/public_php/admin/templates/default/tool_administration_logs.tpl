
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
			<th colspan="10">Logs</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>User</b></td>
			<td><b>Date</b></td>
			<td><b>Action</b></td>
		</tr>
{section name=log loop=$tool_log_list}
		<tr class="{cycle values="row1,row0"}">
			<td>{$tool_log_list[log].logs_id}</td>
			<td>{$tool_log_list[log].logs_user_name}</td>
			<td>{$tool_log_list[log].logs_date|date_format:"%Y/%m/%d %H:%M:%S"}</td>
			<td>{$tool_log_list[log].logs_data}</td>
		</tr>
{/section}
		<tr>
			<th colspan="4" align="center">
<a href="tool_administration.php?toolmode=logs&page={$tool_log_page_first}">|&lt;</a>&nbsp;&nbsp;&nbsp;
<a href="tool_administration.php?toolmode=logs&page={$tool_log_page_previous}">&lt;</a>&nbsp;&nbsp;&nbsp;
Page {$tool_log_page_current} / {$tool_log_page_total}&nbsp;&nbsp;&nbsp;
<a href="tool_administration.php?toolmode=logs&page={$tool_log_page_next}">&gt;</a>&nbsp;&nbsp;&nbsp;
<a href="tool_administration.php?toolmode=logs&page={$tool_log_page_last}">&gt;|</a>
			</th>
		</tr>
		</table>
	</td>
</tr>
</table>

{include file="page_footer.tpl"}
