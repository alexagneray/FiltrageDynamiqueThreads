# FiltrageDynamiqueThreads
Filtre Adaptatif Multi-Threads avec Négociateur Dynamique

Petit environnement multithread, dont le fonctionnement est le suivant : 
- Des Générateurs ajoutent des valeurs entre 0 et 500 dans un std::vector
- Des Correcteurs sont chargés de relire les valeurs ajoutées et de remplacer par -1 les valeurs supérieures à un seuil N
- Un Négociateur est chargé d'ajuster la valeur de N, afin que le taux de valeur corrigé tende vers une valeur P
- Un Observateur est chargé de donner les statistiques des données à intervalle régulier (nombre de valeurs, nombre de corrections et pourcentage de correction)
- Un Nettoyeur est déclenché lorsque le nombre de valeurs dépasse MAX_DATA_LEN, le std::vector est alors vidé
- Le thread principal (fonction main) génère une valeur aléatoire de P toutes les 20s