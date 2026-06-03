#ifndef ECS_H
#define ECS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// ============================================================================
// 1. AUTOMATIC TYPE GENERATION (From components_tags.def)
// ============================================================================

typedef enum {
    COMPONENT_NONE = -1,

    #define COMPONENT(Name, Body) COMPONENT_##Name,
    #define TAG(Name)             TAG_##Name,
    #include "components_tags.def"
    #undef COMPONENT
    #undef TAG

    COMPONENT_COUNT
} ComponentType;

#define COMPONENT(Name, Body) typedef struct Body Name##Component;
#define TAG(Name)             // Tags don't have struct bodies
#include "components_tags.def"
#undef COMPONENT
#undef TAG


// ============================================================================
// 2. HEADER / API DECLARATIONS (CORE ENGINE)
// ============================================================================

#define ECS_INITIAL_CAPACITY 32

extern int32_t ECS_Capacity;
extern int32_t EntityIDs;
extern int32_t ActiveEntityCount;
extern int32_t *ActiveEntities;
extern int32_t *EntityToIndex;

#define BITSET_CHUNKS 4
typedef struct {
    uint64_t chunks[BITSET_CHUNKS];
} ComponentSignature;
extern ComponentSignature *EntitySignatures;

void ECS_Init(void);
void ECS_Shutdown(void);
void ECS_Resize(int32_t newCapacity);

void Signature_SetBit(ComponentSignature *sig, int bitIndex);
void Signature_ClearBit(ComponentSignature *sig, int bitIndex);
bool Signature_TestBit(const ComponentSignature *sig, int bitIndex);
void Signature_ClearAll(ComponentSignature *sig);
bool Signature_Matches(const ComponentSignature *entitySig, const ComponentSignature *systemSig);

#define AddComponent(entity, CompName, ...) \
    Add##CompName(entity, (CompName##Component){__VA_ARGS__})
#define GetComponent(entityID, CompName) (&CompName##ComponentData[entityID])
#define HasComponent(entity, CompName) Has##CompName(entity)
#define RemoveComponent(entity, CompName) Remove##CompName(entity)

#define DECLARE_COMPONENT_TYPE(CompName, TypeName) \
    extern TypeName *CompName##ComponentData; \
    void Add##CompName(const struct Entity *entity, TypeName value); \
    bool Has##CompName(const struct Entity *entity); \
    void Remove##CompName(const struct Entity *entity); \
    void Resize##CompName##Storage(int32_t newCapacity); \
    void Free##CompName##Storage(void);

