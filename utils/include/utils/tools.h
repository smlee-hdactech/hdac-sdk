#ifndef TOOLS_H
#define TOOLS_H

typedef struct mc_MapStringIndex
{
    void *mapObject;
    mc_MapStringIndex()
    {
        mapObject=nullptr;
        Init();
    }

    ~mc_MapStringIndex()
    {
        Destroy();
    }
    void Init();
    void Add(const char* key,int value);
    void Add(const unsigned char* key,int size,int value);
    void Remove(const char* key,int size);
    int Get(const char* key);
    int Get(const unsigned char* key,int size);
    void Destroy();
    void Clear();
} mc_MapStringIndex;

#endif // TOOLS_H
