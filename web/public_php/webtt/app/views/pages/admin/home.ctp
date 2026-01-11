<?php
/**
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       cake
 * @subpackage    cake.cake.libs.view.templates.pages
 * @since         CakePHP(tm) v 0.10.0.1076
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 */
?>
<div class="grid_8">
	<h2 id="page-heading"><?php __('Translation Collaboration'); ?></h3>

	<div class="box menubox"><div class="inbox">
		<div id="admin-actions" class="block">
		
				<h5><?php __('Languages'); ?></h3>
				<ul class="menu">
				<li>
				<?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('controller' => 'languages', 'action' => 'add')); ?> </li>
				</li>
				</ul>

				<h5>Translation Files</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
				</ul>
		
				<h5>Identifiers</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				</ul>
		
				<h5>Translations</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
				</ul>

				<h5>Comments</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
				</ul>

				<h5>Votes</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
				</ul>

				<h5>Users</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('User', true)), array('controller' => 'users', 'action' => 'add')); ?> </li>
				</ul>
		</div>
	</div></div>
</div>

<!--<div class="grid_4">
	<h2 id="page-heading"><?php __('Both'); ?></h3>

	<div class="box menubox"><div class="inbox">
		<div id="admin-actions" class="block">
		</div>
	</div></div>
</div>-->


<div class="grid_8">
	<h2 id="page-heading"><?php __('Translation Pipeline'); ?></h3>
	<div class="box menubox"><div class="inbox">
		<div id="admin-actions" class="block">
				<h5>Imported Translation Files</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
				</ul>

				<h5>File Identifiers</h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
				</ul>

				<h5>Raw Files</h5>
				<ul class="menu">				
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Raw Files', true)), array('controller' => 'raw_files', 'action' => 'index')); ?> </li>
				</ul>

		</div>
	</div></div>

</div>
