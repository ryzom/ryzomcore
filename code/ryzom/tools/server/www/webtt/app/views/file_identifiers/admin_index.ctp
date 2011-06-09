<div class="fileIdentifiers index">
	<h2><?php __('File Identifiers');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('id');?></th>
			<th><?php echo $this->Paginator->sort('translation_file_id');?></th>
			<th><?php echo $this->Paginator->sort('command');?></th>
			<th><?php echo $this->Paginator->sort('translation_index');?></th>
			<th><?php echo $this->Paginator->sort('identifier_id');?></th>
			<th><?php echo $this->Paginator->sort('reference_string');?></th>
			<th><?php echo $this->Paginator->sort('created');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($fileIdentifiers as $fileIdentifier):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $fileIdentifier['FileIdentifier']['id']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($fileIdentifier['TranslationFile']['filename'], array('controller' => 'translation_files', 'action' => 'view', $fileIdentifier['TranslationFile']['id'])); ?>
		</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['command']; ?>&nbsp;</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['translation_index']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($fileIdentifier['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $fileIdentifier['Identifier']['id'])); ?>
		</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['reference_string']; ?>&nbsp;</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['created']; ?>&nbsp;</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['modified']; ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $fileIdentifier['FileIdentifier']['id'])); ?>
			<?php echo $this->Html->link(__('Edit', true), array('action' => 'edit', $fileIdentifier['FileIdentifier']['id'])); ?>
			<?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $fileIdentifier['FileIdentifier']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $fileIdentifier['FileIdentifier']['id'])); ?>
		</td>
	</tr>
<?php endforeach; ?>
	</table>
	<p>
	<?php
	echo $this->Paginator->counter(array(
	'format' => __('Page %page% of %pages%, showing %current% records out of %count% total, starting on record %start%, ending on %end%', true)
	));
	?>	</p>

	<div class="paging">
		<?php echo $this->Paginator->prev('<< ' . __('previous', true), array(), null, array('class'=>'disabled'));?>
	 | 	<?php echo $this->Paginator->numbers();?>
 |
		<?php echo $this->Paginator->next(__('next', true) . ' >>', array(), null, array('class' => 'disabled'));?>
	</div>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>
		<li><?php echo $this->Html->link(__('New File Identifier', true), array('action' => 'add')); ?></li>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
	</ul>
</div>