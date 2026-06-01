# ECS_C - Entity Component System in C

A lightweight, macro-based Entity Component System (ECS) implementation written in C. This is a data-oriented architecture library designed for efficient entity and component management using compile-time code generation.

## Overview

ECS_C provides a flexible framework for building applications using the Entity Component System pattern. Instead of traditional inheritance hierarchies, ECS_C uses composition, where entities are collections of components and tags.

**Key Features:**
- **Macro-based API** - Clean, expressive syntax with compile-time code generation
- **Component Signatures** - Efficient bitmask-based component tracking (256-bit support)
- **Entity Pooling** - Reusable entity IDs with generation counters
- **No Memory Allocation** - Stack-based approach with fixed entity limits
- **High Performance** - Cache-friendly data layout and minimal overhead
- **Tag System** - Zero-cost tag components for efficient filtering
- **Compact Iteration** - ForEachEntityWith macro for filtered entity iteration
- **Definition Files** - Declarative component/tag definitions in `.def` files

## Installation

### As a Module (For Use in Your Project)

If you only need the ECS library, copy just the `ECS` folder to your project:

```bash
# Copy the ECS module to your project
cp -r ECS /path/to/your/project/
```

Then in your `CMakeLists.txt`:

```cmake
# Include the ECS module
add_subdirectory(ECS)

# Link ECS to your executable
add_executable(your_app main.c)
target_link_libraries(your_app PRIVATE ECS)
target_include_directories(your_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ECS/include)
```

And in your C files:

```c
#include "ecs_core.h"
#include "components_tags.h"  // For using defined components/tags
```

### As a Complete Project (For Development/Examples)

To use the entire repository with example components and systems:

```bash
# Clone or download the entire repository
git clone https://github.com/RedCore0/ECS_C.git
cd ECS_C

# Build the example project
mkdir build && cd build
cmake ..
cmake --build .
```

## Prerequisites

- **C99** or later compiler (GCC, Clang, or MSVC)
- **CMake 3.15** or higher

## Quick Start

### 1. Define Components and Tags in `components_tags.def`

Create a definition file to declare your components and tags using the declarative syntax:

**`ECS/include/components_tags.def`**
```c
// Components with their data structure
COMPONENT(Position, {
    float x;
    float y;
})

COMPONENT(Velocity, {
    float vx;
    float vy;
})

COMPONENT(Health, {
    int hp;
    int maxHp;
})

// Tags (zero-cost markers)
TAG(Player)
TAG(Enemy)
TAG(Active)
```

The `.def` file uses the `COMPONENT(Name, Body)` and `TAG(Name)` macros which are automatically expanded by:
- `components_tags.h` - Generates typedefs and declares functions
- `components_tags.c` - Implements component/tag operations

**That's it!** Your components and tags are now ready to use.

### 2. Create an Entity Type

Define entity constructors that initialize entities with default components:

**`src/entities/player_entity.h`**
```c
#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H
#include "ecs_core.h"

struct Entity InstantiatePlayerEntity(float x, float y);

#endif
```

**`src/entities/player_entity.c`**
```c
#include "player_entity.h"
#include "components_tags.h"

struct Entity InstantiatePlayerEntity(float x, float y) {
    struct Entity entity;
    InitializeEntity(&entity);
    
    AddComponent(&entity, Position, (PositionComponent){x, y});
    AddComponent(&entity, Velocity, (PositionComponent){0, 0});
    AddComponent(&entity, Health, (HealthComponent){100, 100});
    AddTag(&entity, Player);
    AddTag(&entity, Active);
    
    return entity;
}
```

### 3. Create Systems

Systems process entities that match certain component signatures:

**`src/systems/movement_system.h`**
```c
#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

void MovementSystem(float deltaTime);

#endif
```

**`src/systems/movement_system.c`**
```c
#include "movement_system.h"
#include "components_tags.h"
#include "ecs_core.h"

void MovementSystem(float deltaTime) {
    // Process all entities that have both Position and Velocity
    ForEachEntityWith(entity_id, COMPONENT_Position, COMPONENT_Velocity) {
        PositionComponent *pos = &PositionComponentData[entity_id];
        VelocityComponent *vel = &VelocityComponentData[entity_id];
        
        pos->x += vel->vx * deltaTime;
        pos->y += vel->vy * deltaTime;
    }
}
```

### 4. Use in Main

