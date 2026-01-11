#!/bin/bash

echo $1;

let "back=($1 & 15) - 1"
let "symbol=($1 >> 4 & 63) - 1"
let "invert=$1 >> 10 & 1"
let "col1R=$1 >> 11 & 255"
let "col1G=$1 >> 19 & 255"
let "col1B=$1 >> 27 & 255"
let "col2R=$1 >> 35 & 255"
let "col2G=$1 >> 43 & 255"
let "col2B=$1 >> 51 & 255"

size="$2"

png_path='/home/api/public_html/data/ryzom/guild_png/';
final_path='/home/api/public_html/data/cache/guild_icons/';
tmp_path="$final_path/tmp";
img_back=`printf $png_path/guild_back_%s_%02d_1.png $size $back`
img_back2=`printf $png_path/guild_back_%s_%02d_2.png $size $back`
img_symbol=`printf $png_path/guild_symbol_%s_%02d.png $size $symbol`
img_final=`printf $final_path/%d_%s.png $1 $size`

convert $img_back -fill "rgb($col1R,$col1G,$col1B)" -colorize 100,100,100 $tmp_path/$1_$2_part_1.png
convert $img_back2 -fill "rgb($col2R,$col2G,$col2B)" -colorize 100,100,100 $tmp_path/$1_$2_part_2.png
convert -composite $tmp_path/$1_$2_part_1.png $tmp_path/$1_$2_part_2.png $tmp_path/$1_$2_back.png

if [[ $invert == 1 ]]
then
	convert -negate $img_symbol $tmp_path/$1_$2_symb.png
	composite -compose plus $tmp_path/$1_$2_back.png  $tmp_path/$1_$2_symb.png $img_final
else
	composite -compose multiply $tmp_path/$1_$2_back.png $img_symbol $img_final
fi

composite -compose copy-opacity $tmp_path/$1_$2_back.png $img_final $img_final

rm $tmp_path/$1_$2_*
