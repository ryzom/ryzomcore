{extends file="layout.tpl"}
{block name=menu}
	<td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=show_user"><i class="icon-user"></i><span class="hidden-tablet"> Profile</span></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=settings"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></td>
        <td width="10" align="center">|</td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=userlist"><i class="icon-th-list"></i><span class="hidden-tablet"> Users</span></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=show_queue&get=todo"><i class="icon-th-list"></i><span class="hidden-tablet"> Queues</span></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=sgroup_list"><i class="icon-briefcase"></i><span class="hidden-tablet"> Support Groups</span></a></td>
        <td width="10" align="center">|</td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=libuserlist"><i class="icon-th-list"></i><span class="hidden-tablet"> Syncing</span></a></td>
        <td height="25" valign="middle" nowrap><a href="?page=logout"><i class="icon-off"></i><span class="hidden-tablet"> Logout </span></a></td>
{/block}

