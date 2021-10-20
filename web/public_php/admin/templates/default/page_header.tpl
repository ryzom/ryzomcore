<html>
<head>
<title>{$nel_tool_title}{if $tool_page_title != ''} - {$tool_page_title}{/if}</title>
<link rel="stylesheet" href="neltool.css" type="text/css">
<script type="text/javascript" src="overlib/overlib_mini.js"></script>
<script type="text/javascript" src="overlib/overlib_anchor_mini.js"></script>
<script type="text/javascript" src="overlib/overlib_draggable_mini.js"></script>
{*$nel_tool_notes_meta*}
{$nel_tool_refresh}
{$nel_tool_extra_css}

{if $iPhone}
<meta name = "viewport" content = "width=device-width">
{/if}

</head>

<body>

<table width="100%" cellpadding="2" cellspacing="0" border="0">
	<tr>
{if !$iPhone}
		<td align="left">
			<table cellpadding="1" cellspacing="0" border="0">
				<tr>
					<td width="35" height="22"><img src="imgs/nel.gif" name="ol_anchor_left"></td>
					<td height="22" class="boxed"><b>{$tool_title}</b></td>
					<td height="22" class="boxed"><b>{$user_info}</b></td>
				</tr>
			</table>
		</td>
{/if}
		<td align="right">
			<table cellpadding="1" cellspacing="0" border="0">
				<tr>
{section name=onemenu loop=$nel_menu}
					 <td height="22" class="boxed">
{if $menu_style == 1 && !$iPhone}
					 	<a href="{$nel_menu[onemenu].application_uri}" onmouseover="return overlib('{$nel_menu[onemenu].application_name}', LEFT, OFFSETY, 20, BGCOLOR, '#aaaaaa', FGCOLOR, '#cccccc');" onmouseout="return nd();" >
					 	<img src="{if $nel_menu[onemenu].application_icon != ""}{$nel_menu[onemenu].application_icon}{else}{$unknown_menu}{/if}" border="0">
					 	</a>
{elseif $menu_style == 2 && !$iPhone}
					 	<a href="{$nel_menu[onemenu].application_uri}">
					 	<img src="{if $nel_menu[onemenu].application_icon != ""}{$nel_menu[onemenu].application_icon}{else}{$unknown_menu}{/if}" border="0"><br />{$nel_menu[onemenu].application_name}
					 	</a>
{else}
					 	<a href="{$nel_menu[onemenu].application_uri}">
					 	{$nel_menu[onemenu].application_name}
					 	</a>
{/if}
					 </td>
{/section}

{if !$iPhone}
					<td width="35" height="22" align="right"><img src="imgs/nel.gif" name="ol_anchor_right"></td>
{/if}
				</tr>
			</table>
		</td>
	</tr>
</table>
