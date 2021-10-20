{block name=content}

	<div class="row-fluid">
		<div class="box col-md-12">
		<div class="panel panel-default">
			<div class="panel-heading">
				<span class="icon-info-sign"></span>{$ip_title}
			</div>
			<div class="panel-body">
				<center>
				<p>{$ip_support}</p>
				<div class="alert alert-error">
				<form enctype="multipart/form-data" method="post" action="index.php?page=plugin&action=install_plugin" id="upload_plugin"  >
				<label for="file">Filename:</label>&nbsp;&nbsp;
				<input type="file" name="file" id="file"></br>
				<progress id="progressBar" value="0" max="100" style="width:300px;"></progress></br>
				<input type="button" value="Upload" onclick="uploadPlugin()"></br>
				<h3 id="status"></h3>
				 {if isset($smarty.get.result) and $smarty.get.result eq "0"}<p>{$ip_file_nfnd}</p>{/if}
				 {if isset($smarty.get.result) and $smarty.get.result eq "2"}<p>{$ip_info_nfound}</p>{/if}
				</div>
				{$ip_message}
				</center>
				<div class="clearfix"></div>
			</div>
		</div>
		</div>
	</div>
		</div><!--/span-->

			</div><!--/row-->
{/block}
