
{include file="page_header.tpl"}

{literal}
<script language="Javascript" type="text/javascript">
<!--
	function CheckAll()
	{
		for (var i=0; i<document.qlist.elements.length; i++)
		{
			var e = document.qlist.elements[i];
			if (e.type == 'checkbox' && e.name != 'allbox')
				e.checked = document.qlist.allbox.checked;
		}
	}

	function CheckToggle(checkname)
	{
		checkname.checked = !checkname.checked;
	}
//-->
</script>
{/literal}

<br>

<table width="100%" border="0" cellpadding="0" cellspacing="10">
<tr>
	<td align="left" valign="top" width="150px">

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Domains</th>
		</tr>
{section name=domain loop=$tool_domain_list}
		<tr>
			<td align="center" class="{if $tool_domain_selected == $tool_domain_list[domain].domain_id}domainlistselected{else}domainlist{/if}"><a href="tool_log_analyser.php?domain={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
		</tr>
{/section}
		</table>

{if $tool_domain_selected}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Shards</th>
		</tr>
{section name=shard loop=$tool_shard_list}
{if $tool_domain_selected == $tool_shard_list[shard].shard_domain_id}
		<tr>
			<td align="center" class="{if $tool_shard_selected == $tool_shard_list[shard].shard_id}shardlistselected{else}shardlist{/if}"><a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_list[shard].shard_id}">{$tool_shard_list[shard].shard_name}</a></td>
		</tr>
{/if}
{/section}
		</table>

{if $tool_file_list}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Files</th>
		</tr>
{section name=file loop=$tool_file_list}
		<tr>
			<td align="center" class="{if $tool_file_selected == $tool_file_list[file].name}shardlistselected{else}shardlist{/if}"><a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&fileview={$tool_file_list[file].code}" onmouseover="return overlib('Date: {$tool_file_list[file].date|date_format:"%Y/%m/%d %H:%M:%S"}<br>Size: {math equation="x / y" x=$tool_file_list[file].size y=1024 format="%d"} KB<br>', OFFSETX, 40, OFFSETY, 10);" onmouseout="return nd();">{$tool_file_list[file].name}</a></td>
		</tr>
{/section}
		</table>
{/if}

{/if}

	</td>

	<td width="10px">&nbsp;</td>

	<td align="right" valign="top">

{if !$tool_domain_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a domain.</td>
		</tr>
		</table>
{elseif $tool_domain_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_domain_error}</span></td>
		</tr>
		</table>
{else}
{if $tool_as_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_as_error}</span></td>
		</tr>
		</table>
		<br>
{/if}


		<form action="tool_log_analyser.php" method="post" name="qlist">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="heads"><input class="check" type="checkbox" name="allbox" value="1" onclick="CheckAll();"></td>
			<td class="heads">AliasName</td>
			<td class="heads">Shard</td>
			<td class="heads">ShortName</td>
			<td class="heads">Hostname</td>
			<td class="heads">Running State</td>
			<td class="heads">Running Tags</td>
			<td class="heads">State</td>
			<td class="heads">Report</td>
			<td class="heads">Counters</td>
			<td class="heads">User SL</td>
			<td class="heads">Tick SL</td>
			<td class="heads">Memory</td>
			<td class="heads">NbPlayers</td>
			<td class="heads">UpTime</td>
		</tr>
{assign var="las_counter" value="0"}
{section name=service loop=$tool_services_list}
{assign var="service_shard_id" value=$tool_services_list[service].ShardName}
{if $tool_shard_filters.$service_shard_id || $tool_shard_filters._all_ || ($tool_shard_filters._unknown_ && !$tool_services_list[service].ShardName)}
{cycle assign="trclass" values="row0,row1"}
{assign var="tdclass1" value=""}
{assign var="tdclass2" value=""}
{if $tool_services_list[service]._flags_.rs_stopped}{assign var="tdclass1" value="class=\"cell_inactive1\""}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_stopped"}{/if}
{if $tool_services_list[service]._flags_.rs_starting}{assign var="tdclass1" value="class=\"cell_inactive1\""}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_starting"}{/if}
{if $tool_services_list[service]._flags_.alert_red}{assign var="trclass" value="row_red"}{/if}
{if $tool_services_list[service]._flags_.alert_orange_dark}{assign var="trclass" value="row_orange_dark"}{/if}
{if $tool_services_list[service]._flags_.alert_orange_light}{assign var="trclass" value="row_orange_light"}{/if}
{assign var="check_name" value=$tool_services_list[service].AliasName}
{assign var="las_counter" value="`$las_counter+1`"}
<!-- {$egs_counter} -->
		<tr class="{$trclass}">
			<td><input class="check" type="checkbox" name="service_{$tool_services_list[service].AliasName}" value="{$tool_services_list[service].AliasName}" {if $tool_service_select_list.$check_name}checked{/if}></td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].AliasName}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{if $tool_services_list[service].ShardName != ""}{$tool_services_list[service].ShardName}{else}?{/if}{if $tool_services_list[service].ShardId != ""}/{$tool_services_list[service].ShardId}{/if}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].ShortName}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].Hostname}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningState}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningTags}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].State}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NoReportSince}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].StartCounter}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UserSpeedLoop}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].TickSpeedLoop}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].ProcessUsedMemory}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NbPlayers}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UpTime}</td>
		</tr>
{/if}
{/section}
		</table>

		<!-- ugly trick to block the first submit button being triggered when hitting ENTER to send the form -->
		<div style="display: none;"><input type="submit" name="fake" value="fake" onclick="alert('PLEASE DO NOT USE THE &lt;ENTER&gt; KEY !'); return false;"></div>
		<!-- end ugly trick :) -->

