Fichier "ctrls.txt" :
---------------------
D�finir le type de chaque control.
C'est compos� de l'identifiant du control(unique) et du type du control.
ATTENTION : Ne pas utiliser le control

Les types �tant :
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
Rien oblige � ce que les num�ros se suivent.
Si le type est mal d�fini, le control va quand m�me �tre ins�r� dans la liste des controls,
mais comme �tant un control ind�fini.



Fichier "texts.txt" :
---------------------
R�pertorie les textes des interfaces en y associant un ID.
C'est ce fichier que l'on pourra localiser en changeant les textes mais pas les identifiants.
[xxx] ou xxx est 1 identifiant � donner au texte.
Suivi du texte.

Ex:
[1]CREATE
[2]Do you want to quit ?

le texte d'ID 1 sera remplac� par "CREATE".
le texte d'ID 2 sera remplac� par "Do you want to quit ?".
ATTENTION il est important que le texte soit directement apr�s le "]" et qu'ils ne comporte pas de "[" ou "]".
En effet les textes pouvant avoir des espaces cela permet d'en tenir compte facilement.


Fichier "textures.txt" :
------------------------
R�pertorie les fichiers de textures utilis�es dans les interfaces en y associant un ID.
[xxx] ou xxx est 1 identifiant � donner � la texture.
Suivi du nom de la texture.

Ex:
[1]Bois.tga
[2]Brique.tga

la texture 1 sera ce qu'il y a dans le fichier bois.tga et
la texture 2 sera ce qu'il y a dans le fichier brique.tga.
ATTENTION il est important que le nom de fichier soit directement apr�s le "]".
En effet les nom de fichier pouvant avoir des espaces cela permet d'en tenir compte facilement.



Fichier "pens.txt" :
--------------------
D�fini les stylos avec lesquels on �crit les diff�rents textes des interfaces.
Un "pen" est compos� d'une taille de texture, d'un RGBA et d'une ombre ou non.

[...] -> permet de d�finir l'aspect du stylo par d�faut.
Sinon pour pr�ciser l'aspect d'un styloen particulier, on met l'ID du stylo entre crochets.

