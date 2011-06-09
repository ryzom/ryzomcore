<div class="translations index">
	<h2><?php __('Translations');?></h2>
	<table cellpadding="0" cellspacing="0">
	<tr>
			<th><?php echo $this->Paginator->sort('id');?></th>
			<th><?php echo $this->Paginator->sort('identifier_id');?></th>
			<th><?php echo $this->Paginator->sort('translation_text');?></th>
			<th><?php echo $this->Paginator->sort('user_id');?></th>
			<th><?php echo $this->Paginator->sort('created');?></th>
			<th><?php echo $this->Paginator->sort('modified');?></th>
			<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
	$i = 0;
	foreach ($translations as $translation):
		$class = null;
		if ($i++ % 2 == 0) {
			$class = ' class="altrow"';
		}
	?>
	<tr<?php echo $class;?>>
		<td><?php echo $translation['Translation']['id']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($translation['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $translation['Identifier']['id'])); ?>
		</td>
		<td><?php echo $translation['Translation']['translation_text']; ?>&nbsp;</td>
		<td>
			<?php echo $this->Html->link($translation['User']['name'], array('controller' => 'users', 'action' => 'view', $translation['User']['id'])); ?>
		</td>
		<td><?php echo $translation['Translation']['created']; ?>&nbsp;</td>
		<td><?php echo $translation['Translation']['modified']; ?>&nbsp;</td>
		<td class="actions">
			<?php echo $this->Html->link(__('View', true), array('action' => 'view', $translation['Translation']['id'])); ?>
			<?php echo $this->Html->link(__('Edit', true), array('action' => 'edit', $translation['Translation']['id'])); ?>
			<?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $translation['Translation']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['Translation']['id'])); ?>
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
		<li><?php echo $this->Html->link(__('New Translation', true), array('action' => 'add')); ?></li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li style="padding-left: 10px"><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Users', true), array('controller' => 'users', 'action' => 'index')); ?> </li>
		<li style="padding-left: 10px"><?php echo $this->Html->link(__('New User', true), array('controller' => 'users', 'action' => 'add')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Votes', true), array('controller' => 'votes', 'action' => 'index')); ?> </li>
		<li style="padding-left: 10px"><?php echo $this->Html->link(__('New Vote', true), array('controller' => 'votes', 'action' => 'add')); ?> </li>
	</ul>
</div>