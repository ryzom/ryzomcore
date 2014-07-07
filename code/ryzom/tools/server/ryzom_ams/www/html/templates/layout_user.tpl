{extends file="layout.tpl"}
{block name=menu}
    <li class="nav-header hidden-tablet">Main</li>
    <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_user"><i class="icon-user"></i><span class="hidden-tablet">Profile</span></a></li>
    <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></li>
    <li class="nav-header hidden-tablet">Actions</li>
    <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=createticket"><i class="icon-pencil"></i><span class="hidden-tablet">Create New Ticket</span></a></li>
    {if isset($hook_info)} {foreach from=$hook_info item=element}<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=layout_plugin&&name={$element.menu_display}"><i class="icon-th-list"></i><span class="hidden-tablet"> {$element.menu_display}</span></a></li>{/foreach}{/if}	
    <li style="margin-left: -2px;"><a href="?page=logout"><i class="icon-off"></i><span class="hidden-tablet"> Logout </span></a></li>
{/block}

