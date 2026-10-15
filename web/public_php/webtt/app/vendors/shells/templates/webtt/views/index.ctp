<?php
/**
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       cake
 * @subpackage    cake.cake.console.libs.templates.views
 * @since         CakePHP(tm) v 0.10.0.1076
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 * WebTT bake template based on 960grid template
 * http://bakery.cakephp.org/articles/tom_m/2010/05/26/960-fluid-grid-system-bake-templates
 *
 */
?>
<div class="grid_3">
	<div class="box menubox">
		<h2><a href="#" id="toggle-admin-actions">Actions</a></h2>
		<div class="inbox">
			<div class="block" id="admin-actions">
				<h5><?php echo "<?php echo __('" . $pluralHumanName . "', true); ?>"; ?></h5>
				<ul class="menu">
				<?php
				if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "index", $scaffoldForbiddenActions)))
					echo "\t<li><?php echo \$this->Html->link(sprintf(__('List %s', true), __('{$pluralHumanName}', true)), array('action' => 'index')); ?> </li>\n";
				if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "add", $scaffoldForbiddenActions)))
					echo "\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('{$singularHumanName}', true)), array('action' => 'add')); ?> </li>\n";
				?>
				</ul><?php echo "\n";
				$done = array();
				foreach ($associations as $_type => $_data)
				{
					foreach ($_data as $alias => $details)
					{
						if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions'])))
						{
							echo "\t\t\t\t<h5><?php echo __('" . Inflector::humanize($details['controller']) . "', true); ?></h5>\n";
							echo "\t\t\t\t<ul class=\"menu\">\n";
							if ($details['controller'] != $this->name && !in_array($details['controller'], $done))
							{
								if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions'])))
									echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List all %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index')); ?> </li>\n";
								if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions'])))
									echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add')); ?> </li>\n";
								$done[] = $details['controller'];
							}
							echo "\t\t\t\t</ul>\n";
						}
					}
				}
				?>
			</div>
		</div>
	</div>
</div>

<div class="grid_13">
	<h2 id="page-heading"><?php echo "<?php __('{$pluralHumanName}');?>";?></h2>
	<table cellpadding="0" cellspacing="0"><?php
		//// TABLE HEADERS
		echo "\t\t<?php \$tableHeaders = \$html->tableHeaders(array("; 

		foreach($fields as $field) {
			if (!(empty($scaffoldForbiddenFields) || !isset($scaffoldForbiddenFields[$scaffoldPrefix]) || (!empty($scaffoldForbiddenFields) && isset($scaffoldForbiddenFields[$scaffoldPrefix]) && !in_array($field, $scaffoldForbiddenFields[$scaffoldPrefix]))))
				continue;

			echo "\$paginator->sort('{$field}'),";
		}
		echo "__('Actions', true),";
		echo "));\n";
		echo "\t\techo '<thead>'.\$tableHeaders.'</thead>'; ?>\n\n"; 

		//// TABLE ROWS
		echo "\t\t<?php
		\$i = 0;
		foreach (\${$pluralVar} as \${$singularVar}):
			\$class = null;
			if (\$i++ % 2 == 0) {
				\$class = ' class=\"altrow\"';
			}
		?>\n";
		echo "\t<tr<?php echo \$class;?>>\n";
			foreach ($fields as $field) {
				if (!(empty($scaffoldForbiddenFields) || !isset($scaffoldForbiddenFields[$scaffoldPrefix]) || (!empty($scaffoldForbiddenFields) && isset($scaffoldForbiddenFields[$scaffoldPrefix]) && !in_array($field, $scaffoldForbiddenFields[$scaffoldPrefix]))))
					continue;

				$isKey = false;
				if (!empty($associations['belongsTo'])) {
					foreach ($associations['belongsTo'] as $alias => $details) {
						if ($field === $details['foreignKey'] && $field !== $primaryKey) {
							$isKey = true;
							echo "\t\t<td>\n\t\t\t<?php echo \$this->Html->link(\${$singularVar}['{$alias}']['{$details['displayField']}'], array('controller' => '{$details['controller']}', 'action' => 'view', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>\n\t\t</td>\n";
							break;
						}
					}
				}
				if ($isKey !== true) {
					echo "\t\t<td><?php echo \${$singularVar}['{$modelClass}']['{$field}']; ?></td>\n";
				}
			}

			echo "\t\t<td class=\"actions\">\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "view", $scaffoldForbiddenActions)))
				echo "\t\t\t<?php echo \$this->Html->link(__('View', true), array('action' => 'view', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "edit", $scaffoldForbiddenActions)))
				echo "\t\t\t<?php echo ' | ' . \$this->Html->link(__('Edit', true), array('action' => 'edit', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "delete", $scaffoldForbiddenActions)))
			 	echo "\t\t\t<?php echo ' | ' . \$this->Html->link(__('Delete', true), array('action' => 'delete', \${$singularVar}['{$modelClass}']['{$primaryKey}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
			echo "\t\t</td>\n";
			echo "\t</tr>\n";

			echo "\t<?php endforeach; ?>\n";
			//// TABLE FOOTER
			echo "\t<?php echo '<tfoot class=\'dark\'>'.\$tableHeaders.'</tfoot>'; ?>\n";
			?>
	</table>

	<?php //// PAGINATION ?>

	<p>
	<?php echo "<?php
	echo \$this->Paginator->counter(array(
	'format' => __('Page %page% of %pages%, showing %current% records out of %count% total, starting on record %start%, ending on %end%', true)
	));
	?>";?>
	</p>

	<div class="paging">
	<?php echo "\t<?php echo \$this->Paginator->prev('<< '.__('previous', true), array(), null, array('class'=>'disabled'));?>\n";?>
		| <?php echo "<?php echo \$this->Paginator->numbers();?>"?> |
	<?php echo "\t<?php echo \$this->Paginator->next(__('next', true).' >>', array(), null, array('class' => 'disabled'));?>\n";?>
	</div>
</div>
<div class="clear"></div>
