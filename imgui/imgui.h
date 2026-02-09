// dear imgui: standalone header file
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#pragma once

// Configuration
// ------------------------

// Define IMGUI_USER_CONFIG to include a user config file
// #define IMGUI_USER_CONFIG "my_imgui_config.h"

//---- Define assertion handler. Defaults to calling assert().
// If your macro uses multiple statements, make sure is enclosed in a 'do { ... } while (0)' block.
#ifndef IM_ASSERT
#include <cassert>
#define IM_ASSERT(_EXPR)  assert(_EXPR)
#endif

//---- Define attributes of all API symbols declarations, e.g. for DLL export/import.
#ifndef IMGUI_API
#define IMGUI_API
#endif

//---- Define attributes of IMGUI_IMPL_API symbols declarations (for backend implementations)
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API  IMGUI_API
#endif

//---- Include necessary headers
#include <float.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <cstring>

//---- Forward declarations
struct ImDrawCmd;       // Draw command list (list of ImDrawCmd)
struct ImDrawData;      // All draw data (shared between multiple windows)
struct ImDrawList;      // Draw command list
struct ImDrawListSharedData; // Shared draw data (font, texture ID)
struct ImFont;          // Font
struct ImFontAtlas;     // Font atlas
struct ImFontConfig;    // Font configuration
struct ImFontGlyph;     // Font glyph
struct ImGuiContext;    // ImGui context (internal state)
struct ImGuiIO;         // Input/output
struct ImGuiInputTextCallbackData; // Input text callback data
struct ImGuiListClipper; // Helper to manually clip large lists
struct ImGuiTextFilter; // Helper to filter text
struct ImGuiTextBuffer; // Helper to build text buffers
struct ImGuiStyle;      // Style
struct ImGuiViewport;   // Viewport
struct ImGuiWindow;     // Window
struct ImGuiWindowClass; // Window class

//---- Basic types
typedef unsigned int ImU32;
typedef unsigned short ImWchar;
typedef signed char ImS8;   // 8-bit signed integer
typedef unsigned char ImU8; // 8-bit unsigned integer
typedef signed short ImS16; // 16-bit signed integer
typedef unsigned short ImU16; // 16-bit unsigned integer
typedef signed int ImS32;   // 32-bit signed integer
typedef signed long long ImS64; // 64-bit signed integer
typedef unsigned long long ImU64; // 64-bit unsigned integer
typedef ImU32 ImGuiID;      // Unique ID used by widgets
typedef int ImGuiDragDropFlags; // Drag and drop flags

//---- Math/Geometry structures
struct ImVec2 {
    float x, y;
    ImVec2() { x = y = 0.0f; }
    ImVec2(float _x, float _y) { x = _x; y = _y; }
    float operator[] (size_t idx) const { return (&x)[idx]; }
    float& operator[] (size_t idx) { return (&x)[idx]; }
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() { x = y = z = w = 0.0f; }
    ImVec4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
    float operator[] (size_t idx) const { return (&x)[idx]; }
    float& operator[] (size_t idx) { return (&x)[idx]; }
};

//---- Enumerations
// Draw list flags
typedef enum ImDrawListFlags_ {
    ImDrawListFlags_None                   = 0,
    ImDrawListFlags_Closed                 = 1 << 0, // Path is closed by default (requires FillConvexPoly)
    ImDrawListFlags_AntiAliasedLines       = 1 << 1, // Enable anti-aliased lines/borders
    ImDrawListFlags_AntiAliasedFill        = 1 << 2, // Enable anti-aliased fills
    ImDrawListFlags_AllowVtxOffset         = 1 << 3, // Allow large meshes (64k+ vertices) with 16-bit indices
    ImDrawListFlags_TextureIDStretched     = 1 << 4, // Texture ID is stretched to cover the entire draw area
} ImDrawListFlags;

// Draw command flags
typedef enum ImDrawCmdFlags_ {
    ImDrawCmdFlags_None                    = 0,
    ImDrawCmdFlags_UserCallbackInvoke      = 1 << 0, // User callback to be called
    ImDrawCmdFlags_UserCallbackHasOutput   = 1 << 1, // User callback may output draw commands
    ImDrawCmdFlags_LayerNext               = 1 << 2, // Switch to next layer
} ImDrawCmdFlags;

// Font Atlas flags
typedef enum ImFontAtlasFlags_ {
    ImFontAtlasFlags_None                  = 0,
    ImFontAtlasFlags_NoPowerOfTwoHeight    = 1 << 0, // Don't round the height to next power of two
    ImFontAtlasFlags_NoMouseCursors        = 1 << 1, // Don't build software mouse cursors into the atlas
    ImFontAtlasFlags_NoBakedLines          = 1 << 2, // Don't build merged lines into the atlas (save a little texture space)
    ImFontAtlasFlags_TextureFromFileHDR    = 1 << 3, // Load texture as HDR
    ImFontAtlasFlags_TexturePixelFormat32  = 1 << 4, // Use 32-bit textures (default is 8-bit)
    ImFontAtlasFlags_TexturePixelFormatRGBA32 = 1 << 5, // Use RGBA32 textures (default is 8-bit)
    ImFontAtlasFlags_TexturePixelFormatRGB32  = 1 << 6, // Use RGB32 textures (default is 8-bit)
    ImFontAtlasFlags_TexturePixelFormatRGBA16 = 1 << 7, // Use RGBA16 textures (default is 8-bit)
    ImFontAtlasFlags_TexturePixelFormatRGBA8  = 1 << 8, // Use RGBA8 textures (default is 8-bit)
} ImFontAtlasFlags;

// Font flags
typedef enum ImFontFlags_ {
    ImFontFlags_None                       = 0,
    ImFontFlags_Italic                     = 1 << 0, // TODO: Not implemented
    ImFontFlags_Bold                       = 1 << 1, // TODO: Not implemented
    ImFontFlags_Underline                  = 1 << 2, // TODO: Not implemented
    ImFontFlags_StrikeOut                  = 1 << 3, // TODO: Not implemented
    ImFontFlags_SuperHinting               = 1 << 4, // TODO: Not implemented
    ImFontFlags_MonoSpace                  = 1 << 5, // TODO: Not implemented
    ImFontFlags_NoKerning                  = 1 << 6, // TODO: Not implemented
    ImFontFlags_OversampleH2               = 1 << 7, // TODO: Not implemented
    ImFontFlags_OversampleV2               = 1 << 8, // TODO: Not implemented
    ImFontFlags_OversampleH4               = 1 << 9, // TODO: Not implemented
    ImFontFlags_OversampleV4               = 1 << 10, // TODO: Not implemented
    ImFontFlags_PixelSnapH                 = 1 << 11, // TODO: Not implemented
    ImFontFlags_PixelSnapV                 = 1 << 12, // TODO: Not implemented
    ImFontFlags_NoRoundAdvance             = 1 << 13, // TODO: Not implemented
    ImFontFlags_NoFitCentering             = 1 << 14, // TODO: Not implemented
    ImFontFlags_Vertical                   = 1 << 15, // TODO: Not implemented
    ImFontFlags_ForceAutoHint              = 1 << 16, // TODO: Not implemented
    ImFontFlags_NoAutoHint                 = 1 << 17, // TODO: Not implemented
    ImFontFlags_GlyphExtraSpacing          = 1 << 18, // TODO: Not implemented
} ImFontFlags;

// Style flags
typedef enum ImGuiStyleFlags_ {
    ImGuiStyleFlags_None                   = 0,
    ImGuiStyleFlags_AlphaPreview           = 1 << 0, // TODO: Not implemented
    ImGuiStyleFlags_AlphaPreviewHalf       = 1 << 1, // TODO: Not implemented
    ImGuiStyleFlags_NoMouseCursorChange    = 1 << 2, // TODO: Not implemented
} ImGuiStyleFlags;

