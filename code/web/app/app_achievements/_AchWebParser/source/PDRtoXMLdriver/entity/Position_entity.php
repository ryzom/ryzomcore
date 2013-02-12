<?php
	class Position extends Entity {
		public $x;
		public $y;
		public $z;
		public $heading;
		public $placeid;

		function Position() {
			$this->setName("position");
			$this->placeid = "place_unknown";
		}

		function loadPlace() {
			global $_DISPATCHER;

			@include_once("script/include_InPoly_class.php");
			$region = array();
			$subregion = false;

			include("script/places/global.php");

			$point = floor($this->x/1000)." ".floor($this->y/1000);

			$pointLocation = new pointLocation();

			$res = $pointLocation->pointInPolygon($point, $region['place_silan'], false);

			if($res != "outside") {
				include("script/places/silan.php");
			}
			else {
				include("script/places/continents.php");
				$region2 = $region;
				foreach($region2 as $key=>$r) {
					$res = $pointLocation->pointInPolygon($point, $r, false);
					if($res != "outside") {
						include("script/places/".$key.".php");
						if($subregion == true) {
							foreach($region as $key2=>$r2) {
								$res2 = $pointLocation->pointInPolygon($point, $r2, false);
								if($res2 != "outside") {
									include("script/places/".$key."/".$key2.".php");
									break;
								}
							}
						}
						break;
					}
				}
			}

			foreach($region as $key=>$r) {
				$res = $pointLocation->pointInPolygon($point, $r, false);
				if($res != "outside") {
					if($this->placeid == "place_unknown") {
						$this->placeid = $key;
					}
					else {
						$tmp = new Position();
						$tmp->x = $this->x;
						$tmp->y = $this->y;
						$tmp->z = $this->z;
						$tmp->heading = $this->heading;
						$tmp->placeid = $key;

						$_DISPATCHER->dispatchEntity($tmp->getName(),$tmp);
					}
					#break;
				}
			}
		}
	}
?>