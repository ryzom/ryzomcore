<?php
$outpost_nbr = &$_GET['outpost_nbr'];
$outpost_txt = &$_GET['outpost_txt'];
$title = "A brief history of the outpost";

switch ($outpost_nbr)
	{
	// Les avant-postes Matis
		case "outpost_matis_03":
			$outpost_txt="Fearing Fen Farm\n\n	Outpost founded in 2492, abandoned in 2493\n Challenge for expert-rank guilds\n Established near the Fearing Fen, the activity of this outpost is centred on harvesting resources and cleaning the marshes invaded by the Goo. Clashes occurred with the Ecowarriors tribe when they discovered that the contaminated materials weren’t destroyed but instead were sent to Yrkanis or to the south-east to undergo various experiments. This led them to accuse the Matis of spreading the Goo. The royal scientists categorically denied these allegations, assuring them that every precaution was taken to avoid contamination.";
		break;
		case "outpost_matis_07":
			$outpost_txt="Finder’s Farm	\n\nOutpost founded in 2491, abandoned in 2493\n Challenge for confirmed-rank guilds\n A Matis scholar, searching for divine inspiration, decided to settle at the outpost to profit from the big trepan and to have access to the rare materials sanctified by the Goddess. He carried out numerous experiments, and, notably, discovered a new drilling powder which made several founts rise to the surface at once. The outpost became a meeting place for prospectors wishing to try this miracle powder. ";
		break;
		case "outpost_matis_15":
			$outpost_txt="Psykopla Knoll Trading Post\n\n	Outpost founded in 2491, abandoned in 2493\n Challenge for apprentice-rank guilds\n A small merchant guild developed trade at this outpost. Initially, many prospectors bartered their material at the site of this big trepan installed by the Karavan. A Tryker peddler had the idea of setting up a structure to facilitate these exchanges, for a modest commission. He created a guild with the Matis merchants which prospered until the outpost was closed.";
		break;
		case "outpost_matis_17":
			$outpost_txt="Wooky Workshop \n\n	Outpost founded in 2490, abandoned in 2493\n Challenge for advanced-rank guilds\n A workshop built near to a vortex leading to the depths of the Prime Roots. It was the first to be established in the Verdant Heights. Its name refers to the beginning of a period of intense activity for the Matis people: setting up outposts, fighting the kitins, and establishing new trading routes.";
		break;
		case "outpost_matis_24":
			$outpost_txt="Westgrove Stronghold \n\n	Outpost founded in 2492, abandoned in 2493\n Challenge for master-rank guilds\n An outpost constructed in the wooded depths of the Grove of Confusion. It was fortified to fight the Dryads tribe, fanatical homins who see themselves as defenders of the plants. Many Matis botanists, accused of torturing the Atysian flora, have perished into their hands.";
		break;
		case "outpost_matis_27":
			$outpost_txt="Ginti Workshop \n\n	Outpost founded in 2491, abandoned in 2493\n Challenge for master-rank guilds\n Sevalda Ginti was one of the most renowned Matis botanists of the 25th century. She was the disciple of Bravichi Lenardi. When the Karavan placed the big trepans in the Grove of Confusion, she left Yrkanis’ greenhouse to go and work deep in the jungle. She carried out much research and perfected armour-crafting thanks to her discoveries on resins. She disappeared in mysterious circumstances.";
		break;
		case "outpost_matis_30":
			$outpost_txt="Berello Gorge Border Post \n\n	Outpost founded in 2491, abandoned in 2493\n Challenge for expert-rank guilds\n Fortified under the command of the Matis captain Arcando Berello, this border post was the site of many confrontations with the Woven Bridles tribe. The stakes were high, as whoever controlled the outpost controlled the passage between the oases in the north and the arid plateaus in the south of the region. Berello demonstrated tenacity and tactical skill, and even the Fyros barbarians came to respect their enemy.";
		break;
	// Les avant-postes ZORAI
		case "outpost_zorai_02":
			$outpost_txt="Gu-Qin Workshop \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for expert-rank guilds\n A workshop which developed under the influence of a Zoraï craftsman named Gu-Qin, who had decided to brave the threat of the Goo. Unfortunately he was contaminated by the purple plague. Despite his apprentices’ care he lapsed into madness. He disappeared one winter’s night; until the outpost was abandoned a few years later, the craftsmen of the workshop swore that they would sometimes hear his demented laughter brought to them through the jungle on the breeze.";
		break;
				case "outpost_zorai_08":
			$outpost_txt="Qai-Du Workshop \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for apprentice-rank guilds\n This workshop was set up by the master jeweller Qai-Du, celebrated member of the Icon Worshippers tribe. His knowledge and piety did much to gain recognition of Zoraï crafts amongst traditionalists, who had long considered the manufacturing of profane objects as a waste of time.";
		break;
			case "outpost_zorai_10":
			$outpost_txt="Sai-Shun Stronghold \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for confirmed-rank guilds\n Built in the middle of the Maiden Grove jungle, the Sai-Shun stronghold was in the front line of the campaign against the Kitins which began in 2490. General Sai-Shun established his quarters in the outpost and swore he would not leave the area before the threat of the Swarming was warded off. He kept his word and fought for two years alongside the Force of Fraternity.";
		break;
			case "outpost_zorai_15":
				$outpost_txt="Zo-Kian Ruins Workshop \n\n	Outpost founded in 2492, abandoned in 2494\n Challenge for master-rank guilds\n This outpost was established to the north-east of the ruins of Zo-Kian, originally a small village founded by Zoraï hunters. These hunters trailed gibbaïs, who at the time were numerous in that part of the jungle, laughing off their people’s superstition that to kill those animals which were so similar to homins brought bad luck. One night the village was attacked by hordes of unknown creatures and completely destroyed. A few years later explorers discovered the existence of superior gibbaïs in the region. Many think that they were responsible for the end of Zo-Kian and that they must have been reacting to the hunters’ attacks.";
			break;
			case "outpost_zorai_16":
				$outpost_txt="Lost Valley Stronghold \n\n	Outpost founded in 2491, abandoned in 2494\n Challenge for master-rank guilds\n This outpost was fortified following repeated attacks by animals contaminated by the Goo. Hotbeds of infection were discovered to the north-west, in the Lost Valley. Several expeditions were sent to try and destroy it, but with little success. When the tree-bore in the outpost began to wilt, contaminated by the purple plague, it sounded the death knell for the fortress which was quickly abandoned.";
			break;
			case "outpost_zorai_22":
				$outpost_txt="Demon’s Crossroads Diplomatic Outpost	\n\n Outpost founded in 2492, abandoned in 2494\n Challenge for advanced-rank guilds\n Soon after the installation of a tree-bore in this outpost, a jungle demon was attracted by the Kamic activity and hung around the surrounding area. He was seething with rage for he was wasted by the Goo. A Zoraï sage, member of the Company of the Eternal Tree, found him and managed to cure his illness. The demon transformed into a luminous spirit then blessed the Zoraï, who became a saint among saints. After this miracle the outpost served as a meeting place for Kamists from the whole region.";
			break;
			case "outpost_zorai_29":
				$outpost_txt="Great Outback Workshop \n\n	Outpost founded in 2491, abandoned in 2494\n Challenge for expert-rank guilds\n This outpost’s activity was developed in the desert region of the Knot of Dementia by Cardarus Vekian, a Fyros gunsmith. The absence of excessive vegetation enabled the artisan to use his people’s manufacturing techniques, which are based on the use of fire, without the risk of starting a blaze. Though wary at first, the traders of Zora ended up taking an interest in the Fyros’ work. Vekian became a renowned craftsman throughout the Witherings and many Zoraï warriors were proud to bear weapons from his forge.";
			break;
	// Les avant-postes FYROS
			case "outpost_fyros_04":
				$outpost_txt="Malmont Farm	\n\n Outpost founded in 2489, abandoned in 2494\n Challenge for expert-rank guilds\n An outpost constructed to the south of the Malmount and the Dragon’s Tail, its activity concentrated on mining resources in this hostile region. A portion of its resources were transported to the village of Dyron and the two workshops nearby. The remaining materials were brought along the road to Pyr under heavy escort.";
			break;
			case "outpost_fyros_09":
				$outpost_txt="Hightowers Farm \n\n	Outpost founded in 2491, abandoned in 2494\n Challenge for advanced-rank guilds\n An outpost whose activity developed under the influence of the Frahar Hunters, who wished to mine the resources of the region. Frequent warrior patrols from this tribe made great contributions to securing the deposits, allowing the harvesters to prospect without fear of being attacked.";
			break;
			case "outpost_fyros_13":
				$outpost_txt="West Blackburn Border Post \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for confirmed-rank guilds\n Constructed atop a nexus situated at the border of the Burnt Forest, this outpost was used to keep up a constant surveillance over the forest’s infernos to avoid them spreading to the oasis region. The soldiers of the empire considered it an honour to be posted to the Western Front.";
			break;
			case "outpost_fyros_14":
				$outpost_txt="Blackburn Trade Post \n\n	Outpost founded in 2489, abandoned in 2494\n Challenge for apprentice-rank guilds\n This outpost was one of the first meeting points established in the Burning Desert. Travellers coming from the south stopped there frequently to rest and trade before reaching the capital. It takes its name from the Burnt Forest which lies to the south-west.";
			break;
			case "outpost_fyros_25":
				$outpost_txt="Southend Dune Farm \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for expert-rank guilds\n This outpost, installed on a nexus discovered by the Kamis in the south of the region, was the cause of friction between the authorities of Pyr and the Fyros clans who refused to give their allegiance to the empire. After being paid a considerable tribute, the chiefs of these barbarians finally tolerated the farm’s existence.";
			break;
			case "outpost_fyros_27":
				$outpost_txt="Woodburn Magic Pole \n\n	Outpost founded in 2490, abandoned in 2494\n Challenge for master-rank guilds\n This outpost, established in the heart of the Burnt Forest, was controlled by imperial elementalists who wished to exploit the power of the fire. New spells were created thanks to the experiments carried out by the magicians, giving the art of mysteries the status as a proper art of war within Fyros society.";
			break;
			case "outpost_fyros_28":
				$outpost_txt="Woodburn Stronghold \n\n	Outpost founded in 2491, abandoned in 2494\n Challenge for master-rank guilds\n Outpost which was fortified on the orders of the Regent Leanon. It protected the Burnt Forest from looters and enemies of the empire. Throughout its duration, the garrison which defended it had to regularly face the Scorchers tribe, magicians who longed to get at the founts of enchanted fire.";
			break;
	// Les avant-postes TRYKER
			case "outpost_tryker_06":
				$outpost_txt="Greenvale Trade Post \n\n	Outpost founded in 2489, abandoned in 2493\n Challenge for apprentice-rank guilds\n Constructed to the west of Fairhaven in a small leafy valley surrounded by cliffs, this outpost very quickly developed many trading activities, notably with the village of Windermeer. Nevertheless, it endured continual attacks by bandits.";
			break;
			case "outpost_tryker_10":
				$outpost_txt="Fount Porch Trade Post \n\n	Outpost founded in 2486, abandoned in 2493\n Challenge for advanced-rank guilds\n The trading activity at this exchange post was developed by the Smugglers tribe. The merchant guilds of the Tryker federation tried several times to control the transactions taking place in the region, but with little success. A popular saying goes that you can find anything at the Fountporch, even the impossible.";
			break;
			case "outpost_tryker_16":
				$outpost_txt="Graveyard Gate Research Centre \n\n	Outpost founded in 2489, abandoned in 2493 \n Challenge for expert-rank guilds\n When a prospector discovered strange bones near this outpost, scholars from the Federation examined them but were unable to identify their origins. A research centre was set up despite protests from the Beachcombers, who weren’t too keen on seeing hordes of drills invade the area around Graveyard Walk. This area was sacred to them, being considered a ritual path towards the tribe’s burial ground.";
			break;
			case "outpost_tryker_22":
				$outpost_txt="Twintops Workshop \n\n	Outpost founded in 2487, abandoned in 2493\n Challenge for expert-rank guilds\n A workshop which expanded under the influence of a rich Tryker craftsman. Scandal broke out when it was discovered that he owed his fortune to a close collaboration with the Slavers tribe. The craftsman was arrested, and the outpost was then run by a federal guild until it was abandoned.";
			break;
			case "outpost_tryker_24":
				$outpost_txt="Windway Workshop \n\n	Outpost founded in 2484, abandoned in 2493\n Challenge for confirmed-rank guilds\n This outpost was built around the first big trepan installed by the Karavan in the New Lands. Thanks to this machine blessed by Jena, the craftsmen of the Silt Sculptors tribe were among the first ones to experiment with the new materials from the depths of Atys. They are the forerunners of the outposts’ craft industry.";
			break;
			case "outpost_tryker_29":
				$outpost_txt="Loria Stronghold \n\n	Outpost founded in 2490, abandoned in 2493\n Challenge for master-rank guilds\n After its big trepan was installed this outpost was attacked by the Lagoon Brothers, who took the precious resources extracted by the drill. A federal army regiment was sent to reconquer the outpost. The soldiers wiped out the pirates and then built a stronghold which they dedicated to Loria, the famous Tryker heroine.";
			break;
			case "outpost_tryker_31":
				$outpost_txt="Whirling Strongold \n\n	Outpost founded in 2489, abandoned in 2493\n Challenge for master-rank guilds\n The strategic positioning of this outpost, next to the vortex leading to Liberty Lake, for a long time made it a favoured target of the bandits and other pirates who stalked the Lagoons of Loria. A fort was constructed in order to repel these attacks. Numerous clashes occurred at its walls before the outpost was abandoned.";
			break;
			case "2":
				$outpost_txt="<b></b><br><br>";
			break;

}
?>