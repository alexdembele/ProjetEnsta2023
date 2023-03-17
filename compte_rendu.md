# Projet Ensta 2023
## Compte rendu Tao Guinot & Alex Dembélé
  



  
### I. Introduction
Ce projet à pour but de paralléliser en c++ un simulateur de tourbillon.  

  
Avant de commencer à travailler, il a fallu changer les makefiles pour compiler avec les bons outils. On a changé le compilateur g++ en mpi++ pour pouvoir compiler avec mpi. L'exécution se fera avec mpirun.

### II. Séparation clcul affichage
La première étape du projet est de séparer le code d'affichage des calculs en deux processus distincts à l'aide de mpi. cela permettra d'accélérer le temps d'exécution car les tâches s'exécuteront en parallèle. Dans notre code, le processus 0 correspond à l'affichage et le processus 1 aux calculs.

Résultats:  
Les FPS sont de 40-50 dans le code non parallélisé, ils sont de 500-600 dans le code parallélisé. On a donc obtenu e=un speed-up d'environ 10-11 en séparant l'affichage du calcul dans notre code.

### III Accélération des calculs
La deuxième étape est d'accélérer les calculs en les parallélisant en mémoire partagés avec openMP.
En examinant le code d'origine, il semble que la méthode Vortices::computeSpeed est parallélisable. En effet, il y a une boucle effectuant la même tâche 9 fois sur des tourbillons différents et changeant une variable vspeed. Il faut donc utiliser une clause de réduction pour que la variable speed soit commune à tous les threads. On test le temps d'exécution de juste cette boucle pour plusieurs threads.

| | |
|---|---|
|Nombre de threads|Temps d'exécution|
|1| 1.65e-7 s |
|2| 1.5e-7 s |
|3| 1.25e-7 s|
|4| 1.65e-7 s |
|5| 1.55e-7s|
|6| 1.35e-7s|
|7| 1.5e-7 s|
|8| 1.45e-7s|

Les mesures ont été faites avec un dt de 0.1
On n'a pu tester cette parallélisation avec au maximum 8 threads car on est limité par nos machines personelles. Mais nous pensons qu'il aurait été intéressant d'essayer avec 9 threads car il y a 9 étapes différentes dans la boucle étudiée.
Cependant, globalement cette parallélisation n'améliore pas du tout le temps d'exécution. En effet, quand on regarde le code en lui même, on constate que la boucle va s'exécuter peu de fois et qu'il y a peu d'opération dans la boucle. Cette parallélisation aurait surement eu plus d'impact pour un grand nombre de vortex.



