<?php
$outpost_nbr = &$_GET['outpost_nbr'];
$outpost_txt = &$_GET['outpost_txt'];
$title = "Brève histoire de l'outpost";
switch ($outpost_nbr)
	{
	// Les avant-postes Matis
		case "outpost_matis_03":
			$outpost_txt="<b>Ferme du Marécage de l'Angoisse</b><br><br>Avant-poste fondé en 2492, abandonné en 2493<br><br>Défi pour guildes de rang expert<br><br>Établi à proximité des Marécages de l'Angoisse, l'activité de cet avant-poste s'est centrée sur la récolte de ressources et le nettoyage des marais envahis par la Goo. Des heurts ont eu lieu avec la tribu des Embourbés quand ces derniers ont découvert que les matières contaminées n'étaient pas détruites, mais envoyées à Yrkanis ou dans le sud-est pour différentes expérimentations. Ils ont alors accusé les Matis de propager la Goo. Les scientifiques royauont nié catégoriquement ces allégations, assurant que toutes les précautions étaient prises pour empêcher toute contamination.";
		break;
		case "outpost_matis_07":
			$outpost_txt="<b>Ferme de l'Inventeur</b><br><br>Avant-poste fondé en 2491, abandonné en 2493<br><br>Défi pour guildes de rang confirmé<br><br>Un savant matis, cherchant l'inspiration divine, décida de s'installer sur l'avant-poste pour profiter du grand trépan et avoir accès à des matériaux rares sanctifiés par la Déesse. Il fit de nombreuses expérimentations, et découvrit notamment une nouvelle poudre de forage permettant de faire surgir de l'Écorce plusieurs sources à la fois. L'avant-poste devint alors le rendez-vous des prospecteurs désireux d'essayer cette poudre miracle.";
		break;
		case "outpost_matis_15":
			$outpost_txt="<b>Poste d'Échange du Monticule des Psykoplas</b><br><br>Avant-poste fondé en 2491, abandonné en 2493<br><br>Défi pour guildes de rang apprenti<br><br>C'est une petite guilde marchande qui développa le commerce sur cet avant-poste. Initialement, de nombreux prospecteurs se retrouvaient sur le site du grand trépan installé par la Karavan pour troquer leurs matériaux. Un colporteur tryker eut l'idée d'installer sur place une structure pour faciliter ces échanges, moyennant une modeste commission. Il créa une guilde avec des négociants matis, qui prospéra jusqu'à la fermeture de l'avant-poste.";
		break;
		case "outpost_matis_17":
			$outpost_txt="<b>Atelier du Réveil</b><br><br>Avant-poste fondé en 2490, abandonné en 2493<br><br>Défi pour guildes de rang initié<br><br>Un atelier construit non loin d'un vortex menant aux profondeurs des Primes Racines. Il a été le premier mis en place sur le Sommet Verdoyant. Son nom fait référence au début d'une période d'intense activité pour le peuple matis : la mise en place des avant-postes, la lutte contre les kitins, et le tracé de nouvelles routes commerciales.";
		break;
		case "outpost_matis_24":
			$outpost_txt="<b>Forteresse du Bosquet Ouest</b><br><br>Avant-poste fondé en 2492, abandonné en 2493<br><br>Défi pour guildes de rang maître<br><br>Un avant-poste construit dans les profondeurs sylvestres du Bosquet de la Confusion. Il a été fortifié pour combattre la tribu des Dryades, des homins fanatiques qui se considèrent comme les défenseurs des plantes. De nombreux botanistes matis ont péri entre leurs mains, accusés de torture envers la flore atysienne.";
		break;
		case "outpost_matis_27":
			$outpost_txt="<b>Atelier de Ginti</b><br><br>Avant-poste fondé en 2491, abandonné en 2493<br><br>Défi pour guildes de rang maître<br><br>Sevalda Ginti fut l'une des plus illustres botanistes matis de la fin du XXVe siècle. Elle fut la disciple de Bravichi Lenardi. Lorsque la Karavan posa des grands trépans dans le Bosquet de la Confusion, elle quitta la serre d'Yrkanis pour aller travailler au cœur de la jungle. Elle fit de nombreuses recherches et perfectionna l'artisanat des armures grâce à ses découvertes sur les résines. Elle disparut dans des circonstances mystérieuses.";
		break;
		case "outpost_matis_30":
			$outpost_txt="<b>Poste Frontière de la Gorge de Berello</b><br><br>Avant-poste fondé en 2491, abandonné en 2493 <br><br>Défi pour guildes de rang expert<br><br>Fortifié sous le commandement du capitaine matis Arcando Berello, ce poste frontière fut le théâtre de nombreux affrontements avec la tribu des Pillards du désert. L'enjeu était de taille, car celui qui contrôlait l'avant-poste contrôlait le passage entre les oasis du nord et les plateaux arides au sud de la région. Berello s'illustra par sa ténacité et son sens tactique, et même les barbares fyros en vinrent à respecter leur ennemi.";
		break;
	// Les avant-postes ZORAI
		case "outpost_zorai_02":
			$outpost_txt="<b>Atelier de Gu-Qin</b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang expert<br><br>Un atelier qui se développa sous l’influence d’un compagnon artisan zoraï nommé Gu-Qin, qui avait décidé de braver la menace de la Goo. Malheureusement, il fut contaminé par le fléau pourpre. Malgré les soins de ses apprentis, Gu-Qin sombra dans la folie. Il disparut une nuit d’hiver ; jusqu’à l’abandon de l’avant-poste quelques années plus tard, les artisans de l’atelier juraient qu’ils entendaient parfois son rire dément porté par les vents de la jungle.";
		break;
				case "outpost_zorai_08":
			$outpost_txt="<b><Atelier de Qai-Du/b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang apprenti<br><br>Cet atelier fut mis en place par le maître joaillier Qai-Du, illustre membre de la tribu des Iconodoules. Son savoir-faire et sa piété firent beaucoup dans la reconnaissance de l’artisanat zoraï auprès des traditionalistes, qui ont longtemps considéré la fabrication d’objets profanes comme une perte de temps.";
		break;
			case "outpost_zorai_10":
			$outpost_txt="<b>Forteresse de Sai-Shun</b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang confirmé<br><br>Construite au milieu de la jungle du Bosquet Vierge, la forteresse de Sai-Shun fut en première ligne dans la campagne contre les kitins qui démarra en 2490. Le général Sai-Shun établit ses quartiers dans l’avant-poste, et jura de ne quitter l’endroit que lorsque la menace de l’Essaim serait écartée. Il tint parole et combattit durant deux ans aux côtés de la Force de la Fraternité.";
		break;
			case "outpost_zorai_15":
				$outpost_txt="<b>Atelier des Ruines de Zo-Kian</b><br><br>Avant-poste fondé en 2492, abandonné en 2494<br><br>Défi pour guildes de rang maître<br><br>Cet avant-poste fut établi au nord-est des ruines de Zo-Kian, à l’origine un petit village fondé par des chasseurs zoraïs. Ces chasseurs pistaient les gibbaïs qui étaient alors nombreux dans cette partie de la jungle, faisant fi des superstitions de leur peuple prétendant que tuer ces animaux proches des homins portait malheur. Une nuit, le village fut attaqué par des hordes de créatures inconnues et complètement détruit. Quelques années plus tard, des explorateurs découvrirent l’existence de gibbaïs supérieurs dans la région. Beaucoup pensent qu’ils seraient responsables de la fin de Zo-Kian : ils auraient réagi aux attaques des chasseurs.";
			break;
			case "outpost_zorai_16":
				$outpost_txt="<b>Forteresse de la Vallée Perdue</b><br><br>Avant-poste fondé en 2491, abandonné en 2494<br><br>Défi pour guildes de rang maître<br><br>Cet avant-poste a été fortifié suite à des attaques répétées d’animaux contaminés par la Goo. Des foyers d’infection furent découverts au nord-ouest, dans la Vallée Perdue. Plusieurs expéditions furent envoyés pour tenter de les détruire, mais sans grand succès. Lorsque l’arbre-vrille présent sur l’avant-poste commença à se faner, contaminé par le fléau pourpre, cela sonne le glas de la forteresse qui fut rapidement abandonnée.";
			break;
			case "outpost_zorai_22":
				$outpost_txt="<b>Avant-Poste Diplomatique du Croisement du Démon</b><br><br>Avant-poste fondé en 2492, abandonné en 2494<br><br>Défi pour guildes de rang initié<br><br>Peu après l’installation d’un arbre-vrille sur cet avant-poste, un démon de la jungle fut attiré par l’activité kamique et rôda dans les environs. Il était plein de rage, la Goo rongeait son essence. Un sage zoraï, membre de la Compagnie de l’Arbre Éternel, le trouva et parvint à le guérir de son mal. Le démon se transforma en esprit lumineux puis bénit le Zoraï, qui devint un saint parmi les saints. Après ce miracle, l’avant-poste servit de lieu de rencontre entre tous les kamistes de toute la région.";
			break;
			case "outpost_zorai_29":
				$outpost_txt="<b>Atelier de l'Arrière-pays</b><br><br>Avant-poste fondé en 2491, abandonné en 2494<br><br>Défi pour guildes de rang expert<br><br>L’activité de cet avant-poste fut développée dans la région désertique du Nœud de la Démence par Cardarus Vekian, un armurier fyros. L’absence de végétation exubérante permit à l’artisan d’utiliser les techniques de fabrication de son peuple, basées sur l’emploi du feu, sans risquer de déclencher un incendie. Méfiants de prime abord, des commerçants de Zora finirent par s’intéresser au travail de ce Fyros. Vekian devint un artisan renommé dans tout le Pays Malade, et de nombreux guerriers zoraïs étaient fiers de porter des armes sorties de sa forge.";
			break;
	// Les avant-postes FYROS
			case "outpost_fyros_04":
				$outpost_txt="<b>Ferme de Malmontagne</b><br><br>Avant-poste fondé en 2489, abandonné en 2494<br><br>Défi pour guildes de rang expert<br><br>Un avant-poste construit au sud de Malmontagne et de la Queue du Dragon, son activité se concentrait sur l'exploitation des ressources dans cette région hostile. Une partie de ces ressources était acheminée vers le village de Dyron et les deux ateliers des environs. Le reste des matériaux prenait la route de Pyr sous bonne escorte.";
			break;
			case "outpost_fyros_09":
				$outpost_txt="<b>Ferme des Hautes Tours</b><br><br>Avant-poste fondé en 2491, abandonné en 2494<br><br>Défi pour guildes de rang initié<br><br>Un avant-poste dont l'activité s'est développé sous l'influence des Chasseurs de Frahars, qui désiraient exploiter les ressources de la région. Les patrouilles fréquentes des guerriers de cette tribu contribuèrent grandement à sécuriser les gisements, permettant aux récolteurs de prospecter sans crainte d'être attaqués.";
			break;
			case "outpost_fyros_13":
				$outpost_txt="<b>Poste Frontière Ouest de la Combustion</b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang confirmé<br><br>Construit sur un nexus localisé aux frontières de la Forêt Enflammée, cet avant-poste a été utilisé pour maintenir une surveillance constante des brasiers de la forêt, afin d'éviter qu'ils ne s'étendent à la région des oasis. Les soldats de l'empire considéraient comme un honneur l'affectation au Poste Frontière Ouest.";
			break;
			case "outpost_fyros_14":
				$outpost_txt="<b>Poste d'Échange de la Combustion</b><br><br>Avant-poste fondé en 2489, abandonné en 2494<br><br>Défi pour guildes de rang apprenti<br><br>Cet avant-poste fut l'un des premiers point de rencontre établi dans le Désert Brûlant. Les voyageurs venant du sud s'y arrêtaient fréquemment pour se reposer et faire du commerce avant de rejoindre la capitale. Il tire son nom de la Forêt Enflammée qui se trouve au sud-ouest.";
			break;
			case "outpost_fyros_25":
				$outpost_txt="<b>Ferme des Dunes du Bas</b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang expert<br><br>Cet avant-poste, installé sur un nexus détecté par les Kamis au sud de la région, a été la cause de frictions entre les autorités de Pyr et des clans de Fyros refusant toute allégeance à l'Empire. Les chefs de ces barbares ont fini par tolérer l'existence de la ferme, après le versement d'un tribut conséquent.";
			break;
			case "outpost_fyros_27":
				$outpost_txt="<b>Pôle Magique des Bois Calcifiés</b><br><br>Avant-poste fondé en 2490, abandonné en 2494<br><br>Défi pour guildes de rang maître<br><br>Cet avant-poste, établi au cœur de la Forêt Enflammée, fut contrôlé par des élémentalistes impériaux qui désiraient exploiter la puissance du feu. De nouveaux sortilèges furent créés grâce aux expérimentations de ces magiciens, imposant l'art des arcanes comme un véritable art de la guerre au sein de la société fyros.";
			break;
			case "outpost_fyros_28":
				$outpost_txt="<b>Forteresse des Bois Calcifiés</b><br><br>Avant-poste fondé en 2491, abandonné en 2494<br><br>Défi pour guildes de rang maître<br><br>Avant-poste qui devint une place fortifiée sur ordre de la Régente Leanon. Il protégeait la Forêt Enflammée des pillards et des ennemis de l'empire. Pendant toute la durée de son activité, la garnison qui le défendait dut affronter régulièrement la tribu des Écorcheurs, des magiciens qui convoitaient les sources de feu enchanté.";
			break;
	// Les avant-postes TRYKER
			case "outpost_tryker_06":
				$outpost_txt="<b>Poste d'Échange de Vertval</b><br><br>Avant-poste fondé en 2489, abandonné en 2493<br><br>Défi pour guildes de rang apprenti<br><br>Construit à l'ouest de Fairhaven dans une petite vallée verdoyante entourée de falaises, cet avant-poste a développé très rapidement de nombreuses activités commerciales, notamment avec le village de Windermeer. Il a néanmoins dû faire face à d'incessantes attaques de bandits.";
			break;
			case "outpost_tryker_10":
				$outpost_txt="<b>Poste d'Échange du Porche des Sources</b><br><br>Avant-poste fondé en 2486, abandonné en 2493<br><br>Défi pour guildes de rang initié<br><br>L'activité commerciale de ce poste d'échange fut développée par la tribu des Contrebandiers. Les guildes marchandes de la fédération tryker tentèrent à plusieurs reprises de contrôler les transactions qui se déroulaient dans la région, mais sans grand succès. Un dicton populaire prétendait que l'on pouvait tout trouver au Porche des Sources, même l'introuvable.";
			break;
			case "outpost_tryker_16":
				$outpost_txt="<b>Centre de Recherche de la Promenade Caverneuse</b><br><br>Avant-poste fondé en 2489, abandonné en 2493<br><br>Défi pour guildes de rang expert<br><br>Lorsque un prospecteur découvrit d'étranges ossements au voisinage de l'avant-poste, des savants de la Fédération les examinèrent, mais ne parvinrent pas à identifier leur origine. Un centre de recherche fut installé malgré les protestations des Déferlants, qui n'appréciaient guère de voir des hordes de foreurs envahir les abords de la Promenade Caverneuse. Cet endroit était en effet sacré pour eux, étant considéré comme un chemin rituel vers le cimetière de la tribu.";
			break;
			case "outpost_tryker_22":
				$outpost_txt="<b>Atelier des Cimes Jumelles</b><br><br>Avant-poste fondé en 2487, abandonné en 2493<br><br>Défi pour guildes de rang expert<br><br>Un atelier qui prit de l'expansion sous l'influence d'un riche artisan tryker. Un scandale éclata quand on découvrit qu'il devait sa fortune à une étroite collaboration avec la tribu des Esclavagistes. L'artisan fut arrêté, et l'avant-poste fut alors géré par une guilde fédérale jusqu'à son abandon.";
			break;
			case "outpost_tryker_24":
				$outpost_txt="<b>Atelier de la Route des Vents</b><br><br>Avant-poste fondé en 2484, abandonné en 2493<br><br>Défi pour guildes de rang confirmé<br><br>Cet avant-poste fut construit autour du premier grand trépan installé par la Karavan sur les nouvelles terres. Grâce à cette machine bénie par Jena, les artisans de la tribu des Sculpteurs de Vase ont très tôt pu expérimenter de nouveaux matériaux issus des profondeurs d'Atys. Ils sont les pères de l'artisanat des avant-postes.";
			break;
			case "outpost_tryker_29":
				$outpost_txt="<b>Forteresse de Loria</b><br><br>Avant-poste fondé en 2490, abandonné en 2493<br><br>Défi pour guildes de rang maître<br><br>Après l'installation de son grand trépan, cet avant-poste fut attaqué par les Frères de la Lagune, qui s'approprièrent les précieuses ressources extraites par la foreuse. Un régiment de l'armée fédérale fut envoyé sur place pour reconquérir l'avant-poste. Les soldats écrasèrent les pirates puis édifièrent une forteresse qu'ils dédièrent à Loria, la célèbre héroïne tryker.";
			break;
			case "outpost_tryker_31":
				$outpost_txt="<b>Forteresse du Tourbillon</b><br><br>Avant-poste fondé en 2489, abandonné en 2493<br><br>Défi pour guildes de rang maître<br><br>L'emplacement stratégique de cet avant-poste, proche du vortex conduisant au Lac de la Liberté, en a fait longtemps une cible privilégiée pour les bandits et autres pirates qui hantent le Lagon de Loria. Un fort a donc été construit afin de repousser les attaques. De nombreux affrontements ont eu lieu devant ses murs, avant que l'avant-poste ne soit abandonné.";
			break;
			case "2":
				$outpost_txt="<b></b><br><br>";
			break;

}
?>