// Config flags
typedef enum ImGuiConfigFlags_ {
    ImGuiConfigFlags_None                  = 0,
    ImGuiConfigFlags_NoMouse               = 1 << 0, // Disable mouse input
    ImGuiConfigFlags_NoMouseCursorChange   = 1 << 1, // Disable mouse cursor changes
    ImGuiConfigFlags_NoKeyboard            = 1 << 2, // Disable keyboard input
    ImGuiConfigFlags_NoGamepad             = 1 << 3, // Disable gamepad input
    ImGuiConfigFlags_NavEnableKeyboard     = 1 << 4, // Enable keyboard navigation
    ImGuiConfigFlags_NavEnableGamepad      = 1 << 5, // Enable gamepad navigation
    ImGuiConfigFlags_NavEnableSetMousePos  = 1 << 6, // Enable mouse position navigation
    ImGuiConfigFlags_NavNoCaptureKeyboard  = 1 << 7, // Disable keyboard capture
    ImGuiConfigFlags_NavNoCaptureMouse     = 1 << 8, // Disable mouse capture
    ImGuiConfigFlags_DockingEnable         = 1 << 9, // Enable docking
    ImGuiConfigFlags_ViewportsEnable       = 1 << 10, // Enable viewports
    ImGuiConfigFlags_ViewportsNoTaskBarIcons = 1 << 11, // Disable task bar icons
    ImGuiConfigFlags_ViewportsNoMerge       = 1 << 12, // Disable viewport merging
    ImGuiConfigFlags_ViewportsNoAutoMerge   = 1 << 13, // Disable automatic viewport merging
    ImGuiConfigFlags_DpiEnableScaleFonts    = 1 << 14, // Enable font scaling
    ImGuiConfigFlags_DpiEnableScaleViewports = 1 << 15, // Enable viewport scaling
    ImGuiConfigFlags_IsSRGB                 = 1 << 16, // TODO: Not implemented
    ImGuiConfigFlags_IsTouchScreen          = 1 << 17, // TODO: Not implemented
} ImGuiConfigFlags;

// Color edit flags
typedef enum ImGuiColorEditFlags_ {
    ImGuiColorEditFlags_None               = 0,
    ImGuiColorEditFlags_NoAlpha            = 1 << 0, // Disable alpha channel
    ImGuiColorEditFlags_NoPicker           = 1 << 1, // Disable color picker
    ImGuiColorEditFlags_NoOptions          = 1 << 2, // Disable options menu
    ImGuiColorEditFlags_NoSmallPreview     = 1 << 3, // Disable small preview
    ImGuiColorEditFlags_NoInputs           = 1 << 4, // Disable inputs
    ImGuiColorEditFlags_NoTooltip          = 1 << 5, // Disable tooltip
    ImGuiColorEditFlags_NoLabel            = 1 << 6, // Disable label
    ImGuiColorEditFlags_NoSidePreview      = 1 << 7, // Disable side preview
    ImGuiColorEditFlags_NoDragDrop         = 1 << 8, // Disable drag and drop
    ImGuiColorEditFlags_AlphaBar           = 1 << 9, // Show alpha bar
    ImGuiColorEditFlags_AlphaPreview       = 1 << 10, // Show alpha preview
    ImGuiColorEditFlags_AlphaPreviewHalf   = 1 << 11, // Show alpha preview as half size
    ImGuiColorEditFlags_HDR                = 1 << 12, // Enable HDR mode
    ImGuiColorEditFlags_DisplayRGB         = 1 << 13, // Display RGB values
    ImGuiColorEditFlags_DisplayHSV         = 1 << 14, // Display HSV values
    ImGuiColorEditFlags_DisplayHex         = 1 << 15, // Display hex values
    ImGuiColorEditFlags_Uint8              = 1 << 16, // Use 8-bit values
    ImGuiColorEditFlags_Float              = 1 << 17, // Use float values
    ImGuiColorEditFlags_PickerHueBar       = 1 << 18, // Use hue bar picker
    ImGuiColorEditFlags_PickerHueWheel     = 1 << 19, // Use hue wheel picker
    ImGuiColorEditFlags_InputRGB           = 1 << 20, // Input RGB values
    ImGuiColorEditFlags_InputHSV           = 1 << 21, // Input HSV values
    ImGuiColorEditFlags_InputHex           = 1 << 22, // Input hex values
} ImGuiColorEditFlags;

// Window flags
typedef enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None                  = 0,
    ImGuiWindowFlags_NoTitleBar            = 1 << 0, // Disable title bar
    ImGuiWindowFlags_NoResize              = 1 << 1, // Disable resizing
    ImGuiWindowFlags_NoMove                = 1 << 2, // Disable moving
    ImGuiWindowFlags_NoScrollbar           = 1 << 3, // Disable scrollbar
    ImGuiWindowFlags_NoScrollWithMouse     = 1 << 4, // Disable scroll with mouse
    ImGuiWindowFlags_NoCollapse            = 1 << 5, // Disable collapsing
    ImGuiWindowFlags_AlwaysAutoResize      = 1 << 6, // Always auto resize
    ImGuiWindowFlags_NoBackground          = 1 << 7, // Disable background
    ImGuiWindowFlags_NoSavedSettings       = 1 << 8, // Disable saved settings
    ImGuiWindowFlags_NoMouseInputs         = 1 << 9, // Disable mouse inputs
    ImGuiWindowFlags_MenuBar               = 1 << 10, // Enable menu bar
    ImGuiWindowFlags_HorizontalScrollbar   = 1 << 11, // Enable horizontal scrollbar
    ImGuiWindowFlags_NoFocusOnAppearing    = 1 << 12, // Disable focus on appearing
    ImGuiWindowFlags_NoBringToFrontOnFocus = 1 << 13, // Disable bring to front on focus
    ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 14, // Always show vertical scrollbar
    ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15, // Always show horizontal scrollbar
    ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16, // Always use window padding
    ImGuiWindowFlags_NoNavInputs           = 1 << 18, // Disable navigation inputs
    ImGuiWindowFlags_NoNavFocus            = 1 << 19, // Disable navigation focus
    ImGuiWindowFlags_UnsavedDocument       = 1 << 20, // Show unsaved document mark
    ImGuiWindowFlags_Tooltip               = 1 << 21, // Tooltip window
    ImGuiWindowFlags_Popup                 = 1 << 22, // Popup window
    ImGuiWindowFlags_Modal                 = 1 << 23, // Modal window
    ImGuiWindowFlags_ChildMenu             = 1 << 24, // Child menu
    ImGuiWindowFlags_Combo                 = 1 << 25, // Combo box
    ImGuiWindowFlags_ListBox               = 1 << 26, // List box
    ImGuiWindowFlags_Child                 = 1 << 27, // Child window
    ImGuiWindowFlags_PopupModal            = ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal, // Popup modal window
} ImGuiWindowFlags;

// TreeNode flags
typedef enum ImGuiTreeNodeFlags_ {
    ImGuiTreeNodeFlags_None                = 0,
    ImGuiTreeNodeFlags_Selected            = 1 << 0, // Selected node
    ImGuiTreeNodeFlags_Framed              = 1 << 1, // Framed node
    ImGuiTreeNodeFlags_AllowItemOverlap    = 1 << 2, // Allow item overlap
    ImGuiTreeNodeFlags_NoTreePushOnOpen    = 1 << 3, // No tree push on open
    ImGuiTreeNodeFlags_NoAutoOpenOnLog     = 1 << 4, // No auto open on log
    ImGuiTreeNodeFlags_DefaultOpen         = 1 << 5, // Default open
    ImGuiTreeNodeFlags_OpenOnDoubleClick   = 1 << 6, // Open on double click
    ImGuiTreeNodeFlags_OpenOnArrow         = 1 << 7, // Open on arrow
    ImGuiTreeNodeFlags_Leaf                = 1 << 8, // Leaf node
    ImGuiTreeNodeFlags_Bullet              = 1 << 9, // Bullet node
    ImGuiTreeNodeFlags_FramePadding        = 1 << 10, // Frame padding
    ImGuiTreeNodeFlags_SpanAvailWidth      = 1 << 11, // Span available width
    ImGuiTreeNodeFlags_SpanFullWidth       = 1 << 12, // Span full width
    ImGuiTreeNodeFlags_NavLeftJumpsBackHere = 1 << 13, // Nav left jumps back here
    ImGuiTreeNodeFlags_CollapsingHeader    = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog, // Collapsing header
} ImGuiTreeNodeFlags;

// Selectable flags
typedef enum ImGuiSelectableFlags_ {
    ImGuiSelectableFlags_None              = 0,
    ImGuiSelectableFlags_DontClosePopups   = 1 << 0, // Don't close popups
    ImGuiSelectableFlags_SpanAllColumns    = 1 << 1, // Span all columns
    ImGuiSelectableFlags_AllowDoubleClick  = 1 << 2, // Allow double click
    ImGuiSelectableFlags_Disabled          = 1 << 3, // Disabled
    ImGuiSelectableFlags_Selected          = 1 << 4, // Selected
} ImGuiSelectableFlags;

