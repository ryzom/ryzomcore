Please don't update files in "translated" directly!

First, be sure you put "translation_tools" in "bin" directory.

How to update translations in UXT files :

1. Update original texts in "work/wk.uxt"
2. Launch A_make_string_diff script
3. Open files in "diff" directory
4. Replace original text with translation between [ and ]
5. The 2 last lines : // REMOVE THE FOLOWING LINE WHEN TRANSLATION IS DONE and // DIFF NOT TRANSLATED
6. Save files
7. Launch AA_clean_string_diff to delete translations help (old values)
8. Launch B_merge_string_diff to merge your translations in "translated"

How to update translations in words files :

1. Update original texts in "work" directory
2. Launch 5_make_words_diff script
3. Open files in "diff" directory
4. Replace original text with translation (separators are <tab>)
5. The 2 last lines : REMOVE THE FOLOWING TWO LINE WHEN TRANSLATION IS DONE and DIFF NOT TRANSLATED
6. Save files
7. Launch 6_merge_words_diff to merge your translations in "translated"
