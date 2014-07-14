<?php
class RespawnPoints extends Entity {
	public $spawns;
	private $region_map;

	function RespawnPoints() {
		$this->setName("respawn_points");

		$this->spawns = array();

		$this->region_map = array();
		$this->region_map['spawn_global_bagne_matis'] = "roots";
		$this->region_map['spawn_global_bagne_nexus'] = "roots";
		$this->region_map['spawn_global_route_gouffre_fyros'] = "roots";
		$this->region_map['spawn_global_route_gouffre_nexus'] = "roots";
		$this->region_map['spawn_global_route_gouffre_tryker'] = "roots";
		$this->region_map['spawn_global_route_gouffre_zorai'] = "roots";
		$this->region_map['spawn_global_sources_fyros'] = "roots";
		$this->region_map['spawn_global_sources_zorai'] = "roots";
		$this->region_map['spawn_global_terre_nexus'] = "roots";
		$this->region_map['spawn_global_terre_zorai'] = "roots";
		$this->region_map['spawn_global_nexus_bagne'] = "roots";
		$this->region_map['spawn_global_nexus_route_gouffre'] = "roots";
		$this->region_map['spawn_global_nexus_terre'] = "roots";
		// 13/13

		$this->region_map['spawn_global_fyros_matis'] = "desert";
		$this->region_map['spawn_global_fyros_route_gouffre'] = "desert";
		$this->region_map['spawn_global_fyros_sources'] = "desert";
		$this->region_map['spawn_global_fyros_to_zorai'] = "desert";
		$this->region_map['spawn_kami_place_pyr'] = "desert";
		$this->region_map['spawn_kami_place_thesos'] = "desert";
		$this->region_map['spawn_karavan_place_pyr'] = "desert";
		// 7/7

		$this->region_map['spawn_global_matis_bagne'] = "forest";
		$this->region_map['spawn_global_matis_fyros'] = "forest";
		$this->region_map['spawn_global_matis_tryker'] = "forest";
		$this->region_map['spawn_kami_place_dyron'] = "forest";
		$this->region_map['spawn_kami_place_yrkanis'] = "forest";
		$this->region_map['spawn_karavan_place_avalae'] = "forest";
		$this->region_map['spawn_karavan_place_davae'] = "forest";
		#$this->region_map['spawn_karavan_place_yrkanis'] = "forest";
		// 8/7

		$this->region_map['spawn_global_tryker_matis'] = "lakes";
		$this->region_map['spawn_global_tryker_route_gouffre'] = "lakes";
		#$this->region_map['spawn_kami_place_fairhaven'] = "lakes";
		$this->region_map['spawn_karavan_place_avendale'] = "lakes";
		$this->region_map['spawn_karavan_place_crystabell'] = "lakes";
		$this->region_map['spawn_karavan_place_fairhaven'] = "lakes";
		$this->region_map['spawn_karavan_place_windermeer'] = "lakes";
		// 7/6

		$this->region_map['spawn_kami_place_hoi_cho'] = "jungle";
		$this->region_map['spawn_kami_place_jen_lai'] = "jungle";
		$this->region_map['spawn_kami_place_min_cho'] = "jungle";
		$this->region_map['spawn_global_zorai_route_gouffre'] = "jungle";
		$this->region_map['spawn_global_zorai_sources'] = "jungle";
		$this->region_map['spawn_global_zorai_terre'] = "jungle";
		$this->region_map['spawn_global_zorai_to_fyros'] = "jungle";
		$this->region_map['spawn_kami_place_zora'] = "jungle";
		#$this->region_map['spawn_karavan_place_zora'] = "jungle";
		// 9/8

	}

	function countRegion($r) {
		$c = 0;

		foreach($this->spawns as $elem) {
			if($this->region_map[$elem] == $r) {
				$c++;
			}
		}

		return $c;
	}
}
?>