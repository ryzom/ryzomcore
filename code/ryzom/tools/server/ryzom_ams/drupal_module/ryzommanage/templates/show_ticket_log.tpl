{block name=content}

            <h2><i class="icon-tag"></i> Log of Ticket #{$ticket_id}</h2>
                <legend>Title: <a href="ams?page=show_ticket&id={$ticket_id}">{$ticket_title}</a></legend>
		<table>
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Timestamp</th>
				    <th>Query</th>
			    </tr>
		    </thead>   
		    <tbody>
			  {foreach from=$ticket_logs item=log}
			  <tr>
				<td>{$log.tLogId}</td>
				<td>{$log.timestamp}</td>
				<td>{$log.query}</td>
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	
{/block}
	