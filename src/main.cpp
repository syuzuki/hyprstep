#include <string>

#include <hyprland/src/plugins/PluginAPI.hpp>

namespace {
    HANDLE g_handle = nullptr;
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    g_handle = handle;

    Debug::log(INFO, "[hyprstep] Initialize plugin");

    const std::string_view hash = __hyprland_api_get_hash();
    if (hash != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(handle, "[hyprstep] Hyprland version mismatch: running hyprland version differs from the plugin was built", {1.0, 0.2, 0.2, 1.0}, 5000);

        throw std::runtime_error("[hyprstep] Hyprland version mismatch");
    }

    return {"hyprstep", "Add additional monitor specifier to find by the positions", "syuzuki", VERSION};
}

APICALL EXPORT void PLUGIN_EXIT() {}
