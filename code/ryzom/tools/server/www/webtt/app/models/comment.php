<?php
class Comment extends AppModel {
	var $name = 'Comment';
	var $displayField = 'id';

	var $scaffoldActions = array("add" => "fk");
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
/*		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'translation_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),*/
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
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
