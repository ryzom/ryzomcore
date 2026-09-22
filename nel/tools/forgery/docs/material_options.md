# Comprendre les matériaux d'un objet Ryzom

Ce document explique, sans jargon technique, à quoi sert chaque réglage
d'un **matériau** dans Ryzom (l'apparence d'un objet 3D : sa couleur, sa
brillance, ses textures, sa transparence...). Il sert de base à une future
fonctionnalité de Ryzom Forgery (`object_editor.py`) qui affichera ces
explications directement en info-bulle (tooltip) au survol de chaque
réglage.

**Format de ce fichier (pour les devs) :** chaque option est une section
`## Titre {#cle-stable}` avec une ligne `**Résumé :**` (le texte court pour
la tooltip) suivie d'une explication complète. La `cle-stable` entre
accolades ne bouge jamais, même si le titre est reformulé — c'est elle que
le code utilisera pour retrouver la bonne section. Un futur script pourra
découper ce fichier sur les en-têtes `## ... {#...}` pour extraire
`(clé, résumé, texte complet)` par option.

---

## Qu'est-ce qu'un matériau ?

Un matériau, c'est la "peau" d'un objet 3D : il dit au jeu comment le
peindre à l'écran. Un objet 3D (un `.shape`) est fait de morceaux de
surface (des triangles), et chaque morceau utilise un matériau qui définit
sa couleur de base, si elle brille, si elle est transparente, et surtout
quelle(s) image(s) (textures) y sont collées.

Un même objet peut avoir plusieurs matériaux différents (par exemple : un
personnage a un matériau pour la peau, un pour les vêtements, un pour les
cheveux).

---

## Les couleurs de base {#basic-colors}

**Résumé :** Les couleurs de fond du matériau, avant même d'appliquer une
texture — un peu comme la couleur d'apprêt sous une peinture.

Un matériau a plusieurs couleurs qui se combinent :

- **Couleur diffuse** : la couleur "normale" de l'objet sous une lumière
  neutre — c'est la couleur principale que vous voyez.
- **Couleur ambiante** : la couleur de l'objet dans l'ombre, là où la
  lumière directe n'arrive pas. Évite que les zones à l'ombre soient
  totalement noires.
- **Couleur spéculaire** : la couleur du reflet brillant qu'on voit quand
  la lumière tape directement dessus (voir *Brillance* ci-dessous).
- **Couleur d'auto-illumination** : une couleur que l'objet émet de
  lui-même, comme s'il était légèrement lumineux, indépendamment de tout
  éclairage de la scène (utile pour des yeux qui brillent, un écran
  allumé, de la lave...).

Si une texture est posée par-dessus (voir plus bas), elle vient
généralement remplacer ou teinter la couleur diffuse plutôt que de rester
invisible.

---

## Opacité (transparence simple) {#opacity}

**Résumé :** À quel point on voit à travers l'objet. 100% = totalement
opaque (comme du bois), 0% = totalement invisible (comme de l'air).

C'est le réglage le plus simple de transparence : un pourcentage unique
appliqué à tout le matériau. Pour une transparence plus fine qui varie
selon l'endroit de la texture (par exemple les bords dentelés d'une
feuille), voir *Mélange (transparence avancée)* plus bas.

---

## Brillance et reflets (spéculaire) {#specular-glossiness}

**Résumé :** Ce qui donne un aspect "poli/métallique" à un objet, avec un
reflet brillant qui bouge selon d'où on regarde — comme la lumière qui
glisse sur une pomme ou une armure.

Deux réglages travaillent ensemble :