Le premier param�tre est la taille de la Font.
Les 4 suivant sont pour le RGBA(couleur(rouge-vert-bleu) et l'alpha(transparence))
Le suivant d�fini s'il y a une ombre ou non (0 pour ne pas ombrer).



Fichier "buttons.txt" :
-----------------------
D�fini l'aspect des boutons.
A Savoir qu'un bouton est a 3 �tats : Enfonc�, Relach�, d�sactiv�.
Qu'il y a 1 texture pour les 3 �tats et d'un RGBA par �tat.

[...] -> permet de d�finir l'aspect des boutons par d�faut.
Sinon pour pr�ciser l'aspect d'un bouton en particulier on met l'ID du bouton entre crochet.

Les 3 premiers param�tres sont des entiers d�terminant les textures � utiliser.
Les suivant sont 3 s�ries de 4 valeur -> 3 RGBA
1 RGBA est compos� d'une valeur pour le rouge, une pour le vert, une pour le bleu et l'alpha.

Ex:
[...]	4 4 0   255 255 255 255   150 150 255 255   100 100 100 255
[2]	4 3 0    55 100 150 255    50 200 150 255   100 100 100 255

Les boutons seront tous avec la texture 4 en mode enfonc� et relach� et 0 en d�sactiv�,
SAUF le bouton d'ID 2 qui sera avec une texture 3 en mode relach�.
Le RGBA respect le m�me ordre que les texture (enfonc� - relach� - d�sactiv�).



Fichier "backgrounds.txt" :
---------------------------
D�fini la texture de background � utiliser dans chaque OSD (fen�tre d'interface).

[...] -> permet de d�finir le background par d�faut.
Sinon pour pr�ciser le background d'un OSD en particulier, on met l'ID de l'OSD entre crochets.

Ex:
[...]	4
[1]	2
[2]	3
Par defaut les OSD auront pour backgroud la texture 4, sauf
l'OSD 1 qui aura la texture 2, et
l'OSD 2 qui aura la texture 3.



Fichiers d�crivant les OSD :
----------------------------
Un OSD est compos� d'une premi�re parti d�crivant la fen�tre de fa�on g�n�ral(position-taille)
et d'une partie �tant la liste de tous les controls qu'il contient,
les control �tant lister par ordre d'affichage.

Ces controls pouvant �tre des :
1.Textes :
2.Bitmaps :
3.Boutons :
4.Listes :
5.Saisies :
6.Radio Boutons :

Une partie d�crivant chaque control est commune :

  "Parent:"		= Un control peut � avoir c coordonn�es relavies � un autre control (p�re).
Il suffit de mettre l'ID du control devant �tre le p�re. 0 ou si on ne pr�cise pas de parent,
�tant l'OSD comme parent.
ATTENTION le parent doit �tre d�clar� avant (A CHANGER PLUS TARD).

  "Origin:"		= Point du parent servant de r�f�rence par rapport au p�re.
9 choix, les m�me que pour le HotSpot,qui sont : BL BM BR ML MM MR TL TM TR.
Bottom(B) Middle(M) Top(T) Left(L) Right(R).
Si le p�re esst un bouton, une Orgin: en BR signifierait qu'on prend comme point de rep�re (0,0);
l'angle en bas � droite du bouton p�re pour afficher le control.

  "HotSpot:"	= D�fini comment afficher le control par rapport � l'origine.
9 choix, les m�me que pour l'"Origin:".
BR signifiera qu'on veut afficher le control en bas � droite de ce point.
BM en desous du point �galement, mais centr� en largeur.

  "X:"		= position X ente 0 et 1 du control.
(d�pendant de la taille de l'OSD) (X positif sur la Droite)

  "Y:"		= position Y ente 0 et 1 du control.
(d�pendant de la taille de l'OSD) (Y positif vers le Haut)

  "X_Pixel:"	= position X en Pixel du control.
(ind�pendant de la taille de l'OSD) (converti puis ajout� � X:)

  "Y_Pixel:"	= position Y en Pixel du control.
(ind�pendant de la taille de l'OSD) (converti puis ajout� � Y:)

  "W:"		= taille de la fen�tre en largeur entre 0 et 1.
(d�pendant de la taille de l'OSD)

  "H:"		= taille de la fen�tre en hauteur entre 0 et 1.
(d�pendant de la taille de l'OSD)

  "W_Pixel:"	= taille de la fen�tre en largeur en pixel.
(independant de la taille de l'OSD)

  "H_Pixel:"	= taille de la fen�tre en hauteur en pixel.
(independant de la taille de l'OSD)

REMARQUE : L'ordre des clefs n'importe pas !


Mais chaque type de control a �galement une partie de script qui lui est propre :
1.Textes :
  D�j� il faut savoir que la Largeur et Hauteur (W: H: W_Pixel: H_Pixel:) ne servent pas pour les textes.
Les textes ont 2 clefs en plus ; l'ID du texte et le stylo � appliquer.
  "Text:"		= ID du text � afficher (voir "texts.txt").
  "Pen:"		= Stylo � utiliser pour �crire le texte (voir "pens.txt").

2.Bitmaps :
  "Texture:"	= Num�ro de la texture � afficher (voir "texture.txt").
  "RGBA:"		= 4 champs rouge vert bleu alpha (entre 0 et 255).

3.Boutons :
  "Function:"	= Num�ro de la fonction � appeler lors de l'appui sur le bouton.
  "Text:"		= ID du text du bouton � afficher.
  "Pen:"		= Stylo � appliquer pour le texte du bouton.

4.Listes :
  "Pen:"		= Stylo pour les textes de la liste � utiliser.

5.Saisies :
  "Function:"	= Num�ro de la fonction � appeler lors de l'appuie de la touche ENTER.
  "Pen:"		= Stylo � utiliser pour �crire.

6.Radio Boutons :
Pour les radios boutons la partie commune de clef ne sert � rien !
  "Buttons"		= Indique le d�part de la liste des boutons qui vont �tre regroup�s.
Mettre des num�ros de control (Boutons seulement) apr�s et qui sont d�j� d�fini avant.
Les boutons vont devenir en quelque sorte des fils du radio bouton.
  "End"		= Indique la fin de la liste.
Mettre cette clef une fois que tous les boutons � regrouper ont �taient list�s.


REMARQUE : Pour chaque control (ligne), l'ordre des clefs n'a aucune importance !


Ex:
Type: 1 X: 0.1 Y: 0.1 Width: 500 Height: 500 MoveX: 0 MoveY: 400 MoveWidth: 400 MoveHeight: 50

[0]			Origin: TL	HotSpot: BR X: 0.1 Y: -0.1 X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Texture: 2 RGBA: 255 128 128  255

[1]	         	Origin: TL	HotSpot: BR X: 0   Y: -0.5 X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 1 Text: 1 Pen: 1
[2]	Parent: 1	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 2 Text: 2 Pen: 1
[3]	Parent: 2	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 3 Text: 3 Pen: 1
[4]	Parent: 3	Origin: BR	HotSpot: TR X: 0   Y:  0   X_Pixel:   0 Y_Pixel:    0	W: 0.25 H: 0 W_Pixel:   0 H_Pixel:  64	Function: 4 Text: 4 Pen: 1
[5]	Buttons: 1 2 3 4 End
