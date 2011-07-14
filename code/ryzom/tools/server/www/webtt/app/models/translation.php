<?php
class Translation extends AppModel {
	var $name = 'Translation';
	var $displayField = 'translation_text';

	var $scaffoldForbiddenActions = array();
	var $scaffoldActions = array("add" => "fk", "index" => "fk");
	//The Associations below have been created with all possible keys, those that are not needed can be removed

	var $belongsTo = array(
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'IdentifierColumn' => array(
			'className' => 'IdentifierColumn',
			'foreignKey' => 'identifier_column_id',
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

	var $hasMany = array(
		'Vote' => array(
			'className' => 'Vote',
			'foreignKey' => 'translation_id',
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
/*		'Comment' => array(
			'className' => 'Comment',
			'foreignKey' => 'translation_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),*/
	);

	function setBest()
	{
		if (!$this->id)
			return false;
		$this->read();
		if (!isset($this->data['Translation']['identifier_id']))
			return false;

		// set best on chosen translation
		$ret = $this->save(array('best' => 1));
		$best_id = $this->id;

		// reset best on other translations
		$ret = $this->updateAll(array('Translation.best' => 0), array(
			'AND' => array(
				'Translation.identifier_id' => $ret['Translation']['identifier_id'],
				'Translation.id !=' => $best_id,
			),
		));
		$this->log($ret);
		// TODO: test!
		
		return $this->id;
	}
}
