# Projet Ensta 2023
## Compte rendu Tao Guinot & Alex Dembélé
  



  
### I. Introduction
Ce projet à pour but de paralléliser en c++ un simulateur de tourbillon.  

  
Avant de commencer à travailler, il a fallu changer les makefiles pour compiler avec les bons outils. On a changé le compilateur g++ en mpi++ pour pouvoir compiler avec mpi. L'exécution se fera avec mpirun.

### II. Séparation calcul affichage
La première étape du projet est de séparer le code d'affichage des calculs en deux processus distincts à l'aide de mpi. cela permettra d'accélérer le temps d'exécution car les tâches s'exécuteront en parallèle. Dans notre code, le processus 0 correspond à l'affichage et le processus 1 aux calculs. Pour gérer la communication avec MPI, on a défini les variables grid cloud vortices dans les deux programmes. Le processus de calcul met à jour les données puis les envoie au processus d'affichage. Le processus d'affichage gère les entrées clavier et envoie les commandes au processus de calcul. 

Résultats:  
Les FPS sont de 40-50 dans le code non parallélisé, ils sont de 500-600 dans le code parallélisé (les deux en fonctionnement avec le bouton p). On a donc obtenu un speed-up d'environ 10-11 en séparant l'affichage du calcul dans notre code.

### III. Accélération des calculs
La deuxième étape est d'accélérer les calculs en les parallélisant en mémoire partagés avec openMP.
En examinant le code d'origine, il semble que la méthode Vortices::computeSpeed est parallélisable. En effet, il y a une boucle effectuant la même tâche 9 fois sur des tourbillons différents et changeant une variable vspeed. Il faut donc utiliser une clause de réduction pour que la variable speed soit commune à tous les threads. Cependant, globalement cette parallélisation ne devrait pas améliorer le temps d'exécution. En effet, quand on regarde le code en lui même, on constate que la boucle va s'exécuter peu de fois et qu'il y a peu d'opération dans la boucle. Cette parallélisation aurait surement eu plus d'impact pour un grand nombre de vortex.  
  
Les méthodes intéressantes à paralléliser sont celles des calcul de runge_kutta. Ce sont celles qui prennent le plus de temps de calcul. Par exemple, un tour de boucle calcul+affichage prends 0.022s à s'exécuter, la méthod de calcul runge_kutta prends 0.019s à s'exécuter. C'est parce qu'elle contient une boucle sur le nombre de point qui est grand.  
  
Pour paralléliser ces boucles, on utilise #pragma parallel for avant la boucle. Cependant, le compilateur nous dit qu'il ignore ces lignes. Après de longue recherche sur internet avec le message d'erreur du compilateur, la seule solution trouvée était de rajouter -fopenmp dans le makefile. Cependant, dans le makefile, il y a bien -fopenmp. De plus, lorsqu'on regarde les courbes d'activités de nos processeurs,lorsqu'on lance notre programme, 4 coeurs se mettent en activité peut importe le nombre de coeur spécifié avec OMP_NUM_THREADS=... . C'est incompréhensible.  
Pour pallier à ce problème, nous avons travailler sur le code  non paralléliser avec MPI. En supprimant tous les paramètres du makefilesauf g++ -fopenmp, on arrive à observer quelque chose.  
  
En effet, en modifiant juste le makefile, nos fps sont abaissés de 45-50 à 30-35. Et quand on rajoue un #pragma parallel for devant la boucle de calcul sur le nombre de point de runge_kutta on passe à 70-80 fps. Pour pouvoir spécifier le nombre de thread on n'a pas pu passer par OMP_NUM_THREADs=... dans le terminal, il a fallu le définir dans le code avant le #pragma parallel for avec omp_set_num_threads(...).  
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
  
Le speed-up augmente jusqu'à 4 threads puis stagne. Il est donc intéressant de paralléliser sur 4 coeurs avec openMP pour se réserver d'autres coeurs pour d'autres tâches. 
### IV. Distribution des calculs

On a essayé de distribuer les calculs sur plusieurs processeurs. Cependant nous avons fait face à de nombreux segfault et problème de communication. Nous avons découpé en nbp processus: le 0 fait l'affichage, les 1 à nbp-2 font du calcul avec un même nombre de donnée, le nbp-1 fait des calculs avec les données restantes ( pour respecter la taille des données qui n'est pas forcément un multiple du nombre de processeur de calcul). Le programme plante immédiatement lorsque nous mettons plus de 3 coeurs ou dit qu'il n'y en a pas assez de disponible alors que normalement si... Quand on met 3 coeurs de calcul, l'affichage initiale se passe bien mais le programme plante lorsqu'on demande de faire des calculs. Il doit y avoir un problème de communication ou de gestion de données.

### V. Approche Mixte
Nous souhaitons paralléliser le calcul de la vitesse en décomposant en sous domaine. Pour ce faire , il faut choisir comment découper en sous domaine. On travail sur un quadrillage, on peut découper selon les lignes ou les colonnes pour que ce soit "facile" de coder le découpage.  Il y aurait un thread qui distribuerait les calculs  aux autres car on ne sait pas si ils seront tous de même durée. Par exemple, si on parallélise avec 4 threads, le thread 0 distribuerait des rectangles de 10 lignes aux trois autres et actualiserait sa grille au fur et à mesure des calculs des autres. Il faut faire attention quand on envoie les sous domaines aux potentielles conditions aux limites dont on aurait besoin, quitte à employer des ghosts-cells. Avec cette stratégie maître esclave, le temps de calcul est bien réparti. Si les grilles sont de grande dimension avec de fortes chances de disparité de temps de calcul  par sous domaines il n'y aurait pas un thread qui finirait bien avant ou après les autres. Si on avait décomposer en 4 grands sous domaines, il y aurait de fortes chances qu'un domaine contiennent plus de calculs que les autres et prennent plus de temps.  
  
Ensuite en mixant les deux approches, il ne faut pas que la répartition des coeurs se chevauchent. Si on utilise nos 8 coeurs pour paralléliser l'approche eulérienne et qu'en même temps on a besoin des 8 coeurs pour l'approche lagrangienne il va y avoir un problème. Il faut aussi veiller à ce qu'on utilise tous nos coeurs en permanence. Il ne faut pas qu'un coeur faisant une des deux tâches finissent bien avant ou après ses semblables ce qui ne serait pas optimale.    
Si il y a un très grand nombre nombre de particules et un maillage de taille raisonnable, le calcul des positions va limiter la parallélisation et il ne faut pas qu'un des coeurs faisant ce calcul soit plus lent que les autres car on ne peut pas commencer le calcul des vitesses avant d'avoir toutes les positions dans runge_kutta.  

Si il y a un maillage de grande dimension et un grand nombre de particules,  avec la procédure maître esclave pour la parallélisation en sous-domaines, il ne devrait pas y avoir de coeurs trop longtemps inactif avant le calcul des nouvelles positions. Le cas où il y aurait des coeurs inactif serai une mauvaise répartition dans les calculs de positions.

### VI. Notice

Le code VortexSimulationQ1 contient le code de la question1. A exécuter avec: mpirun - np 2 ./.........  
Le code de la partie 2 est en commentaire dans runge_kutta mais non fonctionnel comme expliqué précedemment  
Le code de la partie 3 est dans vortexSimulation  





