<?php
class User extends AppModel {
	var $name = 'User';
	var $displayField = 'name';

	var $validate = array(
		'username' => array(
			'alphaNumeric',
/*			'uniqueCheck' => array(
				'rule' => 'isUnique',
				'message' => 'That username has already been taken.',
			),*/
		),
		'email' => array('rule' => 'email', 'message' => 'Wrong format'),
		'name' => array('rule' => 'notEmpty'),
//		'password' => array('rule' => 'notEmpty'),
		'passwd' => array('rule' => 'notEmpty'),
	);

	var $scaffoldForbiddenActions = array("add", "edit", "delete");

	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $hasMany = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'user_id',
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
		'Vote' => array(
			'className' => 'Vote',
			'foreignKey' => 'user_id',
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
