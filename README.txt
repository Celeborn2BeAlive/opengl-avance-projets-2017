Projet OpenGL avancé : LAMBLIN Thibaut et PERRY Nicolas

Sujet : Reflective Shadow Map




/****************************/
/* Utilisation de l'archive */
/****************************/

mkdir opengl-avance-build
cd opengl-avance-build
cmake ../opengl-avance
make
./bin/RSM




/****************************/
/*     Travail effectué     */
/****************************/

Point de départ : template fourni originel


Modification de la librairie glmlv, en particulier de SimpleGeometry : 
	>Utilisation de couleurs à la place des coordonnées de texture
	>Création d'une primitive de scène d'observation

	
Implémentation des différents shaders (shadow map : rsm.x.glsl, rendu : shading.x.glsl inspiré de la version de Yatagawa)

Gestion des différentes textures de la Shadow Map et de leurs utilisations

Gestion des différents buffers liés à la scène

...




/****************************/
/*         Problème         */
/****************************/

Lors de la boucle de rendu final, l'envoi d'une variable uniforme pose problème au niveau du vertex shader (mvp en particulier) impliquant une mauvaise position
