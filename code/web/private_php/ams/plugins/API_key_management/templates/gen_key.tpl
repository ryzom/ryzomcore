<div class="row-fluid">	
				<div class="box col-md-12">
				<div class="box-inner">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> API KEY management</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					<div class="box col-md-4">
					<div class="box-inner">
			<div class="box-header well" data-original-title="">
				<h2><i class="icon-th"></i> Generate Access Key</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<div class="row-fluid">
					<form id="generateKey" class="form-vertical" method="post" action="index.php?page=layout_plugin&&name={$arrkey}">
						<legend>Generate Key</legend>
											
						<div class="control-group ">
						<label class="control-label">Expirey:</label>
							<div class="controls">
							    <div class="input-prepend">
								<span style="margin-left:5px;" class="add-on"><i class="icon-time"></i></span>
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


