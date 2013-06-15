{extends file="layout.tpl"}
{block name=menu}
	<li class="nav-header hidden-tablet">Main</li>
	<li><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></li>
	<li><a class="ajax-link" href="settings.php"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></li>
	<li><a href="logout.php"><i class="icon-lock"></i><span class="hidden-tablet"> Logout</span></a></li>
	<li class="nav-header hidden-tablet">Ticketing</li>
	<li><a class="ajax-link" href="createticket.php"><i class="icon-pencil"></i><span class="hidden-tablet"> New Ticket</span></a></li>
	<li><a class="ajax-link" href="userticketqueue.php"><i class="icon-th-list"></i><span class="hidden-tablet"> Ticket Queue</span></a></li>
{/block}

