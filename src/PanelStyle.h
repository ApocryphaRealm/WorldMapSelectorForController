#pragma once

// An optional look for the map list panel, read from a style file another mod ships (Norden UI - Black's
// "World Map Selector for Controller" option is the first). Without the file the panel wears the menu
// framework's theme and frame exactly as before.
//
// File: Data\SKSE\Plugins\WorldMapSelectorForController_PanelStyle.ini, section [PanelStyle]. Every key is
// optional; a colour is #RRGGBB or #RRGGBBAA. An absent key keeps the framework theme's colour for that part.
namespace WMS::PanelStyle
{
    // Reads the style file, if there is one. Call once, before the panel is first drawn.
    void Load();

    // True when a style file was read.
    bool IsActive();

    // The style's name (sName) for the log and the DevBench state; empty without a style file.
    const std::string& Name();

    // Render thread, inside the panel's window: pushes the style's colours and sizes. Pop undoes exactly what Push did.
    void Push();
    void Pop();

    // Render thread: the text colour of a row - the highlighted row's own colour when the style gives one. Returns
    // true when a colour was pushed (pop it with PopRowText).
    bool PushRowText(bool highlighted);
    void PopRowText(bool pushed);

    // Render thread: the style's highlight under the row whose rectangle is given (window draw list). False when the
    // style has no highlight of its own (ImGui's selected-row colour is used).
    bool DrawRowHighlight(void* drawList, float x0, float y0, float x1, float y1);

    // Render thread: the style's panel edge (lines along the top and bottom that fade out at both ends). True when the
    // style draws its own edge, so the framework theme's frame is not drawn.
    bool DrawPanelEdge(void* drawList, float x0, float y0, float x1, float y1);

    // Render thread, inside the panel's window: the style's separator line (sLine, fading at both ends) in place of
    // ImGui's Separator, which the framework draws as its theme's section rule. False when the style gives no line (call
    // Separator then).
    bool DrawSeparator();

    // The exports the style draws with; checked once when a style file is active.
    bool HasRequiredExports();

    // The style as JSON, for the DevBench state.
    std::string StateJson();
}
