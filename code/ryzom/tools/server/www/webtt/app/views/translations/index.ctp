<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Translations', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Users', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Votes', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('Translations');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('identifier_id'),$paginator->sort('translation_text'),$paginator->sort('user_id'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($translations as $translation):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $translation['Translation']['id']; ?></td>
		<td>
			<?php echo $this->Html->link($translation['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $translation['Identifier']['id'])); ?>
		</td>
		<td><?php echo $translation['Translation']['translation_text']; ?></td>
		<td>
			<?php echo $this->Html->link($translation['User']['name'], array('controller' => 'users', 'action' => 'view', $translation['User']['id'])); ?>
		</td>
		<td><?php echo $translation['Translation']['created']; ?></td>
		<td><?php echo $translation['Translation']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $translation['Translation']['id'])); ?>
			<?php echo ' | ' . $this->Html->link(__('Edit', true), array('action' => 'edit', $translation['Translation']['id'])); ?>
			<?php echo ' | ' . $this->Html->link(__('Delete', true), array('action' => 'delete', $translation['Translation']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['Translation']['id'])); ?>
			| <?php echo $this->Html->link(__('Vote', true), array('controller' => 'votes', 'action' => 'vote', 'translation_id' => $translation['Translation']['id'])); ?>
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
