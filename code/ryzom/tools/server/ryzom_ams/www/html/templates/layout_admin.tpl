{extends file="layout.tpl"}
{block name=menu}
	<li class="nav-header hidden-tablet">Main</li>
	<li><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></li>
	<li><a class="ajax-link" href="settings.php"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></li>
	<li><a href="logout.php"><i class="icon-lock"></i><span class="hidden-tablet"> Logout</span></a></li>
	<li class="nav-header hidden-tablet">Admin</li>
	<li><a class="ajax-link" href="checkuser.php"><i class="icon-user"></i><span class="hidden-tablet">UserList</span></a></li>
	<li><a class="ajax-link" href="banlist.php"><i class="icon-remove"></i><span class="hidden-tablet"> BanList</span></a></li>
	<li class="nav-header hidden-tablet">Ticketing</li>
	<li><a class="ajax-link" href="generalqueue.php"><i class="icon-th-list"></i><span class="hidden-tablet"> General Queue</span></a></li>
	<li><a class="ajax-link" href="personalQueue.php"><i class="icon-tag"></i><span class="hidden-tablet"> Personal Queue</span></a></li>
	<li><a class="ajax-link" href="archive.php"><i class="icon-folder-open"></i><span class="hidden-tablet"> Ticket Archive</span></a></li>
{/block}

