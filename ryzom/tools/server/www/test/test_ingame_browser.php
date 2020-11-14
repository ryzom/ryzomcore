<html>
   <head>
      <title>Ryzom Ring</title>
   </head>
   <body>
<p><H1>Welcome in Ryzom Ring</H1>
</p>

<?php
	// some colors....
	
	for ($i=0; $i<256; ++$i)
	{
//		printf('code = #%02x%02x%02x', $i*16, 255-$i*16, 128);
		printf('<font color="#%02x%02x%02x>-=X=-</font>', $i, 255-$i, 128);
	}
	echo "<br>";
?>

<form action="plan_edit_session.php" method="post">
	<input type="submit" name="button" value="Schedule editing session">
	
	<select name="toto">
	  <option value ="volvo">Volvo</option>
	  <option value ="saab">Saab</option>
	  <option value ="opel" selected="selected">Opel (preselected)</option>
	  <option value ="audi">Audi</option>
	</select>
	
</form>
<form action="plan_anim_session.php" method="post">
	<input type="submit" name="button" value="Schedule animation session">
</form>
<br>

<!--<form action="http://borisb2/ring/edit_session.php" method="post">-->
<form action="edit_session.php" method="post">
<table width="100%" cellpadding="5" border = "5" bgcolor="#003366">
	<tr>
		<td width="20%" align="center">
				<input name="GestSub" value="Gestion abonnement" type="submit" z_btn_tmpl="html_text_button_look2">
		</td>
		<td width="40%" align="right">
				<input name="EnterAtys" value="Rejoindre Atys" type="submit">
		</td>
		<td width="40%">
			<input name="ResumeSession" value="Resume" type="submit">
		</td>
	</tr>
	<tr>
		<td align="center">
			<input name="EnterOutland" value="Outlands" type="submit">
		</td>
		<td align="right">
			<input name="StartAnimSession" value="Start Anim session" type="submit">
		</td>
		<td>
			<input name="StartEditSession" value="Start Edit sessions" type="submit">
		</td>
	</tr>
	<tr>
		<td bgcolor="#662222" align="center">
			<input name="Quit" value="Quit" type="submit" z_btn_tmpl="html_text_button_look2">
		</td>
		<td align="right">
			<input name="Scheduling" value="Schedule edit or anim session" type="submit">
		</td>
		<td></td>
	</tr>
</table>

Calendrier [filter combo box]

<table width="100%" cellspacing="0" border = "2">
	<tr>
		<td width="20%">
		</td>
		<td width="15%">
			<h4>En cours</h4>
		</td>
		<td width="50%">
		</td>
		<td width="15%">
		</td>
	</tr>
	<tr>
		<td>
		</td>
		<td>
			Animateur
		</td>
		<td>
			Description<br>
			blablablable
		</td>
		<td>
			Gerer
		</td>
	</tr>
	<tr>
		<td>
		</td>
		<td>
			Joueur
		</td>
		<td>
			Description<br>
			blablablable
		</td>
		<td>
			Rejoindre
		</td>
	</tr>
</table>
</form>
      
      <hr/>
      <br/>
<p>      <a href="edit_session.php"><h2>Start an editing session</h2></a>       </p>
<p>      <a href="anim_session.php"><h2>Start an Animation session</h2></a>     </p>
<p>      <a href="play_ryzom.php"><h2>Enter Ryzom as usual</h2></a>       </p>
<p/>
<p>      pipo et molo<a href="web_start.php">Reload this page</a> </p>
   </body>
</html>
