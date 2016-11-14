#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdio>
bool is_file_exist(char *fileName);

bool is_file_exist(char *fileName)
{
    if (FILE *file = fopen(fileName, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
} 

int main()
{
    if (is_file_exist("dsh")) {
      std::cout << "POSSIBLE" << '\n';
    }
    return 0;
}

