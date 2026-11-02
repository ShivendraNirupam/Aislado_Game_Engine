#include <engine/core/asserts.h>
#include <engine/core/logger.h>

int main(int argc, char** argv) {
    (void)argv;

    engine::logger_initialize();

    ELOG_FATAL("A test message: %f", 3.14159);
    ELOG_ERROR("A test message: %f", 3.14159);
    ELOG_WARN("A test message: %f", 3.14159);
    ELOG_INFO("A test message: %f", 3.14159);
    ELOG_DEBUG("A test message: %f", 3.14159);
    ELOG_TRACE("A test message: %f", 3.14159);

    ENGINE_ASSERT(1 == 1);  // passes, prints nothing

    // Run the program with any argument to see a failing assertion.
    if (argc > 1) {
        ENGINE_ASSERT_MSG(argc == 1, "Deliberate failure to test assertions.");
    }

    engine::logger_shutdown();
    return 0;
}
