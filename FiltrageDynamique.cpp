#include "FiltrageDynamique.h"

std::atomic<int> FiltrageDynamique::N; // Seuil de correction

std::atomic<double> FiltrageDynamique::P;  // Pourcentage de valeurs à conserver

std::vector<int> FiltrageDynamique::data;
std::mutex FiltrageDynamique::data_mutex;

std::atomic<int> FiltrageDynamique::correcteur_idx; 
std::atomic<int> FiltrageDynamique::corrections_count;

std::mutex FiltrageDynamique::cout_mutex; 

std::condition_variable FiltrageDynamique::data_full_cv; 
std::mutex FiltrageDynamique::data_full_mutex;

void FiltrageDynamique::Générateur(int id)
{
    while(true)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        std::unique_lock<std::mutex> cout_lock(cout_mutex, std::defer_lock);

        int value = std::rand() % 501; // Génère une valeur entre 0 et 500
        data.push_back(value);
    }
}

void FiltrageDynamique::Correcteur(int id)
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

void FiltrageDynamique::Négociateur()
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

void FiltrageDynamique::Observateur()
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


void FiltrageDynamique::Nettoyeur()
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


void FiltrageDynamique::Run()
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
}

FiltrageDynamique& FiltrageDynamique::GetInstance()
{
    static FiltrageDynamique instance;
    return instance;
}

FiltrageDynamique::FiltrageDynamique()
{
    N.store(100);
    P.store(0.5);
    correcteur_idx.store(0);
    corrections_count.store(0);
}