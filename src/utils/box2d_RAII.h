#pragma once
#include <box2d/box2d.h>
#include <my_common_cpp_utils/logger.h>

class Box2dObjectRAII
{
    b2Body* body;
    std::shared_ptr<b2World> world;
    static size_t counter;
public:
    explicit Box2dObjectRAII(b2Body* body, std::shared_ptr<b2World> world);
    ~Box2dObjectRAII();
    Box2dObjectRAII(Box2dObjectRAII&& other) noexcept;
    Box2dObjectRAII& operator=(Box2dObjectRAII&& other) noexcept;
public:
    b2Body* GetBody() const { return body; }
    static size_t GetBodyCounter() { return counter; }
};
