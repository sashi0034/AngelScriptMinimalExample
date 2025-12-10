#include <cassert>
#include <iostream>

#include "angelscript.h"

int main(int argc, char** argv)
{
    asIScriptEngine* engine = asCreateScriptEngine();
    assert(engine != nullptr);

    engine->ShutDownAndRelease();

    std::cout << "AngelScript engine created and released successfully." << std::endl;

    return 0;
}
