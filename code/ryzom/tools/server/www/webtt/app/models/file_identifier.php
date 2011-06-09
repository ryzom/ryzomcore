<?php
class FileIdentifier extends AppModel {
	var $name = 'FileIdentifier';
	var $displayField = 'command';
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'imported_translation_file_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		)
	);
}
