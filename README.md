# ECS_C - Entity Component System in C

A lightweight, macro-based Entity Component System (ECS) implementation written in C. This is a data-oriented architecture library designed for efficient entity and component management using compile-time code generation.

## Overview

ECS_C provides a flexible framework for building applications using the Entity Component System pattern. Instead of traditional inheritance hierarchies, ECS_C uses composition, where entities are collections of components and tags. The library uses powerful macros to simplify entity and component management while maintaining high performance.

**Key Features:**
- **Macro-based API** - Clean, expressive syntax with compile-time code generation
- **Component Signatures** - Efficient bitmask-based component tracking
- **Entity Pooling** - Automatic entity ID recycling for memory efficiency
- **Flexible Storage** - Dynamic allocation with automatic resizing
- **Tag System** - Lightweight component-less categorization
- **System Queries** - Iterate over entities with specific components/tags

## Installation

1. Copy the `ECS` folder to your project directory
2. Include `ecs_core.h` in your main file with the `ECS_IMPLEMENTATION` define:

```c
#define ECS_IMPLEMENTATION
#include "ECS/ecs_core.h"
```

3. Refer to `components_tags.def` to define your components and tags

That's it! No build configuration needed.

## Quick Start

### 1. Define Components and Tags

Create or modify `ECS/components_tags.def`:

```c
COMPONENT(Example, { int value; })
COMPONENT(Position, { float x, y; })
TAG(Default)
TAG(Active)
```

### 2. Initialize and Use

```c
#define ECS_IMPLEMENTATION
#include "ECS/ecs_core.h"

int main(void) {
    // Initialize the ECS system
    ECS_Init();

    // Create an entity with components and tags
    CreateEntity(e_Dummy, {
        AddTag(&e_Dummy, Default);
        AddComponent(&e_Dummy, Example, .value = 42);
    });

    // Access component data
    const int dummyValue = GetComponent(e_Dummy.id, Example)->value;
    printf("Entity %d has example component with value: %d\n", e_Dummy.id, dummyValue);

    // Query entities with specific components/tags
    ForEachEntityWith(entity, TAG_Default) {
        printf("Entity %d has Default Tag\n", entity);
    }

    // Remove component
    RemoveComponent(&e_Dummy, Example);

    // Destroy entity
    DestroyEntity(&e_Dummy);

    // Cleanup
    ECS_Shutdown();
    return 0;
}
```

## Macro Reference

All available macros are defined in `ecs_core.h`. Here are the most commonly used:

### Entity Management

```c
CreateEntity(varName, { ... })     // Create a new entity and initialize it
DestroyEntity(&entity)              // Destroy an entity and recycle its ID
ECS_Init()                           // Initialize the ECS system
ECS_Shutdown()                       // Cleanup and free all resources
```

### Component Operations

```c
AddComponent(&entity, ComponentName, .field = value)  // Add component to entity
HasComponent(&entity, ComponentName)                  // Check if entity has component
GetComponent(entity.id, ComponentName)                // Get pointer to component data
RemoveComponent(&entity, ComponentName)               // Remove component from entity
```

### Tag Operations

```c
AddTag(&entity, TagName)            // Add tag to entity
HasTag(&entity, TagName)            // Check if entity has tag
RemoveTag(&entity, TagName)         // Remove tag from entity
```

### Querying

```c
ForEachEntityWith(entityVar, TAG_Name, COMPONENT_Name) {
    // Process entity matching all specified tags/components
}
```

### Internal Functions (Advanced)

Refer to `ecs_core.h` for additional utility functions:
- `Signature_SetBit()`, `Signature_ClearBit()`, `Signature_TestBit()`
- `Signature_Matches()`, `Signature_ClearAll()`
- Stack-based operations for ID management

## Component Definition Format

Components are defined in `components_tags.def` using the `COMPONENT` macro:

### Component and Tag Definition

Define components and tags in a `.def` file:

```c
COMPONENT(MyComponent, {
    int field1;
    float field2;
    char data[128];
})
```

The second parameter is the struct body containing your component data.

## Tag Definition Format

Tags are defined in `components_tags.def` using the `TAG` macro:

```c
TAG(MyTag)
```

Tags have no associated data—they're purely for categorization.

## Architecture

- **Entity**: An object with a unique ID and generation counter
- **Component**: Data stored in parallel arrays, indexed by entity ID
- **Tag**: A lightweight marker with no associated data
- **Signature**: 64-bit bitmask (256 bits total) tracking which components/tags an entity has
- **System**: Logic that processes entities matching specific component/tag signatures

## System Limits

- **Maximum entities**: Dynamically resizable (starts at 32, doubles as needed)
- **Maximum component types**: 256 (4 chunks × 64 bits)
- **Dynamic allocation**: All data uses malloc/realloc for flexibility

## Example: Complete Game Loop

See `main.c` for a complete working example demonstrating:
- Entity creation with components
- Component access and modification
- Tag-based entity queries
- Entity cleanup

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Happy building!** 🚀
