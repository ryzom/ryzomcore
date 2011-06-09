<?php
class ImportedTranslationFile extends AppModel {
	var $name = 'ImportedTranslationFile';
	var $displayField = 'filename';
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $actsAs = array('Containable');

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
		'FileIdentifier' => array(
			'className' => 'FileIdentifier',
			'foreignKey' => 'imported_translation_file_id',
			'dependent' => true,
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

	var $hasOne = array(
		'RawFile' => array(
			'className' => 'RawFile',
			'foreignKey' => 'filename',
			'dependand' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
		),
	);

}
