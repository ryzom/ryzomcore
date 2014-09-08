<!DOCTYPE html>
<html lang="en">
<head>
	<!--
		Charisma v1.0.0

		Copyright 2012 Muhammad Usman
		Licensed under the Apache License v2.0
		http://www.apache.org/licenses/LICENSE-2.0

		http://usman.it
		http://twitter.com/halalit_usman
	-->
	<meta charset="utf-8">
	<title>{$ams_title}</title>
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<meta name="description" content="ryzom,ams">
	<meta name="author" content="Daan Janssens & Matthew Lagoe">

	<!-- The styles -->
	<link id="bs-css" href="css/bootstrap-classic.css" rel="stylesheet">
    <link id="bs-css" href="css/bootstrap-cerulean.min.css" rel="stylesheet">

    <link href="css/charisma-app.css" rel="stylesheet">
    <link href='css/jquery.noty.css' rel='stylesheet'>
    <link href='css/noty_theme_default.css' rel='stylesheet'>
    <link href='css/elfinder.min.css' rel='stylesheet'>
    <link href='css/elfinder.theme.css' rel='stylesheet'>
    <link href='css/jquery.iphone.toggle.css' rel='stylesheet'>
    <link href='css/uploadify.css' rel='stylesheet'>
    <link href='css/animate.min.css' rel='stylesheet'>

	<!-- jQuery -->
    <script src="js/jquery.min.js"></script>

	<!-- The HTML5 shim, for IE6-8 support of HTML5 elements -->
	<!--[if lt IE 9]>
	  <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
	<![endif]-->

	<!-- The fav icon -->
	<!--<link rel="shortcut icon" href="img/favicon.ico">-->

	<!--custom css file-->
	<link href="css/custom.css" rel="stylesheet">
</head>

