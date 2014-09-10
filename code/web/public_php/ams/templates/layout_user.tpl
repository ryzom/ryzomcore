{extends file="layout.tpl"}
{block name=menu}
	{if $permission eq 1}
		<li class="nav-header hidden-tablet">Main</li>
		<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_user"><span class="icon-user"></span><span class="hidden-tablet">Profile</span></a></li>
		<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><span class="icon-cog"></span><span class="hidden-tablet"> Settings</span></a></li>
		{if isset($hook_info)} {foreach from=$hook_info key=arrkey item=element}{if isset($element.user_display)}<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=layout_plugin&name={$arrkey}"><span class="icon-th-list"></span><span class="hidden-tablet"> {$element.user_display}</span></a></li>{/if}{/foreach}{/if} 
		<li class="nav-header hidden-tablet">Actions</li>
		<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=createticket"><span class="icon-pencil"></span><span class="hidden-tablet">Create New Ticket</span></a></li> 
		<li style="margin-left: -2px;"><a href="?page=logout"><span class="icon-off"></span><span class="hidden-tablet"> Logout </span></a></li>
	{/if}
{/block}

