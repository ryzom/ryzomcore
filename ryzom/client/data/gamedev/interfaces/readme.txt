Fichier "ctrls.txt" :
---------------------
Définir le type de chaque control.
C'est composé de l'identifiant du control(unique) et du type du control.
ATTENTION : Ne pas utiliser le control

Les types étant :
- TEXT
- CAPTURE
- BUTTON
- RADIO
- BITMAP
- LIST

Ex:
[3] TEXT
[1] BITMAP

Dans l'exemple le bouton 3 sera un texte et le 1 une bitmap.
Rien oblige à ce que les numéros se suivent.
Si le type est mal défini, le control va quand même être inséré dans la liste des controls,
mais comme étant un control indéfini.



Fichier "texts.txt" :
---------------------
Répertorie les textes des interfaces en y associant un ID.
C'est ce fichier que l'on pourra localiser en changeant les textes mais pas les identifiants.
[xxx] ou xxx est 1 identifiant à donner au texte.
Suivi du texte.

Ex:
[1]CREATE
[2]Do you want to quit ?

le texte d'ID 1 sera remplacé par "CREATE".
le texte d'ID 2 sera remplacé par "Do you want to quit ?".
ATTENTION il est important que le texte soit directement après le "]" et qu'ils ne comporte pas de "[" ou "]".
En effet les textes pouvant avoir des espaces cela permet d'en tenir compte facilement.


Fichier "textures.txt" :
------------------------
Répertorie les fichiers de textures utilisées dans les interfaces en y associant un ID.
[xxx] ou xxx est 1 identifiant à donner à la texture.
Suivi du nom de la texture.

Ex:
[1]Bois.tga
[2]Brique.tga

la texture 1 sera ce qu'il y a dans le fichier bois.tga et
la texture 2 sera ce qu'il y a dans le fichier brique.tga.
ATTENTION il est important que le nom de fichier soit directement après le "]".
En effet les nom de fichier pouvant avoir des espaces cela permet d'en tenir compte facilement.



Fichier "pens.txt" :
--------------------
Défini les stylos avec lesquels on écrit les différents textes des interfaces.
Un "pen" est composé d'une taille de texture, d'un RGBA et d'une ombre ou non.

[...] -> permet de définir l'aspect du stylo par défaut.
Sinon pour préciser l'aspect d'un styloen particulier, on met l'ID du stylo entre crochets.