<body>
  <div class="container-fluid">
	{if ! isset($no_visible_elements) or  $no_visible_elements eq "FALSE"}
	<div class="navbar">
		<div class="navbar-inner">
			<div class="container-fluid">
				<a class="btn btn-navbar" data-toggle="collapse" data-target=".top-nav.nav-collapse,.sidebar-nav.nav-collapse">
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
				</a>
				<a class="brand" href="index.php"> <img alt="Ryzom Core Logo" src="img/ryzomcore_166_62.png"></a>
				<div class="flags_logged_in">
				      <img onclick="reloadPageWithLanguage('en');" src="img/en.png">
				      <img onclick="reloadPageWithLanguage('fr');" src="img/fr.png">
				</div>

				<!-- theme selector starts -->
				<!--<div class="btn-group pull-right theme-container">
					<a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
						<i class="icon-tint"></i><span class="hidden-phone"> Change Theme / Skin</span>
						<span class="caret"></span>
					</a>
					<ul class="dropdown-menu" id="themes">
						<li><a data-value="classic" href="#"><i class="icon-blank"></i> Classic</a></li>
						<li><a data-value="cerulean" href="#"><i class="icon-blank icon-ok"></i> Cerulean</a></li>
						<li><a data-value="cyborg" href="#"><i class="icon-blank"></i> Cyborg</a></li>
						<li><a data-value="redy" href="#"><i class="icon-blank"></i> Redy</a></li>
						<li><a data-value="journal" href="#"><i class="icon-blank"></i> Journal</a></li>
						<li><a data-value="simplex" href="#"><i class="icon-blank"></i> Simplex</a></li>
						<li><a data-value="slate" href="#"><i class="icon-blank"></i> Slate</a></li>
						<li><a data-value="spacelab" href="#"><i class="icon-blank"></i> Spacelab</a></li>
						<li><a data-value="united" href="#"><i class="icon-blank"></i> United</a></li>
					</ul>
				</div>-->
				<!-- theme selector ends -->
				<!-- user dropdown starts -->
				{if isset($username)}
				<div class="btn-group pull-right">
					<a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
						<i class="icon-user"></i><span class="hidden-phone">{$username}</span>
						<span class="caret"></span>
					</a>
					<ul class="dropdown-menu">
						<li><a href="index.php?page=show_user">Profile</a></li>
						<li><a href="index.php?page=logout">Logout</a></li>
					</ul>
				</div>
				{/if}
				<!-- user dropdown ends -->
			</div>
		</div>
	</div>
	{/if}
	<div class="container-fluid">
		<div class="row-fluid">
			{if ! isset($no_visible_elements) or  $no_visible_elements eq "FALSE"}
			<!-- left menu starts -->
			<div class="span2 main-menu-span">
				<div class="well nav-collapse sidebar-nav">
					<ul class="nav nav-tabs nav-stacked main-menu">
						{block name=menu}{/block}
					</ul>
					<!--<label id="for-is-ajax" class="hidden-tablet" for="is-ajax" style="visibility:hidden;"><div id="uniform-is-ajax" class="checker"><span class="checked"><input style="opacity: 0;" id="is-ajax" type="checkbox"></span></div> Ajax on menu</label>--!>
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
			{/if}

			{if  isset($no_visible_elements) and  $no_visible_elements eq "TRUE"}

				    <div class="flags_no_visible_elements">
				      <img src="img/en.png" onclick="reloadPageWithLanguage('en');"/>
				      <img src="img/fr.png" onclick="reloadPageWithLanguage('fr');"/>
				    </div>

			{/if}

			{block name=content}{/block}
			{if isset($hook_info)}
				<div class="row-fluid">
					{foreach from=$hook_info key=arrkey item=element}
						{if isset($smarty.get.page) and $smarty.get.page eq 'layout_plugin'}
							{include file=$element['TemplatePath']}
						{/if}
					{/foreach}	
				</div>
			{/if}


			{if ! isset($no_visible_elements) or  $no_visible_elements eq "FALSE"}
			</div><!--/#content.span10-->
			{/if}
		</div><!--/fluid-row-->
		{if ! isset($no_visible_elements) or  $no_visible_elements eq "FALSE"}
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
			{if $permission > 1}<p class="pull-right">AMS 0.9.1 Powered by: <a href="http://usman.it/free-responsive-admin-template">Charisma</a></p>{/if}
		</footer>
		{/if}
	</div><!--/.fluid-container-->
  </div>
  	<!-- external javascript
	================================================== -->
	<!-- Placed at the end of the document so the pages load faster -->
	<script>
	function reloadPageWithLanguage(language){
	  if(window.location.search == ""){
	    var url = window.location.href + "?setLang=true&Language=" + language;
	  }else{
	    var url = window.location.href + "&setLang=true&Language=" + language;
	  }
	  window.location.href = url;
	}
	</script>

	<!-- script for file uploading-->
	<script>
		function _(e1)
		{
			return document.getElementById(e1);
			}

		function uploadPlugin()
		{
			var fileObject = _("file").files[0];
			var formdata = new FormData();
			formdata.append("file",fileObject);
			var ajax = new XMLHttpRequest();
			ajax.upload.addEventListener("progress", progressHandler, false);
			ajax.addEventListener("load", completeHandler, false);
			ajax.addEventListener("error", errorHandler, false);
			ajax.addEventListener("abort", abortHandler, false);
			ajax.open("POST", "index.php?page=plugin&action=install_plugin");
			ajax.send(formdata);
			}

		function progressHandler(event)
		{
			var percent = (event.loaded/event.total)*100;
			_("progressBar").value = Math.round(percent);
			}

		function completeHandler(event)
		{
			_("status").innerHTML = event.target.responseText;
			_("progressBar").value = 0;
			}

		function errorHandler(event)
		{
			_("status").innerHTML = "upload Failed";
			}

		function abortHandler(event)
		{
			_("status").innerHTML = "upload Aborted";
			}
	</script>
	<link href="http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css" rel="stylesheet" type="text/css"/>
	<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script>
	<script src="http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js"></script>

	<script>
	 $(document).ready(function() {
     $("#expDate").datepicker({ dateFormat: 'yy-mm-dd' });
	 });
    </script>

	<!-- jQuery -->
	<script src="js/jquery-1.7.2.min.js"></script>
	<!-- jQuery UI -->
	<script src="js/jquery-ui-1.8.21.custom.min.js"></script>
	<!-- transition / effect library -->
	<script src="js/bootstrap-transition.js"></script>
	<!-- alert enhancer library -->
	<script src="js/bootstrap-alert.js"></script>
	<!-- modal / dialog library -->
	<script src="js/bootstrap-modal.js"></script>
	<!-- custom dropdown library -->
	<script src="js/bootstrap-dropdown.js"></script>
	<!-- scrolspy library -->
	<script src="js/bootstrap-scrollspy.js"></script>
	<!-- library for creating tabs -->
	<script src="js/bootstrap-tab.js"></script>
	<!-- library for advanced tooltip -->
	<script src="js/bootstrap-tooltip.js"></script>
	<!-- popover effect library -->
	<script src="js/bootstrap-popover.js"></script>
	<!-- button enhancer library -->
	<script src="js/bootstrap-button.js"></script>
	<!-- accordion library (optional, not used in demo) -->
	<script src="js/bootstrap-collapse.js"></script>
	<!-- carousel slideshow library (optional, not used in demo) -->
	<script src="js/bootstrap-carousel.js"></script>
	<!-- autocomplete library -->
	<script src="js/bootstrap-typeahead.js"></script>
	<!-- tour library -->
	<script src="js/bootstrap-tour.js"></script>
	<!-- library for cookie management -->
	<script src="js/jquery.cookie.js"></script>
	<!-- calander plugin -->
	<script src='js/fullcalendar.min.js'></script>
	<!-- data table plugin -->
	<script src='js/jquery.dataTables.min.js'></script>

	<!-- chart libraries start -->
	<script src="js/excanvas.js"></script>
	<script src="js/jquery.flot.min.js"></script>
	<script src="js/jquery.flot.pie.min.js"></script>
	<script src="js/jquery.flot.stack.js"></script>
	<script src="js/jquery.flot.resize.min.js"></script>
	<!-- chart libraries end -->

	<!-- select or dropdown enhancer -->
	<script src="js/jquery.chosen.min.js"></script>
	<!-- checkbox, radio, and file input styler -->
	<script src="js/jquery.uniform.min.js"></script>
	<!-- plugin for gallery image view -->
	<script src="js/jquery.colorbox.min.js"></script>
	<!-- rich text editor library -->
	<script src="js/jquery.cleditor.min.js"></script>
	<!-- notification plugin -->
	<script src="js/jquery.noty.js"></script>
	<!-- file manager library -->
	<script src="js/jquery.elfinder.min.js"></script>
	<!-- star rating plugin -->
	<script src="js/jquery.raty.min.js"></script>
	<!-- for iOS style toggle switch -->
	<script src="js/jquery.iphone.toggle.js"></script>
	<!-- autogrowing textarea plugin -->
	<script src="js/jquery.autogrow-textarea.js"></script>
	<!-- multiple file upload plugin -->
	<script src="js/jquery.uploadify-3.1.min.js"></script>
	<!-- history.js for cross-browser state change on ajax -->
	<script src="js/jquery.history.js"></script>
	<!-- application script for Charisma demo -->
	<!-- <script src="js/charisma.js"></script> -->
	<!-- help script for page help -->
	<script src="js/help.js"></script>
	<!-- application script for Charisma demo -->
	<script src="js/custom.js"></script>

</body>
</html>
