#include<vector>
#include<iostream>
#include<string>
#include<fstream>
#include<chrono>
#include<cstdlib>
#include<utility>
#include<sstream>
#include<iomanip>

#ifdef WIN32
    #include<windows.h>

#elif __linux__
    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>
    #include <semaphore.h>

    #define THREAD_PRIORITY_ABOVE_NORMAL 10
    #define THREAD_PRIORITY_BELOW_NORMAL 0
    #define THREAD_PRIORITY_NORMAL 5

#endif


// [Складність 3] Можливість обмеження кількості потоків, які одночасно працюють із
// даними (щоб, наприклад, із 100 запущених потоків одночасно виконували обчислення
// лише 8, а інші чекали).
// Проаналізуйте вплив обмеження на швидкодію виконання обчислень.

double wordCount = 0;
std::vector<std::chrono::duration<double>> threads_time;
std::vector<std::string> longestSentence;
int maxSize = 0;

//просто номер першого слова найдовшого речення
long coordOfLongestSent;

#ifdef WIN32
    LONG totalProgress = 0;
    HANDLE sentenceMutex;
    HANDLE hSemaphore;
#elif __linux__
    long totalProgress = 0;
    pthread_mutex_t progressMutex;
    pthread_mutex_t sentenceMutex;
    sem_t semaphore;

#endif

void ProgressIncrement()
{
    #ifdef WIN32
        InterlockedIncrement(&totalProgress);

    #elif __linux__
        pthread_mutex_lock(&progressMutex);
        totalProgress++;
        pthread_mutex_unlock(&progressMutex);

    #endif

    return;
}

void SynchLongestSentenceWrite(std::vector<std::string>& currentSentence, long currentCoord)
{
    #ifdef WIN32
        WaitForSingleObject(sentenceMutex, INFINITE);
       
        coordOfLongestSent = currentCoord;
        maxSize = currentSentence.size();
        longestSentence = currentSentence;
           
        if (! ReleaseMutex(sentenceMutex)) 
        { 
            std::cerr << "Cant Release mutex!" << std::endl;
            std::exit(0);
        } 

    #elif __linux__
        pthread_mutex_lock(&sentenceMutex);
        coordOfLongestSent = currentCoord;
        maxSize = currentSentence.size();
        longestSentence = currentSentence;
        pthread_mutex_unlock(&sentenceMutex);

    #endif

    return;
}

void WaitSemaphore()
{
    #ifdef WIN32
        WaitForSingleObject(hSemaphore, INFINITE);

    #elif __linux__
        sem_wait(&seamaphore);

    #endif

    return;
}

void ReleaseSemaphore()
{
     #ifdef WIN32
        ReleaseSemaphore(hSemaphore, 1, NULL);

    #elif __linux__
        sem_post(&seamaphore);

    #endif

    return;
}

void FindLongestSentence(std::vector<std::string>& text, int threadIndex)
{
    std::vector<std::string> currentSentence;
    std::string endOfSentenceChars = ".!?";
    char coordChar = '&';
    long currentCoord;

    WaitSemaphore();
    
    for(auto &&word : text)
    {
        ProgressIncrement();
        if(word.find(coordChar) != std::string::npos)
        {
            word.pop_back();
            currentCoord = std::stol(word);
            word.clear();

        }

        if (word.find_first_of(endOfSentenceChars) == std::string::npos) 
        {
            currentSentence.push_back(word);
        } 
        else 
        {
            currentSentence.push_back(word);
            if (currentSentence.size() - 1 > maxSize)
            {
               SynchLongestSentenceWrite(currentSentence, currentCoord);
            }
            currentSentence.clear();
        }


        //Sleep(1);
    }

    ReleaseSemaphore();

    return;
}

