<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">
					<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Translation Files', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Translations', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Best Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
				</ul>
				<h5><?php echo __('Translations', true); ?></h5>
				<ul class="menu">
				</ul>
				<h5><?php echo __('Comments', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
				</ul>
				<h5><?php echo __('File Identifiers', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Identifier Columns', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifier Columns', true)), array('controller' => 'identifier_columns', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('Identifiers');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('language_id'),$paginator->sort('translation_file_id'),$paginator->sort('translation_index'),$paginator->sort('identifier'),$paginator->sort('arguments'),$paginator->sort('reference_string'),$paginator->sort('translated'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($identifiers as $identifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $identifier['Identifier']['id']; ?></td>
		<td><?php echo $identifier['Identifier']['language_id']; ?></td>
		<td>
			<?php echo $this->Html->link($identifier['TranslationFile']['filename_template'], array('controller' => 'translation_files', 'action' => 'view', $identifier['TranslationFile']['id'])); ?>
		</td>
		<td><?php echo $identifier['Identifier']['translation_index']; ?></td>
		<td><?php echo $identifier['Identifier']['identifier']; ?></td>
		<td><?php echo $identifier['Identifier']['arguments']; ?></td>
		<td><?php echo $identifier['Identifier']['reference_string']; ?></td>
		<td><?php echo $identifier['Identifier']['translated']; ?></td>
		<td><?php echo $identifier['Identifier']['created']; ?></td>
		<td><?php echo $identifier['Identifier']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $identifier['Identifier']['id'])); ?>
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
