{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-user"></i> Profile of {$target_name}</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Info</legend>
		<table class="table table-striped" >
		    <tbody>
			<tr >
			    <td><strong>Email:</strong></td>
			    <td>{$mail}</td>                           
			</tr>
			{if $firstName neq ""}
			<tr>
			    <td><strong>Firstname:</strong></td>
			    <td>{$firstName}</td>                           
			</tr>
			{/if}
			{if $lastName neq ""}
			<tr>
			    <td><strong>LastName:</strong></td>
			    <td>{$lastName}</td>                           
			</tr>
			{/if}
			{if $country neq ""}
			<tr>
			    <td><strong>Country:</strong></td>
			    <td>{$country}</td>                           
			</tr>
			{/if}
			{if $gender neq 0}
			<tr>
			    <td><strong>Gender:</strong></td>
			    {if $gender eq 1}
			    <td><strong>♂</strong></td>
			    {else if $gender eq 2}
			    <td><strong>♀</strong></td>
			    {/if}
			</tr>
			{/if}
		    </tbody>
		</table>
	    </div>                   
        </div>
    </div><!--/span-->
    
    <div class="box span3">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i>Actions</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
		<div class="btn-group">
                <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
                <ul class="dropdown-menu">
		    <li class="divider"></li>
		    <li><a href="index.php?page=settings&id={$target_id}">Edit User</a></li>
		    <li><a href="index.php?page=createticket&user_id={$target_id}">Send Ticket</a></li>
		    <li class="divider"></li>
                </ul>
              </div>
            </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->

<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i> Tickets of {$target_name}</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Tickets</legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>Title</th>
				    <th>Timestamp</th>
				    <th>Category</th>
				    <th>Status</th>
			    </tr>
		    </thead>   
		    <tbody>
			  {foreach from=$ticketlist item=ticket}
			  <tr>
				<td>{$ticket.title}</td>
				<td class="center"><i>{$ticket.timestamp}</i></td>
				<td class="center">{$ticket.category}</td>

				<td class="center"><span class="label {if $ticket.status eq 0}label-success{else if $ticket.status eq 1}label-warning{else if $ticket.status eq 2}label-important{/if}">{$ticket.statusText}</span></td>  
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	    </div>
	</div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
