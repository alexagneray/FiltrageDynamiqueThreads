/**
 * @file FiltrageDynamiqueThreads.cpp
 * @brief Environnement dans lequel des threads dit "générateurs" 
 *         ajoutent des données numériques entre 0 et 500 dans un std::vector
 *          D'autres threads dit "correcteurs" sont chargés de lire les données
 *          nouvellement ajoutées et de remplacer les valeurs supérieures à un seuil N par -1
 *          Un thread unique nommé négociateur est chargé d'ajuster la valeur de N, de sorte à
 *          ne garder que P % des valeurs dans le vecteur
 *          Le thread principal doit générer une valeur aléatoire de P qui change toutes les 20 secondes
 */


#include "FiltrageDynamique.h"

int main()
{
    FiltrageDynamique& filtdyn = FiltrageDynamique::GetInstance();
    filtdyn.Run();

    return 0;
}

