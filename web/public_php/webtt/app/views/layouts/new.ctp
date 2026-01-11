<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<?php echo $this->Html->charset(); ?>
	<title>
		<?php __('Ryzom Core: Web Translation Tool :: '); ?>
		<?php echo $title_for_layout; ?>
	</title>
	<?php
		echo $this->Html->meta('icon');
		//echo $this->Html->css('cake.generic');
		echo $this->Html->css(array('reset', 'text', 'grid', 'layout', 'nav', 'labelWidth'));
		echo '<!--[if IE 6]>'.$this->Html->css('ie6').'<![endif]-->';
		echo '<!--[if IE 7]>'.$this->Html->css('ie').'<![endif]-->';
		echo $this->Html->script(array('jquery-1.3.2.min.js', 'jquery-ui.js', 'jquery-fluid16.js'));
		echo $scripts_for_layout;
	?>
	<script>
		function fix_layout() {
			$('.dd').each(function(index) {
				if ($.trim($(this).text()) == "")
				{
					$(this).html('&nbsp;');
				}
			});
		}
		$(function() { fix_layout(); });
	</script>
</head>
<body>
	<div class="container_16" style="background: none repeat scroll 0pt 0pt rgba(40, 60, 60, 0.6);">
		<div class="grid_16">
			<div style="text-align: right; float: right">
				<a href="http://dev.ryzom.com/"><img border="0" alt="" src="http://www.ryzom.com/data/logo.gif"></a>
			</div>
			<h2 id="branding" style="float: left">
				<a href="/">Ryzom Core: Web-Based Translation Tool</a>
			</h1>
		</div>
		<div style="clear: both"></div>
	</div>
	<div class="clear" style="height: 10px; width: 100%;" style="clear: both"></div>
	<?php
	if (isset($this->params['prefix']) && $this->params['prefix'] == "admin") {
		?>
		<div class="container_16" style="background: none repeat scroll 0pt 0pt rgba(40, 60, 60, 0.9);">
			<div class="grid_16">
				<div style="margin:5px">
				<h5>
				<?php echo $this->Html->link(__('Back to admin page', true), array('controller' => 'pages', 'action' => 'display', 'prefix' => 'admin', 'home')); ?>
				</h5>
				</div>		
			</div>
			<div style="clear: both"></div>
		</div>
		<div class="clear" style="height: 10px; width: 100%;" style="clear: both"></div>
		<?php
	}
	?>
	<div class="container_16" style="background: none repeat scroll 0pt 0pt rgba(40, 60, 60, 0.9);">
		<div class="grid_16">
		<?php
			if (isset($assocPath)) {
			?>
			<div style="margin:5px; float: left">
			<h5>/
			<?php
			$path = $assocPath;
			$text = null;
			while ($path)
			{
				$model = key($path);
				$path = $path[$model];
				$controller = Inflector::pluralize(Inflector::underscore($model));
				echo $this->Html->link(__(Inflector::pluralize($model), true), array('controller' => $controller, 'action' => 'index'));
				if ($path)
					echo " / ";
			}
			?>
			</h5>
			</div>
			<?php
			}
			?>
			<div style="margin: 5px; float: right"><h5>
			<?php
			if ($this->Session->read('Auth.User.id'))
				echo $this->Html->link(__('Logout', true), array('admin' => false, 'controller' => 'users', 'action' => 'logout'));
			else if ($this->params['controller'] == 'users')
				echo $this->Html->link(__('Register', true), array('admin' => false, 'controller' => 'users', 'action' => 'register'));
			?>
			</h5></div>
			<div style="clear: both"></div>
		</div>
		<div style="clear: both"></div>
	</div>
	<div class="clear" style="height: 10px; width: 100%;" style="clear: both"></div>
	<div class="container_16">
<!--		<div class="clear"></div>-->
		
		<div class="clear" style="height: 10px; width: 100%;"></div>
		
			<?php echo $this->Session->flash(); ?>
			<?php echo $this->Session->flash('auth'); ?>

			<?php echo $content_for_layout; ?>
		
		<div class="clear" style="height: 10px; width: 100%;" style="clear: both"></div>

		<div class="clear"></div>
	</div>
	<?php // echo $this->element('sql_dump'); ?>
</body>
</html>