// Input text flags
typedef enum ImGuiInputTextFlags_ {
    ImGuiInputTextFlags_None               = 0,
    ImGuiInputTextFlags_CharsDecimal       = 1 << 0, // Allow decimal characters
    ImGuiInputTextFlags_CharsHexadecimal   = 1 << 1, // Allow hexadecimal characters
    ImGuiInputTextFlags_CharsUppercase     = 1 << 2, // Force uppercase
    ImGuiInputTextFlags_CharsNoBlank       = 1 << 3, // No blank characters
    ImGuiInputTextFlags_AutoSelectAll      = 1 << 4, // Auto select all
    ImGuiInputTextFlags_EnterReturnsTrue   = 1 << 5, // Enter returns true
    ImGuiInputTextFlags_CallbackCompletion = 1 << 6, // Callback on completion
    ImGuiInputTextFlags_CallbackHistory    = 1 << 7, // Callback on history
    ImGuiInputTextFlags_CallbackAlways     = 1 << 8, // Callback always
    ImGuiInputTextFlags_CallbackCharFilter = 1 << 9, // Callback on character filter
    ImGuiInputTextFlags_AllowTabInput      = 1 << 10, // Allow tab input
    ImGuiInputTextFlags_CtrlEnterForNewLine = 1 << 11, // Ctrl+Enter for new line
    ImGuiInputTextFlags_NoHorizontalScroll = 1 << 12, // No horizontal scroll
    ImGuiInputTextFlags_AlwaysOverwrite    = 1 << 13, // Always overwrite
    ImGuiInputTextFlags_Readonly           = 1 << 14, // Read only
    ImGuiInputTextFlags_Password           = 1 << 15, // Password
    ImGuiInputTextFlags_NoUndoRedo         = 1 << 16, // No undo/redo
    ImGuiInputTextFlags_CharsScientific    = 1 << 17, // Allow scientific notation
    ImGuiInputTextFlags_CallbackResize     = 1 << 18, // Callback on resize
    ImGuiInputTextFlags_CallbackEdit        = 1 << 19, // Callback on edit
} ImGuiInputTextFlags;

// Slider flags
typedef enum ImGuiSliderFlags_ {
    ImGuiSliderFlags_None                  = 0,
    ImGuiSliderFlags_AlwaysClamp           = 1 << 0, // Always clamp value
    ImGuiSliderFlags_Logarithmic           = 1 << 1, // Logarithmic scale
    ImGuiSliderFlags_NoRoundToFormat       = 1 << 2, // No round to format
    ImGuiSliderFlags_NoInput               = 1 << 3, // No input
    ImGuiSliderFlags_InvalidMask_          = 0x7FFFFFFF,
} ImGuiSliderFlags;

// Table flags
typedef enum ImGuiTableFlags_ {
    ImGuiTableFlags_None                   = 0,
    ImGuiTableFlags_Resizable              = 1 << 0, // Resizable columns
    ImGuiTableFlags_Reorderable            = 1 << 1, // Reorderable columns
    ImGuiTableFlags_Hideable               = 1 << 2, // Hideable columns
    ImGuiTableFlags_Sortable               = 1 << 3, // Sortable columns
    ImGuiTableFlags_NoSavedSettings        = 1 << 4, // No saved settings
    ImGuiTableFlags_ContextMenuInBody      = 1 << 5, // Context menu in body
    ImGuiTableFlags_RowBg                  = 1 << 6, // Row background
    ImGuiTableFlags_BordersInnerH          = 1 << 7, // Inner horizontal borders
    ImGuiTableFlags_BordersOuterH          = 1 << 8, // Outer horizontal borders
    ImGuiTableFlags_BordersInnerV          = 1 << 9, // Inner vertical borders
    ImGuiTableFlags_BordersOuterV          = 1 << 10, // Outer vertical borders
    ImGuiTableFlags_BordersAll             = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV,
    ImGuiTableFlags_NoBordersInBody        = 1 << 11, // No borders in body
    ImGuiTableFlags_NoBordersInBodyUntilResize = 1 << 12, // No borders in body until resize
    ImGuiTableFlags_SizingFixedFit         = 1 << 13, // Fixed fit sizing
    ImGuiTableFlags_SizingFixedSame        = 1 << 14, // Fixed same sizing
    ImGuiTableFlags_SizingStretchProp      = 1 << 15, // Stretch proportional sizing
    ImGuiTableFlags_SizingStretchSame      = 1 << 16, // Stretch same sizing
    ImGuiTableFlags_NoHostExtendX          = 1 << 17, // No host extend X
    ImGuiTableFlags_NoHostExtendY          = 1 << 18, // No host extend Y
    ImGuiTableFlags_NoKeepColumnsVisible   = 1 << 19, // No keep columns visible
    ImGuiTableFlags_PreciseWidths          = 1 << 20, // Precise widths
    ImGuiTableFlags_NoClip                 = 1 << 21, // No clipping
    ImGuiTableFlags_NoHeaders              = 1 << 22, // No headers
    ImGuiTableFlags_NoHeadersWidth         = 1 << 23, // No headers width
    ImGuiTableFlags_NoScrollX              = 1 << 24, // No scroll X
    ImGuiTableFlags_NoScrollY              = 1 << 25, // No scroll Y
    ImGuiTableFlags_SortMulti              = 1 << 26, // Multi sort
    ImGuiTableFlags_SortTristate           = 1 << 27, // Tristate sort
    ImGuiTableFlags_SizingMask_            = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingStretchSame,
} ImGuiTableFlags;

// Table column flags
typedef enum ImGuiTableColumnFlags_ {
    ImGuiTableColumnFlags_None             = 0,
    ImGuiTableColumnFlags_DefaultSort      = 1 << 0, // Default sort column
    ImGuiTableColumnFlags_NoSort           = 1 << 1, // No sort
    ImGuiTableColumnFlags_NoSortAscending  = 1 << 2, // No sort ascending
    ImGuiTableColumnFlags_NoSortDescending = 1 << 3, // No sort descending
    ImGuiTableColumnFlags_NoHeaderLabel    = 1 << 4, // No header label
    ImGuiTableColumnFlags_NoHeaderWidth    = 1 << 5, // No header width
    ImGuiTableColumnFlags_WidthFixed       = 1 << 6, // Fixed width
    ImGuiTableColumnFlags_WidthStretch     = 1 << 7, // Stretch width
    ImGuiTableColumnFlags_WidthAuto        = 1 << 8, // Auto width
    ImGuiTableColumnFlags_NoResize         = 1 << 9, // No resize
    ImGuiTableColumnFlags_NoReorder        = 1 << 10, // No reorder
    ImGuiTableColumnFlags_NoHide           = 1 << 11, // No hide
    ImGuiTableColumnFlags_NoClip           = 1 << 12, // No clip
    ImGuiTableColumnFlags_NoHeaderCenter   = 1 << 13, // No header center
    ImGuiTableColumnFlags_NoHeaderSort     = 1 << 14, // No header sort
    ImGuiTableColumnFlags_NoHeaderContextMenu = 1 << 15, // No header context menu
    ImGuiTableColumnFlags_IsEnabled        = 1 << 16, // Is enabled
    ImGuiTableColumnFlags_IsVisible        = 1 << 17, // Is visible
    ImGuiTableColumnFlags_WidthMask_       = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_WidthAuto,
} ImGuiTableColumnFlags;

// Nav input sources
typedef enum ImGuiInputSource_ {
    ImGuiInputSource_None                  = 0, // No input source
    ImGuiInputSource_Mouse                 = 1, // Mouse input
    ImGuiInputSource_Keyboard              = 2, // Keyboard input
    ImGuiInputSource_Gamepad               = 3, // Gamepad input
    ImGuiInputSource_NavKeyboard           = 4, // Navigation keyboard input
    ImGuiInputSource_NavGamepad            = 5, // Navigation gamepad input
    ImGuiInputSource_NavMouse              = 6, // Navigation mouse input
    ImGuiInputSource_COUNT                 = 7,
} ImGuiInputSource;

// Mouse buttons
typedef enum ImGuiMouseButton_ {
    ImGuiMouseButton_Left                  = 0,
    ImGuiMouseButton_Right                 = 1,
    ImGuiMouseButton_Middle                = 2,
    ImGuiMouseButton_COUNT                 = 3,
} ImGuiMouseButton;

// Mouse cursor
typedef enum ImGuiMouseCursor_ {
    ImGuiMouseCursor_Arrow                = 0,
    ImGuiMouseCursor_TextInput            = 1,
    ImGuiMouseCursor_ResizeAll            = 2,
    ImGuiMouseCursor_ResizeNS             = 3,
    ImGuiMouseCursor_ResizeEW             = 4,
    ImGuiMouseCursor_ResizeNESW           = 5,
    ImGuiMouseCursor_ResizeNWSE           = 6,
    ImGuiMouseCursor_Hand                 = 7,
    ImGuiMouseCursor_NotAllowed           = 8,
    ImGuiMouseCursor_COUNT                = 9,
} ImGuiMouseCursor;