- **Niveau spéculaire** : l'intensité du reflet brillant. À 0%, l'objet a
  un rendu mat (comme du tissu ou du bois brut). Plus c'est haut, plus le
  reflet est intense (comme du métal poli ou de l'eau).
- **Brillance (glossiness)** : la taille/netteté de ce reflet. Un reflet
  "serré" et net donne un effet très poli (métal, verre) ; un reflet
  "étalé" et doux donne un effet plus mat même avec du spéculaire (plastique
  légèrement satiné).

---

## Auto-illumination {#self-illumination}

**Résumé :** Rend une partie de l'objet lumineuse par elle-même, sans
dépendre de l'éclairage de la scène — comme une fenêtre allumée la nuit ou
des runes magiques qui brillent.

Un pourcentage d'intensité, appliqué soit à une couleur choisie
spécifiquement pour l'auto-illumination, soit directement à la couleur
diffuse de l'objet (au choix).

---

## Visible des deux côtés (two-sided) {#two-sided}

**Résumé :** Par défaut, une face d'un objet 3D n'est visible que d'un
seul côté (comme une feuille de papier) — cette option la rend visible des
deux côtés.

Normalement, le moteur ne dessine pas l'"arrière" d'une face 3D pour
économiser des ressources (on ne la voit jamais de toute façon, sauf cas
particulier). Mais pour des objets fins comme une feuille, un pan de tissu
ou une vitre, on peut vouloir voir les deux côtés à la fois. C'est à ça que
sert ce réglage.

---

## Éclairage {#lighting}

**Résumé :** Comment le matériau réagit (ou non) aux lumières de la scène.

- **Non éclairé (unlit)** : l'objet garde toujours sa couleur/texture
  d'origine, sans être assombri dans l'ombre ni éclairci par une lumière
  proche. Utile pour une interface, un effet magique ou du texte flottant
  qui doit rester lisible partout.
- **Couleur par sommet (vertex color)** : au lieu (ou en plus) d'une
  texture, la couleur vient directement des points (sommets) qui composent
  le maillage 3D — une technique légère pour ajouter des variations de
  couleur sans texture supplémentaire (ex. feuillage qui varie du vert au
  jaune).
- **Vitrail (stained glass)** : mode d'éclairage spécial pensé pour les
  surfaces en verre coloré, où la lumière traverse et teinte différemment
  selon l'angle.

---

## Type de matériau (shader) {#shader-type}

**Résumé :** Le "mode de rendu" général du matériau — il détermine
comment les textures et les couleurs vont se combiner à l'écran. Chaque
type est adapté à un usage différent.

Un objet peut utiliser un des modes suivants :

- **Normal** : le mode standard, le plus courant. Combine une texture avec
  l'éclairage de façon classique. C'est celui qu'on utilise pour la
  plupart des objets (rochers, meubles, plantes...).
- **Couleur utilisateur (UserColor)** : permet de teinter une partie de
  l'objet avec une couleur au choix, définie par une "carte de masque"
  (une texture qui dit quelles zones peuvent être teintées). C'est ce
  mécanisme qui permet, par exemple, de personnaliser la couleur d'un
  vêtement ou d'une teinture sans avoir à créer une texture différente par
  couleur.
- **Carte de lumière (LightMap)** : utilise une texture qui contient de
  l'éclairage précalculé (des ombres et lumières "cuites" à l'avance dans
  l'image). Économique en performance, mais l'éclairage ne bouge pas avec
  les lumières dynamiques — typique des bâtiments et décors fixes.
- **Spéculaire (Specular)** : une variante qui utilise une texture dédiée
  pour contrôler où et comment l'objet brille (au lieu d'un seul réglage
  uniforme) — utile pour un objet où seules certaines zones sont
  polies/métalliques (ex. les boucles métalliques d'un ceinturon en cuir).
- **Eau (Water)** : mode spécial réservé aux surfaces d'eau, avec des
  réglages dédiés (vagues, reflets du ciel, transparence selon la
  profondeur...), voir *Eau* plus bas.
- **Éclairage par pixel (PerPixelLighting / PerPixelLightingNoSpec)** :
  une méthode d'éclairage plus précise, calculée pour chaque point de
  l'écran plutôt que juste aux coins du maillage — donne un rendu plus
  fin sur les grandes surfaces courbes ou les reliefs, au prix d'un peu
  plus de calcul. La variante "NoSpec" fait la même chose mais sans le
  reflet brillant, pour les objets qui n'ont pas besoin de spéculaire.

*Note technique : un mode "Bump" (relief simulé par texture) existe dans
la liste de l'éditeur 3dsMax mais n'a jamais été implémenté dans le moteur
— il ne fait rien de particulier s'il est sélectionné.*

---

## Mélange (transparence avancée / blend) {#blend}

**Résumé :** Contrôle comment un objet transparent se mélange avec ce qui
est affiché derrière lui — c'est ce qui permet le verre, la fumée, le feu,
les hologrammes...

"Blend" veut dire "mélange". Quand un objet est transparent, le jeu doit
décider comment combiner sa couleur avec celle du fond, pixel par pixel.
Il y a deux grandes façons de faire, activables via ce réglage :

- **Mélange classique (Alpha Blend)** : le résultat est une moyenne entre
  la couleur de l'objet et celle du fond, pondérée par la transparence de
  chaque pixel. C'est le comportement "normal" de la transparence — comme
  regarder à travers une vitre teintée ou du tissu fin. Plus un pixel est
  transparent, plus le fond "passe au travers".
- **Additif (Additive)** : au lieu de mélanger, le jeu *additionne* la
  lumière de l'objet à celle du fond, ce qui l'éclaircit au lieu de le
  cacher. C'est l'effet typique du feu, des étincelles, de la magie ou
  d'un halo lumineux — le résultat est toujours plus lumineux que le fond
  seul, jamais plus sombre.

*Note technique : le réglage expose en réalité deux listes déroulantes
("fonction source" et "fonction destination") qui définissent
précisément comment la couleur de l'objet et celle du fond sont pondérées
avant d'être combinées ; "Alpha Blend" et "Additif" sont des raccourcis
qui préremplissent ces deux listes avec les valeurs les plus courantes.*

---

## Facteurs de mélange (Src/Dst Blend) {#blend-factors}

**Résumé :** Les deux listes déroulantes détaillées du réglage Mélange —
chacune dit "par quoi multiplier cette couleur avant de l'additionner à
l'autre".

La formule utilisée pour combiner un pixel transparent avec ce qu'il y a
déjà à l'écran est toujours la même :

```
résultat = (couleur de l'objet × Src Blend) + (couleur déjà affichée × Dst Blend)
```

"Src" (source) désigne l'objet qu'on est en train de dessiner ; "Dst"
(destination) désigne ce qui est déjà à l'écran, derrière lui. Chaque
liste choisit un "facteur" — un nombre (ou une couleur) par lequel
multiplier avant d'additionner :

- **one** : multiplie par 1 — la couleur est prise à 100%, sans changement.
- **zero** : multiplie par 0 — annule complètement cette couleur.
- **srcalpha** : multiplie par la transparence (alpha) de l'objet dessiné.
  Plus l'objet est transparent, moins ce terme compte.
- **invsrcalpha** : l'inverse — multiplie par *(1 − alpha de l'objet)*.
  Plus l'objet est transparent, plus ce terme compte.
