# ECS_C - Entity Component System in C

A lightweight, macro-based Entity Component System (ECS) implementation written in C. This is a data-oriented architecture library designed for efficient entity and component management using compilation-time code generation.

## Overview

ECS_C provides a flexible framework for building applications using the Entity Component System pattern. Instead of traditional inheritance hierarchies, ECS_C uses composition, where entities are collections of components.

**Key Features:**
- **Macro-based API** - Clean, expressive syntax with compile-time code generation
- **Component Signatures** - Efficient bitmask-based component tracking
- **Entity Pooling** - Reusable entity IDs with generation counters
- **No Memory Allocation** - Stack-based approach with fixed entity limits
- **High Performance** - Cache-friendly data layout and minimal overhead

## Building

### Prerequisites
- C99 or later compiler (GCC, Clang, or MSVC)
- CMake 3.15 or higher

### Build Instructions

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Quick Start

### 1. Create a Component

For a component named `ExampleComponent`, create these files:

**`Components/ExampleComponent/example_component.h`**
```c
#ifndef EXAMPLE_COMPONENT_H
#define EXAMPLE_COMPONENT_H
#include "ecs_core.h"

extern SomeDataType ExampleComponentData[MAX_ENTITIES];

void AddExample(const struct Entity *entity, /* parameters */);
bool HasExample(const struct Entity *entity);
SomeDataType GetExample(const struct Entity *entity);
void RemoveExample(const struct Entity *entity);

#endif
```

**`Components/ExampleComponent/example_component.c`**
```c
#include "example_component.h"

SomeDataType ExampleComponentData[MAX_ENTITIES];

void AddExample(const struct Entity *entity, /* parameters */) {
    ExampleComponentData[entity->id] = /* your data */;
    EntitySignatures[entity->id] |= COMPONENT_EXAMPLE;
}

bool HasExample(const struct Entity *entity) {
    return EntitySignatures[entity->id] & COMPONENT_EXAMPLE;
}

SomeDataType GetExample(const struct Entity *entity) {
    return ExampleComponentData[entity->id];
}

void RemoveExample(const struct Entity *entity) {
    EntitySignatures[entity->id] &= ~COMPONENT_EXAMPLE;
}
```

Add to `ecs_core.h`:
```c
typedef enum {
    COMPONENT_NONE = 0,
    COMPONENT_TEST = 1 << 0,
    COMPONENT_EXAMPLE = 1 << 1,  // Add your component
} ComponentType;
```

### 2. Define an Entity Type

**`Entities/ExampleEntity/example_entity.h`**
```c
#ifndef EXAMPLE_ENTITY_H
#define EXAMPLE_ENTITY_H
#include "ecs_core.h"

struct Entity InstantiateExampleEntity(/* parameters */);

#endif
```

**`Entities/ExampleEntity/example_entity.c`**
```c
#include "example_entity.h"
#include "example_component.h"

struct Entity InstantiateExampleEntity(/* parameters */) {
    struct Entity entity;
    InitializeEntity(&entity);
    AddComponent(&entity, Example, /* parameters */);
    return entity;
}
```

### 3. Update CMakeLists.txt

Add your new source files:
```cmake
add_executable(CEngine
    main.c
    ecs_core.c
    Components/ExampleComponent/example_component.c
    Entities/ExampleEntity/example_entity.c
)

target_include_directories(CEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/ExampleComponent
    ${CMAKE_CURRENT_SOURCE_DIR}/Entities/ExampleEntity
)
```

### 4. Use the Macros

```c
#include "ecs_core.h"
#include "example_entity.h"
#include "example_component.h"

int main(void) {
    InitializeStack(&AvailableIDs);
    
    struct Entity entity = InstantiateEntity(Example, /* args */);
    
    AddComponent(&entity, Example, /* args */);
    
    if (HasComponent(&entity, Example)) {
        SomeDataType data = GetComponent(&entity, Example);
    }
    
    RemoveComponent(&entity, Example);
    DestroyEntity(&entity);
    
    return 0;
}
```

## Macro Reference

```c
// Entity instantiation
InstantiateEntity(ComponentName, ...args)
// Calls: InstantiateComponentNameEntity(...args)

// Component operations
AddComponent(entity, ComponentName, ...args)         // Calls: AddComponentName(entity, ...args)
HasComponent(entity, ComponentName)                 // Calls: HasComponentName(entity)
GetComponent(entity, ComponentName)                 // Calls: GetComponentName(entity)
RemoveComponent(entity, ComponentName)              // Calls: RemoveComponentName(entity)
```

## Function Naming Convention

For macros to work correctly, follow these naming patterns:

| Macro | Required Function | Pattern |
|-------|------------------|---------|
| `InstantiateEntity(Xxx, ...)` | `struct Entity Instantiate**Xxx**Entity(...)` | PascalCase component name |
| `AddComponent(e, Xxx, ...)` | `void Add**Xxx**(const struct Entity *e, ...)` | PascalCase component name |
| `HasComponent(e, Xxx)` | `bool Has**Xxx**(const struct Entity *e)` | PascalCase component name |
| `GetComponent(e, Xxx)` | `ReturnType Get**Xxx**(const struct Entity *e)` | PascalCase component name |
| `RemoveComponent(e, Xxx)` | `void Remove**Xxx**(const struct Entity *e)` | PascalCase component name |

**Example:** For `AddComponent(&e, Example, x, y)` → must define `void AddExample(const struct Entity *e, ...)`

## Architecture

- **Entity**: Unique ID + generation counter
- **Component**: Data stored in parallel arrays, indexed by entity ID
- **Signature**: 64-bit bitmask tracking which components an entity has
- **System**: Logic that processes entities with specific components

## Limits

- Maximum entities: `MAX_ENTITIES` (default 5000)
- Maximum component types: 64
- All data stored in stack-allocated arrays (no dynamic allocation)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Happy building!** 🚀