// Color indices
typedef enum ImGuiCol_ {
    ImGuiCol_Text                         = 0,
    ImGuiCol_TextDisabled                 = 1,
    ImGuiCol_WindowBg                     = 2,
    ImGuiCol_ChildBg                      = 3,
    ImGuiCol_PopupBg                      = 4,
    ImGuiCol_Border                       = 5,
    ImGuiCol_BorderShadow                 = 6,
    ImGuiCol_FrameBg                      = 7,
    ImGuiCol_FrameBgHovered               = 8,
    ImGuiCol_FrameBgActive                = 9,
    ImGuiCol_TitleBg                      = 10,
    ImGuiCol_TitleBgActive                = 11,
    ImGuiCol_TitleBgCollapsed             = 12,
    ImGuiCol_MenuBarBg                    = 13,
    ImGuiCol_ScrollbarBg                  = 14,
    ImGuiCol_ScrollbarGrab                = 15,
    ImGuiCol_ScrollbarGrabHovered         = 16,
    ImGuiCol_ScrollbarGrabActive          = 17,
    ImGuiCol_CheckMark                    = 18,
    ImGuiCol_SliderGrab                   = 19,
    ImGuiCol_SliderGrabActive             = 20,
    ImGuiCol_Button                       = 21,
    ImGuiCol_ButtonHovered                = 22,
    ImGuiCol_ButtonActive                 = 23,
    ImGuiCol_Header                       = 24,
    ImGuiCol_HeaderHovered                = 25,
    ImGuiCol_HeaderActive                 = 26,
    ImGuiCol_Separator                    = 27,
    ImGuiCol_SeparatorHovered             = 28,
    ImGuiCol_SeparatorActive              = 29,
    ImGuiCol_ResizeGrip                   = 30,
    ImGuiCol_ResizeGripHovered            = 31,
    ImGuiCol_ResizeGripActive             = 32,
    ImGuiCol_Tab                          = 33,
    ImGuiCol_TabHovered                   = 34,
    ImGuiCol_TabActive                    = 35,
    ImGuiCol_TabUnfocused                 = 36,
    ImGuiCol_TabUnfocusedActive           = 37,
    ImGuiCol_DockingPreview               = 38,
    ImGuiCol_DockingEmptyBg               = 39,
    ImGuiCol_PlotLines                    = 40,
    ImGuiCol_PlotLinesHovered             = 41,
    ImGuiCol_PlotHistogram                = 42,
    ImGuiCol_PlotHistogramHovered         = 43,
    ImGuiCol_TableHeaderBg                = 44,
    ImGuiCol_TableBorderStrong            = 45,
    ImGuiCol_TableBorderLight             = 46,
    ImGuiCol_TableRowBg                   = 47,
    ImGuiCol_TableRowBgAlt                = 48,
    ImGuiCol_TextSelectedBg               = 49,
    ImGuiCol_DragDropTarget               = 50,
    ImGuiCol_NavHighlight                 = 51,
    ImGuiCol_NavWindowingHighlight        = 52,
    ImGuiCol_NavWindowingDimBg            = 53,
    ImGuiCol_ModalWindowDimBg             = 54,
    ImGuiCol_COUNT                        = 55,
} ImGuiCol;

// Key codes
typedef enum ImGuiKey_ {
    ImGuiKey_None                          = 0,
    ImGuiKey_Tab                           = 1,
    ImGuiKey_LeftArrow                     = 2,
    ImGuiKey_RightArrow                    = 3,
    ImGuiKey_UpArrow                       = 4,
    ImGuiKey_DownArrow                     = 5,
    ImGuiKey_PageUp                        = 6,
    ImGuiKey_PageDown                      = 7,
    ImGuiKey_Home                          = 8,
    ImGuiKey_End                           = 9,
    ImGuiKey_Insert                        = 10,
    ImGuiKey_Delete                        = 11,
    ImGuiKey_Backspace                     = 12,
    ImGuiKey_Space                         = 13,
    ImGuiKey_Enter                         = 14,
    ImGuiKey_Escape                        = 15,
    ImGuiKey_LeftCtrl                      = 16,
    ImGuiKey_LeftShift                     = 17,
    ImGuiKey_LeftAlt                       = 18,
    ImGuiKey_LeftSuper                     = 19,
    ImGuiKey_RightCtrl                     = 20,
    ImGuiKey_RightShift                    = 21,
    ImGuiKey_RightAlt                      = 22,
    ImGuiKey_RightSuper                    = 23,
    ImGuiKey_Menu                          = 24,
    ImGuiKey_0                             = 25,
    ImGuiKey_1                             = 26,
    ImGuiKey_2                             = 27,
    ImGuiKey_3                             = 28,
    ImGuiKey_4                             = 29,
    ImGuiKey_5                             = 30,
    ImGuiKey_6                             = 31,
    ImGuiKey_7                             = 32,
    ImGuiKey_8                             = 33,
    ImGuiKey_9                             = 34,
    ImGuiKey_A                             = 35,
    ImGuiKey_B                             = 36,
    ImGuiKey_C                             = 37,
    ImGuiKey_D                             = 38,
    ImGuiKey_E                             = 39,
    ImGuiKey_F                             = 40,
    ImGuiKey_G                             = 41,
    ImGuiKey_H                             = 42,
    ImGuiKey_I                             = 43,
    ImGuiKey_J                             = 44,
    ImGuiKey_K                             = 45,
    ImGuiKey_L                             = 46,
    ImGuiKey_M                             = 47,
    ImGuiKey_N                             = 48,
    ImGuiKey_O                             = 49,
    ImGuiKey_P                             = 50,
    ImGuiKey_Q                             = 51,
    ImGuiKey_R                             = 52,
    ImGuiKey_S                             = 53,
    ImGuiKey_T                             = 54,
    ImGuiKey_U                             = 55,
    ImGuiKey_V                             = 56,
    ImGuiKey_W                             = 57,
    ImGuiKey_X                             = 58,
    ImGuiKey_Y                             = 59,
    ImGuiKey_Z                             = 60,
    ImGuiKey_F1                            = 61,
    ImGuiKey_F2                            = 62,
    ImGuiKey_F3                            = 63,
    ImGuiKey_F4                            = 64,
    ImGuiKey_F5                            = 65,
    ImGuiKey_F6                            = 66,
    ImGuiKey_F7                            = 67,
    ImGuiKey_F8                            = 68,
    ImGuiKey_F9                            = 69,
    ImGuiKey_F10                           = 70,
    ImGuiKey_F11                           = 71,
    ImGuiKey_F12                           = 72,
    ImGuiKey_Apostrophe                    = 73,
    ImGuiKey_Comma                         = 74,
    ImGuiKey_Minus                         = 75,
    ImGuiKey_Period                        = 76,
    ImGuiKey_Slash                         = 77,
    ImGuiKey_Semicolon                     = 78,
    ImGuiKey_Equal                         = 79,
    ImGuiKey_BracketLeft                   = 80,
    ImGuiKey_Backslash                     = 81,
    ImGuiKey_BracketRight                  = 82,
    ImGuiKey_GraveAccent                   = 83,
    ImGuiKey_CapsLock                      = 84,
    ImGuiKey_ScrollLock                    = 85,
    ImGuiKey_NumLock                       = 86,
    ImGuiKey_PrintScreen                   = 87,
    ImGuiKey_Pause                         = 88,
    ImGuiKey_Keypad0                       = 89,
    ImGuiKey_Keypad1                       = 90,
    ImGuiKey_Keypad2                       = 91,
    ImGuiKey_Keypad3                       = 92,
    ImGuiKey_Keypad4                       = 93,
    ImGuiKey_Keypad5                       = 94,
    ImGuiKey_Keypad6                       = 95,
    ImGuiKey_Keypad7                       = 96,
    ImGuiKey_Keypad8                       = 97,
    ImGuiKey_Keypad9                       = 98,
    ImGuiKey_KeypadDecimal                 = 99,
    ImGuiKey_KeypadDivide                  = 100,
    ImGuiKey_KeypadMultiply                = 101,
    ImGuiKey_KeypadSubtract                = 102,
    ImGuiKey_KeypadAdd                     = 103,
    ImGuiKey_KeypadEnter                   = 104,
    ImGuiKey_KeypadEqual                   = 105,
    ImGuiKey_COUNT                         = 106,
} ImGuiKey;

// Gamepad buttons
typedef enum ImGuiGamepadKey_ {
    ImGuiGamepadKey_DpadUp                 = 0,
    ImGuiGamepadKey_DpadDown               = 1,
    ImGuiGamepadKey_DpadLeft               = 2,
    ImGuiGamepadKey_DpadRight              = 3,
    ImGuiGamepadKey_A                      = 4,
    ImGuiGamepadKey_B                      = 5,
    ImGuiGamepadKey_X                      = 6,
    ImGuiGamepadKey_Y                      = 7,
    ImGuiGamepadKey_LeftBumper             = 8,
    ImGuiGamepadKey_RightBumper            = 9,
    ImGuiGamepadKey_LeftStick              = 10,
    ImGuiGamepadKey_RightStick             = 11,
    ImGuiGamepadKey_Start                  = 12,
    ImGuiGamepadKey_Back                   = 13,
    ImGuiGamepadKey_COUNT                  = 14,
} ImGuiGamepadKey;