std::vector<std::vector<std::string>> SplitText(const std::string& filename, int parts)
{

    std::ifstream inputFile(filename);
    
    if (!inputFile.is_open())
    {
        std::cerr << "Error opening the file!" << std::endl;
        exit(1);
    }

    std::vector<std::vector<std::string>> sentences;
    std::vector<std::string> words;
    std::string line;
    

    while (std::getline(inputFile, line, '.'))
    {   
        line +='.';
        std::string word;
        std::stringstream ss(line);

        while(std::getline(ss, word, ' '))
        {
            if(word != "")
            {
                words.push_back(word);
                wordCount++;
            }
        }
        words.push_back( std::to_string(wordCount - words.size()) + "&"); 

        sentences.push_back(words);
        words.clear();
    }
    inputFile.close();

    int sentencesNum = sentences.size();
    int partSize = sentencesNum/parts;
   
    std::vector<std::vector<std::string>> textParts(parts);

    int baseSize = sentencesNum / parts;
    int remainder = sentencesNum % parts;

    int startIndex = 0;

    
    for (int i = 0; i < parts; ++i)
    {
        int currentPartSize = baseSize + (i < remainder ? 1 : 0); 

        for (int j = startIndex; j < startIndex + currentPartSize; ++j)
        {
            textParts[i].insert(textParts[i].end(), sentences[j].begin(), sentences[j].end());
        }

        startIndex += currentPartSize;
    }

    return textParts;
}


// std::pair<int, int> PriorityController(int numthreads)
// {
//     if(numthreads > 1)
//     {
//         int number;
//         int command;

//         std::cout << "Enter the number of thread to change its priority:\n\n";
//         std::cin >> number;
//         std::cout << "[1] - increase priority\n [2] - decrease priority\n[0] - exit\n\n";
//         std::cin >> command;
        
//         while(command != 0)
//         {
//             switch (command)
//             {
//             case 1:
//                 std::cout << "Thread priority increased";
//                 return std::make_pair(number - 1, THREAD_PRIORITY_ABOVE_NORMAL);

//             case 2:
//                 std::cout << "Thread priority decreased";
//                 return std::make_pair(number - 1, THREAD_PRIORITY_BELOW_NORMAL);

//             default:
//                 std::cout << "Wrong comand, try again\n";
//                 break;
//             }
//         }
//     }

//     return std::make_pair(0, THREAD_PRIORITY_NORMAL);
// }

#ifdef _WIN32
    DWORD WINAPI ThreadFunc(LPVOID lpParam)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto* params = static_cast<std::pair<std::vector<std::string>*, int>*>(lpParam);
        FindLongestSentence(*params->first, params->second);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> executionTime = end - start;
        threads_time[params->second] = executionTime;

        delete params;

        return 0;
    }

#elif __linux__
    void* ThreadFunc(void* arg)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::string>* textPart = static_cast<std::vector<std::string>*>(arg); 
        FindLongestSentence(*textPart);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> executionTime = end - start;
        threads_time.push_back(executionTime);
    }

#endif

#ifdef WIN32
    DWORD WINAPI ProgressUpdateThread(LPVOID) {
        
        while (true) 
        {
            double currentProgress = static_cast<double>(totalProgress);
            double averageProgress = (currentProgress / wordCount) * 100;

            WaitForSingleObject(sentenceMutex, INFINITE);
       
            
            std::cout << std::fixed << std::setprecision(10) << "\rProgress: " << averageProgress << "% completed | "
                  << "Longest sentence: " << maxSize << " words | "
                  << "Coordinate of longest sentence: " << coordOfLongestSent << "  "; 
        
            std::cout.flush(); 

            if (! ReleaseMutex(sentenceMutex)) 
            { 
                std::cerr << "Cant Release mutex!" << std::endl;
                std::exit(0);
            } 
           

            if (averageProgress >= 100) { break; }
            Sleep(10); 
        }
        return 0;
    }

#elif __linux__
    void*  ProgressUpdateThread(void* arg)
    {
        while (true) 
        {
            double currentProgress = static_cast<double>(totalProgress);
            double averageProgress = (currentProgress / wordCount) * 100;

            pthread_mutex_lock(&sentenceMutex);
      
            std::cout << "\rProgress: " << averageProgress << "% completed | "
                  << "Longest sentence: " << maxSize << " words | "
                  << "Coordinate of longest sentence: " << coordOfLongestSent << "  "; 
        
            std::cout.flush(); 

            pthread_mutex_unlock(&sentenceMutex);

            if (averageProgress >= 99) { break; }
            Sleep(10); 
        }
        
    }
#endif

void RestrictThreads(int numThreads)
{
    int maxThreads;
    std::cout << "Enter the max number of threads that can do calculations" << std::endl;
    std::cin >> maxThreads;

    if(maxThreads <= 0)
    {
        std::cout << "Wrong thread amount";
        return;
    }

    #ifdef WIN32
        hSemaphore = CreateSemaphore(NULL, maxThreads, maxThreads, NULL);

    #elif __linux__
        sem_init(&semaphore, 0, maxThreads);
    
    #endif

    return;
}