Le premier paramètre est la taille de la Font.
Les 4 suivant sont pour le RGBA(couleur(rouge-vert-bleu) et l'alpha(transparence))
Le suivant défini s'il y a une ombre ou non (0 pour ne pas ombrer).



Fichier "buttons.txt" :
-----------------------
Défini l'aspect des boutons.
A Savoir qu'un bouton est a 3 états : Enfoncé, Relaché, désactivé.
Qu'il y a 1 texture pour les 3 états et d'un RGBA par état.

[...] -> permet de définir l'aspect des boutons par défaut.
Sinon pour préciser l'aspect d'un bouton en particulier on met l'ID du bouton entre crochet.

Les 3 premiers paramètres sont des entiers déterminant les textures à utiliser.
Les suivant sont 3 séries de 4 valeur -> 3 RGBA
1 RGBA est composé d'une valeur pour le rouge, une pour le vert, une pour le bleu et l'alpha.

Ex:
[...]	4 4 0   255 255 255 255   150 150 255 255   100 100 100 255
[2]	4 3 0    55 100 150 255    50 200 150 255   100 100 100 255

Les boutons seront tous avec la texture 4 en mode enfoncé et relaché et 0 en désactivé,
SAUF le bouton d'ID 2 qui sera avec une texture 3 en mode relaché.
Le RGBA respect le même ordre que les texture (enfoncé - relaché - désactivé).



Fichier "backgrounds.txt" :
---------------------------
Défini la texture de background à utiliser dans chaque OSD (fenêtre d'interface).

[...] -> permet de définir le background par défaut.
Sinon pour préciser le background d'un OSD en particulier, on met l'ID de l'OSD entre crochets.

Ex:
[...]	4
[1]	2
[2]	3
Par defaut les OSD auront pour backgroud la texture 4, sauf
l'OSD 1 qui aura la texture 2, et
l'OSD 2 qui aura la texture 3.



Fichiers décrivant les OSD :
----------------------------
Un OSD est composé d'une première parti décrivant la fenêtre de façon général(position-taille)
et d'une partie étant la liste de tous les controls qu'il contient,
les control étant lister par ordre d'affichage.

Ces controls pouvant être des :
1.Textes :
2.Bitmaps :
3.Boutons :
4.Listes :
5.Saisies :
6.Radio Boutons :

Une partie décrivant chaque control est commune :

  "Parent:"		= Un control peut à avoir c coordonnées relavies à un autre control (père).
Il suffit de mettre l'ID du control devant être le père. 0 ou si on ne précise pas de parent,
étant l'OSD comme parent.
ATTENTION le parent doit être déclaré avant (A CHANGER PLUS TARD).

  "Origin:"		= Point du parent servant de référence par rapport au père.
9 choix, les même que pour le HotSpot,qui sont : BL BM BR ML MM MR TL TM TR.
Bottom(B) Middle(M) Top(T) Left(L) Right(R).
Si le père esst un bouton, une Orgin: en BR signifierait qu'on prend comme point de repère (0,0);
l'angle en bas à droite du bouton père pour afficher le control.

  "HotSpot:"	= Défini comment afficher le control par rapport à l'origine.
9 choix, les même que pour l'"Origin:".
BR signifiera qu'on veut afficher le control en bas à droite de ce point.
BM en desous du point également, mais centré en largeur.

  "X:"		= position X ente 0 et 1 du control.
(dépendant de la taille de l'OSD) (X positif sur la Droite)

  "Y:"		= position Y ente 0 et 1 du control.
(dépendant de la taille de l'OSD) (Y positif vers le Haut)

  "X_Pixel:"	= position X en Pixel du control.
(indépendant de la taille de l'OSD) (converti puis ajouté à X:)

  "Y_Pixel:"	= position Y en Pixel du control.
(indépendant de la taille de l'OSD) (converti puis ajouté à Y:)

  "W:"		= taille de la fenêtre en largeur entre 0 et 1.
(dépendant de la taille de l'OSD)

  "H:"		= taille de la fenêtre en hauteur entre 0 et 1.
(dépendant de la taille de l'OSD)

  "W_Pixel:"	= taille de la fenêtre en largeur en pixel.
(independant de la taille de l'OSD)

  "H_Pixel:"	= taille de la fenêtre en hauteur en pixel.
(independant de la taille de l'OSD)

REMARQUE : L'ordre des clefs n'importe pas !


Mais chaque type de control a également une partie de script qui lui est propre :
1.Textes :
  Déjà il faut savoir que la Largeur et Hauteur (W: H: W_Pixel: H_Pixel:) ne servent pas pour les textes.
Les textes ont 2 clefs en plus ; l'ID du texte et le stylo à appliquer.
  "Text:"		= ID du text à afficher (voir "texts.txt").
  "Pen:"		= Stylo à utiliser pour écrire le texte (voir "pens.txt").

2.Bitmaps :
  "Texture:"	= Numéro de la texture à afficher (voir "texture.txt").
  "RGBA:"		= 4 champs rouge vert bleu alpha (entre 0 et 255).

3.Boutons :
  "Function:"	= Numéro de la fonction à appeler lors de l'appui sur le bouton.
  "Text:"		= ID du text du bouton à afficher.
  "Pen:"		= Stylo à appliquer pour le texte du bouton.

4.Listes :
  "Pen:"		= Stylo pour les textes de la liste à utiliser.

5.Saisies :
  "Function:"	= Numéro de la fonction à appeler lors de l'appuie de la touche ENTER.
  "Pen:"		= Stylo à utiliser pour écrire.

6.Radio Boutons :
Pour les radios boutons la partie commune de clef ne sert à rien !
  "Buttons"		= Indique le départ de la liste des boutons qui vont être regroupés.
Mettre des numéros de control (Boutons seulement) après et qui sont déjà défini avant.
Les boutons vont devenir en quelque sorte des fils du radio bouton.
  "End"		= Indique la fin de la liste.
Mettre cette clef une fois que tous les boutons à regrouper ont étaient listés.


REMARQUE : Pour chaque control (ligne), l'ordre des clefs n'a aucune importance !


Ex:
Type: 1 X: 0.1 Y: 0.1 Width: 500 Height: 500 MoveX: 0 MoveY: 400 MoveWidth: 400 MoveHeight: 50

[0]			Origin: TL	HotSpot: BR X: 0.1 Y: -0.1 X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Texture: 2 RGBA: 255 128 128  255

[1]	         	Origin: TL	HotSpot: BR X: 0   Y: -0.5 X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 1 Text: 1 Pen: 1
[2]	Parent: 1	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 2 Text: 2 Pen: 1
[3]	Parent: 2	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 3 Text: 3 Pen: 1
[4]	Parent: 3	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 4 Text: 4 Pen: 1
[5]	Buttons: 1 2 3 4 End
