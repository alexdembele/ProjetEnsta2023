# Projet Ensta 2023
## Compte rendu Tao Guinot & Alex Dembélé
  



  
### I. Introduction
Ce projet à pour but de paralléliser en c++ un simulateur de tourbillon.  

  
Avant de commencer à travailler, il a fallu changer les makefiles pour compiler avec les bons outils. On a changé le compilateur g++ en mpi++ pour pouvoir compiler avec mpi. L'exécution se fera avec mpirun.

### II. Séparation clcul affichage
La première étape du projet est de séparer le code d'affichage des calculs en deux processus distincts à l'aide de mpi. cela permettra d'accélérer le temps d'exécution car les tâches s'exécuteront en parallèle. Dans notre code, le processus 0 correspond à l'affichage et le processus 1 aux calculs.

Résultats:  
Les FPS sont de 40-50 dans le code non parallélisé, ils sont de 500-600 dans le code parallélisé. On a donc obtenu un speed-up d'environ 10-11 en séparant l'affichage du calcul dans notre code.

### III Accélération des calculs
La deuxième étape est d'accélérer les calculs en les parallélisant en mémoire partagés avec openMP.
En examinant le code d'origine, il semble que la méthode Vortices::computeSpeed est parallélisable. En effet, il y a une boucle effectuant la même tâche 9 fois sur des tourbillons différents et changeant une variable vspeed. Il faut donc utiliser une clause de réduction pour que la variable speed soit commune à tous les threads. Cependant, globalement cette parallélisation ne devrait pas améliorer le temps d'exécution. En effet, quand on regarde le code en lui même, on constate que la boucle va s'exécuter peu de fois et qu'il y a peu d'opération dans la boucle. Cette parallélisation aurait surement eu plus d'impact pour un grand nombre de vortex.  
  
Les méthodes intéressantes à paralléliser sont celles dec alcul de runge_kutta. Ce sont celles qui prennent le plus de temps de calcul. Par exemple, un tour de boucle calcul+affichage prends 0.022s à s'exécuter, la méthod de calcul runge_kutta prends 0.019s à s'exécuter. C'est parce qu'elle contient une boucle sur le nombre de point qui est grand.  
  
Pour paralléliser ces boucles, on utilise #pragma parallel for avant la boucle. Cependant, le compilateur nous dit qu'il ignore ces lignes. Après de longue recherche sur internet avec le message d'erreur du compilateur, la seule solution trouvée était de rajouter -fopenmp dans le makefile. Cependant, dans le makefile, il y a bien -fopenmp. De plus, lorsqu'on regarde les courbes d'activités de nos processeurs,lorsqu'on lance notre programme, 4 coeurs se mettent en activité peut importe le nombre de coeur spécifié avec OMP_NUM_THREADS=... . C'est incompréhensible.  
Pour pallier à ce problème, nous avons travailler sur le code  non paralléliser avec MPI. En supprimant tous les paramètres du makefilesauf g++ -fopenmp, on arrive à observer quelque chose.  
  
En effet, en modifiant juste le makefile, nos fps sont abaissés de 45-50 à 30-35. Et quand on rajoue un #pragma parallel for devant la boucle de calcul sur le nombre de point de runge_kutta on passe à 70-80 fps. Pour pouvoir spécifier le nombre de thread on n'a pas pu passer par OMP_NUM_THREADs=... dans le terminal, il a fallu le définir dans le code avant la pragma parallel for avec omp_set_num_threads(...).  
Voici nos résultats:

||||
|---|---|---|
|Nombre de threads| fps moyen| speed-up|
|1| 33| 1|
|2| 54| 1.6|
|3| 69| 2.1|
|4| 79| 2.4|
|5| 76| 2.3|
|6| 79| 2.4|
|7| 79| 2.4|
|8| 79| 2.4|
  
Le speed-up augmente jusqu'à 4 threads puis stagne. En analysant le code de la boucle de runge_kutta;, cela s'explique par le fait qu'il y a trois parties indépendantes construisant des points et vitesses à partir d'un premier point et d'une première vitesses.


