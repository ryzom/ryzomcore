<?php

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
  'ach_c_neutral' =>
  array (
    'en' => 'neutral',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_allegiance_neutral' =>
  array (
    'en' => 'While being of %s allegiance',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_allegiance_start' =>
  array (
    'en' => 'While being aligned with the ',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_allegiance_and' =>
  array (
    'en' => ' and the ',
    'fr' => '',
    'de' => '',
    'ru' => '',
  ),
  'ach_allegiance_end' =>
  array (
    'en' => ', accomplish the following achievements:',
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