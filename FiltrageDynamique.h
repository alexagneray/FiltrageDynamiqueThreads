#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <algorithm>

class FiltrageDynamique
{
private:
    constexpr const static int GENERATEUR_COUNT = 5; // Nombre de threads générateurs
    constexpr const static int CORRECTEUR_COUNT = 3;  // Nombre de threads correcteurs
    constexpr const static int MAX_DATA_LEN = 1000000; 

    static std::atomic<int> N; // Seuil de correction

    static std::atomic<double> P;  // Pourcentage de valeurs à conserver

    static std::vector<int> data;
    static std::mutex data_mutex;

    static std::atomic<int> correcteur_idx; 
    static std::atomic<int> corrections_count;

    static std::mutex cout_mutex; 

    static std::condition_variable data_full_cv; 
    static std::mutex data_full_mutex;

    static void Générateur(int id);
    static void Correcteur(int id);
    static void Négociateur();
    static void Observateur();
    static void Nettoyeur();

    FiltrageDynamique();
    ~FiltrageDynamique() = default;
public:
    FiltrageDynamique(const FiltrageDynamique& ) = delete;
    FiltrageDynamique& operator=(const FiltrageDynamique&) = delete;
    
    static FiltrageDynamique& GetInstance();

    void Run();
};