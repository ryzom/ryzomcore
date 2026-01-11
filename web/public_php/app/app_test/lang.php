<?php // %2011-09-28T08:12:16+02:00
$__texts = array (
  'access' =>
  array (
    'en' => 'User access this page %s times',
    'fr' => 'L\'utilisateur a accede a cette page %s fois',
    'de' => '',
    'ru' => '',
  ),
);
if(isset($ryzom_texts))
  $ryzom_texts = array_merge ($__texts, $ryzom_texts);
else
  $ryzom_texts = $__texts;
?>