<?php

$guild_name_http = str_replace(' ', '_', $guild_name);
if ($tableau == '')
	$tableau = 'total';

$tabs = array(
	'chantier'	=> hof_tab('episode2_hof_chantier.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type=chantier&race='.$race.'&guild_name='.$guild_name_http.'&noActe='.$noActe, $txt_tab_chantier),
	'craft'		=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type=craft&race='.$race.'&guild_name='.$guild_name_http.'&noActe='.$noActe.'&tableau='.$tableau, $txt_tab_craft),
	'harvest'	=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type=harvest&race='.$race.'&guild_name='.$guild_name_http.'&noActe='.$noActe.'&tableau='.$tableau, $txt_tab_harvest),
	'fight'		=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type=fight&race='.$race.'&guild_name='.$guild_name_http.'&noActe='.$noActe.'&tableau='.$tableau, $txt_tab_fight)
)

?>
