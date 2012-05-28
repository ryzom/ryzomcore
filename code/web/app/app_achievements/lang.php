<?php // %2011-09-28T08:12:16+02:00

$__texts = array (
  'ach_summary' =>
  array (
    'en' => 'Summary',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_summary_header' =>
  array (
    'en' => 'Recent Achievements',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_summary_stats' =>
  array (
    'en' => 'Statistics',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_summary_stats_total' =>
  array (
    'en' => 'Total',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
);


if(isset($ryzom_texts))
  $ryzom_texts = array_merge ($__texts, $ryzom_texts);
else
  $ryzom_texts = $__texts;
?>