#include <iostream>
#include "resources.h"
#include "dynres.h"
#include <dlfcn.h>
#include <string>
#include <vector>

void printString(int id, void *resdll)
{
    uint32_t buffer_size = GetStringSize(resdll, id) + 3;
    char *buffer = new char[buffer_size];
    LoadString(resdll, id, buffer, buffer_size);
    std::cout << buffer << '\n';
    delete[] buffer;
}

int main(int count, const char **args)
{
    const char *default_res = "./libCRes.so";
    const char *res = default_res;
    if(count > 1)
        res = args[1];

    std::cout << "loading strings from sharable object '" << res << "'\n";
    void *h = dlopen(res, RTLD_NOW);
    if(h != nullptr) {
        printString(1, h); // sentinel value in the string resource generated file
        printString(IDS_VERSION, h);
        printString(IDS_APPNAME, h);
        printString(IDS_CHAPTER_ONE, h);
        dlclose(h);
    }
    return 0;
}
