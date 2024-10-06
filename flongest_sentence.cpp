#include<vector>
#include<iostream>
#include<string>
#include<fstream>
#include<chrono>
#include<cstdlib>
#include<utility>
#include<sstream>

#ifdef _WIN32
    #include<windows.h>

#elif __linux__
    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>

#endif



std::vector<std::string> longestSentence;
int maxSize = 0;


void FindLongestSentence(const std::vector<std::string>& text)
{
    std::vector<std::string> currentSentence;
    std::string endOfSentenceChars = ".!?";
    
    for(auto &&word : text)
    {
        if(word.find_first_of(endOfSentenceChars) == std::string::npos)
        {
            currentSentence.push_back(word);
        }
        else if((word.size() == 1) && (word.find_first_of(endOfSentenceChars) == std::string::npos))
        {
            currentSentence[currentSentence.size()-1] += word;
            if(currentSentence.size() > maxSize)
            {
                maxSize = currentSentence.size();
                longestSentence = currentSentence;
            }

            currentSentence.clear();
        }
        else
        {
            currentSentence.push_back(word);
            if(currentSentence.size() > maxSize)
            {
                maxSize = currentSentence.size();
                longestSentence = currentSentence;
            }

            currentSentence.clear();
        } 
    }

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
            }
        } 

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


std::pair<int, int> PriorityController(int numthreads)
{
    if(numthreads > 1)
    {
        int number;
        int command;

        std::cout << "Enter the number of thread to change its priority:\n\n";
        std::cin >> number;
        std::cout << "[1] - increase priority\n [2] - decrease priority\n[0] - exit\n\n";
        std::cin >> command;
        
        while(command != 0)
        {
            switch (command)
            {
            case 1:
                std::cout << "Thread priority increased";
                return std::make_pair(number, THREAD_PRIORITY_ABOVE_NORMAL);

            case 2:
                std::cout << "Thread priority decreased";
                return std::make_pair(number, THREAD_PRIORITY_BELOW_NORMAL);

            default:
                std::cout << "Wrong comand, try again\n";
                break;
            }
        }
    }

    return std::make_pair(0, THREAD_PRIORITY_NORMAL);
}

#ifdef _WIN32
    DWORD WINAPI ThreadFunc(LPVOID lpParam)
    {
        std::vector<std::string>* textPart = static_cast<std::vector<std::string>*>(lpParam); 
        FindLongestSentence(*textPart);
        return 0;
    }

#elif __linux__
    void* ThreadFunc(void* arg)
    {
        std::vector<std::string>* textPart = static_cast<std::vector<std::string>*>(arg); 
        FindLongestSentence(*textPart);
    }

#endif


std::chrono::duration<double> CreateMultipleThreads(std::vector<std::vector<std::string>>& text, const int numThreads)
{
    std::pair<int, int> thread_prioty = PriorityController(numThreads);
    auto start = std::chrono::high_resolution_clock::now();

    #ifdef _WIN32
        HANDLE hThreads[numThreads];
        DWORD threadIds[numThreads];

        for(int i = 0; i < numThreads; i++)
        {
            hThreads[i] = CreateThread
            (
                NULL,        
                0,           
                ThreadFunc,  
                &text[i],   
                0,           
                &threadIds[i]    
            );

            if (hThreads[i] == NULL) 
            {
                std::cerr << "Cant create thread " << i+1 << std::endl;
                exit(1);
            }

            if(i == thread_prioty.first)
            {
                SetThreadPriority(hThreads[thread_prioty.first], thread_prioty.second);
            }
        }
        
        WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

        for(int i = 0; i < numThreads; i++)
        {
            CloseHandle(hThreads[i]);
        }

    #elif __linux__
        std::vector<pthread_t> threads(numThreads);  
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        for(int i = 0; i < numThreads; i++)
        {
            int result = pthread_create
            (
                threads[i],
                &attr,
                ThreadFunc,
                &text[i]
            )

            if (result != 0) 
            {
                std::cerr << "Cant create thread " << i + 1 << std::endl;
            }

            if (i == thread_priority.first) 
            {
                struct sched_param param;
                param.sched_priority = (thread_priority.second == THREAD_PRIORITY_ABOVE_NORMAL) ? 10 : 0;
                pthread_setschedparam(threads[i], SCHED_RR, &param);
            }
        }

        for (int i = 0; i < numThreads; i++) 
        {
            pthread_join(threads[i], nullptr);
        }

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
    int numThreads = 8;
    std::string fileName = "Book.txt";

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
    
    return 0;
}
