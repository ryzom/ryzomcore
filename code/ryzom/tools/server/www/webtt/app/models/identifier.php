<?php
class Identifier extends AppModel {
	var $name = 'Identifier';
	var $displayField = 'identifier';
	var $actsAs = array('Containable');
	var $validate = array(
		'translation_index' => array(
			'numeric' => array(
				'rule' => array('numeric'),
				//'message' => 'Your custom message here',
				//'allowEmpty' => false,
				//'required' => false,
				//'last' => false, // Stop validation after this rule
				//'on' => 'create', // Limit validation to 'create' or 'update' operations
			),
		),
		'identifier' => array(
			'A_Za_z0_9' => array(
				'rule' => '/[A-Za-z0-9_@]+/',
				'message' => 'Identifier must consist only of the following caracteres: "A-Z", "a-z", "0-9", "@" and "_"',
				//'allowEmpty' => false,
				//'required' => false,
				//'last' => false, // Stop validation after this rule
				//'on' => 'create', // Limit validation to 'create' or 'update' operations
			),
		),
	);
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'Language' => array(
			'className' => 'Language',
			'foreignKey' => 'language_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		)
	);

	var $hasMany = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'identifier_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),
		'FileIdentifier' => array(
			'className' => 'FileIdentifier',
			'foreignKey' => 'identifier_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		)
	);

}
