<div class="votes index">
	<h2><?php __('Source files');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('filename');?></th>
			<th><?php echo $this->Paginator->sort('size');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($rawFiles as $object):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $object['RawFile']['filename']; ?>&nbsp;</td>
		<td><?php echo $object['RawFile']['size']; ?>&nbsp;</td>
		<td><?php echo $this->Time->nice($object['RawFile']['modified']); ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $object['RawFile']['filename'])); ?>
			<?php echo $this->Html->link(__('Import', true), array('action' => 'import', $object['RawFile']['filename'])); ?>
			<?php echo $this->Html->link(__('Export', true), array('action' => 'export', $object['RawFile']['filename']), null, sprintf(__('Are you sure you want to export # %s?', true), $object['RawFile']['filename'])); ?>
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
		<li><?php echo $this->Html->link(__('List Raw Files', true), array('controller' => 'raw_files', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index')); ?> </li>
		<li style="padding-left: 10px"><?php echo $this->Html->link(__('New Translation', true), array('controller' => 'translations', 'action' => 'add')); ?> </li>
	</ul>
</div>