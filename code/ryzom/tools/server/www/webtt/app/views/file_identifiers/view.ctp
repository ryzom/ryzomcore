<div class="fileIdentifiers view">
<h2><?php  __('File Identifier');?></h2>
	<dl><?php $i = 0; $class = ' class="altrow"';?>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Id'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['id']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Translation File'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $this->Html->link($fileIdentifier['TranslationFile']['filename'], array('controller' => 'translation_files', 'action' => 'view', $fileIdentifier['TranslationFile']['id'])); ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Command'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['command']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Translation Index'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['translation_index']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Identifier'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $this->Html->link($fileIdentifier['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $fileIdentifier['Identifier']['id'])); ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Reference String'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['reference_string']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Created'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['created']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Modified'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $fileIdentifier['FileIdentifier']['modified']; ?>
			&nbsp;
		</dd>
	</dl>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>
		<li><?php echo $this->Html->link(__('Edit File Identifier', true), array('action' => 'edit', $fileIdentifier['FileIdentifier']['id'])); ?> </li>
		<li><?php echo $this->Html->link(__('Delete File Identifier', true), array('action' => 'delete', $fileIdentifier['FileIdentifier']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $fileIdentifier['FileIdentifier']['id'])); ?> </li>
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New File Identifier', true), array('action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
	</ul>
</div>
