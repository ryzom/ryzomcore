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
 *
 * WebTT bake template based on 960grid template
 * http://bakery.cakephp.org/articles/tom_m/2010/05/26/960-fluid-grid-system-bake-templates
 *
 */
?>
<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo "<?php echo __('" . $pluralHumanName . "', true); ?>"; ?></h5>
			<ul class="menu">
			<?php if (strpos($action, 'add') === false): ?><?php
				if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "delete", $scaffoldForbiddenActions))) {
				echo "\t\t\t"; ?><li><?php echo "<?php echo \$this->Html->link(sprintf(__('Delete this %s', true), __('" . Inflector::humanize(Inflector::underscore($modelClass)) . "', true)),array('action' => 'delete', \$this->Form->value('{$modelClass}.{$primaryKey}')), null, sprintf(__('Are you sure you want to delete # %s?', true), \$this->Form->value('{$modelClass}.{$primaryKey}'))); ?>";?></li><?php
				} ?>
			<?php endif;?><?php
				if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "index", $scaffoldForbiddenActions))) {
				echo "\t\t\t"; ?><li><?php echo "<?php echo \$this->Html->link(sprintf(__('List all %s', true), __('{$pluralHumanName}', true)), array('action' => 'index'));?>";?></li><?php
				} ?>
			</ul>
			<?php
					$done = array();
					foreach ($associations as $_type => $_data)
					{
						foreach ($_data as $alias => $details)
						{
							if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions'])))
							{
								echo "\n\t\t\t<h5>".Inflector::humanize($details['controller'])."</h5>";
								echo "\n\t\t\t<ul class=\"menu\">\n";
								if ($details['controller'] != $this->name && !in_array($details['controller'], $done)) {
									if ($_type == 'hasMany' && strpos($action, 'add') === false)
									{
										echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List related %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index', '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
										if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions'])))
											echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New related %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add', '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
									}

									if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions'])))
										echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List all %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index')); ?> </li>\n";
									if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions'])))
										echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add')); ?> </li>\n";
									$done[] = $details['controller'];
								}
								echo "\t\t\t</ul>\n";
							}
						}
					}
			?>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php echo "<?php printf(__('" . Inflector::humanize($action) . " %s', true), __('{$singularHumanName}', true)); ?>";?></h2>
    
	<div class="<?php echo $pluralVar;?> form">
	<?php echo "<?php echo \$this->Form->create('{$modelClass}');?>\n";?>
		<fieldset>
			<?php if (strpos($action, 'add') === false): ?>
			<legend><?php echo "<?php printf(__('{$singularHumanName} # %s', true), \$this->Form->value('{$modelClass}.{$primaryKey}')); ?>";?></legend>
			<?php else:;?>
	 		<legend><?php echo "<?php printf(__('{$singularHumanName}', true)); ?>";?></legend>
			<?php endif;?>
	<?php
			echo "\t<?php\n";
			foreach ($fields as $field) {
				if (!(empty($scaffoldForbiddenFields) || !isset($scaffoldForbiddenFields[$scaffoldPrefix]) || (!empty($scaffoldForbiddenFields) && isset($scaffoldForbiddenFields[$scaffoldPrefix]) && !in_array($field, $scaffoldForbiddenFields[$scaffoldPrefix]))))
					continue;

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
			echo "<div class=\"box\">\n";
			echo "<?php echo \$this->Form->end(__('Submit', true));?>\n";
			echo "</div>";
		?>
	</div>

</div>
<div class="clear"></div>
