<?php

$guild_name_http = str_replace(' ', '_', $guild_name);

$tabs_tableaux = array(
	'total'		=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type='.$type.'&race='.$race.'&guild_name='.$guild_name_http.'&tableau=total', $txt_tab_total),
	'faction'	=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type='.$type.'&race='.$race.'&guild_name='.$guild_name_http.'&tableau=faction', $txt_tab_faction),
	'chantier'	=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type='.$type.'&race='.$race.'&guild_name='.$guild_name_http.'&tableau=chantier', $txt_tab_chantier),
	'acte'		=> hof_tab('episode2_hof_classement.php?shard='.$shard.'&lang='.$lang.'&user_login='.$user_login.'&faction='.$faction.'&type='.$type.'&race='.$race.'&guild_name='.$guild_name_http.'&tableau=acte', $txt_tab_acte)
)
?>
