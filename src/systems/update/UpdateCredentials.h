#pragma once
#include <string>

class UpdateCredentials
{
public:
    explicit UpdateCredentials(const char *space = "ota-updates") : name(space) {}
    std::string load() const;
    bool save(const std::string &value) const;

private:
    const char *name;
};
