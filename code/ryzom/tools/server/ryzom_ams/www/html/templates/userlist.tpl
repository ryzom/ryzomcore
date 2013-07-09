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
						<table class="table table-striped table-bordered bootstrap-datatable datatable">
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
								{if $element.permission eq 2}<td class="center"><span class="label label-warning">Admin</span></td>{/if}
								<td class="center">
									<a class="btn btn-primary" href="index.php?page=show_user&id={$element.id}"><i class=" icon-pencil icon-white"></i>Show User</a>
									<a class="btn btn-info" href="index.php?page=settings&id={$element.id}"><i class=" icon-pencil icon-white"></i>Edit User</a>
								</td>
								
							</tr>
							{/foreach}
					
						  </tbody>
					  </table>            
					</div>
				</div><!--/span-->
			
			</div><!--/row-->
{/block}

