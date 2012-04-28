<?php

class CharacterPdr extends PdrUtil {

	/**
	 * Factory method to create new instance using default path values from config.php
	 */
	static public function createDefault(){
		return new self(SHARD_SAVE, SHEETID_DIR, TEMP_UNPACK);
	}

	/**
	 * Load character from shard binary file
	 *
	 * @param int $char_id
	 * @return SimpleXML object or boolean FALSE on error
	 */
	function load($char_id){
		$char_save = $this->getSaveFileName($char_id >> 4, $char_id & 0x0F);
		$xml_file  = $this->getXmlFileName($char_id);

		if($this->extract($char_save, $xml_file)){
			return simplexml_load_file($xml_file);
		}

		// extract failed
		return false;
	}

	/**
	 * @param int $uid  user id
	 * @param int $slot character slot, starting from 0
	 * @return string character save path + filename
	 */
	function getSaveFileName($uid, $slot){
		return parent::getSaveFileName(sprintf('characters/%03d/account_%d_%d_pdr.bin', $uid, $uid, $slot));
	}

	/**
	 * @param $char_id
	 * return string character xml file in unpack directory
	 */
	function getXmlFileName($char_id){
		return parent::getXmlFileName(sprintf('character_%d.xml', $char_id));
	}
}
