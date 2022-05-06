<?php
	function stats_render() {
		global $DBc;

		$html = "";

		$res = $DBc->sqlQuery("SELECT SUM(sp_money) as all_money, AVG(sp_money) as avg_money, COUNT(*) as playercount FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''");

		$html .= "<b>Total characters</b>: ".nf($res[0]['playercount'])."<p>";

		#$res = $DBc->sqlQuery("SELECT SUM(sp_money) as anz FROM stat_players");
		$html .= "<b>Total money</b>: ".nf($res[0]['all_money'])."<p>";

		#$res = $DBc->sqlQuery("SELECT AVG(sp_money) as anz FROM stat_players");
		$html .= "<b>Average money</b>: ".nf($res[0]['avg_money'])."<p>";

		$res = $DBc->sqlQuery("SELECT sp_money FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1");
		$html .= "<b>Mean money</b>: ".nf($res[0]['sp_money'])."<p>";



		return $html;
	}
?>