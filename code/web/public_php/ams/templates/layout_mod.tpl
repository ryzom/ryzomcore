{extends file="layout.tpl"}
{block name=menu}
	{if $permission eq 2}
	<li class="nav-header hidden-tablet">Main</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php"><span class="icon-home"></span><span class="hidden-tablet"> Dashboard</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_user"><span class="icon-user"></span><span class="hidden-tablet"> Profile</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><span class="icon-cog"></span><span class="hidden-tablet"> Settings</span></a></li>
        {if isset($hook_info)} 
			{foreach from=$hook_info key=arrkey item=element}
				{if isset($element.menu_display)}
					<li style="margin-left: -2px;">
						<a class="ajax-link" href="index.php?page=layout_plugin&name={$arrkey}">
							<span class="icon-th-list"></span>
							<span class="hidden-tablet"> {$element.menu_display}</span>
						</a>
					</li>
				{/if}
			{/foreach}
		{/if}
	<li class="nav-header hidden-tablet">Admin</li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=userlist"><span class="icon-th-list"></span><span class="hidden-tablet"> Users</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_queue&get=todo"><span class="icon-th-list"></span><span class="hidden-tablet"> Queues</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=sgroup_list"><span class="icon-briefcase"></span><span class="hidden-tablet"> Support Groups</span></a></li>
        <li class="nav-header hidden-tablet">Actions</li>

       <li style="margin-left: -2px;"><a href="?page=logout"><span class="icon-off"></span><span class="hidden-tablet"> Logout </span></a></li>
	{/if}
{/block}

