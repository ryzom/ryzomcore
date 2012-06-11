<?php
	class XMLapi extends DataSource {
		private $xml_path;

		function XMLapi() {
			parent::__construct();

			$this->xml_path = $CONF['xml_path'];
		}

		function getData($ident,$field,$type) {
			switch($type) {
				case "c_stats":
					$path = $this->xml_path."full/".$ident.".xml";
					break;
				case "c_items":
					$path = $this->xml_path."item/".$ident.".xml";
					break;
				default:
					return false;
					break;
			}
			$xml = new SimpleXMLElement($string);
		}

		function writeData($ident,$field,$data,$type) {
			return false;
		}

	}
?>