<?php

class GuildPdr extends PdrUtil {

	/**
	 * Factory method to create new instance using default path values from config.php
	 */
	static public function createDefault(){
		return new self(SHARD_SAVE, SHEETID_DIR, TEMP_UNPACK);
	}

	/**
	 * Load guild info from shard binary file
	 *
	 * @param int $guild_id
	 * @return SimpleXML object or boolean FALSE on error
	 */
	function load($guild_id){
		$guild_save = $this->getSaveFileName($guild_id);
		$xml_file   = $this->getXmlFileName($guild_id);

		if($this->extract($guild_save, $xml_file)){
			return simplexml_load_file($xml_file);
		}

		// extract failed
		return false;
	}

	/**
	 * @param int $guild_id
	 * @return string full path to guild binary file
	 */
	function getSaveFileName($guild_id){
		// chop off shard component from guild id
		return parent::getSaveFileName(sprintf('guilds/guild_%05d.bin', $guild_id & 0xFFFFF));
	}

	/**
	 * @param $guild_id
	 * return string full path to extracted guild xml file
	 */
	function getXmlFileName($guild_id){
		return parent::getXmlFileName(sprintf('guild_%d.xml', $guild_id));
	}
}

