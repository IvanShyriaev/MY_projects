#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<utility>

#ifdef _WIN32
    #include<windows.h>
    #include<shlobj.h>
    typedef HANDLE ProcessID;
    typedef HANDLE ThreadID;

    #define PATH_SEPARATOR "\\"
    std::string rootDirectory = "D:\\";
#elif __linux__
    #include<sys/stat.h>
    #include<cstdlib>
    #include<unistd.h>
    #include<sys/wait.h>
    #include<csignal>
    #include<sys/types.h>
    #include<dirent.h>
    #include<cstring>

    typedef pid_t ProcessID;
    typedef pid_t ThreadID;

    #define PATH_SEPARATOR "/"
    std::string rootDirectory = "/";
#endif


void KillProcess(ProcessID pid) 
{
#ifdef _WIN32
    if (!pid) 
    {
        std::cout << "Process is not running" << std::endl;
        return;
    }

    if (TerminateProcess(pid, 1)) 
    {
        std::cout << "Process killed successfully" << std::endl;
    } 
    else 
    {
        std::cout << "Cannot kill process: " << GetLastError() << std::endl;
    }
#else
    if (pid <= 0) 
    {
        std::cout << "Process is not running" << std::endl;
        return;
    }

    if (kill(pid, SIGKILL) == 0) 
    {
        std::cout << "Process killed successfully" << std::endl;
    } 
    else
    {
        std::cerr << "Cannot kill process: " << errno << std::endl;
    }
#endif
}

void SuspendProcess(ProcessID pid, ThreadID tid) 
{
#ifdef _WIN32
    if (!pid) 
    {
        std::cout << "Process is not running" << std::endl;
        return;
    }

    if (SuspendThread(tid) == -1) 
    {
        std::cout << "Failed to suspend process: " << GetLastError() << std::endl;
    } 
    else 
    {
        std::cout << "Process suspended" << std::endl;
    }
#else
    if (pid <= 0) 
    {
        std::cout << "Process is not running" << std::endl;
        return;
    }

    if (kill(pid, SIGSTOP) == 0) 
    {
        std::cout << "Process suspended" << std::endl;
    } 
    else 
    {
        std::cerr << "Failed to suspend process: " << errno << std::endl;
    }
#endif
}

void ResumeProcess(ThreadID tid, ProcessID pid) 
{
#ifdef _WIN32
    if (ResumeThread(tid) == -1) 
    {
        std::cerr << "Failed to resume process: " << GetLastError() << std::endl;
    } 
    else 
    {
        std::cout << "Process resumed" << std::endl;
    }
#else
    if (pid <= 0) 
    {
        std::cout << "Process is not running" << std::endl;
        return;
    }

    if (kill(pid, SIGCONT) == 0) 
    {
        std::cout << "Process resumed" << std::endl;
    } 
    else 
    {
        std::cerr << "Failed to resume process: " << errno << std::endl;
    }
#endif
}

void ProcessController(ProcessID pid, ThreadID tid) 
{
    int input;
    std::cout << "Enter command:\n 1 - kill process\n 2 - suspend process\n 3 - resume process\n 4 - pass \n" << std::endl;
    std::cin >> input;

    while(true) 
    {
        switch (input) 
        {
        case 0:
            std::cout << "Enter command:\n 1 - kill process\n 3 - resume process\n" << std::endl;
            std::cin >> input;
            break;

        case 1:
            KillProcess(pid);
            return;

        case 2:
            SuspendProcess(pid, tid);
            input = 0;
            break;

        case 3:
            ResumeProcess(tid, pid);
            return;

        case 4:
            return;

        default:
            std::cout << "Wrong command" << std::endl;
            break;
        }
    }
}


void ListFilesAndDirectories(const std::string& currentPath,
                             std::vector<std::string>& directories,
                             std::vector<std::string>& files) 
{
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    std::string searchPath = currentPath + PATH_SEPARATOR + "*"; 
    hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        std::cerr << "Failed to list directory. Error: " << GetLastError() << std::endl;
        return;
    }

    do 
    {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
            {
                directories.push_back(findFileData.cFileName);
            }
        } 
        else 
        {
            files.push_back(findFileData.cFileName);
        }
    } 
    while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

#elif __linux__
    DIR* dir;
    struct dirent* entry;

    dir = opendir(currentPath.c_str());
    if (!dir) 
    {
        std::cerr << "Failed to list directory: " << strerror(errno) << std::endl;
        return;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_DIR) 
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                directories.push_back(entry->d_name);
            }
        } 
        else 
        {
            files.push_back(entry->d_name);
        }
    }

    closedir(dir);
#endif
}

void DisplayDirectoryContents(const std::vector<std::string>& directories,
                              const std::vector<std::string>& files) 
{
    std::cout << "Directories:" << std::endl;
    for (size_t i = 0; i < directories.size(); ++i) 
    {
        std::cout << "  [" << i + 1 << "] " << directories[i] << std::endl;
    }

    std::cout << "Files:" << std::endl;
    for (size_t i = 0; i < files.size(); ++i) 
    {
        std::cout << "  " << files[i] << std::endl;
    }

    std::cout << std::endl;
}

std::string GetUserInput() 
{
    std::string input;
    std::cout << "Enter directory number to navigate or '..' to go back or end to end the program: " << std::endl;
    std::cout << "Type 'zip [directory number]' to select the directory to super-zip" << std::endl;
    std::getline(std::cin, input);
    return input;
}

