#include <format>
#include <string>
#include <string_view>
#include <stdexcept>

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
                    case 'b': mon = g_pCompositor->getMonitorInDirection(mon, c); break;
                    default: Debug::log(WARN, std::format("[hyprstep] Ignoring invalid monitor name part: {}, name={}", c, name)); break;
                }
                if (!mon) {
                    Debug::log(WARN, std::format("[hyprstep] No monitor found at: {}, name={}", std::string_view(name).substr(0, name.length() - view.length()).substr(5), name));
                    return nullptr;
                }
            }

            return mon;
        } else {
            return reinterpret_cast<GetMonitor>(g_monitorHook->m_original)(compositor, name);
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
