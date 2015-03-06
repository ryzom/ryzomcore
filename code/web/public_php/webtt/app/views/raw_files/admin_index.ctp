<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5>Raw Files</h5>
				<ul class="menu">				
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Raw Files', true)), array('action' => 'index')); ?> </li>
				</ul>
				<h5>Imported Translation Files</h5>
<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('Raw Files');?></h2>
	<table cellpadding="0" cellspacing="0">
		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('filename'),$paginator->sort('size'),$paginator->sort('modified'),__('Actions', true),));
echo '<thead>'.$tableHeaders.'</thead>'; ?>

<?php
		$i = 0;
		foreach ($rawFiles as $rawFile):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $rawFile['RawFile']['filename']; ?></td>
		<td><?php echo $rawFile['RawFile']['size']; ?></td>
		<td><?php echo $this->Time->nice($rawFile['RawFile']['modified']); ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $rawFile['RawFile']['filename'])); ?>
			<?php echo ' | ' . $this->Html->link(__('Import', true), array('action' => 'import', $rawFile['RawFile']['filename'])); ?>
<!--			<?php echo ' | ' . $this->Html->link(__('Export', true), array('action' => 'export', $rawFile['RawFile']['filename'])); ?>-->
<!--			<?php echo ' | ' . __('Export', true); ?>-->
<!--			<?php echo ' | ' . $this->Html->link(__('Delete', true), array('action' => 'delete', $rawFile['RawFile']['filename']), null, sprintf(__('Are you sure you want to delete # %s?', true), $rawFile['RawFile']['filename'])); ?>-->
		</td>
	</tr>
<?php endforeach; ?>
<?php echo '<tfoot class=\'dark\'>'.$tableHeaders.'</tfoot>'; ?>	</table>

	
	<p>
	<?php
	echo $this->Paginator->counter(array(
	'format' => __('Page %page% of %pages%, showing %current% records out of %count% total, starting on record %start%, ending on %end%', true)
	));
	?>	</p>

	<div class="paging">
		<?php echo $this->Paginator->prev('<< '.__('previous', true), array(), null, array('class'=>'disabled'));?>
	 | 	<?php echo $this->Paginator->numbers();?>
 |
		<?php echo $this->Paginator->next(__('next', true).' >>', array(), null, array('class' => 'disabled'));?>
	</div>
</div>
<div class="clear"></div>
<? echo $this->element('sql_dump');?>