std::string BrowseForFolder() 
{
    std::string currentPath = rootDirectory;
    
    while (true) {
        std::vector<std::string> directories;
        std::vector<std::string> files;

        ListFilesAndDirectories(currentPath, directories, files);

        std::cout << "Current Directory: " << currentPath << std::endl;
        DisplayDirectoryContents(directories, files);

        std::string input = GetUserInput();
        
        try
        {
            if (input == "..") 
            {
                size_t pos = currentPath.find_last_of(PATH_SEPARATOR);
                if (pos != std::string::npos) 
                {
                    currentPath = currentPath.substr(0, pos); 
                    if (currentPath.empty()) currentPath = rootDirectory;
                }
            }
            else if(input.find("zip") != std::string::npos)
            {
                int dirIndex = std::stoi(input.substr(3)) - 1;
                if (dirIndex >= 0 && dirIndex < directories.size()) 
                {
                    currentPath += PATH_SEPARATOR + directories[dirIndex] + PATH_SEPARATOR;
                }
                std::cout<<"You selected the "<< currentPath<< std::endl;
                return currentPath;
            }
            else if(input == "end")
            {
                std::cout<< " -- END -- "<<std::endl;
                return "";
            }
            else 
            {
                int dirIndex = std::stoi(input) - 1; 
                if (dirIndex >= 0 && dirIndex < directories.size()) 
                {
                    currentPath += PATH_SEPARATOR + directories[dirIndex];
                }
            }
        }
        catch(const std::invalid_argument& e)
        {
            std::cout<< "Invalid argument, try again"<<std::endl; 
        }
    }
}

bool Create_Process(const char* path, std::string& commandLine)
{
    #ifdef _WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        if (!CreateProcess(path, 
                        const_cast<char*>(commandLine.c_str()),  
                        NULL,        
                        NULL,        
                        FALSE,       
                        0,           
                        NULL,        
                        NULL,        
                        &si,         
                        &pi))        
        {
            std::cerr << "Failed to create archive. Error: " << GetLastError() << std::endl;
            return  false;
        }
        
        ProcessController(pi.hProcess, pi.hThread);

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;

    #elif __linux__
            pid_t pid = fork(); 

        if (pid < 0) 
        { 
            std::cerr << "Failed to fork process." << std::endl;
            return false;
        } 
        else if (pid == 0) 
        { 
            char* args[] = {const_cast<char*>(path), const_cast<char*>(commandLine.c_str()), NULL};
            
            execv(path, args); 

            std::cerr << "Failed to execute process." << std::endl;
            exit(1); 
        }
        else
        {
            int status;
            waitpid(pid, &status, 0); 

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            {
                return true; 
            }
            else 
            {
                std::cerr << "Child process terminated with an error." << std::endl;
                return false;
            }
        }
    #endif
}
// NO IDEA || RE_DO
bool ArchiveFiles(const std::vector<std::string>& filePaths, const std::string& archivePath) 
{
#ifdef _WIN32
    const char* sevenZipPath = "C:\\Program Files\\7-Zip\\7z.exe";

    std::string commandLine = "\"" + std::string(sevenZipPath) + "\" a \"" + archivePath + "\"";
    
    for (const auto& filePath : filePaths) 
    {
        commandLine += " \"" + filePath + "\"";
    }

    if (!Create_Process(sevenZipPath, commandLine)) 
    {
        std::cerr << "Failed to create archive: " << archivePath << std::endl;
        return false; 
    }
    std::cout << "Archive created: " << archivePath << std::endl;

#else
    const char* sevenZipPath = "zip"; 

    std::string commandLine = zipPath;
    commandLine += " \"" + archivePath + "\"";

    for (const auto& filePath : filePaths) 
    {
        commandLine += " \"" + filePath + "\""; 
    }

    if (!Create_Process(sevenZipPath, commandLine)) 
    {
        std::cerr << "Failed to create archive: " << archivePath << std::endl;
        return false;
    }

    std::cout << "Archive created: " << archivePath << std::endl;
#endif

    return true; 
}


std::string GetFileExtension(const std::string& filePath)
{
    size_t dotPosition = filePath.find_last_of('.');
    if (dotPosition != std::string::npos) {
        return filePath.substr(dotPosition + 1);
    }
    return "";  
}




void AgregateFiles(const std::string& directoryPath) 
{
    std::map<std::string, std::vector<std::string>> files;

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        std::cerr << "Failed to open directory: " << directoryPath << std::endl;
        return;
    }

    do 
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
        {
            std::string fileName = findFileData.cFileName;

            std::string fileExtension = GetFileExtension(fileName);
            std::string filePath = directoryPath + PATH_SEPARATOR + fileName;

            if (files.find(fileExtension) == files.end()) 
            {
                files.insert({fileExtension, {filePath}});
            } 
            else 
            {
                files[fileExtension].push_back(filePath);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

#else
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) 
    {
        std::cerr << "Failed to open directory: " << directoryPath << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) 
    {
        if (entry->d_type == DT_REG) 
        {
            std::string fileName = entry->d_name;
            std::string fileExtension = GetFileExtension(fileName);
            std::string filePath = directoryPath + PATH_SEPARATOR + fileName;

            if (files.find(fileExtension) == files.end()) 
            {
                files.insert({fileExtension, {filePath}});
            } 
            else 
            {
                files[fileExtension].push_back(filePath);
            }
        }
    }

    closedir(dir);
#endif

    for (const auto& pair : files) 
    {
        std::string archivePath = directoryPath + PATH_SEPARATOR + pair.first + ".zip"; // Change to ".7z" if using 7z on Linux
        if (!ArchiveFiles(pair.second, archivePath)) 
        {
            return;
        }
    }
}

int main()
{
    std::string directoryPath = BrowseForFolder();

    if(directoryPath.empty())
    {
        std::cerr<<"No directory selected"<< std::endl;
        return 1;
    }

    AgregateFiles(directoryPath);

    std::cout << "Archiving complete." << std::endl;
    
    return 0;
}