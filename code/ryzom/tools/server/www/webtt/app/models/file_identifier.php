<?php
class FileIdentifier extends AppModel {
	var $name = 'FileIdentifier';
	var $displayField = 'command';
	var $order = 'FileIdentifier.id';

	var $scaffoldForbiddenActions = array("index", "add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'imported_translation_file_id',
			'conditions' => '',
			'fields' => '',
//			'order' => ''
		),
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
//			'order' => ''
		)
	);
}
