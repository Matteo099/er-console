#pragma once

class SystemService {
public:
    virtual ~SystemService() = default;

    virtual void init() = 0;
    virtual void update() = 0;
};