{if $las_counter > 0}

		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Database : </b></td>
			<td width="25%"><select name="service_search_database">
								<option value="0" {if $tool_form_service_search_database == 0}selected{/if} >Action Log (0)</option>
								<option value="1" {if $tool_form_service_search_database == 1}selected{/if} >Chat Log (1)</option>
							</select>
				</td>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td align="right" width="100px"><b>File Name : </b></td>
			<td width="25%"><input type="text" name="service_search_file_name" value="{$tool_form_service_search_file_name}" style="width:98%;"></td>
			<td><span class="alert">{$tool_file_name_error_msg}</span></td>
		</tr>
		<tr>
			<td align="right" width="100px"><b>Start Date : </b></td>
			<td width="25%"><input type="text" name="service_search_start_date" value="{$tool_form_service_search_start_date}" style="width:98%;"></td>
			<td><span class="alert">{$tool_start_date_error_msg}</span></td>
		</tr>
		<tr>
			<td align="right" width="100px"><b>End Date : </b></td>
			<td width="25%"><input type="text" name="service_search_end_date" value="{$tool_form_service_search_end_date}" style="width:98%;"></td>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td colspan="4"><hr size="1"></td>
		</tr>
		<tr>
			<td align="right" width="100px"><b>EID Search : </b></td>
			<td width="25%"><textarea name="service_eids" rows="3">{$tool_form_service_eids}</textarea></td>
			<td><input type="submit" name="services_las" value="search eids"></td>
		</tr>
		<tr>
			<td align="right" width="100px"><b>Text Search : </b></td>
			<td width="25%"><input type="text" name="service_text" value="{$tool_form_service_text}" style="width:98%;"></td>
			<td><input type="submit" name="services_las" value="search text"></td>
		</tr>
		</table>

		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Command : </b></td>
			<td>&nbsp;<input type="text" name="service_command" value="{$tool_execute_command}" size="50">&nbsp;
				<input type="submit" name="services_las" value="execute">&nbsp;
			</td>
		</tr>
		</table>

{if $tool_execute_command}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th>Command Results for '{$tool_execute_command}' :</th>
		</tr>
		<tr>
			<td><textarea width="100%" rows="50" class="command" readonly >{section name=exe loop=$tool_execute_result}{$tool_execute_result[exe]}{/section}</textarea></td>
		</tr>
		</table>
{/if}

{else}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">No LAS to work with or select a shard on the left!</span></td>
		</tr>
		</table>
{/if}

		</form>

{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
