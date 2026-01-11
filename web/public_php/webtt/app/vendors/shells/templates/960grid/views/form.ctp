<?php
/**
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       cake
 * @subpackage    cake.cake.console.libs.templates.views
 * @since         CakePHP(tm) v 1.2.0.5234
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 */
?>
<div class="grid_4">	
	<div class="box">			
		<?php //// ACTIONS ?>
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="block" id="admin-actions">			
			<h5><?php echo $pluralHumanName; ?></h5>
			<ul class="menu">
			<?php if (strpos($action, 'add') === false): ?>
					<li><?php echo "<?php echo \$this->Html->link(__('Delete', true), array('action' => 'delete', \$this->Form->value('{$modelClass}.{$primaryKey}')), null, sprintf(__('Are you sure you want to delete # %s?', true), \$this->Form->value('{$modelClass}.{$primaryKey}'))); ?>";?></li>
			<?php endif;?>
					<li><?php echo "<?php echo \$this->Html->link(sprintf(__('List %s', true), __('{$pluralHumanName}', true)), array('action' => 'index'));?>";?></li>
			</ul>
			<?php
					$done = array();
					foreach ($associations as $type => $data) {							
						foreach ($data as $alias => $details) {								
							echo "\n\t\t\t<h5>".Inflector::humanize($details['controller'])."</h5>";
							echo "\n\t\t\t<ul class=\"menu\">\n";
							if ($details['controller'] != $this->name && !in_array($details['controller'], $done)) {
								echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index')); ?> </li>\n";
								echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add')); ?> </li>\n";
								$done[] = $details['controller'];
							}
							echo "\t\t\t</ul>\n";
						}							
					}
			?>
		</div>
	</div>
</div>

<div class="grid_12">
    <h2 id="page-heading"><?php echo "<?php printf(__('" . Inflector::humanize($action) . " %s', true), __('{$singularHumanName}', true)); ?>";?></h2>
    
	<div class="<?php echo $pluralVar;?> form">
	<?php echo "<?php echo \$this->Form->create('{$modelClass}');?>\n";?>
		<fieldset>
	 		<legend><?php echo "<?php printf(__('{$singularHumanName} Record', true)); ?>";?></legend>
	<?php
			echo "\t<?php\n";
			foreach ($fields as $field) {
				if (strpos($action, 'add') !== false && $field == $primaryKey) {
					continue;
				} elseif (!in_array($field, array('created', 'modified', 'updated'))) {
					echo "\t\techo \$this->Form->input('{$field}');\n";
				}
			}
			if (!empty($associations['hasAndBelongsToMany'])) {
				foreach ($associations['hasAndBelongsToMany'] as $assocName => $assocData) {
					echo "\t\techo \$this->Form->input('{$assocName}');\n";
				}
			}
			echo "\t?>\n";
	?>
		</fieldset>
	<?php
		echo "<?php echo \$this->Form->end(__('Submit', true));?>\n";
	?>
	</div>

</div>
<div class="clear"></div>
