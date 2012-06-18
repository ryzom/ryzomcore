<?php
	class DataTable {
		private $table;

		function DataTable(&$res) {
			$this->table = $res;
		}

		function countRows(&$cond) {
			$rules = $this->parseCond($cond);

			$res = 0;
			
			foreach($this->table as $elem) {
				$m = true;

				foreach($rules as $r) {
					$tmp = '
						if($elem[$r[0]] '.$r[1].') { }
						else {
							$m = false;
						}
					';
					
					try {
						eval($tmp);
					}
					catch(Exception $e) {
						return $e->getMessage();
					}
				}

				if($m == true) {
					$res++;
				}
			}

			return $res;
		}

		function getRows(&$cond) {
			$rules = $this->parseCond($cond);

			$res = array();
			
			foreach($this->table as $elem) {
				$m = true;

				foreach($rules as $r) {
					$tmp = '
						if($elem[$r[0]] '.$r[1].') { }
						else {
							$m = false;
						}
					';
					
					try {
						eval($tmp);
					}
					catch(Exception $e) {
						return $e->getMessage();
					}
				}

				if($m == true) {
					$res[] = $elem;
				}
			}

			return $res;
		}

		private function parseCond(&$cond) {
			$c = array();

			$tmp = explode("and",strtolower($cond));

			foreach($tmp as $elem) {
				$matches = array();
				preg_match("#([a-zA-Z0-9_]+) ?([.]*)#", trim($elem), $matches);

				$c[] = array($matches[1],$matches[2]);
			}

			return $c;
		}
	}
?>