std::chrono::duration<double> CreateMultipleThreads(std::vector<std::vector<std::string>>& text, int numThreads)
{
    //std::pair<int, int> thread_prioty = PriorityController(numThreads);
    auto start = std::chrono::high_resolution_clock::now();
    RestrictThreads(numThreads);


    #ifdef _WIN32
        sentenceMutex = CreateMutex(NULL, FALSE, NULL);
       

        HANDLE hThreads[numThreads];
        DWORD threadIds[numThreads];

        for(int i = 0; i < numThreads; i++)
        {
            auto* params = new std::pair<std::vector<std::string>*, int>(&text[i], i); 
            hThreads[i] = CreateThread
            (
                NULL,        
                0,           
                ThreadFunc,  
                params,   
                0,           
                &threadIds[i]    
            );

            if (hThreads[i] == NULL) 
            {
                std::cerr << "Cant create thread " << i+1 << std::endl;
                exit(1);
            }

            // if(i == thread_prioty.first)
            // {
            //     SetThreadPriority(hThreads[thread_prioty.first], thread_prioty.second);
            // }
        }

        HANDLE hProgressThread = CreateThread
        (
            NULL,
            0, 
            ProgressUpdateThread,
            NULL, 
            0,
            NULL
        );
        
        WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

        WaitForSingleObject(hProgressThread, INFINITE);

        for(int i = 0; i < numThreads; i++)
        {
            CloseHandle(hThreads[i]);
        }

        CloseHandle(hProgressThread);
        CloseHandle(sentenceMutex);
        CloseHandle(hSemaphore);

    #elif __linux__
        pthread_mutex_init(&progressMutex, NULL);
        pthread_mutex_init(&sentenceMutex, NULL);
        sem_init(&semaphore, 0, 8);

        std::vector<pthread_t> threads(numThreads);  
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        for(int i = 0; i < numThreads; i++)
        {
            int result = pthread_create
            (
                &threads[i],
                &attr,
                ThreadFunc,
                &text[i]
            )

            if (result != 0) 
            {
                std::cerr << "Cant create thread " << i + 1 << std::endl;
            }

            // if (i == thread_priority.first) 
            // {
            //     struct sched_param param;
            //     param.sched_priority = (thread_priority.second == THREAD_PRIORITY_ABOVE_NORMAL) ? 10 : 0;
            //     pthread_setschedparam(threads[i], SCHED_OTHER, &param);
            // }
        }

        pthread_t progressThread;
        if(!pthread_create(&progressThread, &ProgressUpdateThread, NULL, NULL))
        {
            std::cerr << "Cant create thread for tracking progress" << std::endl;
        }
        pthread_join(progressThread, NULL);

        for (int i = 0; i < numThreads; i++) 
        {
            pthread_join(threads[i], nullptr);
        }

        pthread_mutex_destroy(&progressMutex);
        pthread_mutex_destroy(&sentenceMutex);
        sem_destroy(&semaphore);
        pthread_attr_destroy(&attr);

    #endif

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> executionTime = end - start;

    std::cout << "\n__________________________________________________\n" 
              << "Completed in " << executionTime.count() << " seconds" 
              << "\n__________________________________________________\n";

    return executionTime; 
}

    

int main()
{
    int numThreads = 16;
    threads_time.resize(numThreads);

    std::string fileName = "random_text copy.txt";

    std::vector<std::vector<std::string>> text = SplitText(fileName, numThreads);
    auto time = CreateMultipleThreads(text, numThreads);

    std::string outputName = fileName.substr(0, fileName.find('.')) + "_output.txt";

    std::ofstream outFile(outputName);
    if (!outFile) 
    {
        std::cerr << "File could not be opened!" << std::endl;
        return 0;
    }

    outFile << "Longest sentence: " << maxSize << " words    " << "Completed in: " << time.count() << " seconds\n\n" ;
    std::cout << "Longest sentence: " << maxSize << " words\n";
    for (auto &&word : longestSentence)
    {
        outFile << word << " ";
        std::cout << word << " "; 
    }
    outFile.close();

    std::cout << "\n Thread times:" << std::endl;
    for(int i = 0; i < threads_time.size(); i++)
    {
        auto time = threads_time[i];
        std::cout << i + 1 << " : " << time.count() << "\n";
    }
    
    return 0;
}


