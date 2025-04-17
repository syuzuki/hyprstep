#include <format>
#include <string>
#include <string_view>
#include <stdexcept>

#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

namespace {
    using GetMonitor = PHLMONITOR (*)(CCompositor*, const std::string&);

    HANDLE         g_handle      = nullptr;
    CFunctionHook* g_monitorHook = nullptr;

    [[noreturn]]
    void initError(const char* message) {
        HyprlandAPI::addNotification(g_handle, message, {1.0, 0.2, 0.2, 1.0}, 5000);
        Debug::log(ERR, message);
        throw std::runtime_error(message);
    }

    PHLMONITOR getMonitorInDirectionFix(CCompositor& compositor, PHLMONITOR sourceMonitor, char dir) {
        // CCompositor::getMonitorInDirection() ignores sourceMonitor, so temporary fix by this function

        if (!sourceMonitor)
            return nullptr;

        const auto POSA  = sourceMonitor->vecPosition;
        const auto SIZEA = sourceMonitor->vecSize;

        auto       longestIntersect        = -1;
        PHLMONITOR longestIntersectMonitor = nullptr;

        for (auto const& m : compositor.m_vMonitors) {
            if (m == sourceMonitor)
                continue;

            const auto POSB  = m->vecPosition;
            const auto SIZEB = m->vecSize;
            switch (dir) {
                case 'l':
                    if (STICKS(POSA.x, POSB.x + SIZEB.x)) {
                        const auto INTERSECTLEN = std::max(0.0, std::min(POSA.y + SIZEA.y, POSB.y + SIZEB.y) - std::max(POSA.y, POSB.y));
                        if (INTERSECTLEN > longestIntersect) {
                            longestIntersect        = INTERSECTLEN;
                            longestIntersectMonitor = m;
                        }
                    }
                    break;
                case 'r':
                    if (STICKS(POSA.x + SIZEA.x, POSB.x)) {
                        const auto INTERSECTLEN = std::max(0.0, std::min(POSA.y + SIZEA.y, POSB.y + SIZEB.y) - std::max(POSA.y, POSB.y));
                        if (INTERSECTLEN > longestIntersect) {
                            longestIntersect        = INTERSECTLEN;
                            longestIntersectMonitor = m;
                        }
                    }
                    break;
                case 't':
                case 'u':
                    if (STICKS(POSA.y, POSB.y + SIZEB.y)) {
                        const auto INTERSECTLEN = std::max(0.0, std::min(POSA.x + SIZEA.x, POSB.x + SIZEB.x) - std::max(POSA.x, POSB.x));
                        if (INTERSECTLEN > longestIntersect) {
                            longestIntersect        = INTERSECTLEN;
                            longestIntersectMonitor = m;
                        }
                    }
                    break;
                case 'b':
                case 'd':
                    if (STICKS(POSA.y + SIZEA.y, POSB.y)) {
                        const auto INTERSECTLEN = std::max(0.0, std::min(POSA.x + SIZEA.x, POSB.x + SIZEB.x) - std::max(POSA.x, POSB.x));
                        if (INTERSECTLEN > longestIntersect) {
                            longestIntersect        = INTERSECTLEN;
                            longestIntersectMonitor = m;
                        }
                    }
                    break;
            }
        }

        if (longestIntersect != -1)
            return longestIntersectMonitor;

        return nullptr;
    }

    PHLMONITOR getMonitorFromStringWrapper(CCompositor* compositor, const std::string& name) {
        static const std::string_view PREFIX = "step:";

        if (name.starts_with(PREFIX)) {
            std::string_view view = name;
            view.remove_prefix(PREFIX.length());

            if (view.starts_with('o')) {
                view.remove_prefix(1);
            } else {
                Debug::log(WARN, std::format("[hyprstep] Invalid base monitor part: name={}", name));
                return nullptr;
            }
            PHLMONITOR mon = g_pCompositor->getMonitorFromVector({0, 0});
            if (!mon) {
                Debug::log(
                    WARN,
                    std::format("[hyprstep] No base monitor found at: {}, name={}", std::string_view(name).substr(0, name.length() - view.length()).substr(PREFIX.length()), name));
                return nullptr;
            }

            while (!view.empty()) {
                char c = view.at(0);
                view.remove_prefix(1);

                switch (c) {
                    case 'l':
                    case 'r':
                    case 'u':
                    case 't':
                    case 'd':
                    case 'b': mon = getMonitorInDirectionFix(*g_pCompositor, mon, c); break;
                    default: Debug::log(WARN, std::format("[hyprstep] Ignoring invalid monitor name part: {}, name={}", c, name)); break;
                }
                if (!mon) {
                    Debug::log(WARN, std::format("[hyprstep] No monitor found at: {}, name={}", std::string_view(name).substr(0, name.length() - view.length()).substr(5), name));
                    return nullptr;
                }
            }

            return mon;
        } else {
            return reinterpret_cast<GetMonitor>(g_monitorHook->m_pOriginal)(compositor, name);
        }
    }
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    g_handle = handle;

    Debug::log(INFO, "[hyprstep] Initialize plugin");

    const std::string_view hash = __hyprland_api_get_hash();
    if (hash != GIT_COMMIT_HASH) {
        initError("[hyprstep] Hyprland version mismatch: running hyprland version differs from the plugin was built");
    }

    auto funcs = HyprlandAPI::findFunctionsByName(handle, "getMonitorFromString");
    for (const auto& f : funcs) {
        if (f.demangled.starts_with("CCompositor::getMonitorFromString(")) {
            if (g_monitorHook != nullptr) {
                initError("[hyprstep] Multiple functions found: CCompositor::getMonitorFromString");
            }

            g_monitorHook = HyprlandAPI::createFunctionHook(handle, f.address, reinterpret_cast<void*>(getMonitorFromStringWrapper));
        }
    }
    if (g_monitorHook == nullptr) {
        initError("[hyprstep] No function found: CCompositor::getMonitorFromString");
    }
    g_monitorHook->hook();

    return {"hyprstep", "Add additional monitor specifier to find by the positions", "syuzuki", VERSION};
}

APICALL EXPORT void PLUGIN_EXIT() {}
