#include "tools.h"
#include <map>
#include <string>
using namespace std;

void mc_MapStringIndex::Init()
{
    Destroy();
    mapObject=new std::map<string, int>;
}

void mc_MapStringIndex::Destroy()
{
    if(mapObject)
    {
        delete (std::map<string, int>*)mapObject;
    }
}

void mc_MapStringIndex::Clear()
{
    Init();
}

void mc_MapStringIndex::Add(const char* key, int value)
{
    ((std::map<string, int>*)mapObject)->insert(std::pair<string, int>(string(key), value));
}

void mc_MapStringIndex::Add(const unsigned char* key, int size, int value)
{
    ((std::map<string, int>*)mapObject)->insert(std::pair<string, int>(string(key,key+size), value));
}

void mc_MapStringIndex::Remove(const char* key,int size)
{
    ((std::map<string, int>*)mapObject)->erase(string(key,key+size));
}

int mc_MapStringIndex::Get(const char* key)
{
    std::map<string, int>::iterator it;
    int value=-1;
    it=((std::map<string, int>*)mapObject)->find(string(key));
    if (it != ((std::map<string, int>*)mapObject)->end())
    {
        value=it->second;
    }
    return value;
}

int mc_MapStringIndex::Get(const unsigned char* key,int size)
{
    std::map<string, int>::iterator it;
    int value=-1;
    it=((std::map<string, int>*)mapObject)->find(string(key,key+size));
    if (it != ((std::map<string, int>*)mapObject)->end())
    {
        value=it->second;
    }
    return value;
}
