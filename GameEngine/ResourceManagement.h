#pragma once
#include <cstdint>
#include <vector>
#include <queue>

struct ResourceHandle
{
    union
    {
        uint64_t Value;

        struct
        {
            uint64_t Index : 32; // Slot index in table
            uint64_t Generation : 16; // Increments on reuse
            uint64_t AccessCount : 8;
            uint64_t IsValid : 1;
            uint64_t IsDeleted : 1;
            uint64_t IsStale : 1;
            uint64_t Unused : 5;
        };
    };

    ResourceHandle() : Value(0) {}
};

class BaseResource
{
private:
    static inline std::vector<void*> ResourceTable;
    static inline std::vector<uint16_t> Generations;
    static inline std::queue<uint32_t> FreeList;

    template<typename T>
    static ResourceHandle RegisterResource(T* resource)
    {
        uint32_t index;
        uint16_t gen;

        if (!FreeList.empty())
        {
            // Reuse an old slot
            index = FreeList.front();
            FreeList.pop();
            gen = ++Generations[index];  // increment generation to invalidate old handles
            ResourceTable[index] = resource;
        }
        else
        {
            // Create new slot
            index = static_cast<uint32_t>(ResourceTable.size());
            gen = 1;
            ResourceTable.push_back(resource);
            Generations.push_back(gen);
        }

        ResourceHandle handle{};
        handle.Index = index;
        handle.Generation = gen;
        handle.IsValid = 1;
        return handle;
    }

    static void UnregisterResource(ResourceHandle handle)
    {
        if (handle.Index < ResourceTable.size())
        {
            ResourceTable[handle.Index] = nullptr;
            FreeList.push(handle.Index);
        }
    }
public:

    virtual void* GetResource(ResourceHandle handle)
    {
        if (handle.Index >= ResourceTable.size()) return nullptr;
        if (Generations[handle.Index] != handle.Generation) return nullptr;
        return ResourceTable[handle.Index];
    }
};