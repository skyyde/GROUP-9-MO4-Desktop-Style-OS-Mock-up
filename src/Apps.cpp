#include "Apps.h"
#include "AppState.h"

void Apps::RenderApp1(AppState& state)
{
    (void)state;
    // KYLA - October 6: create App 1 with content/design and open/close behavior.
    // Use state.showApp1 to control whether its ImGui window is visible.
}

void Apps::RenderApp2(AppState& state)
{
    (void)state;
    // NICOS - October 6: create App 2 with different content/design.
    // Use state.showApp2 to control whether its ImGui window is visible.
}

void Apps::Render(AppState& state)
{
    RenderApp1(state);
    RenderApp2(state);
}
