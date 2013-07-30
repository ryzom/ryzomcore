{extends file="layout.tpl"}
{block name=menu}
	<td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php"><h5> Dashboard</h5></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=show_user"><h5> Profile</h5></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=settings"><h5> Settings</h5></a></td>
        <td width="10" align="center">|</td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=userlist"><h5> Users</h5></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=show_queue&get=todo"><h5> Queues</h5></a></td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=sgroup_list"><h5> Support Groups</h5></a></td>
        <td width="10" align="center">|</td>
        <td height="25" valign="middle" nowrap><a class="ajax-link" href="index.php?page=libuserlist"><h5> Syncing</h5></a></td>
        <td height="25" valign="middle" nowrap><a href="?page=logout"><h5> Logout </h5></a></td>
{/block}