- **srccolor** : multiplie par la couleur (RGB) de l'objet dessiné.
- **invsrccolor** : multiplie par *(1 − couleur de l'objet)*, soit sa
  couleur "négative".
- **blendConstantColor** / **blendConstantInvColor** : multiplie par une
  couleur fixe à part (ou son inverse) plutôt que par une couleur venant du
  pixel — rarement utilisé, réservé à des effets spéciaux.
- **blendConstantAlpha** / **blendConstantInvAlpha** : pareil, mais avec
  juste la transparence de cette couleur fixe.

Les deux raccourcis du réglage Mélange combinent ces facteurs ainsi :

- **Alpha Blend** = Src `srcalpha`, Dst `invsrcalpha` → `résultat = objet ×
  alpha + fond × (1 − alpha)` — le mélange classique décrit plus haut.
- **Additif** = Src `one`, Dst `one` → `résultat = objet + fond` —
  l'addition décrite plus haut.

---

## Test alpha (Alpha Test) {#alpha-test}

**Résumé :** Une transparence "tout ou rien" : chaque pixel est soit
totalement visible, soit totalement invisible — pas de dégradé entre les
deux. Pratique pour le feuillage ou le grillage.

Contrairement au *Mélange* (qui donne des dégradés de transparence doux),
le test alpha regarde juste si un pixel de la texture est "assez opaque"
ou pas : s'il l'est, on le dessine normalement ; sinon, on ne le dessine
pas du tout, comme s'il n'existait pas. C'est souvent utilisé pour des
textures avec des trous nets (une feuille découpée, une grille, une
clôture), car c'est moins coûteux en performance que le mélange classique
et évite certains problèmes d'affichage (objets transparents qui
s'affichent dans le mauvais ordre).

---

## Décalage de profondeur (Z-Bias) {#z-bias}

**Résumé :** Un petit "coup de pouce" qui évite qu'une décalcomanie
(comme une tache de sang ou une pancarte plaquée sur un mur) scintille en
se confondant avec la surface juste en dessous.

Le jeu détermine ce qui est devant ou derrière quoi en comparant la
distance (la "profondeur") de chaque surface par rapport à la caméra.
Quand deux surfaces sont exactement à la même distance (par exemple, un
autocollant collé pile sur un mur), le jeu peut "hésiter" entre les deux
d'une image à l'autre, ce qui crée un scintillement visuel désagréable
(souvent appelé "z-fighting"). Le décalage de profondeur triche légèrement
sur la distance calculée pour trancher cette hésitation et garder
l'élément du dessus visible en permanence.

---

## Écriture dans le tampon de profondeur (Z-Write) {#z-write}

**Résumé :** Réglage technique qui décide si un objet transparent "bloque"
ou non les objets qui sont derrière lui, pour savoir qui doit être dessiné
en premier.

Normalement, chaque objet opaque enregistre sa distance à la caméra pour
que les objets qui apparaissent plus tard sachent s'ils doivent être
cachés derrière ou non. Pour les objets transparents (vitres, fumée...),
on force souvent la désactivation de cet enregistrement, sinon un
objet transparent pourrait à tort cacher complètement ce qu'il y a
derrière lui alors qu'on doit voir au travers. Ce réglage permet de forcer
ce comportement dans un sens ou dans l'autre selon le besoin.

---

## Les textures (jusqu'à 4 images par matériau) {#texture-slots}

**Résumé :** Les images appliquées sur la surface de l'objet. Un matériau
peut en combiner jusqu'à 4 en même temps, chacune avec un rôle différent
selon le type de matériau choisi.

Une texture, c'est simplement une image 2D "collée" sur la surface 3D. Un
matériau peut utiliser jusqu'à **4 textures en même temps**, combinées
entre elles (voir *Combinaison des textures* plus bas). Leur rôle change
selon le *Type de matériau* choisi :

- En mode **Normal**, les textures servent en général juste de motifs
  génériques qu'on combine (par exemple : une texture de base + une
  texture de détail zoomée pour ajouter du grain de près).
