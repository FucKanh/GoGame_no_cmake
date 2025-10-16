#include "Audio.h"
#include <filesystem>
namespace fs = std::filesystem;
namespace AudioUtil {
    std::string assetPath(const std::string& rel) {
        for (auto r: {".", "./assets", "../assets", "../../assets"}) {
            fs::path p = fs::path(r) / rel;
            if (fs::exists(p)) return p.string();
        }
        return rel;
    }
}
