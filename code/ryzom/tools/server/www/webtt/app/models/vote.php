<?php
class Vote extends AppModel {
	var $name = 'Vote';
	var $displayField = 'translation_id';

	var $scaffoldForbiddenActions = array("delete");
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'translation_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'User' => array(
			'className' => 'User',
			'foreignKey' => 'user_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		)
	);
}
