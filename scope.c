#include <stdio.h>
#include <stdbool.h>

// Scoping support.
#define __scope(object_type, \
                object_initial_value, \
                scope_initializer_body, \
                object_initializer_body, \
                object_finalizer_body, \
                ...) \
{ \
    __label__ __scope_exit; \
    enum { __scope_is_finalized, ## __VA_ARGS__, __scope_size }; \
    object_type __scope_objects[__scope_size] = { object_initial_value }; \
    scope_initializer_body; \
    inline void __scope_object_initializer(object_type *object) \
    { \
        object_initializer_body; \
    } \
    inline void __scope_object_finalizer(object_type *object) \
    { \
        object_finalizer_body; \
    } \
    inline void __scope_finalizer() \
    { \
        bool *scope_is_finalized = (bool *) &local(__scope_is_finalized); \
        if (!(*scope_is_finalized)) \
        { \
            for (size_t i = 1; i < __scope_size; ++i) \
            { \
                __scope_object_finalizer(&local(i)); \
            } \
            *scope_is_finalized = true; \
        } \
    } \
    for (size_t i = 1; i < __scope_size; ++i) \
    { \
        __scope_object_initializer(&local(i)); \
    } \
    (void) (0);

#define local(object) __scope_objects[object]

#define endscope \
    __scope_exit: \
    __scope_finalizer(); \
     (void) (0); \
}

#define scope_break \
    __scope_finalizer(); \
    break

#define scope_continue \
    __scope_finalizer(); \
    continue

#define scope_return \
    __scope_finalizer(); \
    return

#define scope_leave \
    __scope_finalizer(); \
    goto __scope_exit

#define scope_finalize \
    __scope_finalizer()

// Simple scope example.
#define simple_scope_initializer_body puts("Initializing scope.")
#define simple_scope_object_initializer_body object_init(object)
#define simple_scope_object_finalizer_body object_free(object)

#define scope(...) \
    __scope(size_t, \
            0, \
            simple_scope_initializer_body, \
            simple_scope_object_initializer_body, \
            simple_scope_object_finalizer_body, \
            ## __VA_ARGS__)

void object_init(size_t *object)
{
    printf("Init object: %zu.\n", *object);
}

void object_free(size_t *object)
{
    printf("Free object: %zu.\n", *object);
}

void scoped_function()
scope(object1)
{
    local(object1) = 1;
    scope_return;
    local(object1) = 2;
} endscope;

int main()
{
    // Regular scope.
    scope(object1, object2)
    {
        local(object1) = 1;

        scope() { printf("Hello from empty scope!\n"); } endscope;

        // Breakable scope.
        while (true)
        {
            scope(object1)
            {
                local(object1) = 666;
                scope_break;
            } endscope;
        }

        scope(object1)
        {
            local(object1) = 42;
            scope_leave;
            printf("!\n");
        } endscope;

        local(object2) = 2;
    } endscope;

    scoped_function();

    scope(another_example);
    printf("another_example.\n");
    endscope;

    return 0;
}
