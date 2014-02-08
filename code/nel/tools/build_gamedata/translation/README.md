Translation Procedure

Run the respective make diff tool, this will create a diff of work/wk file with translation/wk file.

The wk language is used while developing.

Open the diff file for this wk, and remove the NOT TRANSLATED tag at the bottom.

Run the respective merge diff tool.

After you are done developing, and things need to be translated, proceed as follows.

Now run the make diff tool again, this will create the diff between the translation/wk file and the translation languages.

Translate the diff files and remove the NOT TRANSLATED tag.

Run the merge diff tool to merge the translations in.