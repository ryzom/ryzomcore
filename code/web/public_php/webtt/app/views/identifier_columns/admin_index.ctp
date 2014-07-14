<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Identifier Columns', true); ?></h5>
				<ul class="menu">
					<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifier Columns', true)), array('action' => 'index')); ?> </li>
	<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Identifier Column', true)), array('action' => 'add')); ?> </li>
				</ul>
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Translations', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('Identifier Columns');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('identifier_id'),$paginator->sort('column_name'),$paginator->sort('reference_string'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($identifierColumns as $identifierColumn):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $identifierColumn['IdentifierColumn']['id']; ?></td>
		<td>
			<?php echo $this->Html->link($identifierColumn['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $identifierColumn['Identifier']['id'])); ?>
		</td>
		<td><?php echo $identifierColumn['IdentifierColumn']['column_name']; ?></td>
		<td><?php echo $identifierColumn['IdentifierColumn']['reference_string']; ?></td>
		<td><?php echo $identifierColumn['IdentifierColumn']['created']; ?></td>
		<td><?php echo $identifierColumn['IdentifierColumn']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $identifierColumn['IdentifierColumn']['id'])); ?>
			<?php echo ' | ' . $this->Html->link(__('Edit', true), array('action' => 'edit', $identifierColumn['IdentifierColumn']['id'])); ?>
			<?php echo ' | ' . $this->Html->link(__('Delete', true), array('action' => 'delete', $identifierColumn['IdentifierColumn']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $identifierColumn['IdentifierColumn']['id'])); ?>
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
