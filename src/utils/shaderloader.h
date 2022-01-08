#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "../common.h"
#include "../math/vector.h"

namespace ShaderLoader
{
    // Loads a VS and FS from disk with same name and returns a gl program
    u32 LoadSimpleShader(const std::string& name);
}