// User data types
//------------------------

// Vector types
struct ImVec2 {
    float x, y;
    ImVec2() : x(0.0f), y(0.0f) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec3 {
    float x, y, z;
    ImVec3() : x(0.0f), y(0.0f), z(0.0f) {}
    ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    ImVec4(const ImVec3& v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {}
};

// ImVector template class
template<typename T>
struct ImVector {
    T* Data;
    int Size;
    int Capacity;
    
    ImVector() : Data(nullptr), Size(0), Capacity(0) {}
    ~ImVector() { if (Data) delete[] Data; }
    
    void push_back(const T& v) {
        if (Size >= Capacity) {
            Capacity = Capacity ? Capacity * 2 : 8;
            T* new_data = new T[Capacity];
            if (Data) {
                memcpy(new_data, Data, Size * sizeof(T));
                delete[] Data;
            }
            Data = new_data;
        }
        Data[Size++] = v;
    }
};

// ImGuiMod flags
typedef enum ImGuiMod_ {
    ImGuiMod_None   = 0,
    ImGuiMod_Ctrl   = 1 << 0,
    ImGuiMod_Shift  = 1 << 1,
    ImGuiMod_Alt    = 1 << 2,
    ImGuiMod_Super  = 1 << 3,
} ImGuiModFlags;

// Color types
typedef ImVec4 ImColor;

// Draw vertex
struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
    ImDrawVert() : pos(0, 0), uv(0, 0), col(0) {}
    ImDrawVert(const ImVec2& _pos, const ImVec2& _uv, ImU32 _col) : pos(_pos), uv(_uv), col(_col) {}
};

// Draw index
typedef uint16_t ImDrawIdx;

// Draw command
struct ImDrawCmd {
    ImDrawCmd() : clip_rect(0, 0, 0, 0), texture_id(nullptr), vtx_offset(0), idx_offset(0), elem_count(0), flags(0), user_callback(nullptr), user_callback_data(nullptr) {}
    ImVec4 clip_rect;
    void* texture_id;
    uint32_t vtx_offset;
    uint32_t idx_offset;
    uint32_t elem_count;
    ImDrawCmdFlags flags;
    void (*user_callback)(const ImDrawList* parent_list, const ImDrawCmd* cmd);
    void* user_callback_data;
};

// Draw list shared data
struct ImDrawListSharedData {
    ImDrawListSharedData() : tex_uvscale(1.0f, 1.0f), tex_uvoffset(0.0f, 0.0f), clip_rectFullscreen(0.0f, 0.0f, 0.0f, 0.0f) {}
    ImVec2 tex_uvscale;
    ImVec2 tex_uvoffset;
    ImVec4 clip_rectFullscreen;
};

// Draw list
struct ImDrawList {
    ImDrawList() : vtx_buffer(), idx_buffer(), cmd_buffer(), flags(0), texture_id(nullptr), vtx_current_idx(0), idx_current_idx(0), clip_rect_stack(), texture_id_stack(), is_channels_split(false), channels(), channels_current(0), shared_data(nullptr) {}
    std::vector<ImDrawVert> vtx_buffer;
    std::vector<ImDrawIdx> idx_buffer;
    std::vector<ImDrawCmd> cmd_buffer;
    ImDrawListFlags flags;
    void* texture_id;
    uint32_t vtx_current_idx;
    uint32_t idx_current_idx;
    std::vector<ImVec4> clip_rect_stack;
    std::vector<void*> texture_id_stack;
    bool is_channels_split;
    std::vector<ImDrawList*> channels;
    int channels_current;
    ImDrawListSharedData* shared_data;
};

// Draw data
struct ImDrawData {
    ImDrawData() : valid(false), cmd_lists(nullptr), cmd_lists_count(0), total_idx_count(0), total_vtx_count(0), display_pos(0, 0), display_size(0, 0), framebuffer_scale(1, 1) {}
    bool valid;
    ImDrawList** cmd_lists;
    int cmd_lists_count;
    int total_idx_count;
    int total_vtx_count;
    ImVec2 display_pos;
    ImVec2 display_size;
    ImVec2 framebuffer_scale;
};

// Font glyph
struct ImFontGlyph {
    ImFontGlyph() : codepoint(0), advance_x(0.0f), x0(0.0f), y0(0.0f), x1(0.0f), y1(0.0f), u0(0.0f), v0(0.0f), u1(0.0f), v1(0.0f) {}
    ImWchar codepoint;
    float advance_x;
    float x0, y0, x1, y1;
    float u0, v0, u1, v1;
};

// Font configuration
struct ImFontConfig {
    ImFontConfig() : Name(nullptr), SizePixels(0.0f), OversampleH(2), OversampleV(1), PixelSnapH(false), GlyphExtraSpacing(0, 0), GlyphOffset(0, 0), GlyphRanges(nullptr), MergeMode(false), FontData(nullptr), FontDataSize(0), FontDataOwnedByAtlas(false), FontNo(nullptr), FontBuilderFlags(0), RasterizerMultiply(1.0f), RasterizerFlags(0), EllipsisChar(0x0085) {}
    const char* Name;
    float SizePixels;
    int OversampleH;
    int OversampleV;
    bool PixelSnapH;
    ImVec2 GlyphExtraSpacing;
    ImVec2 GlyphOffset;
    const ImWchar* GlyphRanges;
    bool MergeMode;
    unsigned char* FontData;
    int FontDataSize;
    bool FontDataOwnedByAtlas;
    ImFont* FontNo;
    int FontBuilderFlags;
    float RasterizerMultiply;
    int RasterizerFlags;
    ImWchar EllipsisChar;
};

// Custom rectangle for font atlas
struct ImFontAtlasCustomRect {
    unsigned short Width, Height;
    unsigned short X, Y;
    unsigned int GlyphID;
    float AdvanceX;
    float PosX, PosY;
    float U0, V0, U1, V1;
    bool GlyphColored;
    void* UserData;
    
