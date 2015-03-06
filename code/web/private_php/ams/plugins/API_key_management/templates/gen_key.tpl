<div class="row-fluid">	
				<div class="box col-md-12">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title>
						<span class="icon-user"></span> API KEY management
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><span class="icon-cog"></span></a>
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="box col-md-4">
					<div class="panel panel-default">
			<div class="panel-heading" data-original-title="">
				<span class="icon-th"></span> Generate Access Key
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
					<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
				</div>
			</div>
			<div class="panel-body">
				<div class="row-fluid">
					<form id="generateKey" class="form-vertical" method="post" action="index.php?page=layout_plugin&&name={$arrkey}">
						<legend>Generate Key</legend>
											
						<div class="control-group ">
						<label class="control-label">Expirey:</label>
							<div class="controls">
							    <div class="input-prepend">
								<span style="margin-left:5px;" class="add-on"><span class="icon-time"></span></span>
									<input type="text" value="Expiry Date" placeholder="Expiry Date" name="expDate" id="expDate" class="input-xlarge">		
							    </div>
							</div>
						</div>
						<div class="control-group">
							<label class="control-label"></label>
							<div class="controls">
								<button type="submit" name="gen_key" value="true" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Generate Key</button>
							</div>
						</div>
					</form>		
				</div>                   
			</div>
			</div>
		</div><!--/span-->
				</div><!--/span-->
				</div><!--/span-->
			</div><!--/row-->


