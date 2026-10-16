<?php
// ______                           _____ _                   _   _____           _
// | ___ \                         /  ___| |                 | | |_   _|         | |
// | |_/ /   _ _______  _ __ ___   \ `--.| |__   __ _ _ __ __| |   | | ___   ___ | |___
// |    / | | |_  / _ \| '_ ` _ \   `--. \ '_ \ / _` | '__/ _` |   | |/ _ \ / _ \| / __|
// | |\ \ |_| |/ / (_) | | | | | | /\__/ / | | | (_| | | | (_| |   | | (_) | (_) | \__ \
// \_| \_\__, /___\___/|_| |_| |_| \____/|_| |_|\__,_|_|  \__,_|   \_/\___/ \___/|_|___/
//        __/ |
//       |___/
//
// Ryzom - MMORPG Framework <https://ryzom.com/dev/>
// Copyright (C) 2019  Winch Gate Property Limited
// This program is free software: read https://ryzom.com/dev/copying.html for more details

function is_cli()
{
	if ( defined('STDIN') )
	{
		return true;
	}

	if ( php_sapi_name() === 'cli' )
	{
		return true;
	}

	if ( array_key_exists('SHELL', $_ENV) ) {
		return true;
	}

	if ( empty($_SERVER['REMOTE_ADDR']) and !isset($_SERVER['HTTP_USER_AGENT']) and count($_SERVER['argv']) > 0)
	{
		return true;
	}

	if ( !array_key_exists('REQUEST_METHOD', $_SERVER) )
	{
		return true;
	}

	return false;
}

if (!is_cli())
	die('no soup for you');

function are_you_sure() {
	$reply = null;
	while ("yes\n" != $reply) {
			echo "Are you sure to do that? Type 'yes' if you are or CTRL+C to quit: ";
			$reply = fgets(STDIN);
	}
}

if(count($argv) < 4) die("${argv[0]} <uid> <slot> basic|paying|silver|premium\n");

$templates = array(
	'basic' => array('account_11076_2_pdr.bin', false),
	'paying' => array('account_11076_2_pdr.bin', true),
	'silver' => array('account_11076_3_pdr.bin', true),
	'premium' => array('account_11076_4_pdr.bin', false),
);

$uid = $argv[1];
$slot = $argv[2];
$templ = $argv[3];

$www_path = '/home/nevrax/www';
$pdr2xml = '/home/nevrax/bin/pdr2xml';
$xml2pdr = '/home/nevrax/bin/xml2pdr';
$source_path = '/home/nevrax/backups/shard/premerge_characters/'.substr(strval($uid), -3);
$destination_path = '/home/nevrax/shard/save/characters/'.substr(strval($uid), -3);

echo "============================================== OLD CHARS ========\n";
exec_safe("ls -la ${source_path}/account_${uid}*", true);

echo "\n";
echo "============================================== NEW CHARS ========\n";
exec_safe("ls -la ${destination_path}/account_${uid}*", true);

echo "\n\n";
echo "Creating the new char with his old char with a **$templ** pack and **".($templates[$templ][1]?"keep levels":"no levels")."**\n";

#are_you_sure();

exec_safe("cp $www_path/restoration/".($templates[$templ][0])." template_pdr.bin");
exec_safe("$pdr2xml template_pdr.bin");

$itemid = 0;

$cid = $uid*16+$slot;

if($cid == 0) die("bad cid $cid\n");

$bin_fn = "account_${uid}_${slot}_pdr.bin";

if(!file_exists("$source_path/$bin_fn")) {
	die("source doesn't exists $cid: $uid $slot\n");
}

exec_safe("cp $source_path/$bin_fn account_pdr.bin");

exec_safe("$pdr2xml account_pdr.bin");

$src = simplexml_load_file("account_pdr.xml");
$dst = simplexml_load_file("template_pdr.xml");

xml_copy("/xml/_HairType/@value");
xml_copy("/xml/_HairColor/@value");
xml_copy("/xml/HairType/@value");
xml_copy("/xml/HairColor/@value");
xml_copy("/xml/GabaritHeight/@value");
xml_copy("/xml/GabaritTorsoWidth/@value");
xml_copy("/xml/GabaritArmsWidth/@value");
xml_copy("/xml/GabaritLegsWidth/@value");
xml_copy("/xml/GabaritBreastSize/@value");
xml_copy("/xml/MorphTarget1/@value");
xml_copy("/xml/MorphTarget2/@value");
xml_copy("/xml/MorphTarget3/@value");
xml_copy("/xml/MorphTarget4/@value");
xml_copy("/xml/MorphTarget5/@value");
xml_copy("/xml/MorphTarget6/@value");
xml_copy("/xml/MorphTarget7/@value");
xml_copy("/xml/MorphTarget8/@value");
xml_copy("/xml/EyesColor/@value");
xml_copy("/xml/Tattoo/@value");
xml_copy("/xml/EntityBase/_SheetId/@value");
xml_copy("/xml/EntityBase/_Race/@value");
xml_copy("/xml/EntityBase/_Gender/@value");
xml_copy("/xml/EntityBase/_Size/@value");

