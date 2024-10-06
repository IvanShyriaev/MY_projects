#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>


std::string GenerateRandomWord()
{
    int length = rand() % 8 + 3; 
    std::string word;
    
    for (int i = 0; i < length; ++i)
    {
        char randomChar = 'a' + rand() % 26;
        word += randomChar;
    }
    
    return word;
}


std::string GenerateRandomSentence()
{
    int wordCount = rand() % rand()%10000 + 5; 
    std::string sentence;
    
    for (int i = 0; i < wordCount; ++i)
    {
        sentence += GenerateRandomWord(); 
        
        if (i < wordCount - 1) 
        {
            sentence += " ";
        }
    }
    
    sentence += '.'; 
    return sentence;
}


void GenerateRandomText(int sentenceCount)
{
    std::string text;
    std::ofstream outFile("random_text.txt");

    for (int i = 0; i < sentenceCount; ++i)
    {
        text += GenerateRandomSentence(); 
        outFile << text << " ";
        text.clear();
    }

    outFile.close();

    return;
}

int main()
{
    srand(time(0));

    int sentenceCount;
    std::cout << "enter the number of sentences to generate";
    std::cin >> sentenceCount;

    GenerateRandomText(sentenceCount);

    return 0;
}