#define DEFINE_COMPONENT_TYPE(CompName, TypeName) \
    TypeName *CompName##ComponentData = NULL; \
    void Resize##CompName##Storage(int32_t newCapacity) { \
        TypeName *temp = (TypeName*)realloc(CompName##ComponentData, newCapacity * sizeof(TypeName)); \
        if (!temp) { \
            fprintf(stderr, "ECS Error: Realloc failed for component storage: " #CompName "\n"); \
            exit(EXIT_FAILURE); \
        } \
        CompName##ComponentData = temp; \
    } \
    void Free##CompName##Storage(void) { \
        free(CompName##ComponentData); \
        CompName##ComponentData = NULL; \
    } \
    void Add##CompName(const struct Entity *entity, TypeName value) { \
        CompName##ComponentData[entity->id] = value; \
        Signature_SetBit(&EntitySignatures[entity->id], COMPONENT_##CompName); \
    } \
    bool Has##CompName(const struct Entity *entity) { \
        return Signature_TestBit(&EntitySignatures[entity->id], COMPONENT_##CompName); \
    } \
    void Remove##CompName(const struct Entity *entity) { \
        Signature_ClearBit(&EntitySignatures[entity->id], COMPONENT_##CompName); \
    }

static inline ComponentSignature CreateSignatureFromIDs(const int ids[], int count) {
    ComponentSignature sig = {0};
    for (int i = 0; i < count; i++) {
        Signature_SetBit(&sig, ids[i]);
    }
    return sig;
}

#define ForEachEntityWith(idVarName, ...) \
    for (struct { int i; ComponentSignature sig; int active; } _loop = { \
        .i = 0, \
        .sig = CreateSignatureFromIDs((int[]){__VA_ARGS__}, sizeof((int[]){__VA_ARGS__}) / sizeof(int)), \
        .active = 1 \
    }; \
    _loop.active && _loop.i < ActiveEntityCount; \
    _loop.i++) \
    for (int idVarName = ActiveEntities[_loop.i]; _loop.active; _loop.active = 0) \
        if (Signature_Matches(&EntitySignatures[idVarName], &_loop.sig))

#define AddTag(entity, TagName) AddTag_##TagName(entity)
#define HasTag(entity, TagName) HasTag_##TagName(entity)
#define RemoveTag(entity, TagName) RemoveTag_##TagName(entity)

#define DECLARE_TAG_TYPE(TagName) \
    void AddTag_##TagName(const struct Entity *entity); \
    bool HasTag_##TagName(const struct Entity *entity); \
    void RemoveTag_##TagName(const struct Entity *entity);

#define DEFINE_TAG_TYPE(TagName) \
    void AddTag_##TagName(const struct Entity *entity) { \
        Signature_SetBit(&EntitySignatures[entity->id], TAG_##TagName); \
    } \
    bool HasTag_##TagName(const struct Entity *entity) { \
        return Signature_TestBit(&EntitySignatures[entity->id], TAG_##TagName); \
    } \
    void RemoveTag_##TagName(const struct Entity *entity) { \
        Signature_ClearBit(&EntitySignatures[entity->id], TAG_##TagName); \
    }

#define CreateEntity(entityVarName, ...) \
struct Entity entityVarName; \
InitializeEntity(&entityVarName); \
printf("Spawned Entity %d\n", entityVarName.id); \
__VA_ARGS__

struct Entity {
    int32_t id;
    int32_t generation;
};

typedef struct {
    int32_t *arr;
    int32_t top;
    int32_t capacity;
} Stack;

extern Stack AvailableIDs;

void InitializeStack(Stack *stack, int32_t initialCapacity);
void FreeStack(Stack *stack);
void ResizeStack(Stack *stack, int32_t newCapacity);
bool StackEmpty(const Stack *stack);
bool StackFull(const Stack *stack);
void PushToStack(Stack *stack, int32_t id);
int32_t PopFromStack(Stack *stack);
int32_t PeekStack(const Stack *stack);

void InitializeEntity(struct Entity *entity);
void DestroyEntity(struct Entity *entity);

#define COMPONENT(Name, Body) DECLARE_COMPONENT_TYPE(Name, Name##Component)
#define TAG(Name)             DECLARE_TAG_TYPE(Name)
#include "components_tags.def"
#undef COMPONENT
#undef TAG

#endif // ECS_H


// ============================================================================
// 3. IMPLEMENTATION LOGIC
// ============================================================================
#ifdef ECS_IMPLEMENTATION
#undef ECS_IMPLEMENTATION

#define COMPONENT(Name, Body) DEFINE_COMPONENT_TYPE(Name, Name##Component)
#define TAG(Name)             DEFINE_TAG_TYPE(Name)
#include "components_tags.def"
#undef COMPONENT
#undef TAG

int32_t ECS_Capacity = 0;
int32_t EntityIDs = 0;
int32_t ActiveEntityCount = 0;
int32_t *ActiveEntities = NULL;
int32_t *EntityToIndex = NULL;
ComponentSignature *EntitySignatures = NULL;

Stack AvailableIDs;

inline void ECS_Init(void) {
    ECS_Capacity = ECS_INITIAL_CAPACITY;

    ActiveEntities = (int32_t*)malloc(ECS_Capacity * sizeof(int32_t));
    EntityToIndex = (int32_t*)malloc(ECS_Capacity * sizeof(int32_t));
    EntitySignatures = (ComponentSignature*)calloc(ECS_Capacity, sizeof(ComponentSignature));

    if (!ActiveEntities || !EntityToIndex || !EntitySignatures) {
        fprintf(stderr, "ECS Error: Initial memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    InitializeStack(&AvailableIDs, ECS_Capacity);

    #define COMPONENT(Name, Body) Resize##Name##Storage(ECS_Capacity);
    #define TAG(Name)
    #include "components_tags.def"
    #undef COMPONENT
    #undef TAG
}

inline void ECS_Resize(const int32_t newCapacity) {
    if (newCapacity <= ECS_Capacity) return;

    int32_t *temp_ActiveEntities = (int32_t*)realloc(ActiveEntities, newCapacity * sizeof(int32_t));
    if (!temp_ActiveEntities) {
        fprintf(stderr, "ECS Error: Runtime memory reallocation failed for ActiveEntities.\n");
        ECS_Shutdown();
        exit(EXIT_FAILURE);
    }
    ActiveEntities = temp_ActiveEntities;

    int32_t *temp_EntityToIndex = (int32_t*)realloc(EntityToIndex, newCapacity * sizeof(int32_t));
    if (!temp_EntityToIndex) {
        fprintf(stderr, "ECS Error: Runtime memory reallocation failed for EntityToIndex.\n");
        ECS_Shutdown();
        exit(EXIT_FAILURE);
    }
    EntityToIndex = temp_EntityToIndex;

    ComponentSignature *temp_EntitySignatures = (ComponentSignature*)realloc(EntitySignatures, newCapacity * sizeof(ComponentSignature));
    if (!temp_EntitySignatures) {
        fprintf(stderr, "ECS Error: Runtime memory reallocation failed for EntitySignatures.\n");
        ECS_Shutdown();
        exit(EXIT_FAILURE);
    }
    EntitySignatures = temp_EntitySignatures;

    for (int32_t i = ECS_Capacity; i < newCapacity; i++) {
        Signature_ClearAll(&EntitySignatures[i]);
    }

    ResizeStack(&AvailableIDs, newCapacity);

    #define COMPONENT(Name, Body) Resize##Name##Storage(newCapacity);
    #define TAG(Name)
    #include "components_tags.def"
    #undef COMPONENT
    #undef TAG

    ECS_Capacity = newCapacity;
}

inline void ECS_Shutdown(void) {
    free(ActiveEntities);
    free(EntityToIndex);
    free(EntitySignatures);
    ActiveEntities = NULL;
    EntityToIndex = NULL;
    EntitySignatures = NULL;

    FreeStack(&AvailableIDs);

    #define COMPONENT(Name, Body) Free##Name##Storage();
    #define TAG(Name)
    #include "components_tags.def"
    #undef COMPONENT
    #undef TAG
}

inline void InitializeStack(Stack *stack, const int32_t initialCapacity) {
    stack->capacity = initialCapacity;
    stack->top = -1;
    stack->arr = (int32_t*)malloc(stack->capacity * sizeof(int32_t));
    if (!stack->arr) {
        fprintf(stderr, "ECS Error: Stack tracking allocation failed.\n");
        exit(EXIT_FAILURE);
    }
}

inline void ResizeStack(Stack *stack, const int32_t newCapacity) {
    int32_t *temp = (int32_t*)realloc(stack->arr, newCapacity * sizeof(int32_t));

    if (!temp) {
        fprintf(stderr, "ECS Error: Stack tracking resizing failed.\n");
        ECS_Shutdown();
        exit(EXIT_FAILURE);
    }

    stack->arr = temp;
    stack->capacity = newCapacity;
}

inline void FreeStack(Stack *stack) {
    free(stack->arr);
    stack->arr = NULL;
    stack->top = -1;
    stack->capacity = 0;
}

inline bool StackEmpty(const Stack *stack) {
    return (stack->top == -1);
}

inline bool StackFull(const Stack *stack) {
    return (stack->top == stack->capacity - 1);
}

inline void PushToStack(Stack *stack, const int32_t id) {
    if (StackFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->arr[++stack->top] = id;
    printf("Pushed ID %d to recycle pool\n", id);
}

inline int32_t PopFromStack(Stack *stack) {
    if (StackEmpty(stack)) {
        return -1;
    }
    const int32_t popped = stack->arr[stack->top];
    stack->top--;
    printf("Reclaimed recycled ID %d\n", popped);
    return popped;
}

inline int32_t PeekStack(const Stack *stack) {
    if (StackEmpty(stack)) {
        return -1;
    }
    return stack->arr[stack->top];
}

inline void Signature_SetBit(ComponentSignature *sig, const int bitIndex) {
    const int chunk = bitIndex / 64;
    const int bit   = bitIndex % 64;
    sig->chunks[chunk] |= (1ULL << bit);
}

inline void Signature_ClearBit(ComponentSignature *sig, const int bitIndex) {
    const int chunk = bitIndex / 64;
    const int bit   = bitIndex % 64;
    sig->chunks[chunk] &= ~(1ULL << bit);
}

inline bool Signature_TestBit(const ComponentSignature *sig, const int bitIndex) {
    const int chunk = bitIndex / 64;
    const int bit   = bitIndex % 64;
    return (sig->chunks[chunk] & (1ULL << bit)) != 0;
}

inline void Signature_ClearAll(ComponentSignature *sig) {
    for (int i = 0; i < BITSET_CHUNKS; i++) {
        sig->chunks[i] = 0;
    }
}

inline bool Signature_Matches(const ComponentSignature *entitySig, const ComponentSignature *systemSig) {
    for (int i = 0; i < BITSET_CHUNKS; i++) {
        if ((entitySig->chunks[i] & systemSig->chunks[i]) != systemSig->chunks[i]) {
            return false;
        }
    }
    return true;
}

inline void InitializeEntity(struct Entity *entity) {
    const int32_t recycledId = PopFromStack(&AvailableIDs);
    if (recycledId != -1) {
        entity->id = recycledId;
    } else {
        if (EntityIDs >= ECS_Capacity) {
            ECS_Resize(ECS_Capacity * 2);
        }
        entity->id = EntityIDs;
        EntityIDs++;
        entity->generation = 0;
    }

    ActiveEntities[ActiveEntityCount] = entity->id;
    EntityToIndex[entity->id] = ActiveEntityCount;
    ActiveEntityCount++;
}

inline void DestroyEntity(struct Entity *entity) {
    Signature_ClearAll(&EntitySignatures[entity->id]);

    const int32_t indexOfDestroyEntity = EntityToIndex[entity->id];
    const int32_t indexOfLastEntity = ActiveEntityCount - 1;
    const int32_t lastEntityId = ActiveEntities[indexOfLastEntity];

    ActiveEntities[indexOfDestroyEntity] = lastEntityId;
    EntityToIndex[lastEntityId] = indexOfDestroyEntity;

    ActiveEntityCount--;

    PushToStack(&AvailableIDs, entity->id);
    entity->generation++;
    entity->id = -1;
}

#endif // ECS_IMPLEMENTATION