**`src/main.c`**
```c
#include "ecs_core.h"
#include "components_tags.h"
#include "entities/player_entity.h"
#include "systems/movement_system.h"

int main(void) {
    InitializeStack(&AvailableIDs);
    
    // Create a player entity
    struct Entity player = InstantiatePlayerEntity(10.0f, 20.0f);
    
    // Modify components
    VelocityComponentData[player.id].vx = 5.0f;
    VelocityComponentData[player.id].vy = 0.0f;
    
    // Run systems
    MovementSystem(0.016f);  // 16ms frame time
    
    // Query component data
    if (HasComponent(&player, Health)) {
        HealthComponent health = GetComponent(player.id, Health);
        printf("Player health: %d/%d\n", health.hp, health.maxHp);
    }
    
    // Check tags
    if (HasTag(&player, Active)) {
        printf("Player is active\n");
    }
    
    // Cleanup
    RemoveComponent(&player, Position);
    RemoveComponent(&player, Velocity);
    RemoveComponent(&player, Health);
    DestroyEntity(&player);
    
    return 0;
}
```

### 5. Update CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(ECS_Example C)

set(CMAKE_C_STANDARD 99)

# Add the ECS library
add_subdirectory(ECS)

# Create executable
add_executable(example
    src/main.c
    src/entities/player_entity.c
    src/systems/movement_system.c
)

# Link libraries and include directories
target_link_libraries(example PRIVATE ECS)
target_include_directories(example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/systems
)
```

## Macro Reference

### Component and Tag Definition

Define components and tags in a `.def` file:

```c
// In components_tags.def
COMPONENT(ComponentName, {
    type field1;
    type field2;
})

TAG(TagName)
```

### Component Operations

```c
// Add component to entity
AddComponent(&entity, ComponentName, (ComponentNameComponent){...})

// Check if entity has component
HasComponent(&entity, ComponentName)

// Get component data
GetComponent(entity.id, ComponentName)

// Remove component from entity
RemoveComponent(&entity, ComponentName)
```

### Tag Operations

```c
// Add tag to entity
AddTag(&entity, TagName)

// Check if entity has tag
HasTag(&entity, TagName)

// Remove tag from entity
RemoveTag(&entity, TagName)
```

### Entity Operations

```c
// Initialize a new entity
InitializeEntity(&entity)

// Destroy/recycle entity
DestroyEntity(&entity)

// Instantiate entity (calls: InstantiateComponentNameEntity(...))
InstantiateEntity(ComponentName, ...args)
```

### System Iteration

```c
// Iterate all entities with specific components
ForEachEntityWith(entity_id, COMPONENT_Type1, COMPONENT_Type2) {
    // entity_id is the entity being processed
    Type1Component *comp1 = &Type1ComponentData[entity_id];
    Type2Component *comp2 = &Type2ComponentData[entity_id];
}
```

## Component Naming Conventions

To properly use the generated macros, follow these naming patterns:

| Definition | Generated Functions | Example |
|-----------|-------------------|---------|
| `COMPONENT(Test, {...})` | `AddTest`, `HasTest`, `GetTest`, `RemoveTest` | `AddComponent(&e, Test, val)` |
| `TAG(Active)` | `AddTag_Active`, `HasTag_Active`, `RemoveTag_Active` | `AddTag(&e, Active)` |
| `InstantiateEntity(Test)` | Calls `InstantiateTestEntity(...)` | PascalCase match required |

## Architecture

- **Entity**: Unique ID (0-4999) + generation counter
- **Component**: Data stored in parallel arrays, indexed by entity ID
- **Signature**: 256-bit bitmask tracking which components/tags an entity has
- **System**: Functions that iterate entities with specific signatures
- **Tag**: Zero-overhead marker components for entity classification
- **Definition File**: Centralized component/tag declarations

## Limits

- **Maximum entities**: `MAX_ENTITIES` (default 5000, configurable in `ECS/include/ecs_core.h`)
- **Maximum component/tag types**: 256 (4 x 64-bit chunks)
- **Memory model**: Stack-allocated arrays (no dynamic allocation)
- **Thread safety**: Not thread-safe; synchronize externally if needed

## File Structure

```
ECS_C/
├── ECS/                          # Core ECS library
│   ├── include/
│   │   ├── ecs_core.h           # Core ECS declarations
│   │   ├── components_tags.h    # Generated from .def file
│   │   └── components_tags.def  # Define your components/tags here
│   ├── src/
│   │   ├── ecs_core.c
│   │   └── components_tags.c    # Generated implementations
│   └── CMakeLists.txt
├── src/                          # Example application
│   ├── main.c
│   ├── entities/
│   ├── systems/
│   └── components_tags.h         # Example definitions
└── CMakeLists.txt
```

## Using Components and Tags

### Module Usage (Copy ECS folder only)

1. Copy `ECS/` to your project
2. Create your own `components_tags.def` in `ECS/include/`
3. Include and build as described above

### Full Repository Usage

The example project demonstrates:
- Component definition in `ECS/include/components_tags.def`
- Entity creation patterns in `src/entities/`
- System implementation in `src/systems/`
- Complete usage in `src/main.c`

Run the example:
```bash
mkdir build && cd build
cmake ..
cmake --build .
./example  # or example.exe on Windows
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Happy building!** 🚀
