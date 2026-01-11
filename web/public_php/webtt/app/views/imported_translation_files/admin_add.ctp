<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Imported Translation Files', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Languages</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('controller' => 'languages', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Raw Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Raw Files', true)), array('controller' => 'raw_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>File Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Add %s', true), __('Imported Translation File', true)); ?></h2>
    
	<div class="importedTranslationFiles form">
	<?php echo $this->Form->create('ImportedTranslationFile');?>
		<fieldset>
				 		<legend><?php printf(__('Imported Translation File', true)); ?></legend>
					<?php
		echo $this->Form->input('language_id');
		echo $this->Form->input('filename');
		echo $this->Form->input('merged');
		echo $this->Form->input('file_last_modified_date');
	?>
		</fieldset>
	<?php echo $this->Form->end(__('Submit', true));?>
	</div>

</div>
<div class="clear"></div>
