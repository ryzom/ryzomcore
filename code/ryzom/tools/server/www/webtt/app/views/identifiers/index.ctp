<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index')); ?> </li>
				</ul>				<h5><?php echo __('Languages', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Translations', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<p class="help">Choose an identifier you want to translate and click "View details and comments" to see details and its translations or use shortcut actions, ie. "Add Translation", "List Translations", "Add Comment".</p>
	<h2 id="page-heading"><?php __('Identifiers');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('translation_file_id'),$paginator->sort('identifier'),$paginator->sort('arguments'),$paginator->sort('reference_string'),$paginator->sort('translated'),__('Actions', true),));
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
		<td>
			<?php echo $this->Html->link($identifier['TranslationFile']['filename_template'], array('controller' => 'translation_files', 'action' => 'view', $identifier['TranslationFile']['id'])); ?>
		</td>
		<td><?php echo $identifier['Identifier']['identifier']; ?></td>
		<td><?php echo $identifier['Identifier']['arguments']; ?></td>
		<td><?php echo $identifier['Identifier']['reference_string']; ?></td>
		<td><?php echo $identifier['Identifier']['translated']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View details and comments', true), array('action' => 'view', $identifier['Identifier']['id'])); ?>
			| <?php echo $this->Html->link(__('Add Comment', true), array('controller' => 'comments', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id'])); ?>
			| <?php echo $this->Html->link(__('Add Translation', true), array('controller' => 'translations', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id'])); ?>
			| <?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index', 'identifier_id' => $identifier['Identifier']['id'])); ?>
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
