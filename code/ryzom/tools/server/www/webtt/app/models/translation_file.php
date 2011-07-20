<?php
class TranslationFile extends AppModel {
	var $name = 'TranslationFile';
	var $displayField = 'filename_template';

	var $scaffoldForbiddenActions = array("add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");

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
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'translation_file_id',
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
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'translation_file_id',
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
