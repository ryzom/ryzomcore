{extends file="layout.tpl"}
{block name=menu}
	<li class="nav-header hidden-tablet">Main</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_user"><i class="icon-user"></i><span class="hidden-tablet"> Profile</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></li>
	{if isset($hook_info)} {foreach from=$hook_info key=arrkey item=element}<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=layout_plugin&&name={$arrkey}"><i class="icon-th-list"></i><span class="hidden-tablet"> {$element.menu_display}</span></a></li>{/foreach}{/if} 
	<li class="nav-header hidden-tablet">Admin</li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=userlist"><i class="icon-th-list"></i><span class="hidden-tablet"> Users</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_queue&get=todo"><i class="icon-th-list"></i><span class="hidden-tablet"> Queues</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=sgroup_list"><i class="icon-briefcase"></i><span class="hidden-tablet"> Support Groups</span></a></li>
        <li class="nav-header hidden-tablet">Actions</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=plugins"><i class="icon-th-list"></i><span class="hidden-tablet"> Plugins</span></a></li>  
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=syncing"><i class="icon-th-list"></i><span class="hidden-tablet"> Syncing</span></a></li>
        <li style="margin-left: -2px;"><a href="?page=logout"><i class="icon-off"></i><span class="hidden-tablet"> Logout </span></a></li>
{/block}

