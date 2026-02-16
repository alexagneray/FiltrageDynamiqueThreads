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

#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <algorithm>

constexpr const int GENERATEUR_COUNT = 5; // Nombre de threads générateurs
constexpr const int CORRECTEUR_COUNT = 3;  // Nombre de threads correcteurs
constexpr const int MAX_DATA_LEN = 1000000; 

std::atomic<int> N {100}; // Seuil de correction

std::atomic<double> P {0.2};  // Pourcentage de valeurs à conserver

std::vector<int> data;
std::mutex data_mutex;

std::atomic<int> correcteur_idx{0}; 
std::atomic<int> corrections_count{0};

std::mutex cout_mutex; 

std::condition_variable data_full_cv; 
std::mutex data_full_mutex;





void Générateur(int id)
{
    while(true)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        std::unique_lock<std::mutex> cout_lock(cout_mutex, std::defer_lock);

        int value = std::rand() % 501; // Génère une valeur entre 0 et 500
        data.push_back(value);
    }
}

void Correcteur(int id)
{
    while(true)
    {  
        int local_idx;
        
        std::unique_lock<std::mutex> cout_lock(cout_mutex, std::defer_lock);

        local_idx = correcteur_idx.load();
        correcteur_idx++;

        std::lock_guard<std::mutex> lock(data_mutex);
        if(local_idx < data.size())
        {
            int& value = data[local_idx];
            if (value > N)
            {
                corrections_count++;

                value = -1;
            }
        }
    }
}

void Négociateur()
{
    std::unique_lock<std::mutex> data_lock(data_mutex, std::defer_lock);
    std::unique_lock<std::mutex> cout_lock(cout_mutex, std::defer_lock);

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Attendre 5 secondes avant de recalculer P

        data_lock.lock();
        double current_percentage = (data.size() > 0) ? static_cast<double>(corrections_count.load()) / data.size() : 0.0;
        data_lock.unlock();


        if (current_percentage > P)
        {
            N += 10;

            cout_lock.lock();
            std::cout << "Négociateur a augmenté N à " << N << " pour réduire les corrections." << std::endl;
            cout_lock.unlock();
        }
        else if (current_percentage < P)
        {
            N -= 10;

            cout_lock.lock();
            std::cout << "Négociateur a diminué N à " << N << " pour augmenter les corrections." << std::endl;
            cout_lock.unlock();
        }
    }

}

void Observateur()
{
    int data_size = 0;
    int corrections_count_local = 0;
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::unique_lock<std::mutex> lock(data_mutex);
        data_size = data.size();
        lock.unlock();

        corrections_count_local = corrections_count.load();


        double current_percentage = (data_size > 0) ? static_cast<double>(corrections_count_local) / data_size : 0.0;

        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "Observateur: " << data_size << " valeurs, " << corrections_count_local << " corrections, pourcentage de corrections: " << current_percentage * 100 << "%" << std::endl;

    }
}


void Nettoyeur()
{
    while(true)
    {
        std::unique_lock<std::mutex> data_full_lock(data_full_mutex);
        data_full_cv.wait_for(data_full_lock, std::chrono::seconds(1),
                        []()
                        {
                            int data_size = 0;
                            std::unique_lock<std::mutex> data_lock(data_mutex);
                            data_size = data.size();
                            data_lock.unlock();

                            return data_size >= MAX_DATA_LEN; 
                        });
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Attendre 10 secondes avant de nettoyer les données

        std::lock_guard<std::mutex> lock(data_mutex);
        data.erase(data.begin(), data.end());
        correcteur_idx = 0;
        corrections_count = 0;
    }
}
int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Initialiser le générateur de nombres aléatoires

    std::vector<std::thread> generateurs;
    std::vector<std::thread> correcteurs;

    for (int i = 0; i < GENERATEUR_COUNT; ++i)
    {
        generateurs.emplace_back(Générateur, i);
    }

    for (int i = 0; i < CORRECTEUR_COUNT; ++i)
    {
        correcteurs.emplace_back(Correcteur, i);
    }

    std::thread negociateur(Négociateur);
    std::thread observateur(Observateur);
    std::thread nettoyeur(Nettoyeur);


    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(20)); // Attendre 5 secondes avant de recalculer P

        double new_P = static_cast<double>(std::rand()) / RAND_MAX;
        P.store(new_P);

        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "Le thread principal a mis à jour P à " << new_P * 100 << "%" << std::endl;
    }

    // Attendre que tous les threads générateurs et correcteurs terminent
    for (auto& gen : generateurs)
    {
        gen.join();
    }
    
    for (auto& corr : correcteurs)
    {
        corr.join();
    }

    negociateur.join();
    observateur.join();
    nettoyeur.join();

    return 0;
}

