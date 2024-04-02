for src in "de" "en" "es" "fr" "ru"
do
	for dst in "de" "en" "es" "fr" "ru"
	do
		if [ $src != $dst ]
		then
			echo "$src $dst"
			wget -O deepl_terms/$src-$dst.json "http://tears.family/STABLE/deeplist/term-export.php?src=$src&dst=$dst&modules=deepl_alang;deepl_atys;deepl_gaming;deepl_homin;deepl_locations;deepl_mobs;deepl_op;deepl_organizations;deepl_others;deepl_places;deepl_r2;&format=JSON"
		fi
	done
done

