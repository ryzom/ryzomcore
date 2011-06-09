<div class="languages index">
	<h2><?php __('Languages');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('id');?></th>
			<th><?php echo $this->Paginator->sort('name');?></th>
			<th><?php echo $this->Paginator->sort('code');?></th>
			<th><?php echo $this->Paginator->sort('created');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($languages as $language):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $language['Language']['id']; ?>&nbsp;</td>
		<td><?php echo $language['Language']['name']; ?>&nbsp;</td>
		<td><?php echo $language['Language']['code']; ?>&nbsp;</td>
		<td><?php echo $language['Language']['created']; ?>&nbsp;</td>
		<td><?php echo $language['Language']['modified']; ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $language['Language']['id'])); ?>
			<?php echo $this->Html->link(__('Edit', true), array('action' => 'edit', $language['Language']['id'])); ?>
			<?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $language['Language']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $language['Language']['id'])); ?>
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
		<li><?php echo $this->Html->link(__('New Language', true), array('action' => 'add')); ?></li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add')); ?> </li>
	</ul>
</div>