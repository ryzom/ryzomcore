<?php 
/*
	http://bakery.cakephp.org/articles/Jippi/2007/03/25/null-behavior
*/
class NullBehavior extends ModelBehavior {
	var $settings = array();

	/**
	 * Enter description here...
	 *
	 * @param AppModel $model
	 * @param unknown_type $config
	 */
	function setup(&$model, $config = array())
	{
		$this->settings[$model->name] = $config;
	}

	/**
	 * Enter description here...
	 *
	 * @param AppModel $model
	 */
	function beforeSave(&$model)
	{
		foreach ($this->settings[$model->name] as $field)
		{
			if(
				true === array_key_exists($field,$model->data[$model->name]) &&
				true === empty($model->data[$model->name][$field]) &&
				0 === strlen($model->data[$model->name][$field]) )
			{
				$model->data[$model->name][$field] = null;
			}
		}
		return true;
	}
}
?> 
