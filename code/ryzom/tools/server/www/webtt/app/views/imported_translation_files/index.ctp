<div class="importedTranslationFiles index">
	<h2><?php __('Imported Translation Files');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('id');?></th>
			<th><?php echo $this->Paginator->sort('language_id');?></th>
			<th><?php echo $this->Paginator->sort('filename');?></th>
			<th><?php echo $this->Paginator->sort('created');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($importedTranslationFiles as $importedTranslationFile):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $importedTranslationFile['ImportedTranslationFile']['id']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($importedTranslationFile['Language']['name'], array('controller' => 'languages', 'action' => 'view', $importedTranslationFile['Language']['id'])); ?>
		</td>
		<td><?php echo $importedTranslationFile['ImportedTranslationFile']['filename']; ?>&nbsp;</td>
		<td><?php echo $importedTranslationFile['ImportedTranslationFile']['created']; ?>&nbsp;</td>
		<td><?php echo $importedTranslationFile['ImportedTranslationFile']['modified']; ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $importedTranslationFile['ImportedTranslationFile']['id'])); ?>
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
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
	</ul>
</div>