    ImFontAtlasCustomRect() { Width = Height = 0; X = Y = 0xFFFF; GlyphID = 0; AdvanceX = 0.0f; PosX = PosY = U0 = V0 = U1 = V1 = 0.0f; GlyphColored = false; UserData = nullptr; }
    bool IsPacked() const { return X != 0xFFFF; }
};

// Font atlas
struct ImFontAtlas {
    ImFontAtlas() : TexID(nullptr), TexWidth(0), TexHeight(0), TexPixels(nullptr), TexPixelsAlpha8(nullptr), TexPixelsRgba32(nullptr), ConfigDataSize(0), ConfigData(nullptr), GlyphRangesBuild(nullptr), CustomRects(), CustomRectsNames(), Flags(0), Locked(false) {}
    void* TexID;
    int TexWidth, TexHeight;
    unsigned char* TexPixels;
    unsigned char* TexPixelsAlpha8;
    ImU32* TexPixelsRgba32;
    int ConfigDataSize;
    void* ConfigData;
    ImVector<ImWchar>* GlyphRangesBuild;
    ImVector<ImFontAtlasCustomRect> CustomRects;
    ImVector<const char*> CustomRectsNames;
    ImFontAtlasFlags Flags;
    bool Locked;
};

// Font
struct ImFont {
    ImFont() : ConfigData(nullptr), FallbackFont(nullptr), AtlasTexID(nullptr), TexUvScale(1, 1), TexUvOffset(0, 0), FontSize(0), Ascent(0), Descent(0), Glyphs(), IndexAdvanceX(), IndexLookup(), FallbackAdvanceX(0), Scale(1), MetricsTotalSurface(0), ConfigDataCount(0) {}
    const ImFontConfig* ConfigData;
    ImFont* FallbackFont;
    void* AtlasTexID;
    ImVec2 TexUvScale;
    ImVec2 TexUvOffset;
    float FontSize;
    float Ascent;
    float Descent;
    ImVector<ImFontGlyph> Glyphs;
    ImVector<float> IndexAdvanceX;
    ImVector<ImWchar> IndexLookup;
    float FallbackAdvanceX;
    float Scale;
    float MetricsTotalSurface;
    int ConfigDataCount;
};

// ImGui context
struct ImGuiContext {
    ImGuiContext() : IO(), Style(), Fonts(), Windows(), CurrentWindow(nullptr), HoveredWindow(nullptr), HoveredId(nullptr), ActiveId(nullptr), ActiveIdWindow(nullptr), FocusWindow(nullptr), DragDropActive(false), DragDropPayload(nullptr), DragDropSourceFlags(0), DragDropTarget(nullptr), MouseCursor(ImGuiMouseCursor_Arrow), MouseCursorPrevious(ImGuiMouseCursor_Arrow), MousePos(0, 0), MousePosPrev(0, 0), MouseDelta(0, 0), MouseClicked[ImGuiMouseButton_COUNT](false), MouseClickedPos[ImGuiMouseButton_COUNT](0, 0), MouseClickedTime[ImGuiMouseButton_COUNT](0), MouseDoubleClicked[ImGuiMouseButton_COUNT](false), MouseReleased[ImGuiMouseButton_COUNT](false), MouseDown[ImGuiMouseButton_COUNT](false), MouseDownDuration[ImGuiMouseButton_COUNT](0), MouseDownDurationPrev[ImGuiMouseButton_COUNT](0), MouseDownWasDoubleClick[ImGuiMouseButton_COUNT](false), MouseDragMaxDistanceSqr[ImGuiMouseButton_COUNT](0), MouseDragThreshold(0), KeyDown[ImGuiKey_COUNT](false), KeyPressed[ImGuiKey_COUNT](false), KeyMods(0), KeyCtrl(false), KeyShift(false), KeyAlt(false), KeySuper(false), InputQueueCharacters(), InputTextState(nullptr), ScrollbarClickDelta(0), Time(0), DeltaTime(0), FrameCount(0), WantCaptureMouse(false), WantCaptureKeyboard(false), WantTextInput(false), WantSetMousePos(false), WantSaveIniSettings(false), NavActive(false), NavInputSource(ImGuiInputSource_None), NavId(nullptr), NavIdPrev(nullptr), NavIdTabCounter(0), NavIdSubMenuDepth(0), NavMousePos(0, 0), NavMousePosPrev(0, 0), NavScoringNoResults(false), NavMoveScoringItems(), NavMoveScoringRefMousePos(0, 0), NavWindowingTarget(nullptr), NavWindowingHighlightFlags(0), NavWindowingToggleLayer(false), Viewports(), PlatformIO(nullptr), DrawData(nullptr), FrameScopePushedIdStack(nullptr), FrameScopePushedIdLast(nullptr), ColorModifiers(), StyleModifiers(), FontStack(), FontSizeStack(), ItemFlagsStack(), ItemWidthStack(), TextWrapPosStack(), WindowStack(), WindowPosStack(), WindowSizeStack(), WindowScrollStack(), ColumnsStack(), GroupStack(), TabBarStack(), MenuBarStack(), TooltipOverrideCount(0), TooltipOverrideNextWindowFlags(0), TooltipOverridePos(FLT_MAX, FLT_MAX), SettingsLoaded(false), SettingsDirtyTimer(0), IniSavingToMemory(false), IniSavingToMemoryBuffer(nullptr), IniSavingToMemorySize(0), PrivateUserData(nullptr) {}
    ImGuiIO IO;
    ImGuiStyle Style;
    ImFontAtlas Fonts;
    ImVector<ImGuiWindow*> Windows;
    ImGuiWindow* CurrentWindow;
    ImGuiWindow* HoveredWindow;
    ImGuiID HoveredId;
    ImGuiID ActiveId;
    ImGuiWindow* ActiveIdWindow;
    ImGuiWindow* FocusWindow;
    bool DragDropActive;
    ImGuiPayload* DragDropPayload;
    ImGuiDragDropFlags DragDropSourceFlags;
    void* DragDropTarget;
    ImGuiMouseCursor MouseCursor;
    ImGuiMouseCursor MouseCursorPrevious;
    ImVec2 MousePos;
    ImVec2 MousePosPrev;
    ImVec2 MouseDelta;
    bool MouseClicked[ImGuiMouseButton_COUNT];
    ImVec2 MouseClickedPos[ImGuiMouseButton_COUNT];
    float MouseClickedTime[ImGuiMouseButton_COUNT];
    bool MouseDoubleClicked[ImGuiMouseButton_COUNT];
    bool MouseReleased[ImGuiMouseButton_COUNT];
    bool MouseDown[ImGuiMouseButton_COUNT];
    float MouseDownDuration[ImGuiMouseButton_COUNT];
    float MouseDownDurationPrev[ImGuiMouseButton_COUNT];
    bool MouseDownWasDoubleClick[ImGuiMouseButton_COUNT];
    float MouseDragMaxDistanceSqr[ImGuiMouseButton_COUNT];
    float MouseDragThreshold;
    bool KeyDown[ImGuiKey_COUNT];
    bool KeyPressed[ImGuiKey_COUNT];
    ImGuiKeyModFlags KeyMods;
    bool KeyCtrl;
    bool KeyShift;
    bool KeyAlt;
    bool KeySuper;
    ImVector<ImWchar> InputQueueCharacters;
    ImGuiInputTextState* InputTextState;
    float ScrollbarClickDelta;
    float Time;
    float DeltaTime;
    int FrameCount;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    bool WantTextInput;
    bool WantSetMousePos;
    bool WantSaveIniSettings;
    bool NavActive;
    ImGuiInputSource NavInputSource;
    ImGuiID NavId;
    ImGuiID NavIdPrev;
    int NavIdTabCounter;
    int NavIdSubMenuDepth;
    ImVec2 NavMousePos;
    ImVec2 NavMousePosPrev;
    bool NavScoringNoResults;
    ImVector<ImGuiNavItemData> NavMoveScoringItems;
    ImVec2 NavMoveScoringRefMousePos;
    ImGuiWindow* NavWindowingTarget;
    ImGuiWindowFlags NavWindowingHighlightFlags;
    bool NavWindowingToggleLayer;
    ImVector<ImGuiViewport*> Viewports;
    ImGuiPlatformIO* PlatformIO;
    ImDrawData* DrawData;
    ImGuiID* FrameScopePushedIdStack;
    ImGuiID* FrameScopePushedIdLast;
    ImVector<ImVec4> ColorModifiers;
    ImVector<ImGuiStyle> StyleModifiers;
    ImVector<ImFont*> FontStack;
    ImVector<float> FontSizeStack;
    ImVector<ImGuiItemFlags> ItemFlagsStack;
    ImVector<float> ItemWidthStack;
    ImVector<float> TextWrapPosStack;
    ImVector<ImGuiWindow*> WindowStack;
    ImVector<ImVec2> WindowPosStack;
    ImVector<ImVec2> WindowSizeStack;
    ImVector<ImVec2> WindowScrollStack;
    ImVector<ImGuiColumnsSet> ColumnsStack;
    ImVector<ImGuiGroupData> GroupStack;
    ImVector<ImGuiTabBar*> TabBarStack;
    ImVector<ImGuiMenuBarData> MenuBarStack;
    int TooltipOverrideCount;
    ImGuiWindowFlags TooltipOverrideNextWindowFlags;
    ImVec2 TooltipOverridePos;
    bool SettingsLoaded;
    float SettingsDirtyTimer;
    bool IniSavingToMemory;
    char* IniSavingToMemoryBuffer;
    int IniSavingToMemorySize;
    void* PrivateUserData;
};

// Backend flags
typedef enum ImGuiBackendFlags_ {
    ImGuiBackendFlags_None                 = 0,
    ImGuiBackendFlags_HasMouseCursors      = 1 << 0,
    ImGuiBackendFlags_HasSetMousePos       = 1 << 1,
    ImGuiBackendFlags_PlatformHasViewports = 1 << 2,
    ImGuiBackendFlags_HasMouseHoveredViewport = 1 << 3,
    ImGuiBackendFlags_RendererHasVtxOffset  = 1 << 4,
    ImGuiBackendFlags_RendererHasViewports  = 1 << 5,
    ImGuiBackendFlags_RendererHasTextureAtlas = 1 << 6,
} ImGuiBackendFlags;

// Viewport flags
typedef enum ImGuiViewportFlags_ {
    ImGuiViewportFlags_None                = 0,
    ImGuiViewportFlags_NoDecoration        = 1 << 0,
    ImGuiViewportFlags_NoTaskBarIcon       = 1 << 1,
    ImGuiViewportFlags_NoFocusOnAppearing  = 1 << 2,
    ImGuiViewportFlags_NoFocusOnClick      = 1 << 3,
    ImGuiViewportFlags_NoInputs            = 1 << 4,
    ImGuiViewportFlags_NoRendererClear     = 1 << 5,
    ImGuiViewportFlags_TopMost             = 1 << 6,
    ImGuiViewportFlags_MenuBar             = 1 << 7,
    ImGuiViewportFlags_TabBar              = 1 << 8,
    ImGuiViewportFlags_DesktopMain         = 1 << 9,
    ImGuiViewportFlags_Secondary           = 1 << 10,
    ImGuiViewportFlags_Tooltip             = 1 << 11,
    ImGuiViewportFlags_Popup               = 1 << 12,
    ImGuiViewportFlags_Modal               = 1 << 13,
    ImGuiViewportFlags_ChildMenu           = 1 << 14,
    ImGuiViewportFlags_TableHeader         = 1 << 15,
    ImGuiViewportFlags_TableRow            = 1 << 16,
} ImGuiViewportFlags;

// ImGui IO
struct ImGuiIO {
    ImGuiIO() : ConfigFlags(0), ConfigInputTextCursorBlinkRate(0.8f), ConfigInputTextEnterKeepActive(false), ConfigWindowsResizeFromEdges(false), ConfigWindowsMoveFromTitleBarOnly(false), ConfigWindowsMemoryCompactTimer(60.0f), ConfigDockingAlwaysTabBar(false), ConfigDockingTransparentPayload(true), ConfigViewportsNoDecoration(false), ConfigViewportsNoTaskBarIcon(false), ConfigViewportsNoAutoMerge(false), ConfigViewportsNoDefaultParent(false), ConfigViewportsNoImGuiWindow(false), ConfigDpiScaleFunc(nullptr), ConfigDpiScaleAllowNone(false), Fonts(), FontDefault(nullptr), FontGlobalScale(1.0f), FontAllowUserScaling(false), DisplaySize(0, 0), DisplayFramebufferScale(1, 1), DeltaTime(1.0f / 60.0f), IniFilename(nullptr), LogFilename(nullptr), KeyMap[ImGuiKey_COUNT](0), KeysDown[ImGuiKey_COUNT](false), KeysDownDuration[ImGuiKey_COUNT](0), KeysDownDurationPrev[ImGuiKey_COUNT](0), KeysPressed[ImGuiKey_COUNT](false), MousePos(0, 0), MousePosPrev(0, 0), MouseDelta(0, 0), MouseWheel(0), MouseWheelH(0), MouseClicked[ImGuiMouseButton_COUNT](false), MouseClickedPos[ImGuiMouseButton_COUNT](0, 0), MouseClickedTime[ImGuiMouseButton_COUNT](0), MouseDoubleClicked[ImGuiMouseButton_COUNT](false), MouseReleased[ImGuiMouseButton_COUNT](false), MouseDown[ImGuiMouseButton_COUNT](false), MouseDownDuration[ImGuiMouseButton_COUNT](0), MouseDownDurationPrev[ImGuiMouseButton_COUNT](0), MouseDownWasDoubleClick[ImGuiMouseButton_COUNT](false), MouseDragMaxDistanceSqr[ImGuiMouseButton_COUNT](0), MouseCursor(ImGuiMouseCursor_None), WantCaptureMouse(false), WantCaptureKeyboard(false), WantTextInput(false), WantSetMousePos(false), WantSaveIniSettings(false), NavActive(false), NavVisible(false), Framerate(0.0f), MetricsRenderVertices(0), MetricsRenderIndices(0), MetricsRenderWindows(0), MetricsActiveWindows(0), MetricsActiveAllocations(0), UserData(nullptr), PlatformUserData(nullptr), BackendPlatformName(nullptr), BackendRendererName(nullptr), BackendAPIVersion(0), BackendFlags(0), RenderDrawData(nullptr), SetClipboardTextFn(nullptr), GetClipboardTextFn(nullptr), ClipboardUserData(nullptr), ImeSetInputScreenPosFn(nullptr), ImeWindowHandle(nullptr), GetMouseCursorPosFn(nullptr), SetMouseCursorPosFn(nullptr), SetMouseCursorFn(nullptr), UpdateMousePosFn(nullptr), UpdateMouseButtonsFn(nullptr), UpdateMouseWheelFn(nullptr), UpdateKeyboardFn(nullptr), UpdateGamepadFn(nullptr), UpdateImeFn(nullptr), InputQueueCharacters() {}
    ImGuiConfigFlags ConfigFlags;
    float ConfigInputTextCursorBlinkRate;
    bool ConfigInputTextEnterKeepActive;
    bool ConfigWindowsResizeFromEdges;
    bool ConfigWindowsMoveFromTitleBarOnly;
    float ConfigWindowsMemoryCompactTimer;
    bool ConfigDockingAlwaysTabBar;
    bool ConfigDockingTransparentPayload;
    bool ConfigViewportsNoDecoration;
    bool ConfigViewportsNoTaskBarIcon;
    bool ConfigViewportsNoAutoMerge;
    bool ConfigViewportsNoDefaultParent;
    bool ConfigViewportsNoImGuiWindow;
    float (*ConfigDpiScaleFunc)(float);
    bool ConfigDpiScaleAllowNone;
    ImFontAtlas Fonts;
    ImFont* FontDefault;
    float FontGlobalScale;
    bool FontAllowUserScaling;
    ImVec2 DisplaySize;
    ImVec2 DisplayFramebufferScale;
    float DeltaTime;
    const char* IniFilename;
    const char* LogFilename;
    int KeyMap[ImGuiKey_COUNT];
    bool KeysDown[ImGuiKey_COUNT];
    float KeysDownDuration[ImGuiKey_COUNT];
    float KeysDownDurationPrev[ImGuiKey_COUNT];
    bool KeysPressed[ImGuiKey_COUNT];
    ImVec2 MousePos;
    ImVec2 MousePosPrev;
    ImVec2 MouseDelta;
    float MouseWheel;
    float MouseWheelH;
    bool MouseClicked[ImGuiMouseButton_COUNT];
    ImVec2 MouseClickedPos[ImGuiMouseButton_COUNT];
    float MouseClickedTime[ImGuiMouseButton_COUNT];
    bool MouseDoubleClicked[ImGuiMouseButton_COUNT];
    bool MouseReleased[ImGuiMouseButton_COUNT];
    bool MouseDown[ImGuiMouseButton_COUNT];
    float MouseDownDuration[ImGuiMouseButton_COUNT];
    float MouseDownDurationPrev[ImGuiMouseButton_COUNT];
    bool MouseDownWasDoubleClick[ImGuiMouseButton_COUNT];
    float MouseDragMaxDistanceSqr[ImGuiMouseButton_COUNT];
    ImGuiMouseCursor MouseCursor;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    bool WantTextInput;
    bool WantSetMousePos;
    bool WantSaveIniSettings;
    bool NavActive;
    bool NavVisible;
    float Framerate;
    int MetricsRenderVertices;
    int MetricsRenderIndices;
    int MetricsRenderWindows;
    int MetricsActiveWindows;
    int MetricsActiveAllocations;
    void* UserData;
    void* PlatformUserData;
    const char* BackendPlatformName;
    const char* BackendRendererName;
    int BackendAPIVersion;
    ImGuiBackendFlags BackendFlags;
    ImDrawData* RenderDrawData;
    void (*SetClipboardTextFn)(void*, const char*);
    const char* (*GetClipboardTextFn)(void*);
    void* ClipboardUserData;
    void (*ImeSetInputScreenPosFn)(void*, ImVec2);
    void* ImeWindowHandle;
    ImVector<ImWchar> InputQueueCharacters;
    
