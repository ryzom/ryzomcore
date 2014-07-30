<?php
	class XMLgenerator {
		private $def = array();
		private $files = array();
		private $wildcard = array();

		function XMLgenerator() {
			//load xml def & filegen

			#$this->def['xml/CLEAR'] = array("stats");
			require_once("xmldef/public.php");
			$this->files["public"] = new XMLfile("public");

			require_once("xmldef/logs.php");
			$this->files["logs"] = new XMLfile("logs");

			require_once("xmldef/stats.php");
			$this->files["stats"] = new XMLfile("stats");

			require_once("xmldef/faction.php");
			$this->files["faction"] = new XMLfile("faction");

			require_once("xmldef/inventory.php");
			$this->files["inventory"] = new XMLfile("inventory");

			require_once("xmldef/shop.php");
			$this->files["shop"] = new XMLfile("shop");

			require_once("xmldef/fame.php");
			$this->files["fame"] = new XMLfile("fame");

			require_once("xmldef/knowledge.php");
			$this->files["knowledge"] = new XMLfile("knowledge");

			require_once("xmldef/social.php");
			$this->files["social"] = new XMLfile("social");

			require_once("xmldef/skills.php");
			$this->files["skills"] = new XMLfile("skills");

			require_once("xmldef/missions.php");
			$this->files["missions"] = new XMLfile("missions");

			require_once("xmldef/debug.php");
			$this->files["debug"] = new XMLfile("debug");
		}

		function addWildcard($w,$ident) {
			$this->wildcard[] = array($ident,$w);
		}

		function xml_split($pathid,$name,$attrs,$open) {
			global $tmp_log_xmlgen_time;
			$microstart = explode(' ',microtime());
			$start_time = $microstart[0] + $microstart[1];

			#echo $pathid." => ".$name."<br>";
			if(is_array($this->def[$pathid])) {
				foreach($this->def[$pathid] as $elem) {
					#echo $elem."<br>";
					$this->files[$elem]->addXML($name,$attrs,$open);
				}
			}

			foreach($this->wildcard as $elem) {
				if($elem[1] == substr($pathid,0,strlen($elem[1]))) {
					$this->files[$elem[0]]->addXML($name,$attrs,$open);
				}
			}

			$microstop = explode(' ',microtime());
			$stop_time = $microstop[0] + $microstop[1];

			$tmp_log_xmlgen_time += ($stop_time - $start_time);
		}

		function generate() {
			global $cdata,$CONF;

			foreach($this->files as $elem) {
				$xml = '<?xml version="1.0" encoding="UTF-8" ?>'."\n";
				$xml .= "<xml>\n";
				$xml .= "	<cached>".time()."</cached>\n";
				$xml .= "	<uniqueid>".$cdata['cid']."</uniqueid>\n";
				#$xml .= "	<accountid>".$cdata['aid']."</accountid>\n";
				#$xml .= "	<charslotid>".$cdata['sid']."</charslotid>\n";

				$xml .= $elem->generate('	');

				$xml .= "</xml>";

				$cid = ($cdata['aid']*16+$cdata['sid']);


				//store
				$pth = $CONF['export_xml_path'].$elem->getIdent()."/".($cid%10);

				if(!is_dir($pth)) {
					mkdir($pth,0777,true);
				}

				$f = fopen($pth."/".$cid.".xml","w");
				fwrite($f,$xml);
				fclose($f);

				$old = umask();
				chmod($pth."/".$cid.".xml", 0777);
				umask($old);
			}
		}
	}
?>