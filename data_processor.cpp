/* The purpose of this program is to classify data points as most likely originating from one underlying 
* distribution or another. The user can specify the range of the random data as well as the mean and standard 
* deviation of the hypothesized underlying distributions. Templates are employed to handle different data 
* types and each data point is generated, processed, and classified in a separate thread to make use of 
* concurrency.
*/

#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;

template<typename T>
class DataGenerator 
{        
private:        
    T data_start{};
    T data_end{};
    int dist_samples{};
public:
    DataGenerator() : data_start{ 0.0 }, data_end{ 0.0 }, dist_samples{ 0 } {}
    T Generate(T a, T b, int n) 
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_start = a;
        data_end = b;
        dist_samples = n;
        std::random_device rd;
        std::default_random_engine generator;
        generator.seed(rd());
        for (int i = 0; i < n; i++) 
        {
            std::uniform_real_distribution<T> distribution(data_start, data_end);
            T output = distribution(generator);
            return output;
        }
    } 
};

template<typename T>
class DataProcessor 
{
private: 
    T sample; 
    T d1_mean;
    T d1_std;
    T d2_mean;
    T d2_std;
public:
    DataProcessor() : sample{ 10.0 }, d1_mean { 0.0 }, d1_std{ 0.0 }, d2_mean{ 0.0 }, d2_std{ 0.0 } {}
    std::pair<T, T> Process(T samp, T d1, T s1, T d2, T s2)
    {
        std::lock_guard<std::mutex> lock(mtx);
        sample = samp;
        d1_mean = d1;
        d1_std = s1;
        d2_mean = d2;
        d2_std = s2;
        auto z1 = (sample - d1_mean) / d1_std;
        auto z2 = (sample - d2_mean) / d2_std;
        std::pair<T, T> zscores;
        {
            return std::make_pair(z1, z2);
        }
        return zscores;
    }
};

template<typename T>
class DataClassifier 
{
private:
    std::pair<T, T> zscores;
public:
    DataClassifier() : zscores{ 0.0, 0.0 } {}
    std::string Classify(std::pair<T, T> zs)
    {
        std::lock_guard<std::mutex> lock(mtx);
        zscores = zs;
        if (abs(zscores.first) < abs(zscores.second))
        {
            return std::string("Distribution 1\n");
        }
        else
        {
            return std::string("Distribution 2\n");
        }
    }
};

int main()
{
    // Change the number of samples here:
    int dist_samples = 10;
    std::vector<std::jthread> threads(dist_samples);

    // Select the data type(s) here:
    DataGenerator<float> DG{};
    DataProcessor<float> DP{};
    DataClassifier<float> DC{};

    for (int i = 0; i < dist_samples; i++) 
    {
        threads[i] = std::jthread([&]
            {
                // Change the start and end points of the data distribution here:
                auto sample = DG.Generate(1, 100, dist_samples);
                // Change the mean and standard deviation of the first and second distribution, respectively, here:
                auto zscores = DP.Process(sample, 30, 4, 90, 12);
                std::cout << DC.Classify(zscores);
            });
    }
    for (int i = 0; i < dist_samples; i++) {
        threads[i].join();
    }
}
