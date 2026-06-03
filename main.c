#define  ECS_IMPLEMENTATION
#include "ECS/ecs_core.h"

int main(void) {
    ECS_Init();

    CreateEntity(e_Dummy, {
        AddTag(&e_Dummy, Default);
        AddComponent(&e_Dummy , Example, .value = 1);
    });

    const int DummyValue = GetComponent(e_Dummy.id, Example)->value;
    printf("Entity %d has example component with value: %d\n", e_Dummy.id, DummyValue);
    ForEachEntityWith(entity, TAG_Default) {
        printf("Entity %d has Default Tag\n", entity);
    }

    RemoveComponent(&e_Dummy, Example);
    DestroyEntity(&e_Dummy);
    ECS_Shutdown();
    return 0;
}