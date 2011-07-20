<?php
class IdentifierColumn extends AppModel {
	var $name = 'IdentifierColumn';
	var $displayField = 'column_name';

	var $scaffoldForbiddenActions = array("add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");

	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		)
	);

	var $hasMany = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'identifier_column_id',
			'dependent' => true,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),
	);
}
