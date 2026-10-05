#include "Apps.h"
#include "AppState.h"
#include "imgui.h"

void RenderApps(AppState& /*state*/)
{
    // Currently shows the ImGui demo to verify rendering; the two apps are pending.
    ImGui::ShowDemoWindow();

    // TODO: Replace the demo with two distinct screens opened from the taskbar.
    // Give each screen its own title and placeholder information.
    // TODO: Show each app only when its state.showFirstApp or state.showSecondApp
    // flag is true. Pass the same flag to ImGui::Begin so closing an app updates
    // the shared state used by the taskbar controls and running-app indicators.
}
