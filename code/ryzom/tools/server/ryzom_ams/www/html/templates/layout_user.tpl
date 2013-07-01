{extends file="layout.tpl"}
{block name=menu}
    <li class="nav-header hidden-tablet">Main</li>
    <li style="margin-left: -2px;" class="active"><a class="ajax-link" href="index.php"><i class="icon-home"></i><span class="hidden-tablet"> Dashboard</span></a></li>
    <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><i class="icon-cog"></i><span class="hidden-tablet"> Settings</span></a></li>
    <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=userlist"><i class="icon-home"></i><span class="hidden-tablet"> Demo Userlist</span></a></li>
    <li class="nav-header hidden-tablet">Actions</li>
    <li style="margin-left: -2px;"><a href="?page=logout"><i class="icon-off"></i><span class="hidden-tablet"> Logout </span></a></li>
{/block}

