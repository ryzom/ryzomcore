
 ====================
      ZVIEWER 
 ====================


ZVIEWER permet de visioner une ou plusieurs zones exportées depuis 3DSMax 
au format de NeL, en utilisant le moteur.


1. Le fichier zviewer.cfg
   -----------------------

Le répertoire d'où est exécuté le programme doit contenir le fichier ZVIEWER.CFG
qui contient les paramètres de configuration. Sans ce fichier aucune zone ne sera 
affichée.
Lorsqu'il n'y a pas de fichier de configuration à l'execution un fichier type est
généré.

Exemple de config :

FullScreen = 0;	
Width = 800;    
Height = 600; 
Depth = 32;	
Position = { 1088.9, -925.7, -1234.0 }; 
EyesHeight = 1.80;  
Background = { 100, 100, 255 }; 
ZFar = 1000.0;	               
LandscapeTileNear = 50.0;       
LandscapeThreshold = 0.001;    
BanksPath = "C:\WINDOWS\Bureau\zviewer\" ;
Bank = "fyros_landf.bank" ;
TilesPath = "C:\WINDOWS\Bureau\zviewer\tiles" ;
ZonesPath = "C:\WINDOWS\Bureau\zviewer\zones" ;
IgPath = "C:\WINDOWS\Bureau\zviewer\ig" ;
ShapePath = "C:\WINDOWS\Bureau\zviewer\shapes" ;
MapsPath = "C:\WINDOWS\Bureau\zviewer\maps" ;
Zones = { "3_AC.zonel","3_AD.zonel" }; 
Zonessave = { "3_AC.zonel","3_AD.zonel" }; 
Zones = { "3_AC.zonel","3_AD.zonel" }; 
Ig = { "3_AC.ig","3_AD.ig" }; 

Avec :

Fullscreen :
0 : windowed 
1 : fullscreen

Width :
largeur de la fenetre.

Height :
hauteur de la fenetre.

Depth :
nombre de couleurs (32bits,24bits,...).

Position :
position initiale (x,y,z). Correspond aux coordonnées dans Max (Generic Units).
La position est automatiquement snapée au sol quelque soit la coordonnée en z.

EyesHeight :
hauteur des yeux pour le mode Walk.

Background :
couleur rgb du fond.

ZFar :
distance de clipping. Plus la distance est grande, plus on voit loin.

LandscapeTileNear :
valeur du near. Plus cette valeur est grande, plus la zone du near est grande.

LandscapeThreshold :
seuil utilisé pour la subdivision adaptative. Quand la valeur augmente, la qualité
de la subdivision diminue (reduit le temps de calcul). Mis au minimum par defaut (0.001).

BanksPath :
chemin des banks (<bank>.bank et <bank>.farbank).

Bank :
nom de la bank (meme racine que la farbank)

ZonesPath :
chemin des zones.

Zones :
liste des noms des zones


2. Les déplacements
   ----------------

Les déplacements se font à l'aide de la combinaison clavier/souris. 
Il existe 2 modes de déplacement : FreeLook et Walk (defaut). 

- Le mode FreeLook permet de se déplacer dans la direction du regard(idem cstrike). 
- Dans le mode Walk le déplacement se fait au niveau du sol, à hauteur des yeux. 
  L'utilisateur reste collé au sol.

La souris permet d'orienter le regard dans toutes les directions
Le clavier permet de se déplacer.


3. Les touches
   -----------
ESC      : quitter le viewer
F1       : Afficher/Masquer les infos
F3       : basculer entre le mode plein et le mode fil de fer
F12      : faire un screenshot
ESPACE   : basculer entre les modes Walk et Free-Look
RETURN   : libérer ou capturer la souris
UP       : avancer
DOWN     : reculer
LEFT     : déplacement latéral à gauche
RIGHT    : déplacement latéral à droite
SHIFT    : augmenter l'altitude 
CTRL     : baisser d'altitude 
1        : 5 km/h
2        : 10 km/h
3        : 20 km/h
4        : 40 km/h
5        : 80 km/h
6        : 160 km/h
7        : 320 km/h
8        : 640 km/h
PAGEUP   : augmenter la distance de clipping
PAGEDOWN : diminuer la distance de clipping
HOME     : augmenter le near
END      : diminuer le near
INSERT   : augmenter le threshold
DELETE   : diminuer le threshold
ADD      : augmenter la hauteur des yeux
SUBTRACT : diminuer la hauteur des yeux

