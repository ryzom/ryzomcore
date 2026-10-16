<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('File Identifiers', true); ?></h5>
				<ul class="menu">				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('action' => 'index')); ?> </li>
				</ul>				<h5><?php echo __('Imported Translation Files', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('File Identifiers');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('imported_translation_file_id'),$paginator->sort('command'),$paginator->sort('translation_index'),$paginator->sort('identifier_id'),$paginator->sort('arguments'),$paginator->sort('reference_string'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($fileIdentifiers as $fileIdentifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $fileIdentifier['FileIdentifier']['id']; ?></td>
		<td>
			<?php echo $this->Html->link($fileIdentifier['ImportedTranslationFile']['filename'], array('controller' => 'imported_translation_files', 'action' => 'view', $fileIdentifier['ImportedTranslationFile']['id'])); ?>
		</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['command']; ?></td>
		<td><?php echo $fileIdentifier['FileIdentifier']['translation_index']; ?></td>
		<td>
			<?php echo $this->Html->link($fileIdentifier['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $fileIdentifier['Identifier']['id'])); ?>
		</td>
		<td><?php echo $fileIdentifier['FileIdentifier']['arguments']; ?></td>
		<td><?php echo $fileIdentifier['FileIdentifier']['reference_string']; ?></td>
		<td><?php echo $fileIdentifier['FileIdentifier']['created']; ?></td>
		<td><?php echo $fileIdentifier['FileIdentifier']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $fileIdentifier['FileIdentifier']['id'])); ?>
		</td>
	</tr>
	<?php endforeach; ?>
	<?php echo '<tfoot class=\'dark\'>'.$tableHeaders.'</tfoot>'; ?>
	</table>

	
	<p>
	<?php
	echo $this->Paginator->counter(array(
	'format' => __('Page %page% of %pages%, showing %current% records out of %count% total, starting on record %start%, ending on %end%', true)
	));
	?>	</p>

	<div class="paging">
		<?php echo $this->Paginator->prev('<< '.__('previous', true), array(), null, array('class'=>'disabled'));?>
		| <?php echo $this->Paginator->numbers();?> |
		<?php echo $this->Paginator->next(__('next', true).' >>', array(), null, array('class' => 'disabled'));?>
	</div>
</div>
<div class="clear"></div>
