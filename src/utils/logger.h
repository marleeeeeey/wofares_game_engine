#pragma once
#include <entt/entt.hpp>
#include <my_cpp_utils/logger.h>

template <>
struct fmt::formatter<entt::entity> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const entt::entity& entity, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", static_cast<uint32_t>(entity));
    }
};
