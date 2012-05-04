<?php

class PdrUtil {
	/**
	 * Shard seems to be saving guild file every few seconds.
	 * Increase cache valid timer a bit.
	 *
	 * @const int cache expire in seconds
	 */
	const CACHE_MIN_TIME = 15;

	/**
	 * Where does shard keep it's character, guild files
	 *
	 * @var string
	 */
	private $_shard_save;

	/**
	 * Directory where sheet_id.bin is
	 *
	 * @var string
	 */
	private $_sheetid_dir;

	/**
	 * Directory for extracted character, guild xml files
	 *
	 * @var string
	 */
	private $_unpack_dir;

	/**
	 * Public Constructor for PdrUtil
	 *
	 * @param string $save_path
	 * @param string $unpack    temp unpack directory, default is /tmp/
	 */
	function __construct($shard_save, $sheetid_dir, $unpack = '/tmp/'){
		$this->_shard_save  = $shard_save;
		$this->_sheetid_dir = $sheetid_dir;

		$this->_unpack_dir  = $unpack;
	}

	function setShardSaveDirectory($dir){
		$this->_shard_save = $dir;
	}
	function getShardSaveDirectory(){
		return $this->_shard_save;
	}

	function setSheetIdDirectory($dir){
		$this->_sheetid_dir = $dir;
	}
	function getSheetIdDirectory(){
		return $this->_sheetid_dir;
	}

	function setUnpackDirectory($dir){
		$this->_unpack_dir = $dir;
	}
	function getUnpackDirectory(){
		return $this->_unpack_dir;
	}

	/**
	 * Extract $pdr file to $xml file
	 *
	 * @param string $pdr
	 * @param string $xml
	 * @return bool
	 */
	function extract($pdr, $xml){
		if(!file_exists($pdr)){
			return false;
		}

		$pdr_mtime = filemtime($pdr);
		if(file_exists($xml)){
			$xml_mtime = filemtime($xml);
		}else{
			$xml_mtime = 0;
		}

		$diff = $pdr_mtime - $xml_mtime;
		if($diff > self::CACHE_MIN_TIME){
			// remove expired xml file
			@unlink($xml);

			// change working directory to unpack directory to keep pdr_util log file in one place
			$pwd = getcwd();
			chdir($this->_unpack_dir);

			// run pdr_util
			$cmd = sprintf(' -s%s -x -o%s %s', $this->_sheetid_dir, $xml, $pdr);
			exec(CMD_PDR_UTIL.' '.$cmd);

			// change working directory back what it was before
			chdir($pwd);
		}

		// if pdr_util failed, then there is no xml file
		return file_exists($xml);
	}

	/**
	 * @param string $fname
	 * @return string ShardSaveDirectory + fname
	 */
	function getSaveFileName($fname){
		return $this->getShardSaveDirectory().'/'.$fname;
	}

	/**
	 * @param string $fname
	 * return string TempDirectory + $fname
	 */
	function getXmlFileName($fname){
		return $this->getUnpackDirectory().'/'.$fname;
	}

}

