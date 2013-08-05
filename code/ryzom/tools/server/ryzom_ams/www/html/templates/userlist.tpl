{block name=content}
<div class="row-fluid">		
				<div class="box span12">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> Members</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					<div class="box-content">
						<table class="table table-striped table-bordered">
						  <thead>
							  <tr>
								  <th>Id</th>
								  <th>Username</th>
								  <th>Email</th>
								  <th>Permission</th>
								  <th>Action</th>
							  </tr>
						  </thead>   
						  <tbody>
							{foreach from=$userlist item=element}
							<tr>
								<td>{$element.id}</td>
								<td class="center"><a href="index.php?page=show_user&id={$element.id}">{$element.username}</a></td>
								<td class="center">{$element.email}</td>
								{if $element.permission eq 1}<td class="center"><span class="label label-success">User</span></td>{/if}
								{if $element.permission eq 2}<td class="center"><span class="label label-warning">Moderator</span></td>{/if}
								{if $element.permission eq 3}<td class="center"><span class="label label-important">Admin</span></td>{/if}
								<td class="center">
									<div class="btn-group" style="display: inline-block;">
										<a class="btn btn-primary" href="index.php?page=show_user&id={$element.id}"><i class=" icon-eye-open icon-white"></i> Show User</a>
									</div>
									<div class="btn-group" style="display: inline-block;">
										<a class="btn btn-info" href="index.php?page=settings&id={$element.id}"><i class=" icon-pencil icon-white"></i> Edit User</a>
									</div>
									{if isset($isAdmin) and $isAdmin eq 'TRUE' and $element.id neq 1}
									<div class="btn-group" style="display: inline-block;">
									<button class="btn btn-primary dropdown-toggle" data-toggle="dropdown"><i class=" icon-star icon-white"></i> Change Role<span class="caret"></span></button>
									<ul class="dropdown-menu">
									    <li class="divider"></li>
										{if $element.permission eq 1}
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=2">Make Moderator</a></li>
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=3">Make Admin</a></li>
										{else if $element.permission eq 2 }
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=1">Demote to User</a></li>
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=3">Make Admin</a></li>
										{else if $element.permission eq 3 }
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=1">Demote to User</a></li>
										<li><a href="index.php?page=change_permission&user_id={$element.id}&value=2">Demote to Moderator</a></li>
										{/if}
										<li class="divider"></li>
									</ul>
									 
								      </div>
									{/if}
								</td>
								
							</tr>
							{/foreach}
					
						  </tbody>
					  </table>
					  <div style="width: 300px; margin:0px auto;">
						<ul class="pagination">
							<li><a href="index.php?page=userlist&pagenum=1">&laquo;</a></li>
							{foreach from=$links item=link}
							<li {if $link == $currentPage}class="active"{/if}><a href="index.php?page=userlist&pagenum={$link}">{$link}</a></li>
							{/foreach}
							<li><a href="index.php?page=userlist&pagenum={$lastPage}">&raquo;</a></li>
						</ul>
					  </div>
					</div>
				</div><!--/span-->
			
			</div><!--/row-->
{/block}