- En mode **Carte de lumière**, la première texture est l'image
  principale (le "dessin" de l'objet), le reste n'est pas utilisé.
- En mode **Couleur utilisateur**, la première texture est l'image
  principale et la seconde est le masque qui dit où appliquer la teinte
  personnalisée.
- En mode **Spéculaire**, la première texture est l'image principale et
  la seconde définit où/comment l'objet brille.
- En mode **Eau**, les textures servent à des rôles bien spécifiques
  (reflet du ciel de jour/nuit, relief des vagues, déplacement de l'eau...
  voir *Eau* plus bas) — c'est le seul mode qui peut utiliser plus de 4
  textures dans l'éditeur 3dsMax, mais au-delà de 4, elles ne sont **pas**
  conservées dans le fichier final du jeu.

*Note technique : l'éditeur 3dsMax affiche 8 emplacements de texture,
mais le moteur de jeu n'en garde réellement que 4 une fois l'objet
exporté — les emplacements 5 à 8 n'existent que dans l'éditeur, pour les
réglages du mode Eau qui ne sont pas stockés comme de simples textures de
matériau.*

### Plusieurs images alternatives par emplacement (Multi Bitmap) {#multi-bitmap}

**Résumé :** Au lieu d'une seule image fixe, un emplacement de texture peut
contenir jusqu'à 8 images alternatives — le jeu choisit automatiquement
laquelle afficher selon le contexte (la saison, la qualité d'un objet...).

Normalement, un emplacement de texture pointe vers une seule image fixe.
Mais on peut à la place lui assigner un "jeu d'images" (jusqu'à 8), et
laisser le jeu choisir automatiquement laquelle utiliser au moment de
l'affichage plutôt qu'au moment de la création de l'objet. Le choix est
fait par le code du jeu selon le contexte, par exemple :

- La **saison actuelle** du continent (un arbre ou un bâtiment peut avoir
  une apparence différente en hiver qu'en été).
- La **qualité ou la variante d'un objet** (un même modèle d'équipement
  peut afficher une texture différente selon sa qualité, par exemple pour
  distinguer un objet de qualité supérieure au premier coup d'œil).

Si aucune image n'a été choisie explicitement pour un contexte donné, la
première image du jeu est utilisée par défaut. Ce mécanisme permet de
réutiliser un seul objet 3D pour plusieurs apparences, plutôt que de devoir
créer un objet complet différent pour chaque variante.

**Exemples concrets tirés des fiches Georges du jeu :**

- Sur une créature, le champ `Texture` (fiche `.creature`, type
  `_creature_texture_equipment.typ`) choisit l'image parmi celles
  disponibles pour un emplacement de texture donné. La valeur spéciale
  `-1` ("Season") dit au jeu de plutôt utiliser la saison actuelle pour
  choisir l'image, au lieu d'une valeur fixe.
- Sur un objet équipable (arme, armure...), le champ `map_variant` (fiche
  d'objet, type `item_map.typ`) choisit l'image selon la **qualité** de
  l'objet : `Low Quality`(0), `Medium Quality`(1), `High Quality`(2),
  `Super Quality`(3), `XL Quality`(4), `Suprem Quality`(5), `Divine
  Quality`(6), `Obiwan Quality`(7) — jusqu'à 8 apparences différentes pour
  un seul modèle 3D selon la qualité de fabrication de l'objet.
- Pour l'apparence générale d'une créature selon la région où elle vit, le
  type `_creature_texture.typ` propose : `none`(0, étiqueté ainsi dans la
  fiche mais correspond en pratique à la Forêt), `Lacustre`(1), `Desert`(2),
  `Jungle`(3), `Primr`(4, Racines Primordiales), `goo`(5, la Goo) — une
  créature peut ainsi avoir un pelage/une texture différente selon
  l'écosystème où elle apparaît, sans dupliquer son modèle 3D.
- Pour une apparence qui change avec la saison du jeu, le moteur définit
  `Spring`(0), `Summer`(1), `Autumn`(2), `Winter`(3)
  (`EGSPD::CSeason` dans `ryzom/common/src/game_share/season.h`) — ce sont
  ces mêmes 4 premiers indices que choisit automatiquement la valeur
  spéciale `-1` ("Season") du champ `Texture` mentionné ci-dessus, en
  fonction de la saison en cours sur le serveur.

Le nombre exact d'apparences disponibles (jusqu'à 8) et ce que chaque
valeur représente dépend donc entièrement de la fiche Georges (ou, pour la
saison, du moteur lui-même) qui utilise ce mécanisme — le format du
`.shape`, lui, ne voit qu'un simple numéro d'image à choisir.

*Note technique : dans l'éditeur 3dsMax, ce réglage remplace le type
d'image "Bitmap" habituel par un type spécial "Nel Multi Bitmap" sur
l'emplacement de texture concerné — ce n'est pas visible dans la liste des
réglages du matériau lui-même, mais dans le choix du type de la texture à
cet emplacement. Le moteur le stocke sous la classe `CTextureMultiFile`
(`nel/include/nel/3d/texture_multi_file.h`), une liste de noms de fichiers
plus un index "actuellement sélectionné" ; le jeu appelle
`selectTextureSet()` à l'exécution pour changer cet index (voir par
exemple `continent.cpp` pour le changement de saison, ou
`character_3d.cpp`/`player_cl.cpp` pour la qualité/variante d'un objet
porté). `pynel` sait déjà lire cette classe (`ryzom_shape.py`'s
`_parse_texture_multi_file`) et expose `file_names`/`selected_index` en
plus du nom actuellement sélectionné.*

### Coordonnées générées automatiquement {#tex-gen}

**Résumé :** Au lieu de "coller" une texture avec un calage fixe, on peut
demander au jeu de calculer lui-même comment la positionner, typiquement
pour un effet de reflet qui doit suivre le point de vue.

Normalement, chaque texture est calée sur l'objet une fois pour toutes
(ses "coordonnées UV"). Mais pour un effet comme un reflet d'environnement
sur une surface brillante ou courbe, ce calage fixe ne fonctionne pas
puisque le reflet doit changer selon l'angle de vue. Ce réglage dit au jeu
de calculer la position de la texture en temps réel plutôt que d'utiliser
un calage fixe.

---

### Filtrage et répétition des textures {#texture-filtering}

**Résumé :** Comment une texture se comporte hors de son image de base
(répétition ou non) et comment elle est lissée à l'écran (net/pixellisé ou
doux) selon qu'on est proche ou loin de l'objet.

Ces réglages ne concernent pas le matériau dans son ensemble, mais chaque
image (texture) individuellement — chacune des 4 textures d'un matériau a
les siens.

- **Répétition horizontale / verticale (Wrap S / Wrap T)** : que fait la
  texture quand ses coordonnées de calage (UV) dépassent le cadre de
  l'image, séparément dans le sens horizontal et vertical. Par défaut, la
  texture se répète en tuile (utile pour un sol, un mur, un tissu qui
  couvre une grande surface avec une petite image) ; on peut aussi la
  figer sur son dernier pixel de bord plutôt que de la répéter, pour éviter
  une couture visible sur une texture qui ne boucle pas proprement.
- **Filtrage de grossissement (Mag Filter)** : comment la texture est
  lissée quand on est **proche** de l'objet (l'image est agrandie à
  l'écran) — soit nette/pixellisée (chaque pixel de la texture reste un
  petit carré net), soit adoucie (les pixels voisins sont mélangés pour un
  rendu plus doux). Le réglage normal du moteur est le rendu adouci.
- **Filtrage de rapetissement (Min Filter)** : la même idée mais pour
  quand on est **loin** de l'objet (l'image est réduite à l'écran) — avec
  en plus la question de l'utilisation ou non des "mipmaps" (des copies
  pré-réduites de la texture, préparées à l'avance pour éviter le
  scintillement/moiré des petits détails vus de loin). Le réglage normal
  du moteur combine lissage doux et mipmaps pour le rendu le plus propre
  possible à toutes les distances.