    // AddInputCharacter function
    void AddInputCharacter(unsigned short c) {
        InputQueueCharacters.push_back(c);
    }
};

// Direction
typedef enum ImGuiDir_ {
    ImGuiDir_None   = -1,
    ImGuiDir_Left   = 0,
    ImGuiDir_Right  = 1,
    ImGuiDir_Up     = 2,
    ImGuiDir_Down   = 3,
} ImGuiDir;

// ImGui style
struct ImGuiStyle {
    ImGuiStyle() : Alpha(1.0f), DisabledAlpha(0.5f), WindowPadding(8, 8), WindowRounding(0), WindowBorderSize(1), WindowMinSize(32, 32), WindowTitleAlign(0.0f, 0.5f), WindowMenuButtonPosition(ImGuiDir_None), ChildRounding(0), ChildBorderSize(1), PopupRounding(0), PopupBorderSize(1), FramePadding(4, 3), FrameRounding(0), FrameBorderSize(0), ItemSpacing(8, 4), ItemInnerSpacing(4, 4), TouchExtraPadding(0, 0), IndentSpacing(21), ScrollbarSize(16), ScrollbarRounding(9), GrabMinSize(10), GrabRounding(0), TabRounding(0), TabBorderSize(0), TabMinWidthForCloseButton(0), ColorButtonPosition(ImGuiDir_Right), ButtonTextAlign(0.5f, 0.5f), SelectableTextAlign(0.0f, 0.0f), DisplayWindowPadding(19, 19), DisplaySafeAreaPadding(3, 3), MouseCursorScale(1.0f), AntiAliasedLines(true), AntiAliasedFill(true), CurveTessellationTol(1.25f), CircleTessellationMaxError(0.3f), Colors() {
        Colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        Colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        Colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        Colors[ImGuiCol_ChildBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.00f);
        Colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        Colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        Colors[ImGuiCol_FrameBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        Colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        Colors[ImGuiCol_CheckMark]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        Colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        Colors[ImGuiCol_Button]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        Colors[ImGuiCol_ButtonActive]           = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        Colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        Colors[ImGuiCol_HeaderActive]           = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        Colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.52f, 0.52f, 0.60f, 0.60f);
        Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
        Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
        Colors[ImGuiCol_Tab]                    = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
        Colors[ImGuiCol_TabHovered]             = ImVec4(0.33f, 0.33f, 0.34f, 0.83f);
        Colors[ImGuiCol_TabActive]              = ImVec4(0.23f, 0.23f, 0.24f, 0.83f);
        Colors[ImGuiCol_TabUnfocused]           = ImVec4(0.08f, 0.08f, 0.09f, 0.50f);
        Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.13f, 0.13f, 0.14f, 0.50f);
        Colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.33f, 0.34f, 1.00f);
        Colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        Colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        Colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        Colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        Colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        Colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        Colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        Colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        Colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        Colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        Colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }
    float Alpha;
    float DisabledAlpha;
    ImVec2 WindowPadding;
    float WindowRounding;
    float WindowBorderSize;
    ImVec2 WindowMinSize;
    ImVec2 WindowTitleAlign;
    int WindowMenuButtonPosition;
    float ChildRounding;
    float ChildBorderSize;
    float PopupRounding;
    float PopupBorderSize;
    ImVec2 FramePadding;
    float FrameRounding;
    float FrameBorderSize;
    ImVec2 ItemSpacing;
    ImVec2 ItemInnerSpacing;
    ImVec2 TouchExtraPadding;
    float IndentSpacing;
    float ScrollbarSize;
    float ScrollbarRounding;
    float GrabMinSize;
    float GrabRounding;
    float TabRounding;
    float TabBorderSize;
    float TabMinWidthForCloseButton;
    int ColorButtonPosition;
    ImVec2 ButtonTextAlign;
    ImVec2 SelectableTextAlign;
    ImVec2 DisplayWindowPadding;
    ImVec2 DisplaySafeAreaPadding;
    float MouseCursorScale;
    bool AntiAliasedLines;
    bool AntiAliasedFill;
    float CurveTessellationTol;
    float CircleTessellationMaxError;
    ImVec4 Colors[ImGuiCol_COUNT];
};

