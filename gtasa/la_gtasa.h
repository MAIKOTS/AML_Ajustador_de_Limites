#pragma once

#include <la.h>

struct CSAPool
{
    struct Flags
    {
        uint8_t id : 7;
        bool    empty : 1;
    };
    void*       objects;
    Flags*      flags;
    int32_t     count;
    int32_t     firstfree;
    bool        ownsAllocs;
    bool        locked;
    char        pad[2];
};
inline CSAPool* AllocatePool(size_t count, size_t size)
{
    CSAPool *p = new CSAPool;
    p->objects = new char[size * count];
    p->count = count;
    p->firstfree = -1;
    p->ownsAllocs = true;
    
    p->flags = new CSAPool::Flags[count];
    for (size_t i = 0; i < count; ++i)
    {
        p->flags[i].id = 0;
        p->flags[i].empty = true;
    }
    return p;
}

class GTASA : public ILAModule
{
public:
    virtual const char* GetGameName() { return "GTA: San Andreas"; }
    virtual const char* GetLibName() { return "libGTASA.so"; }
    virtual const char* GetConfigName() { return "AML_LimitAdjuster.SA"; }
    virtual eLoadedGame GetGameEnum() { return eLoadedGame::GTA_SA; }
};