*Note technique : sans ces deux derniers réglages remis à leur valeur
normale (lissage + mipmaps), une texture prend un aspect "strié"/pixellisé
inhabituel même si son image et son matériau sont par ailleurs corrects —
voir le bug corrigé dans `pynel` (2026-08) où ces valeurs, lues mais
jamais mémorisées, étaient remises à "aucun lissage" à chaque sauvegarde
d'un `.shape`.*

Deux réglages supplémentaires, plus rarement utiles :

- **Image en niveaux de gris utilisée comme transparence
  (LoadGrayscaleAsAlpha)** : normalement, une image apporte de la couleur
  (rouge/vert/bleu) et, séparément, un canal de transparence (alpha). Ce
  réglage dit au jeu : "cette image n'a qu'un seul canal en noir et blanc
  — au lieu de l'utiliser comme une couleur grise, utilise-le comme la
  transparence d'un autre canal". C'est une astuce pour économiser de la
  mémoire quand une image ne sert qu'à définir *où* quelque chose est
  visible/transparent (un masque), sans avoir besoin d'y stocker de vraie
  couleur — par exemple pour un motif découpé (grillage, feuillage,
  dentelle) où seule la silhouette compte, la couleur venant d'ailleurs.
- **Format d'envoi à la carte graphique (UploadFormat)** : réglage
  technique bas niveau qui précise dans quel format de couleur la texture
  doit être envoyée à la carte graphique (compressée, pleine qualité...).
  En pratique, presque toujours laissé sur "Auto" (le jeu choisit tout
  seul le format le plus adapté) — il n'y a normalement pas besoin d'y
  toucher.

