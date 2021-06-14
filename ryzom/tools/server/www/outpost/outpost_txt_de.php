<?php
$outpost_nbr = &$_GET['outpost_nbr'];
$outpost_txt = &$_GET['outpost_txt'];
$title = "Eine kurze Zusammenfassung der Geschichte der Aussenposten";

switch ($outpost_nbr)
	{
	// Les avant-postes Matis
		case "outpost_matis_03":
			$outpost_txt="Gefürchteter Sumpf - Hof \n\n	Außenposten 2492 gegründet, 2493 aufgegeben\n Challenge für Gilden mit Expertenrang\n Befindet sich in der Nähe des gefürchteten Sumpfes und konzentriert sich auf den Rohstoffabbau und das Säubern der Goo-infizierten Sümpfe. Es kam zu Auseinandersetzungen mit dem Stamm der Ökokrieger, als diese herausfanden, dass das angesteckte Material nicht zerstört wurde, sondern nach Yrkanis oder in den Südosten gesandt wurden, um verschiedene Experimente damit durchzuführen. Daraufhin beschuldigten sie die Matis, das Goo zu verbreiten. Die königlichen Forscher dementierten diese Beschuldigungen kategorisch und beteuerten, es werde alles daran gesetzt, um jegliche Ansteckung zu vermeiden.";
		break;
		case "outpost_matis_07":
			$outpost_txt="Hof des Entdeckers \n\n	Außenposten 2491 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem bestätigten Rang\n Ein Matis-Gelehrter auf der Suche nach einer göttlichen Eingebung beschloss, sich auf dem Außenposten nieder zu lassen, um vom großen Trepan zu profitieren und sich Zugang zu den seltenen Materialien zu verschaffen, die der Göttin heilig sind. Er führte zahlreiche Experimente durch und entdeckte dabei ein neues Drillpuder, mit dem mehrere Quellen gleichzeitig aus dem Boden schossen. Der Außenposten wurde zum Treffpunkt für Rohstoffabbauer, die dieses Wundermittel ausprobieren wollten. ";
		break;
		case "outpost_matis_15":
			$outpost_txt="Psykopla Hügel - Handelsposten \n\n	Außenposten 2491 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Lehrlingsrang\n Durch eine kleine Händlergilde entstand Handel auf diesem Außenposten. Anfangs betrieben viele Abbauer Tauschhandel mit ihrem Material an diesem Ort, wo die Karavan einen großen Trepan errichtete. Ein Tryker-Hausierer hatte die Idee, dort einen Platz einzurichten, um den Tauschhandel zu vereinfachen und das gegen eine geringe Kommission. Er gründete eine Gilde mit Matis-Händlern, deren Geschäft dort florierte bis der Außenposten geschlossen wurde. ";
		break;
		case "outpost_matis_17":
			$outpost_txt="Wooky Werkstatt \n\n	Außenposten 2490 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem fortgeschrittenen Rang\n Ein Workshop, der neben einem Wirbel zu den Tiefen der Urwurzeln errichtet wurde. Es war der erste Außenposten der grünen Anhöhen. Seine Name spielt auf eine Zeit an, in der das Matis-Volk zahlreichen Tätigkeiten nachging: Errichtung von Außenposten, Kampf gegen die Kitins und Anlegen von neuen Handelsstraßen. ";
		break;
		case "outpost_matis_24":
			$outpost_txt="Westwald - Festung \n\n	Außenposten 2492 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Ein Außenposten, der in den bewaldeten Tiefen des Kapwaldes errichtet wurde. Er wurde befestigt, um den Stamm der Dryaden zu bekämpfen: Fanatische Homins, die sich als Pflanzenschützer ansehen. Sie beschuldigten viele Matis-Botaniker, der Flora von Atys zu schaden und brachten sie um. ";
		break;
		case "outpost_matis_27":
			$outpost_txt="Ginti - Werkstatt \n\n	Außenposten 2491 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Sevalda Ginti war einer der renommiertesten Matis-Botaniker des 25. Jahrhunderts. Sie war Bravichi Lenardis Schülerin. Als die Karavan die großen Trepane im Kapwald errichteten, verließ sie das Treibhaus von Yrkanis, um im tiefen Dschungel zu arbeiten. Sie stellte viele Nachforschungen an und perfektionierte die Rüstungsherstellung dank ihres Wissens über Harz. Sie verschwand unter mysteriösen Umständen.";
		break;
		case "outpost_matis_30":
			$outpost_txt="Berello Schlucht - Grenzposten \n\n	Außenposten 2491 gegründet, 2493 aufgegeben\n Unter dem Kommando der Matiskapitäns Arcando Berello befestigt. Dieser Grenzposten war der Schauplatz zahlreicher Auseinandersetzungen mit dem Stamm der Wasserplünderer. Es stand viel auf dem Spiel, denn wer auch immer die Kontrolle über den Außenposten übernehmen sollte, hatte auch die Kontrolle über den Durchgang zwischen den Oasen im Norden und den dürren Plateaus im Süden der Region. Berello war hartnäckig und wandte eine gute Taktik an, sodass sogar die barbarischen Fyros ihren Feind respektierten.";
		break;
	// Les avant-postes ZORAI
		case "outpost_zorai_02":
			$outpost_txt="Gu-Qin - Werkstatt \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für eine Gilde mit Expertenrang\n Ein Workshop, der sich unter dem Einfluss eines Zorai-Handwerkers namens Gu-Qin entwickelt hat, der beschlossen hatte, dem Goo die Stirn zu bieten. Doch unglücklicherweise wurde er von der lilafarbenen Pest angestochen. Trotz seiner vorsichtigen Lehrlinge verfiel er dem Wahnsinn. Eines Nachts im Winter verschwand er. Bis zum Verlassen des Außenpostens ein paar Jahre später schworen die Handwerker des Außenpostens, der Dschungelwind würde ihnen sein Gelächter zu Ohren tragen.";
		break;
				case "outpost_zorai_08":
			$outpost_txt="Qai-Du - Werkstatt \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für Gilden mit Lehrlingsrang\n Dieser Workshop wurde vom Meisterjuwelier Qai-Du gegründet, ein gefeiertes Mitglied des Stammes der Ikonenanbeter. Sein Wissen und seine Frömmigkeit trugen viel zur Anerkennung der Zorai-Handwerkstätigkeiten bei Traditionalisten bei, die das Anfertigen von profanen Gegenständen lange als unnütz angesehen hatten.";
		break;
			case "outpost_zorai_10":
			$outpost_txt="Sai-Shun - Festung \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für Gilden mit bestätigtem Rang\n Inmitten des jungfräulichen Wäldchens errichtet, war die Festung Sai-Shun an vorderster Front in der Antikitin-Kampagne, die 2490 anfing. General Sai-Shun ließ sich auf dem Außenposten nieder und schwor, die Gegend nicht zu verlassen, ehe die Bedrohung des Großen Schwarms nachgelassen habe. Er hielt Wort und kämpfte während zwei Jahren an der Seite der Bruderschaft. ";
		break;
			case "outpost_zorai_15":
				$outpost_txt="Zo-Kian Ruinen - Werkstatt \n\n	Außenposten 2492 gegründet, 2494 aufgegeben\n Challenge für Gilden mit Meisterrang\n Dieser Außenposten wurde nordöstlich der Ruinen von Zo-Kian errichtet, einem kleinen Dorf, das von Zorai-Jägern errichtet worden war. Diese Jäger verfolgten Gibbais, die zu der Zeit zahlreich in dem Teil des Dschungels waren, und machten sich über ihr Volk lustig, das glaubte, es würde Unglück bringen, diese Tiere zu töten. Eines Nachts wurde das Dorf von Horden unbekannter Kreaturen angegriffen und völlig zerstört. Ein paar Jahre später entdeckten Forscher entwickeltere Gibbais in der Region. Viele glauben, sie seien für die Zerstörung von Zo-Kian verantwortlich und hätten nur auf die Angriffe der Jäger reagiert.";
			break;
			case "outpost_zorai_16":
				$outpost_txt="Vergessenes Tal - Festung \n\n	Außenposten 2491 gegründet, 2491 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Dieser Außenposten wurde befestigt, nachdem er mehrmals von Goo-infizierten Tieren angegriffen worden war. Im Nordwesten wurden Infektionsherde im Verlorenen Tal entdeckt. Es wurden mehrere Expeditionen entsandt, um diese zu zerstören, doch ohne Erfolg. Als der Baumbohrer des Außenpostens durch seine Infektion mit der lilafarbenen Plage zu verwelken begann, bedeutete das das Ende der Festung, die daraufhin schnell aufgegeben wurde.";
			break;
			case "outpost_zorai_22":
				$outpost_txt="Dämonenkreuzung - Diplomatischer Aussenposten \n\n	Außenposten 2492 gegründet, 2494 aufgegeben\n Challenge für Gilden mit fortgeschrittenem Rang\n Kurz nach der Installation eines Baumbohrers auf diesem Außenposten zogen die kamistischen Machenschaften einen Dschungeldämonen an, der sich in der Gegend aufhielt. Er kochte vor Wut, denn er war vom Goo angesteckt worden. Ein Zorai-Weiser, Mitglied der Gemeinschaft des Ewigen Baumes, fand ihn und heilte seine Krankheit. Aus dem Dämonen wurde ein heller Kopf, der den Zorai sehr wichtig war und zu einem Heiligen wurde. Nach diesem Wunder wurde der Außenposten zu einem Treffpunkt für Kamisten der ganzen Region.";
			break;
			case "outpost_zorai_29":
				$outpost_txt="Grosses Buschland - Werkstatt \n\n	Außenposten 2491 gegründet, 2494 aufgegeben\n Challenge für Gilden mit Expertenrang\n Die Tätigkeit dieses Außenpostens in der Wüstenregion des Knoten des Wahnsinns wurde von Cardarus Vekian gegründet, einem Fyros-Waffenschmied. Dank der mangelnden Vegetation konnte der Handwerker die auf Feuer beruhenden Techniken seines Volkes anwenden, ohne ein Feuer zu entfachen. Obwohl die Händler aus Zora anfangs besorgt waren, interessierten sie sich doch für die Arbeit der Fyros. Vekian wurde ein bekannter Handwerker in den verdörrenden Landen und viele Zorai-Krieger waren stolz, Waffen aus seiner Fabrik zu tragen.";
			break;
	// Les avant-postes FYROS
			case "outpost_fyros_04":
				$outpost_txt="Malmont - Hof \n\n	Außenposten 2489 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem Expertenrang\n Ein Außenposten, der südlich von Malmount und Dragon's Tail errichtet wurde. Seine Tätigkeiten konzentrieren sich auf die Minenressourcen dieser feindlich gesinnten Region. Ein Teil dieser Ressourcen wurde zu dem Dorf von Dyron und den beiden nahe gelegenen Workshops transportiert. Das restliche Material wurde mit einer Eskorte nach Pyr gebracht.";
			break;
			case "outpost_fyros_09":
				$outpost_txt="Hochtürme - Hof \n\n	Außenposten 2491 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem fortgeschrittenen Rang\n Ein Außenposten, dessen Tätigkeit sich unter dem Einfluss der Frahar-Jäger entwickelte, die die Ressourcen der Gegend verminen wollten. Durch häufige Kriegerpatrouillen dieses Stammes wurden die Quellen weitgehend gesichert und das ermöglichte den Rohstoffabbauern, ihrer Beschäftigung ohne Angst vor einem Angriff nachzugehen.";
			break;
			case "outpost_fyros_13":
				$outpost_txt="West Schwarzbrand - Grenzposten \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem bestätigten Rang\n Auf einem Nexus am Rande des Verbrannten Waldes errichtet. Dieser Außenposten wurde genutzt, um die Infernen des Waldes stets im Auge zu behalten und eine Verbreitung auf die Oasenregion zu vermeiden. Für die Soldaten des Imperiums war es eine Ehre, an der Westfront zu dienen.";
			break;
			case "outpost_fyros_14":
				$outpost_txt="Schwarzbrand - Handelsposten \n\n	Außenposten 2489 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem Lehrlingsrang\n Dieser Außenposten war einer der ersten Treffpunkte der Brennenden Wüste. Reisende aus dem Süden machten regelmäßig dort Halt, um sich auszuruhen und Geschäfte zu machen, ehe sie die Hauptstadt erreichten. Sein Name kommt vom verbrannten Wald, der südwestlich davon liegt.";
			break;
			case "outpost_fyros_25":
				$outpost_txt="Südenden Düne - Hof \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem Expertenrang\n Dieser auf einem Nexus errichtete Außenposten im Süden der Region wurde von den Kamis entdeckt und sorgte für einen Streit zwischen den Autoritäten von Pyr und den Fyros-Clans, die sich weigerten, dem Imperium Treue zu schwören. Sie mussten einen hohen Preis zahlen, bis die Chefs dieser Barbaren die Farm endlich tolerierten.";
			break;
			case "outpost_fyros_27":
				$outpost_txt="Holzbrand - Magischer Ort \n\n	Außenposten 2490 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Dieser Außenposten, im Herzen des verbrannten Waldes, wurde einst von imperialen Elementalisten kontrolliert, die sich die Macht des Feuers zu Nutzen machen wollten. Dank der Experimente, die die Magier durchführten, entstanden neue Sprüche und verliehen der Kunst der Mysterien ein Kriegsstatut in der Fyros-Gesellschaft.";
			break;
			case "outpost_fyros_28":
				$outpost_txt="Holzbrand - Festung \n\n	Außenposten 2491 gegründet, 2494 aufgegeben\n Challenge für Gilden mit einem Meister-Rang\n Außenposten wurde auf Orden des Regenten Leanon hin befestigt. Er schützte den verbrannten Wald vor Plünderern und Feinden des Imperiums. Während der Außenposten aktiv war, musste die Garnison, die für seine Verteidigung zuständig war, regelmäßig gegen den Stamm der Scorcher kämpfen- Magier die an den Herd des verzauberten Feuers wollten.";
			break;
	// Les avant-postes TRYKER
			case "outpost_tryker_06":
				$outpost_txt="Grünes Tal - Handelsposten \n\n	Außenposten 2489 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Lehrlingsrang\n Der Außenposten liegt westlich von Fairhaven, in einem kleinen Tal mit Bäumen und Felsen. Auf diesem Außenposten entwickelte sich schnell Handel, hauptsächlich mit dem Dorf Windermeer. Trotzdem wurde er andauernd von Banditen angegriffen.";
			break;
			case "outpost_tryker_10":
				$outpost_txt="Quellhalle - Handelsposten \n\n	Außenposten 2486 gegründet, 2493 aufgegben\n Challenge für Gilden mit einem fortgeschrittenen Rang\n Der Stamm der Schmuggler betrieb hier Handel. Die Handelsgilden der Tryker-Föderation versuchten mehrmals, die Transaktionen zu kontrollieren, die in der Region über die Bühne gingen, doch das blieb erfolglos. Man sagt, in Fountporch könne man alles finden, auch das Unmögliche.";
			break;
			case "outpost_tryker_16":
				$outpost_txt="Friedhofspforte - Forschungszentrum \n\n	Außenposten 2489 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Expertenrang\n Als ein Rohstoffabbauer eigenartige Knochen in der Nähe dieses Außenpostens fand, wurden sie von Wissenschaftlern der Föderation untersucht. Sie konnten den Ursprung dieser Knochen nicht bestimmen. Trotz des Protests der Strandbrecher wurde ein Forschungszentrum eingerichtet. Diese hatten Angst, Horden von Drillen würden in der Gegend um Graveyard Walk auftauchen, die sie als heilig ansahen, da es sich um einen rituellen Pfad zum Friedhof des Stammes handelte.";
			break;
			case "outpost_tryker_22":
				$outpost_txt="Zwillingsgipfel - Werkstatt \n\n	Außenposten 2487 gegründet, 2493 aufgegeben\n Challenge für eine Gilde mit einem Expertenrang\n Ein Workshop, der unter dem Einfluss eines reichen Tryker-Handwerkers wuchs. Ein Skandal brach aus, als bekannt wurde, dass er seinen Reichtum einer engen Zusammenarbeit mit dem Stamm der Sklaventreiber zu verdanken hatte. Der Handwerker wurde festgenommen und der Außenposten wurde ab dem Zeitpunkt von einer föderalen Gilde geführt, bis er aufgegeben wurde.";
			break;
			case "outpost_tryker_24":
				$outpost_txt="Windiger Weg - Werkstatt \n\n	Außenposten 2484 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem bestätigten Rang\n Dieser Außenposten wurde um den ersten großen Trepan errichtet, den die Karavan in den alten Landen installiert hatte. Dank dieser von Jena gesegneten Maschine waren die Handwerker der Schlammbildhauer mit die ersten, die mit den neuen Materialien aus den Tiefen von Atys experimentierten. Sie waren Vorläufer für den Handel auf dem Außenposten.";
			break;
			case "outpost_tryker_29":
				$outpost_txt="Loria's - Festung \n\n	Außenposten 2490 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Nachdem sein großer Trepan installiert worden war, wurde dieser Außenposten von den Lagunenbrüdern angegriffen, die die wertvollen Ressourcen an sich nahmen, die der Bohrer abgebaut hatte. Ein Regiment der föderalen Armee wurde entsandt, um den Außenposten zurück zu erobern. Die Soldaten besiegten die Piraten und errichteten dann eine Festung, die sie Loria, der berühmten Tryker-Heldin, widmeten.";
			break;
			case "outpost_tryker_31":
				$outpost_txt="Wirbeln - Festung \n\n	Außenposten 2489 gegründet, 2493 aufgegeben\n Challenge für Gilden mit einem Meisterrang\n Durch seine strategische Lage in der Nähe des Wirbels zum Freiheitssee war dieser Außenposten lange ein beliebtes Angriffsziel von Banditen und Piraten, die in den Lagunen von Loria lungerten. Es kam oft zu Auseinandersetzungen vor seinen Türen, ehe er aufgegeben wurde.";
			break;
			case "2":
				$outpost_txt="<b></b><br><br>";
			break;

}
?>