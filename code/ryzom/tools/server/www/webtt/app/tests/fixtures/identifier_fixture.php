<?php
/* Identifier Fixture generated on: 2011-05-29 19:17:37 : 1306689457 */
class IdentifierFixture extends CakeTestFixture {
	var $name = 'Identifier';

	var $fields = array(
		'id' => array('type' => 'integer', 'null' => false, 'default' => NULL, 'length' => 10, 'key' => 'primary'),
		'translation_file_id' => array('type' => 'integer', 'null' => true, 'default' => NULL, 'length' => 10),
		'translation_index' => array('type' => 'integer', 'null' => true, 'default' => NULL, 'length' => 10),
		'identifier' => array('type' => 'string', 'null' => true, 'default' => NULL, 'collate' => 'utf8_general_ci', 'charset' => 'utf8'),
		'reference_string' => array('type' => 'text', 'null' => true, 'default' => NULL, 'collate' => 'utf8_general_ci', 'charset' => 'utf8'),
		'translated' => array('type' => 'boolean', 'null' => true, 'default' => '0'),
		'created' => array('type' => 'datetime', 'null' => true, 'default' => NULL),
		'modified' => array('type' => 'datetime', 'null' => true, 'default' => NULL),
		'indexes' => array('PRIMARY' => array('column' => 'id', 'unique' => 1)),
		'tableParameters' => array('charset' => 'utf8', 'collate' => 'utf8_general_ci', 'engine' => 'MyISAM')
	);

	var $records = array(
		array(
			'id' => 1,
			'translation_file_id' => 1,
			'translation_index' => 1,
			'identifier' => 'Lorem ipsum dolor sit amet',
			'reference_string' => 'Lorem ipsum dolor sit amet, aliquet feugiat. Convallis morbi fringilla gravida, phasellus feugiat dapibus velit nunc, pulvinar eget sollicitudin venenatis cum nullam, vivamus ut a sed, mollitia lectus. Nulla vestibulum massa neque ut et, id hendrerit sit, feugiat in taciti enim proin nibh, tempor dignissim, rhoncus duis vestibulum nunc mattis convallis.',
			'translated' => 1,
			'created' => '2011-05-29 19:17:37',
			'modified' => '2011-05-29 19:17:37'
		),
	);
}
