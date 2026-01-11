<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Languages', true); ?></h5>
				<ul class="menu">
					<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Translation Files', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<p class="help">Choose a language and click "List Translation Files" to see project language files you can translate.</p>
	<h2 id="page-heading"><?php __('Languages');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('name'),$paginator->sort('code'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($languages as $language):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $language['Language']['id']; ?></td>
		<td><?php echo $language['Language']['name']; ?></td>
		<td><?php echo $language['Language']['code']; ?></td>
		<td><?php echo $language['Language']['created']; ?></td>
		<td><?php echo $language['Language']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $language['Language']['id'])); ?>
			| <?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index', 'language_id' => $language['Language']['id'])); ?>
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
