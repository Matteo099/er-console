#pragma once

class SystemService {
public:
    virtual ~SystemService() = default;

    virtual void init(void* params = nullptr) = 0;
    virtual void update() = 0;
};
