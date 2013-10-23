<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo __('Translation Files', true); ?></h5>
				<ul class="menu">
					<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Languages', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('controller' => 'languages', 'action' => 'add')); ?> </li>
				</ul>
				<h5><?php echo __('Imported Translation Files', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
				</ul>
				<h5><?php echo __('Identifiers', true); ?></h5>
				<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				</ul>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php __('Translation Files');?></h2>
	<table cellpadding="0" cellspacing="0">		<?php $tableHeaders = $html->tableHeaders(array($paginator->sort('id'),$paginator->sort('language_id'),$paginator->sort('filename_template'),$paginator->sort('created'),$paginator->sort('modified'),__('Actions', true),));
		echo '<thead>'.$tableHeaders.'</thead>'; ?>

		<?php
		$i = 0;
		foreach ($translationFiles as $translationFile):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
	<tr<?php echo $class;?>>
		<td><?php echo $translationFile['TranslationFile']['id']; ?></td>
		<td>
			<?php echo $this->Html->link($translationFile['Language']['name'], array('controller' => 'languages', 'action' => 'view', $translationFile['Language']['id'])); ?>
		</td>
		<td><?php echo $translationFile['TranslationFile']['filename_template']; ?></td>
		<td><?php echo $translationFile['TranslationFile']['created']; ?></td>
		<td><?php echo $translationFile['TranslationFile']['modified']; ?></td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $translationFile['TranslationFile']['id'])); ?>
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
