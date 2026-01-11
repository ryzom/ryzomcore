
{include file="page_header.tpl"}

{literal}
<script language="Javascript" type="text/javascript">
<!--
	function CheckAll(checklist)
	{
		for (var i=0; i<checklist.elements.length; i++)
		{
			var e = checklist.elements[i];
			if (e.type == 'checkbox' && e.name != 'allbox')
				e.checked = checklist.allbox.checked;
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
			<td align="center" class="{if $tool_domain_selected == $tool_domain_list[domain].domain_id}domainlistselected{else}domainlist{/if}"><a href="tool_event_entities.php?domain={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
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
			<td align="center" class="{if $tool_shard_selected == $tool_shard_list[shard].shard_id}shardlistselected{else}shardlist{/if}"><a href="tool_event_entities.php?domain={$tool_domain_selected}&shard={$tool_shard_list[shard].shard_id}">{$tool_shard_list[shard].shard_name}</a></td>
		</tr>
{/if}
{/section}
		</table>
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


		<form action="tool_event_entities.php" method="post" name="qlist">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="heads"><input class="check" type="checkbox" name="allbox" value="1" onclick="CheckAll(document.qlist);"></td>
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
{assign var="service_counter" value="0"}
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
{assign var="service_counter" value="`$service_counter+1`"}
<!-- {$egs_counter} -->
		<tr class="{$trclass}">
			<td><input class="check" type="checkbox" name="service_{$tool_services_list[service].AliasName}" value="{$tool_services_list[service].AliasName}" {if $tool_service_select_list.$check_name || (!$tool_service_select_list && $tool_services_list[service]._flags_.rs_online)}checked{/if}></td>
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

{if $service_counter > 0}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Commands : </b></td>
			<td>&nbsp;
				<input type="submit" name="services_ee" value="display entities"	onclick="if (confirm('Are you sure you want to DISPLAY ENTITIES from the selected services ?')) return true; return false;">&nbsp;
			</td>
		</tr>
		</table>
		</form>

{if $tool_entity_data}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_event_entities.php" method="post" name="elist">

{assign var="entity_service" value="n/a"}
{section name=entity loop=$tool_entity_data}
{if $entity_service != $tool_entity_data[entity].service}
		{assign var="entity_service" value=$tool_entity_data[entity].service}
		<tr>
			<th colspan="16">{$entity_service}</th>
		</tr>
		<tr>
			<td class="heads">Service</td>
			<td class="heads">Entity</td>
			<td class="heads">Name</td>
			<td class="heads">State</td>
			<td class="heads">Param 1</td>
			<td class="heads">Param 2</td>
		</tr>
{/if}
{cycle assign="trclass" values="row0,row1"}
		<tr class="{$trclass}">
			<td>{$tool_entity_data[entity].service_id}</td>
			<td>{$tool_entity_data[entity].entity}</td>
			<td>{$tool_entity_data[entity].entity_name}</td>
			<td><input type="text" name="{$tool_entity_data[entity].service_code}_entity_state_{$tool_entity_data[entity].entity_string}"  value="{$tool_entity_data[entity].entity_state}"></td>
			<td><input type="text" name="{$tool_entity_data[entity].service_code}_entity_param1_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_param1}"></td>
			<td><input type="text" name="{$tool_entity_data[entity].service_code}_entity_param2_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_param2}"></td>
		</tr>
<!-- code: {$tool_entity_data[entity].service} -- {$tool_entity_data[entity].service_code} -->
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_service_{$tool_entity_data[entity].entity_string}" value="{$entity_service}">
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_entity_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity}">
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_entity_name_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_name}">
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_entity_state_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_state}">
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_entity_param1_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_param1}">
		<input type="hidden" name="{$tool_entity_data[entity].service_code}_source_entity_param2_{$tool_entity_data[entity].entity_string}" value="{$tool_entity_data[entity].entity_param2}">
{/section}
		<tr>
			<td colspan="16" align="center"><input type="submit" name="services_ee" value="update entities" onclick="if (confirm('Are you sure you want to UPDATE the selected entities ?')) this.form.submit(); else return false;"></td>
		</tr>

		<input type="hidden" name="requested_service_list" value="{$requested_service_list}">
		</form>
		</table>
{/if}

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
			<td class="row0"><span class="alert">No AIS to work with!</span></td>
		</tr>
		</table>
{/if}

{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
