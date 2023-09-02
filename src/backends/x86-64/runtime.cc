#include <cassert>
#include <memory>
#include <unordered_map>

static uint64_t *stack_start = nullptr;
static std::unordered_map<uint64_t *, uint64_t> allocations;

extern "C"
{
    void *nl_runtime_alloc(uint64_t num_bytes)
    {
        void *p = malloc(num_bytes);
        // printf("Malloc'd %p\n", p);
        allocations[(uint64_t *)p] = num_bytes;
        return p;
    }

    void nl_register_stack_start(uint64_t *p)
    {
        stack_start = p;
    }

    void run_gc_over_memory(uint64_t *lower, uint64_t *higher)
    {
        // printf("lower: %p higher: %p ; %ld \n", lower, higher, higher-lower);
        for (uint64_t *p = lower; p < higher; p += 8)
        {
            auto it = allocations.find((uint64_t*)((void*)*p));
            if (it == allocations.end())
            {
                continue;
            }
            // printf("free %p\n", ((void*)*p));
            auto* base = it->first;
            auto size = it->second;
            allocations.erase(it);
            run_gc_over_memory(base, base + size);
            free(base);
        }
    }

    void nl_runtime_run_gc(uint64_t *stack_end)
    {
        // printf("ss: %p se: %p\n", stack_start, stack_end);
        assert(stack_end <= stack_start && "Stack grows downwards");
        run_gc_over_memory(stack_end, stack_start);
    }
}