$v = $src->EntityBase->_Name['value'];
$v = strstr($v, '(', true);
$dst->EntityBase->_Name['value'] = $v.'(Atys)';

$dst->_Missions->CharId['value'] = eid();

// copy pet owner only for the 2 template who have a pet
if($templates[$templ][0] == 'account_11076_3_pdr.bin' || $templates[$templ][0] == 'account_11076_4_pdr.bin')
	$dst->_PlayerPets->__Val__->OwnerId['value'] = eid();

$result = $dst->xpath('//_ItemId/@value');

while(list( , $node) = each($result)) {
	$node[0] = itemid();
}

xml_copy("/xml/_FirstConnectedTime/@value");
xml_copy("/xml/_LastConnectedTime/@value");
xml_copy("/xml/_PlayedTime/@value");

// keep levels/bricks/phrase if needed
if($templates[$templ][1]) {
	xml_copy_full("//_BoughtPhrases");
	xml_copy_full("//_KnownBricks");
	xml_copy_full("/xml/_MemorizedPhrases");
	xml_copy_full("/xml/EntityBase/_PhysCharacs", $dst->EntityBase);
	xml_copy_full("/xml/EntityBase/_PhysScores", $dst->EntityBase);
	xml_copy_full("/xml/EntityBase/_Skills", $dst->EntityBase);
	xml_copy_full("//_KnownPhrases");
	xml_copy_full("/xml/SkillPoints");
	xml_copy_full("/xml/SpentSkillPoints");

// Remove marauder plan
	xml_remove("/xml/_KnownBricks[@value='bcbahb.sbrick']");
	xml_remove("/xml/_KnownBricks[@value='bcbahg.sbrick']");
	xml_remove("/xml/_KnownBricks[@value='bcbahh.sbrick']");
	xml_remove("/xml/_KnownBricks[@value='bcbahp.sbrick']");
	xml_remove("/xml/_KnownBricks[@value='bcbahs.sbrick']");
	xml_remove("/xml/_KnownBricks[@value='bcbahv.sbrick']");
}

$dom = new DOMDocument();
$dom->formatOutput = true;
$dom->preserveWhiteSpace = false;
$dom->loadXML($dst->asXML());
$txt_xml = $dom->saveXML();

// remove xml version="1.0"
$txt_xml = substr($txt_xml, 22);

$tmp_path = '/home/nevrax/tmp/restore_char/';
exec_safe("mkdir -p $tmp_path");

file_put_contents("{$tmp_path}account_conv_pdr.xml", $txt_xml);

exec_safe("$xml2pdr {$tmp_path}account_conv_pdr.xml");

exec_safe("mkdir -p $destination_path");

if(file_exists("$destination_path/$bin_fn")) {
	exec_safe("cp -a -f $destination_path/$bin_fn $destination_path/${bin_fn}_restore_char");
}

exec_safe("cp -a -f {$tmp_path}account_conv_pdr.bin $destination_path/$bin_fn");

exec_safe("rm -f {$tmp_path}account_conv_pdr.xml {$tmp_path}account_conv_pdr.bin");


echo "New char is in $destination_path/$bin_fn\n";

function itemid() {
	global $itemid;
	$id = "101";
	$id = bcmul($id, "16777216");
	$id = bcadd($id, $itemid);
	$itemid = $itemid + 1;
	$id = bcmul($id, "4294967296");
	$id = bcadd($id, time());
	return $id;
}

function eid() {
	global $cid;
	return sprintf("(0x%010x:00:00:86)", $cid);
}

function xml_remove($xml_path) {
global $dst;
	foreach($dst->xpath($xml_path) as $node) {
			unset($node[0]);
	}
}

function xml_copy($xml_path) {
	global $src, $dst;
	$s = $src->xpath($xml_path);
	$d = $dst->xpath($xml_path);
	if(!isset($s[0]) || !isset($s[0][0])) {
			echo "element not found: ";
			print_r($xml_path);
			echo "\n";
			return;
	}
	$d[0][0] = $s[0][0];
}

function xml_copy_full($xml_path, $dst_path='') {
global $src, $dst;

	if($dst_path=='') $d = $dst;
	else $d = $dst_path;
	xml_remove($xml_path);
	$result = $src->xpath($xml_path);
	while(list( , $node) = each($result)) {
			xml_merge($d, $node);
	}
}

function xml_merge(&$base, $add)
{
	if ( $add->count() != 0 )
		$new = $base->addChild($add->getName());
	else
		$new = $base->addChild($add->getName(), $add);

	foreach ($add->attributes() as $a => $b)
	{
		$new->addAttribute($a, $b);
	}

	if ( $add->count() != 0 )
	{
		foreach ($add->children() as $child)
		{
			xml_merge($new, $child);
		}
	}
}

function exec_safe($cmd, $disp = false) {
	$res = exec($cmd, $output, $return_var);
	if($disp) print_r($output);
	if($return_var == 1) {
		echo "Failed to execute: '$cmd'\n";
		print_r($res);
		print_r($output);
		print_r($return_var);
		//die;
	}
}
