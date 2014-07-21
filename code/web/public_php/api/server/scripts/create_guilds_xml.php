<?php

include('../../common/db_lib.php');
//require_once('../server_functions_item.php');

$guilds_cache_timeout = 60*60;
$time = time();

$db = new ServerDatabase('localhost', 'ryzom_api', 'sg4gfgh45bgguifsd', 'ring_live');

function save_xml($filename, $xml) {
	$filename = "/home/api/public_html/data/cache/guilds/$filename.xml";
	$txt_xml = $xml->asXML();
    // save in clear xml
	file_put_contents($filename, $txt_xml);
    // save in clear compressed xml
	$gz = gzopen($filename.'.gz','w9');
	gzwrite($gz, $txt_xml);
	gzclose($gz);
}

function create_guild($out, $filename, $shard) {
	global $db, $time, $guilds_cache_timeout;

	$src = simplexml_load_file($filename);

	echo "$filename\n";

	$guild = $out->addChild('guild');
	$guild->addChild('gid', $src->Id['value']);
	$guild->addChild('name', $src->_Name['value']);
	$guild->addChild('race', $src->Race['value']);
	$guild->addChild('icon', $src->Icon['value']);
	$guild->addChild('creation_date', $src->CreationDate['value']);

	$desc = (string)$src->_Description['value'];
	$desc = iconv("UTF-8", "ISO-8859-1", $desc);
	$replace = array('&', '<', '>');
	$into    = array('&amp;', '&lt;', '&gt;');
	$desc = str_replace($replace, $into, $desc);
	$guild->addChild('description', $desc);

	// create the full guild xml

	$xml = simplexml_load_string('<guild/>');

	$cache = $xml->addChild('cache');
	$cache->addAttribute('created', $time);
	$cache->addAttribute('expire', $time+$guilds_cache_timeout);

	$xml->addChild('name', $src->_Name['value']);
	$xml->addChild('gid', $src->Id['value']);
	$xml->addChild('race', $src->Race['value']);
	$xml->addChild('icon', $src->Icon['value']);
	$xml->addChild('creation_date', $src->CreationDate['value']);
	$xml->addChild('shard', $shard);
	$xml->addChild('description', $desc);
	$xml->addChild('money', $src->Money['value']);
	$xml->addChild('building', $src->Building['value']);

	$xml->addChild('cult', strtolower($src->DeclaredCult['value']));
	$xml->addChild('civ', strtolower($src->DeclaredCiv['value']));

	$motd = (string)$src->_MessageOfTheDay['value'];
	$motd = iconv("UTF-8", "ISO-8859-1", $motd);
	$motd = str_replace($replace, $into, $motd);
	$xml->addChild('motd', $motd);

	// guild members
	$members = $xml->addChild('members');
	$i = 0;
	do {
		$m = $src->Members->__Key__[$i]['value'];
		if(!isset($m)) break;
		$m2 = $src->Members->__Val__[$i];
		$mem = $members->addChild('member');
		$cid = hexdec(substr($m2->Members->Id['value'], 3, 10));
		$mem->addChild('cid', $cid);
		$sql = "SELECT char_name FROM characters WHERE char_id=$cid";
		$result = $db->query($sql);
		$name = $db->fetch_row($result);
		$db->free_result($result);
		$mem->addChild('name', $name[0]);
		$mem->addChild('grade', $m2->Members->Grade['value']);
		$mem->addChild('joined_date', $m2->Members->EnterTime['value']);
		$i++;
	} while(true);

	// guild fame
	$factions = array('fyros', 'matis', 'tryker', 'zorai', 'kami', 'karavan');
	$fames = $xml->addChild('fames');
	$i = 0;
	do {
		$f = $src->FameContainer->FameContainer->__Parent__->Entries->__Key__[$i]['value'];
		if(!isset($f)) break;
		$f2 = $src->FameContainer->FameContainer->__Parent__->Entries->__Val__[$i];
		$fac = basename($f2->Sheet['value'], '.faction');
		if(in_array($fac, $factions)) {
			$fames->addChild($fac, $f2->Fame['value']);
		}
		$i++;
	} while(true);

	// guild items
/*	$inventory = $xml->addChild('room');
	$result = $src->xpath('/xml/GuildInventory/_Items');
	while(list( , $node) = each($result)) {
		ryzom_item_insert($inventory, $node);
	}
*/
	save_xml("guild_".$src->Id['value'], $xml);
}

function create_guilds_xml($shard) {
	global $time, $guilds_cache_timeout;

	$xml = simplexml_load_string('<guilds/>');

	$cache = $xml->addChild('cache');
	$cache->addAttribute('created', $time);
	$cache->addAttribute('expire', $time+$guilds_cache_timeout);
	$xml->addChild('shard', $shard);

    foreach(glob("/home/api/public_html/data/cache/guilds/guild_?????.xml") as $fn) {
		create_guild($xml, $fn, $shard);
	}
//	$dirname = "tmp/$shard";
//	if ($handle = opendir($dirname)) {
//		while (false !== ($file = readdir($handle))) {
//			if (end(explode(".", $file)) == 'xml') {
//				create_guild($xml, "$dirname/$file");
//			}
//		}
//		closedir($handle);
//	}

	save_xml("guilds_$shard", $xml);
}

create_guilds_xml('atys');

?>