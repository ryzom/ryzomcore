<html>
<?php
if (isset($_GET['lang']))
{
	$lang = &$_GET['lang'];
	if ($lang != 'fr' && $lang != 'en' && $lang != 'de')
		die('invalid lang');
}
include 'outpost_txt_'.$lang.'.php';
?>
<head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<title><?echo "$title" ?> -</title>
</head>

<body bgcolor="#667788" text="#000000">
<?php
	//$outpost_txt1 = nl2br($outpost_txt);
	$outpost_txt1 = $outpost_txt;
//	echo "$outpost_txt1";
	$outpost_txt1 = str_replace("\n\n","\n",$outpost_txt1);
	$outpost_txt1 = str_replace("\n\n","\n",$outpost_txt1);
	$outpost_txt1 = str_replace("\n","<br><br>",$outpost_txt1);
	$outpost_array = explode("<br><br>",$outpost_txt1);
?>
<table border="0" cellspacing="5">
<tr>
	<td><h1><?php echo $outpost_array[0]; ?></h1></td>
</tr>
<tr>
        <td><h4><?php echo $outpost_array[1]; ?></h4></td>
</tr>
<tr>
        <td><h4><?php echo $outpost_array[2]; ?></h4></td>
</tr>
<tr>
        <td><?php for ($i=3; $i < sizeof($outpost_array); ++$i) echo $outpost_array[$i]; ?></td>
</tr>

</table>
</body>
</html>