// ImGui viewport
struct ImGuiViewport {
    ImGuiViewport() : ID(0), Flags(0), Pos(0, 0), Size(0, 0), WorkPos(0, 0), WorkSize(0, 0), DpiScale(1.0f), ParentViewport(nullptr), PlatformHandle(nullptr), PlatformHandleRaw(nullptr), RendererHandle(nullptr), RendererHandleRaw(nullptr), PlatformUserData(nullptr), RendererUserData(nullptr), DrawDataList(nullptr), DrawDataListCount(0), LastFrameActive(0), IsPlatformWindow(false), IsPlatformWindowOwned(false), IsFocused(false), IsHovered(false), Window(NULL), MenuBarWindow(NULL) {}
    ImGuiID ID;
    ImGuiViewportFlags Flags;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 WorkPos;
    ImVec2 WorkSize;
    float DpiScale;
    ImGuiViewport* ParentViewport;
    void* PlatformHandle;
    void* PlatformHandleRaw;
    void* RendererHandle;
    void* RendererHandleRaw;
    void* PlatformUserData;
    void* RendererUserData;
    ImDrawList** DrawDataList;
    int DrawDataListCount;
    int LastFrameActive;
    bool IsPlatformWindow;
    bool IsPlatformWindowOwned;
    bool IsFocused;
    bool IsHovered;
    ImGuiWindow* Window;
    ImGuiWindow* MenuBarWindow;
};

// ImGui window
struct ImGuiWindow {
    ImGuiWindow() : ID(0), Flags(0), BeginOrder(0), LastFrameActive(0), LastFrameFocused(0), LastFrameHovered(0), LastFrameVisible(0), LastFrameCollapsed(0), LastFramePopupOpen(0), LastFrameMenuBarHovered(0), LastFrameViewportHovered(0), Pos(0, 0), Size(0, 0), SizeContents(0, 0), SizeContentsExplicit(0, 0), SizeContentsActual(0, 0), SizeContentsMin(0, 0), SizeContentsMax(FLT_MAX, FLT_MAX), ContentRegionRect(0, 0, 0, 0), InnerRect(0, 0, 0, 0), ClipRect(0, 0, 0, 0), Scroll(0, 0), ScrollMax(0, 0), ScrollTarget(FLT_MAX, FLT_MAX), ScrollTargetCenterRatio(0.3f, 0.3f), ItemWidth(0.0f), ItemWidthStackCurrent(0), TextWrapPos(0.0f), TextWrapPosStackCurrent(0), ItemsHeight(0.0f), ItemsHeightStackCurrent(0), AllowKeyboardFocus(true), AllowKeyboardFocusStackCurrent(true), Appearing(false), Hidden(false), IsActive(false), WasActive(false), WriteAccessed(false), Collapsed(false), WantCollapseToggle(false), SkipItems(false), AppearingPos(0, 0), Scroll(0, 0), ScrollMax(0, 0), ScrollTarget(FLT_MAX, FLT_MAX), ScrollTargetCenterRatio(0.3f, 0.3f), ScrollbarSizes(0, 0), ScrollbarX(false), ScrollbarY(false), Active(true), HookIdNext(0), AutoFitFramesX(0), AutoFitFramesY(0), AutoFitOnlyGrows(false), AutoPosLastDirection(ImGuiDir_None), HiddenFramesCanSkipItems(0), HiddenFramesCannotSkipItems(0), SetWindowPosAllowFlags(0), SetWindowSizeAllowFlags(0), SetWindowCollapsedAllowFlags(0), SetWindowDockAllowFlags(0), SetWindowPosVal(0, 0), SetWindowPosPivot(0, 0), IDStack(), DC() {}
    ImGuiID ID;
    ImGuiWindowFlags Flags;
    int BeginOrder;
    int LastFrameActive;
    int LastFrameFocused;
    int LastFrameHovered;
    int LastFrameVisible;
    int LastFrameCollapsed;
    int LastFramePopupOpen;
    int LastFrameMenuBarHovered;
    int LastFrameViewportHovered;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 SizeContents;
    ImVec2 SizeContentsExplicit;
    ImVec2 SizeContentsActual;
    ImVec2 SizeContentsMin;
    ImVec2 SizeContentsMax;
    ImVec2 ContentRegionRect;
    ImVec2 InnerRect;
    ImVec2 ClipRect;
    ImVec2 Scroll;
    ImVec2 ScrollMax;
    ImVec2 ScrollTarget;
    ImVec2 ScrollTargetCenterRatio;
    float ItemWidth;
    int ItemWidthStackCurrent;
    float TextWrapPos;
    int TextWrapPosStackCurrent;
    float ItemsHeight;
    int ItemsHeightStackCurrent;
    bool AllowKeyboardFocus;
    int AllowKeyboardFocusStackCurrent;
    bool Appearing;
    bool Hidden;
    bool IsActive;
    bool WasActive;
    bool WriteAccessed;
    bool Collapsed;
    bool WantCollapseToggle;
    bool SkipItems;
    ImVec2 AppearingPos;
    ImVec2 Scroll;
    ImVec2 ScrollMax;
    ImVec2 ScrollTarget;
    ImVec2 ScrollTargetCenterRatio;
    ImVec2 ScrollbarSizes;
    bool ScrollbarX;
    bool ScrollbarY;
    bool Active;
    ImGuiID HookIdNext;
    int AutoFitFramesX;
    int AutoFitFramesY;
    bool AutoFitOnlyGrows;
    ImGuiDir AutoPosLastDirection;
    int HiddenFramesCanSkipItems;
    int HiddenFramesCannotSkipItems;
    ImGuiCond SetWindowPosAllowFlags;
    ImGuiCond SetWindowSizeAllowFlags;
    ImGuiCond SetWindowCollapsedAllowFlags;
    ImGuiCond SetWindowDockAllowFlags;
    ImVec2 SetWindowPosVal;
    ImVec2 SetWindowPosPivot;
    ImVector<ImGuiID> IDStack;
    struct ImGuiWindowTempData DC;
};
 
 // Drag and drop payload
struct ImGuiPayload {
    void* Data;
    int DataSize;
    ImGuiID SourceId;
    ImGuiID SourceParentId;
    int DataFrameCount;
    char DataType[32 + 1];
    bool Preview;
    bool Delivery;
    
    ImGuiPayload() { Clear(); }
    void Clear() { Data = NULL; DataSize = 0; SourceId = SourceParentId = 0; DataFrameCount = -1; DataType[0] = 0; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const { return Preview; }
    bool IsDelivery() const { return Delivery; }
};