---

## Combinaison des textures (multi-texturing) {#multitexture}

**Résumé :** Quand plusieurs textures sont actives sur le même matériau,
ce réglage explique comment elles se combinent entre elles (l'une
au-dessus de l'autre, l'une teinte l'autre, elles s'additionnent...).

Ce réglage existe séparément pour chacune des 4 textures actives, et
séparément aussi pour la couleur (RGB) et la transparence (alpha) de
chaque texture — mais le principe est le même partout : on choisit une
**opération**, et jusqu'à 3 **ingrédients** avec lesquels cette opération
travaille.

**Les opérations possibles :**

- **Remplacer** : la texture remplace complètement ce qu'il y avait avant.
- **Moduler (multiplier)** : la texture assombrit/teinte ce qu'il y avait
  avant, comme un calque de teinte semi-transparent — l'opération la plus
  courante pour, par exemple, appliquer une texture d'ombre au sol.
- **Additionner** : la texture éclaircit ce qu'il y avait avant en
  s'ajoutant à sa couleur (effet lumineux, comme pour le mélange
  *Additif* vu plus haut, mais au niveau d'une seule texture).
- **Mélanger (Interpolate)** : fait un dosage progressif entre deux
  ingrédients, contrôlé par un troisième — pratique pour faire fondre
  progressivement deux textures l'une dans l'autre (ex. transition entre
  de l'herbe et de la terre sur un terrain).

**Les ingrédients possibles**, utilisés par les opérations ci-dessus :

- **Texture** : la texture de cet emplacement.
- **Résultat précédent** : ce qui a déjà été calculé par la texture
  précédente dans la pile (permet d'empiler les effets les uns sur les
  autres).
- **Diffuse** : la couleur diffuse de base du matériau (voir *Les couleurs
  de base*).
- **Constante** : une couleur fixe choisie à la main pour ce matériau, qui
  ne vient d'aucune texture ni de l'éclairage.

*Note technique : il existe aussi un réglage avancé, l'"adressage de
texture", qui couvre des techniques plus pointues comme les reflets par
cube-map ou le relief simulé par produit scalaire (bump mapping "DP3").
Ce sont des techniques spécialisées, rarement modifiées à la main par un
artiste — la plupart du temps, les préréglages ("Setup Stage" :
Single/Add/Multiply/Blend/BlendMask/Mix Two Textures) suffisent à obtenir
l'effet voulu sans avoir à comprendre ce niveau de détail.*

---

## Eau (Water) {#water}

**Résumé :** Un ensemble de réglages qui n'apparaissent que si le *Type de
matériau* est réglé sur "Eau" — ils contrôlent l'apparence des surfaces
d'eau (mer, lac, rivière) : ses vagues, ses reflets, sa transparence selon
la profondeur.

Ce groupe de réglages est un cas à part : contrairement à tous les autres
réglages de ce document, ils ne sont pas enregistrés dans le matériau
final du fichier `.shape` — l'eau du jeu fonctionne comme un système
séparé qui relit ces réglages uniquement au moment de l'export depuis
3dsMax. On y retrouve, entre autres : l'intensité du relief des vagues et
leur vitesse de déplacement, l'utilisation du reflet du ciel de la scène
(de jour et/ou de nuit, au-dessus et/ou en dessous de la surface), le
calcul d'un reflet en temps réel, et le réglage de l'effet "fresnel" (plus
on regarde l'eau de loin/à plat, plus elle reflète comme un miroir ; plus
on la regarde de près/à la verticale, plus on voit à travers).

---

## Couleur utilisateur personnalisée {#user-color}

**Résumé :** La couleur exacte utilisée quand le *Type de matériau* est
réglé sur "Couleur utilisateur" — voir ce mode plus haut.

---

## Matrice de texture exportée {#export-texture-matrix}

**Résumé :** Réglage avancé qui permet d'animer ou déformer une texture
(la faire glisser, tourner, s'étirer) indépendamment de l'objet lui-même,
par exemple pour un tapis roulant ou une texture d'eau qui défile.

*Note technique : Rotation et Répétition (Scale) ne peuvent pas être
combinées sur le même canal de texture — les remettre à zéro/un l'une pour
éditer l'autre. Ce n'est pas une limite de Patina mais du matériel
graphique lui-même : une texture répétée (wrap) puis tournée ne s'aligne
plus proprement sur les bords de l'objet, faisant apparaître des tuiles en
trop ou en moins selon l'angle — un artefact qui se produirait de la même
façon avec n'importe quel outil ayant généré cette matrice. Vérifié
(2026-08-28) : aucun shape du jeu ne combine les deux, ce n'est donc pas
une perte réelle.*
