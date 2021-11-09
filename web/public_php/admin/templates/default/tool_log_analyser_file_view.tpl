
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

{if $tool_file_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_file_error}</span></td>
		</tr>
		</table>
		<br>
{else}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="heads">Viewing File : {$tool_view_file_data.name}
				[ Downloads :
				<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&downloadraw=1&fileview={$tool_view_file_data.code}">RAW</a>
				&nbsp;-&nbsp;
				<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&downloadparsed=1&fileview={$tool_view_file_data.code}">PARSED</a>
				]
				{* [<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&download=1&fileview={$tool_view_file_data.code}">download</a>] *}
				{* [<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&delete=1&fileview={$tool_view_file_data.code}">delete</a>] *}
			</td>
		</tr>
{section name=line loop=$tool_file_output}
{cycle assign="trclass" values="row0,row1"}
		<tr class="{$trclass}">
			<td>{$tool_file_output[line]}</td>
		</tr>
{/section}
		<tr>
			<th align="center">
{if $tool_view_line_start_previous > -1}
				<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&viewstart={$tool_view_line_start_previous}&fileview={$tool_view_file_data.code}">&lt;Previous</a>
{else}
				Beginning
{/if}
				&nbsp;|&nbsp;
{if $tool_view_line_start_next > -1}
				<a href="tool_log_analyser.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}&viewstart={$tool_view_line_start_next}&fileview={$tool_view_file_data.code}">Next&gt;</a>
{else}
				End
{/if}
			</th>
		</tr>
		</table>
		<br>
{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
