<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Identifiers', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Best Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
			</ul>

			<h5>Comments</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
			</ul>

			<h5>File Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifier Columns</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifier Columns', true)), array('controller' => 'identifier_columns', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Add %s', true), __('Identifier', true)); ?></h2>
    
	<div class="identifiers form">
	<?php echo $this->Form->create('Identifier');?>
		<fieldset>
				 		<legend><?php printf(__('Identifier', true)); ?></legend>
					<?php
		echo $this->Form->input('language_id');
		echo $this->Form->input('translation_file_id');
		echo $this->Form->input('translation_index');
		echo $this->Form->input('identifier');
		echo $this->Form->input('arguments');
		echo $this->Form->input('reference_string');
		echo $this->Form->input('translated');
	?>
		</fieldset>
		<div class="box">
<?php echo $this->Form->end(__('Submit', true));?>
</div>	</div>

</div>
<div class="clear"></div>
