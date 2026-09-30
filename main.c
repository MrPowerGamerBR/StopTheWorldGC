#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct GCObject GCObject;

struct GCObject {
    GCObject* next;
    int32_t type;
    bool marked;
    int32_t size;

    void (*cleanupFunction)(void*);
};

typedef struct {
    GCObject gcObject;
    char* data;
    int32_t length;
} CustomString;

typedef struct {
    GCObject gcObject;
    CustomString* nested;
} NestedStringHolder;

typedef struct {
    GCObject gcObject;
    CustomString* string;
    CustomString* string2;
    NestedStringHolder* nested;
} StringHolder;

void CustomString_free(CustomString* string) {
    printf("Cleaning up...\n");
    free(string->data);
}

static int32_t allocatedBytes = 0;

static GCObject* lastAllocatedGCObject = nullptr;
static GCObject* gcRoot;

GCObject* GarbageCollector_calloc(int32_t type, size_t count, size_t size, void* cleanupFunction) {
    GCObject* gcObjectOnTheHeap = calloc(count, size);

    if (cleanupFunction != nullptr)
        gcObjectOnTheHeap->cleanupFunction = cleanupFunction;

    gcObjectOnTheHeap->type = type;
    gcObjectOnTheHeap->next = lastAllocatedGCObject;
    gcObjectOnTheHeap->size = size;
    allocatedBytes += size;
    lastAllocatedGCObject = gcObjectOnTheHeap;

    return gcObjectOnTheHeap;
}

void GarbageCollector_walk(GCObject* gcObject) {
    if (gcObject == nullptr)
        return;

    printf("Walking object %d\n", gcObject->type);
    gcObject->marked = true;

    switch (gcObject->type) {
        case 0: break;
        case 1: {
            StringHolder* holder = (StringHolder*) gcObject;
            GarbageCollector_walk(&holder->string->gcObject);
            GarbageCollector_walk(&holder->string2->gcObject);
            GarbageCollector_walk(&holder->nested->gcObject);
            break;
        }
        case 2: {
            NestedStringHolder* holder = (NestedStringHolder*) gcObject;
            GarbageCollector_walk(&holder->nested->gcObject);
            break;
        }
        default: abort();
    }
}

void GarbageCollector_gc() {
    {
        // Unmark everything
        GCObject* gcObject = lastAllocatedGCObject;
        while (gcObject != nullptr) {
            gcObject->marked = 0;
            gcObject = gcObject->next;
        }
    }

    {
        // Now we mark everything that IS referenced by something, from the gcRoot
        if (gcRoot != nullptr)
            GarbageCollector_walk(gcRoot);
    }

    {
        GCObject* previousObject = nullptr;
        GCObject* gcObject = lastAllocatedGCObject;
        while (gcObject != nullptr) {
            bool shouldFree = false;
            printf("GCObject found and it is %d\n", gcObject->marked);

            // If it isn't marked, we free it
            if (gcObject->marked != 1) {
                if (previousObject != nullptr) {
                    previousObject->next = gcObject->next;
                } else {
                    // We unallocated the first object of the list, rebind to the next one
                    lastAllocatedGCObject = gcObject->next;
                }

                if (gcObject->cleanupFunction != nullptr)
                    gcObject->cleanupFunction(gcObject);

                allocatedBytes -= gcObject->size;
                shouldFree = true;
            } else {
                previousObject = gcObject;
            }

            GCObject* oldGcObject = gcObject;
            gcObject = gcObject->next;
            if (shouldFree)
                free(oldGcObject);
        }
    }

    printf("Allocated bytes: %d\n", allocatedBytes);
}

CustomString* CustomString_create() {
    CustomString* string = (CustomString*) GarbageCollector_calloc(0, 1, sizeof(CustomString), CustomString_free);
    string->data = calloc(16, sizeof(char));
    return string;
}

int main() {
    CustomString* string = CustomString_create();
    CustomString* string2 = CustomString_create();

    StringHolder* holder = (StringHolder*) GarbageCollector_calloc(1, 1, sizeof(StringHolder), nullptr);
    holder->string = string;
    holder->string2 = string2;
    holder->nested = (NestedStringHolder*) GarbageCollector_calloc(2, 1, sizeof(NestedStringHolder), nullptr);
    holder->nested->nested = CustomString_create();
    gcRoot = (GCObject*) holder;

    GarbageCollector_gc();

    holder->nested = nullptr;

    GarbageCollector_gc();

    gcRoot = nullptr;

    GarbageCollector_gc();

    return 0;
}
