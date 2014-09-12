{extends file="layout.tpl"}
{block name=content}

		<!-- topbar starts -->
	<div class="navbar">
		<div class="navbar-inner">
			<div class="container-fluid">
				<a class="btn btn-navbar" data-toggle="collapse" data-target=".top-nav.nav-collapse,.sidebar-nav.nav-collapse">
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
				</a>
				<a class="brand" href="index.php"> <img alt="Ryzom Core Logo" src="img/ryzomcore.png"></a>

				<!-- theme selector starts -->
				<div class="btn-group pull-right theme-container">
					<a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
						<span class="icon-tint"></span><span class="hidden-phone"> Change Theme / Skin</span>
						<span class="caret"></span>
					</a>
					<ul class="dropdown-menu" id="themes">
						<li><a data-value="classic" href="#"><span class="icon-blank"></span> Classic</a></li>
						<li><a data-value="cerulean" href="#"><span class="icon-blank icon-ok"></span> Cerulean</a></li>
						<li><a data-value="cyborg" href="#"><span class="icon-blank"></span> Cyborg</a></li>
						<li><a data-value="redy" href="#"><span class="icon-blank"></span> Redy</a></li>
						<li><a data-value="journal" href="#"><span class="icon-blank"></span> Journal</a></li>
						<li><a data-value="simplex" href="#"><span class="icon-blank"></span> Simplex</a></li>
						<li><a data-value="slate" href="#"><span class="icon-blank"></span> Slate</a></li>
						<li><a data-value="spacelab" href="#"><span class="icon-blank"></span> Spacelab</a></li>
						<li><a data-value="united" href="#"><span class="icon-blank"></span> United</a></li>
					</ul>
				</div>
				<!-- theme selector ends -->
				<button onclick="show_help('test')">Help Test</button>
				<!-- user dropdown starts -->
				<div class="btn-group pull-right">
					<a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
						<span class="icon-user"></span><span class="hidden-phone"> admin</span>
						<span class="caret"></span>
					</a>
					<ul class="dropdown-menu">
						<li><a href="#">Profile</a></li>
						<li class="divider"></li>
						<li><a href="login.php">Logout</a></li>
					</ul>
				</div>
				<!-- user dropdown ends -->


			</div>
		</div>
	</div>
	<!-- topbar ends -->
		<div class="container-fluid">
		<div class="row-fluid">

			<!-- left menu starts -->
			<div class="span2 main-menu-span">
				<div class="well nav-collapse sidebar-nav">
					<ul class="nav nav-tabs nav-stacked main-menu">
						<li class="nav-header hidden-tablet">Main</li>
						<li style="margin-left: -2px;" class="active"><a class="ajax-link" href="?page=home"><span class="icon-home"></span><span class="hidden-tablet"> Dashboard</span></a></li>
						<li class="nav-header hidden-tablet">Sample Section</li>
						<li style="margin-left: -2px;"><a href="?page=login"><span class="icon-lock"></span><span class="hidden-tablet"> Login Page</span></a></li>
					</ul>
					<label id="for-is-ajax" class="hidden-tablet" for="is-ajax" style="visibility:hidden;"><div id="uniform-is-ajax" class="checker"><span class="checked"><input style="opacity: 0;" id="is-ajax" type="checkbox"></span></div> Ajax on menu</label>
				</div><!--/.well -->
			</div><!--/span-->
			<!-- left menu ends -->

			<noscript>
				<div class="alert alert-block span10">
					<h4 class="alert-heading">Warning!</h4>
					<p>You need to have <a href="http://en.wikipedia.org/wiki/JavaScript" target="_blank">JavaScript</a> enabled to use this site.</p>
				</div>
			</noscript>

			<div id="content" class="span10">
			<!-- content starts -->


			<div class="sortable row-fluid ui-sortable">
				<a data-original-title="6 new members." data-rel="tooltip" class="well span3 top-block" href="#">
					<span class="icon32 icon-red icon-user"></span>
					<div>Total Members</div>
					<div>507</div>
					<span class="notification">6</span>
				</a>

				<a data-original-title="4 new pro members." data-rel="tooltip" class="well span3 top-block" href="#">
					<span class="icon32 icon-color icon-star-on"></span>
					<div>Pro Members</div>
					<div>228</div>
					<span class="notification green">4</span>
				</a>

				<a data-original-title="$34 new sales." data-rel="tooltip" class="well span3 top-block" href="#">
					<span class="icon32 icon-color icon-cart"></span>
					<div>Sales</div>
					<div>$13320</div>
					<span class="notification yellow">$34</span>
				</a>

				<a data-original-title="12 new messages." data-rel="tooltip" class="well span3 top-block" href="#">
					<span class="icon32 icon-color icon-envelope-closed"></span>
					<div>Messages</div>
					<div>25</div>
					<span class="notification red">12</span>
				</a>
			</div>

			<div class="row-fluid">
				<div class="box col-md-12">
				<div class="panel panel-default">
					<div class="panel-heading">
						<span class="icon-info-sign"></span> Introduction
					</div>
					<div class="panel-body">
						<h1>Charisma <small>free, premium quality, responsive, multiple skin admin template.</small></h1>
						<p>Its a live demo of the template. I have created Charisma to ease the repeat work I have to do on my projects. Now I re-use Charisma as a base for my admin panel work and I am sharing it with you :)</p>
						<p><b>All pages in the menu are functional, take a look at all, please share this with your followers.</b></p>

						<div class="clearfix"></div>
					</div>
				</div>
				</div>
			</div>

			<div class="row-fluid sortable ui-sortable">


				<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-user"></span> Member Activity
						<div class="box-icon">
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="panel-body">
						<div class="panel-body">
							<ul class="dashboard-list">
								<li>
									<a href="#">
										<img class="dashboard-avatar" alt="Usman" src="http://www.gravatar.com/avatar/f0ea51fa1e4fae92608d8affee12f67b.png?s=50"></a>
										<strong>Name:</strong> <a href="#">Usman
									</a><br>
									<strong>Since:</strong> 17/05/2012<br>
									<strong>Status:</strong> <span class="label label-success">Approved</span>
								</li>
								<li>
									<a href="#">
										<img class="dashboard-avatar" alt="Sheikh Heera" src="http://www.gravatar.com/avatar/3232415a0380253cfffe19163d04acab.png?s=50"></a>
										<strong>Name:</strong> <a href="#">Sheikh Heera
									</a><br>
									<strong>Since:</strong> 17/05/2012<br>
									<strong>Status:</strong> <span class="label label-warning">Pending</span>
								</li>
								<li>
									<a href="#">
										<img class="dashboard-avatar" alt="Abdullah" src="http://www.gravatar.com/avatar/46056f772bde7c536e2086004e300a04.png?s=50"></a>
										<strong>Name:</strong> <a href="#">Abdullah
									</a><br>
									<strong>Since:</strong> 25/05/2012<br>
									<strong>Status:</strong> <span class="label label-important">Banned</span>
								</li>
								<li>
									<a href="#">
										<img class="dashboard-avatar" alt="Saruar Ahmed" src="http://www.gravatar.com/avatar/564e1bb274c074dc4f6823af229d9dbb.png?s=50"></a>
										<strong>Name:</strong> <a href="#">Saruar Ahmed
									</a><br>
									<strong>Since:</strong> 17/05/2012<br>
									<strong>Status:</strong> <span class="label label-info">Updates</span>
								</li>
							</ul>
						</div>
					</div>
					</div>
				</div><!--/span-->

				<div class="box col-md-8">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-list-alt"></span> Realtime Traffic
						<div class="box-icon">
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="panel-body">
						<div id="realtimechart" style="height: 190px; padding: 0px; position: relative;"><canvas height="190" width="466" class="base"></canvas><canvas style="position: absolute; left: 0px; top: 0px;" height="190" width="466" class="overlay"></canvas><div class="tickLabels" style="font-size:smaller"><div class="yAxis y1Axis" style="color:#545454"><div class="tickLabel" style="position:absolute;text-align:right;top:177px;right:448px;width:18px">0</div><div class="tickLabel" style="position:absolute;text-align:right;top:132px;right:448px;width:18px">25</div><div class="tickLabel" style="position:absolute;text-align:right;top:86px;right:448px;width:18px">50</div><div class="tickLabel" style="position:absolute;text-align:right;top:41px;right:448px;width:18px">75</div><div class="tickLabel" style="position:absolute;text-align:right;top:-5px;right:448px;width:18px">100</div></div></div></div>
							<p class="clearfix">You can update a chart periodically to get a real-time effect by using a timer to insert the new data in the plot and redraw it.</p>
							<p>Time between updates: <input id="updateInterval" value="" style="text-align: right; width:5em" type="text"> milliseconds</p>
					</div>
					</div>
				</div><!--/span-->
			</div><!--/row-->

			<div class="row-fluid sortable ui-sortable">
				<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-list"></span> Buttons
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><span class="icon-cog"></span></a>
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="panel-body buttons">
						<p class="btn-group">
							  <button class="btn">Left</button>
							  <button class="btn">Middle</button>
							  <button class="btn">Right</button>
						</p>
						<p>
							<button class="btn btn-small"><span class="icon-star"></span> Icon button</button>
							<button class="btn btn-small btn-primary">Small button</button>
							<button class="btn btn-small btn-danger">Small button</button>
						</p>
						<p>
							<button class="btn btn-small btn-warning">Small button</button>
							<button class="btn btn-small btn-success">Small button</button>
							<button class="btn btn-small btn-info">Small button</button>
						</p>
						<p>
							<button class="btn btn-small btn-inverse">Small button</button>
							<button class="btn btn-large btn-primary btn-round">Round button</button>
							<button class="btn btn-large btn-round"><span class="icon-ok"></span></button>
							<button class="btn btn-primary"><span class="icon-edit icon-white"></span></button>
						</p>
						<p>
							<button class="btn btn-mini">Mini button</button>
							<button class="btn btn-mini btn-primary">Mini button</button>
							<button class="btn btn-mini btn-danger">Mini button</button>
							<button class="btn btn-mini btn-warning">Mini button</button>
						</p>
						<p>
							<button class="btn btn-mini btn-info">Mini button</button>
							<button class="btn btn-mini btn-success">Mini button</button>
							<button class="btn btn-mini btn-inverse">Mini button</button>
						</p>
					</div>
					</div>
				</div><!--/span-->

				<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-list"></span> Buttons
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><span class="icon-cog"></span></a>
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="panel-body  buttons">
						<p>
							<button class="btn btn-large">Large button</button>
							<button class="btn btn-large btn-primary">Large button</button>
						</p>
						<p>
							<button class="btn btn-large btn-danger">Large button</button>
							<button class="btn btn-large btn-warning">Large button</button>
						</p>
						<p>
							<button class="btn btn-large btn-success">Large button</button>
							<button class="btn btn-large btn-info">Large button</button>
						</p>
						<p>
							<button class="btn btn-large btn-inverse">Large button</button>
						</p>
						<div class="btn-group">
							<button class="btn btn-large">Large Dropdown</button>
							<button class="btn btn-large dropdown-toggle" data-toggle="dropdown"><span class="caret"></span></button>
							<ul class="dropdown-menu">
								<li><a href="#"><span class="icon-star"></span> Action</a></li>
								<li><a href="#"><span class="icon-tag"></span> Another action</a></li>
								<li><a href="#"><span class="icon-download-alt"></span> Something else here</a></li>
								<li class="divider"></li>
								<li><a href="#"><span class="icon-tint"></span> Separated link</a></li>
							</ul>
						</div>

					</div>
					</div>
				</div><!--/span-->

				<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-list"></span> Weekly Stat
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><span class="icon-cog"></span></a>
							<a href="#" class="btn btn-minimize btn-round"><span class="icon-chevron-up"></span></a>
							<a href="#" class="btn btn-close btn-round"><span class="icon-remove"></span></a>
						</div>
					</div>
					<div class="panel-body">
						<ul class="dashboard-list">
							<li>
								<a href="#">
									<span class="icon-arrow-up"></span>
									<span class="green">92</span>
									New Comments
								</a>
							</li>
						  <li>
							<a href="#">
							  <span class="icon-arrow-down"></span>
							  <span class="red">15</span>
							  New Registrations
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-minus"></span>
							  <span class="blue">36</span>
							  New Articles
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-comment"></span>
							  <span class="yellow">45</span>
							  User reviews
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-arrow-up"></span>
							  <span class="green">112</span>
							  New Comments
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-arrow-down"></span>
							  <span class="red">31</span>
							  New Registrations
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-minus"></span>
							  <span class="blue">93</span>
							  New Articles
							</a>
						  </li>
						  <li>
							<a href="#">
							  <span class="icon-comment"></span>
							  <span class="yellow">254</span>
							  User reviews
							</a>
						  </li>
						</ul>
					</div>
					</div>
				</div><!--/span-->
			</div><!--/row-->




					<!-- content ends -->
			</div><!--/#content.span10-->
				</div><!--/fluid-row-->

		<hr>

		<div class="modal hide fade" id="myModal">
			<div class="modal-header">
				<button type="button" class="close" data-dismiss="modal">×</button>
				<h3>Settings</h3>
			</div>
			<div class="modal-body">
				<p>Here settings can be configured...</p>
			</div>
			<div class="modal-footer">
				<a href="#" class="btn" data-dismiss="modal">Close</a>
				<a href="#" class="btn btn-primary">Save changes</a>
			</div>
		</div>

		<footer>
			<p class="pull-right">Powered by: <a href="http://usman.it/free-responsive-admin-template">Charisma</a></p>
		</footer>

	</div><!--/.fluid-container-->

{/block}

