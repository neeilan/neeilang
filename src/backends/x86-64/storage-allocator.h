
#include <optional>
#include <string>
#include <vector>

constexpr uint8_t Register   = 1;
constexpr uint8_t FpRegister = 1 << 1;
constexpr uint8_t Literal    = 1 << 2;
constexpr uint8_t MemoryRef  = 1 << 3;
constexpr uint8_t Any        = Register | FpRegister | Literal | MemoryRef;

class StorageAllocator {
public:  
    struct Handle {
        std::string name;
        uint8_t type;
        std::optional<std::string> underlyingMemory; // If we store, anywhere other that 'push' it can go?
    };

    struct Context {};

    struct Spec {
        std::optional<std::string> name;
        std::vector<Handle> exclude;
        uint8_t type = Any;
    };

    Handle get(Spec s) {
        if (s.name) {
            // Is the requested storage available?
            // If not, how can we make it available?
        }
        // What's available 

    }
    



};