#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "../common.h"
#include "../math/vector.h"

namespace FileLoader
{
    POSSIBLE_INLINE std::vector<std::string> readFile(const std::string& filename)
    {
        std::ifstream f(filename);
        std::string line;
        std::vector<std::string> out;

        while(std::getline(f, line))
        {
            out.push_back(line);
        }
        return out;
    }

    POSSIBLE_INLINE u64 countToken(const std::vector<std::string>& data, const std::string& token)
    {
        return std::count_if(data.begin(), data.end(), [&](std::string l) -> bool { return l.rfind(token, 0) == 0; });
    }


    POSSIBLE_INLINE Vector3 parseVector3(const std::string& line)
    {
        f32 x, y, z;
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ' '); // TODO: Pass a substring to this function instead
        std::getline(ss, token, ' '); x = std::stof(token);
        std::getline(ss, token, ' '); y = std::stof(token);
        std::getline(ss, token, ' '); z = std::stof(token);
        return Vector3(x, y, z);
    }

    POSSIBLE_INLINE Vector2 parseVector2(const std::string& line)
    {
        f32 x, y;
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ' '); // TODO: Pass a substring to this function instead
        std::getline(ss, token, ' '); x = std::stof(token);
        std::getline(ss, token, ' '); y = std::stof(token);
        return Vector2(x, y);
    }

    POSSIBLE_INLINE void parseFaces(u64* vI, u64* tI, u64* nI, const std::string& line)
    {
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ' '); // TODO: Pass a substring to this function instead
        std::getline(ss, token, '/'); vI[0] = std::stoull(token) - 1;
        std::getline(ss, token, '/'); tI[0] = std::stoull(token) - 1;
        std::getline(ss, token, ' '); nI[0] = std::stoull(token) - 1;

        std::getline(ss, token, '/'); vI[1] = std::stoull(token) - 1;
        std::getline(ss, token, '/'); tI[1] = std::stoull(token) - 1;
        std::getline(ss, token, ' '); nI[1] = std::stoull(token) - 1;
        
        std::getline(ss, token, '/'); vI[2] = std::stoull(token) - 1;
        std::getline(ss, token, '/'); tI[2] = std::stoull(token) - 1;
        std::getline(ss, token, ' '); nI[2] = std::stoull(token) - 1;
    }
}

