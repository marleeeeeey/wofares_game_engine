#include <box2d/box2d.h>
#include <my_common_cpp_utils/Logger.h>

class Box2dObjectRAII
{
public:
    explicit Box2dObjectRAII(b2Body* body, std::shared_ptr<b2World> world);
    ~Box2dObjectRAII();
    Box2dObjectRAII(Box2dObjectRAII&& other) noexcept;
    Box2dObjectRAII& operator=(Box2dObjectRAII&& other) noexcept;

    b2Body* GetBody() const { return body; }
private:
    b2Body* body;
    std::shared_ptr<b2World> world;
};
