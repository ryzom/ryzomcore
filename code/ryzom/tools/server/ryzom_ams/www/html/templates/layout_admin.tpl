{extends file="layout.tpl"}
{block name=menu}
	<li class="nav-header hidden-tablet">Main</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></li>
	<li class="nav-header hidden-tablet">Admin</li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=libuserlist"><i class="icon-th-list"></i><span class="hidden-tablet"> Liblist</span></a></li>
        <li class="nav-header hidden-tablet">Actions</li>
        <li style="margin-left: -2px;"><a href="?page=logout"><i class="icon-off"></i><span class="hidden-tablet"> Logout </span></a></li>

{/block}