==================== ZVIEWER ENGLISH ====================
 
ZViewer allows you to view one or more zones exported from 3DSMax inside the NeL Engine

1. zviewer.cfg File
   ----------------


When you run ZViewer for the first time if no  ZVIEWER.CFG configuration file is located one will be created with default values.
The Configuration file defines the zones to load and other vital information and will not work in it's default state

Config Example :

FullScreen = 0;	
Width = 800;    
Height = 600; 
Depth = 32;	
Position = { 1088.9, -925.7, -1234.0 }; 
EyesHeight = 1.80;  
Background = { 100, 100, 255 }; 
ZFar = 1000.0;	               
LandscapeTileNear = 50.0;       
LandscapeThreshold = 0.001;    
BanksPath = "C:\WINDOWS\Bureau\zviewer\" ;
Bank = "fyros_landf.bank" ;
TilesPath = "C:\WINDOWS\Bureau\zviewer\tiles" ;
ZonesPath = "C:\WINDOWS\Bureau\zviewer\zones" ;
IgPath = "C:\WINDOWS\Bureau\zviewer\ig" ;
ShapePath = "C:\WINDOWS\Bureau\zviewer\shapes" ;
MapsPath = "C:\WINDOWS\Bureau\zviewer\maps" ;
Zones = { "3_AC.zonel","3_AD.zonel" }; 
Zonessave = { "3_AC.zonel","3_AD.zonel" }; 
Zones = { "3_AC.zonel","3_AD.zonel" }; 
Ig = { "3_AC.ig","3_AD.ig" }; 

With :

Fullscreen :
0 : windowed 
1 : fullscreen

Width :
width of the window.

Height :
height of the window.

Depth :
Color depth (32bits,24bits,...).

Position :
Initial position (x,y,z). Corresponding to Max Coordinates (Generic Units).
The position is automatically snaped to the ground whatever the Z-Coordinate value is.

EyesHeight :
Eyes height in Walk mode.

Background :
rgb background color.

ZFar :
Clipping distance. the higher the distance is, the further u will see.

LandscapeTileNear :
Near value. the higher the value is, the wider the near zone will be.

LandscapeThreshold :
Threshold used for Adaptative Degradation. the higher this value is, the lower 
the Subdivision Quality will be (Reduce calculation time). Minimal value by default (0.001)

BanksPath :
Banks path (same path for <bank>.bank and <bank>.farbank).

Bank :
Banks Name (same prefix for .bank and .farbank)

ZonesPath :
Zones path.

Zones :
Zones names List.


2. Moving
   ------

Moving is done by using both Keyboard and Mouse.
- Change Look orientation using Mouse
- Move using Keyboard

Two move modes are available : FreeLook and Walk (by default).
- In FreeLook mode, The camera follows Look orientation (same as cstrike)
- In Walk Mode, The camera is snaped to the ground, at eyes height.


3. Keys
   ----

ESC: exit ZViewer
F1: toggles information
F3: toggles between full and the wire frame mode
F12: screenshot
SPACE: toggles between Walk and Free-Look mode
RETURN: release or capture mouse
UP: move forward
DOWN: move back
LEFT: turn left
RIGHT: turn right
SHIFT: increases altitude
CTRL: decreases altitude
1: 5 km/h
2: 10 km/h
3: 20 km/h
4: 40 km/h
5: 80 km/h
6: 160 km/h
7: 320 km/h
8: 640 km/h
PAGEUP: increases the clipping distance
PAGEDOWN: decreases the clipping distance
HOME: increases tilenear
END: decreases the tilenear
INSERT: increases threshold
DELETE: decreases threshold
ADD: increases the eye height
SUBTRACT: decreases the eye height
