<div class="identifiers index">
	<h2><?php __('Identifiers');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('id');?></th>
			<th><?php echo $this->Paginator->sort('language_id');?></th>
			<th><?php echo $this->Paginator->sort('translation_index');?></th>
			<th><?php echo $this->Paginator->sort('identifier');?></th>
			<th><?php echo $this->Paginator->sort('reference_string');?></th>
			<th><?php echo $this->Paginator->sort('translated');?></th>
			<th><?php echo $this->Paginator->sort('created');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($identifiers as $identifier):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $identifier['Identifier']['id']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($identifier['Language']['name'], array('controller' => 'languages', 'action' => 'view', $identifier['Language']['id'])); ?>
		</td>
		<td><?php echo $identifier['Identifier']['translation_index']; ?>&nbsp;</td>
		<td><?php echo $identifier['Identifier']['identifier']; ?>&nbsp;</td>
		<td><?php echo $identifier['Identifier']['reference_string']; ?>&nbsp;</td>
		<td><?php echo $identifier['Identifier']['translated']; ?>&nbsp;</td>
		<td><?php echo $identifier['Identifier']['created']; ?>&nbsp;</td>
		<td><?php echo $identifier['Identifier']['modified']; ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $identifier['Identifier']['id'])); ?>
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
		<li><?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index')); ?> </li>
		<li style="padding-left: 10px"><?php echo $this->Html->link(__('New Translation', true), array('controller' => 'translations', 'action' => 'add')); ?> </li>
	</ul>
</div>