#include "global.h"
#include "player_styles.h"
#include "constants/flags.h"
#include "constants/songs.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "global.fieldmap.h"
#include "gpu_regs.h"
#include "list_menu.h"
#include "menu.h"
#include "palette.h"
#include "player_pc.h"
#include "script_menu.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "window.h"
#include "international_string_util.h"

// Player OW palette tags (from event_object_movement.c)
#define PAL_TAG_BRENDAN    0x1100
#define PAL_TAG_MAY        0x1110
#define PAL_TAG_RS_BRENDAN 0x1122
#define PAL_TAG_RS_MAY     0x1123

#define STYLE_MENU_MAX_VISIBLE 8

#include "data/player_styles.h"

// Forward declarations
static void StyleMenu_ProcessInput(u8 taskId);

// ============================================================
// Helper functions
// ============================================================

static u16 DarkenColor555(u16 color, u8 percent)
{
    u8 r = (color & 0x1F) * percent / 100;
    u8 g = ((color >> 5) & 0x1F) * percent / 100;
    u8 b = ((color >> 10) & 0x1F) * percent / 100;
    return r | (g << 5) | (b << 10);
}

static u16 GrayscaleColor555(u16 color)
{
    u8 r = color & 0x1F;
    u8 g = (color >> 5) & 0x1F;
    u8 b = (color >> 10) & 0x1F;
    u8 gray = (r + g + b) / 3;
    return gray | (gray << 5) | (gray << 10);
}

// ============================================================
// Palette override functions
// ============================================================

u16 GetPlayerStyle(void)
{
    return VarGet(VAR_PLAYER_STYLE);
}

static void ApplyStyleOverlay(u16 *pal, const struct PlayerStyleData *style, bool8 isFemale, bool8 isOW)
{
    u8 i;

    // Index 4 = shoes/lower body (safe to change, not used for skin)
    if (isOW)
        pal[4] = style->clothesOutline;

    pal[5] = style->clothesLight;
    pal[6] = style->clothesMain;

    if (!isFemale)
    {
        // Male: index 7 = clothes dark, index 8 = outline
        pal[7] = style->clothesDark;
        pal[8] = style->clothesOutline;
    }
    else
    {
        // Female: indices 7-8 are hair, only change if style specifies
        if (style->hairLight != 0xFFFF)
        {
            pal[7] = style->hairLight;
            pal[8] = style->hairDark;
        }
    }

    pal[9] = style->whiteParts;
    pal[10] = style->bagMain;
    pal[11] = style->bagShadow;
    pal[12] = style->accentMain;
    pal[13] = style->accentDark;
    pal[14] = style->hatWhite;

    if (style->grayscaleSkin)
    {
        for (i = 1; i <= 4; i++)
            pal[i] = GrayscaleColor555(pal[i]);

        if (isFemale && style->hairLight == 0xFFFF)
        {
            // Also grayscale hair for female if no explicit hair override
            pal[7] = GrayscaleColor555(pal[7]);
            pal[8] = GrayscaleColor555(pal[8]);
        }
    }
}

void ApplyPlayerStyleToOWPalette(u8 paletteSlot, bool8 isFemale)
{
    u16 styleIdx = GetPlayerStyle();
    const struct PlayerStyleData *style;

    if (styleIdx == 0 || styleIdx > PLAYER_STYLE_COUNT)
        return;

    style = &sPlayerStyles[styleIdx - 1];
    ApplyStyleOverlay(&gPlttBufferUnfaded[OBJ_PLTT_ID(paletteSlot)], style, isFemale, TRUE);
    CpuCopy16(&gPlttBufferUnfaded[OBJ_PLTT_ID(paletteSlot)],
              &gPlttBufferFaded[OBJ_PLTT_ID(paletteSlot)],
              PLTT_SIZE_4BPP);
}

void ApplyPlayerStyleToTrainerPalette(u16 paletteOffset, bool8 isFemale)
{
    u16 styleIdx = GetPlayerStyle();
    const struct PlayerStyleData *style;

    if (styleIdx == 0 || styleIdx > PLAYER_STYLE_COUNT)
        return;

    style = &sPlayerStyles[styleIdx - 1];
    ApplyStyleOverlay(&gPlttBufferUnfaded[paletteOffset], style, isFemale, FALSE);
    CpuCopy16(&gPlttBufferUnfaded[paletteOffset],
              &gPlttBufferFaded[paletteOffset],
              PLTT_SIZE_4BPP);
}

void RefreshPlayerOWPalette(void)
{
    struct ObjectEvent *obj;
    u8 palSlot;
    u16 palTag;
    bool8 isFemale;

    if (gPlayerAvatar.objectEventId == 0 && gObjectEvents[0].active == 0)
        return;

    obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!obj->active)
        return;

    palSlot = gSprites[obj->spriteId].oam.paletteNum;
    isFemale = (gSaveBlock2Ptr->playerGender != MALE);

    if (FlagGet(FLAG_PLAYER_STYLE_RS))
        palTag = isFemale ? PAL_TAG_RS_MAY : PAL_TAG_RS_BRENDAN;
    else
        palTag = isFemale ? PAL_TAG_MAY : PAL_TAG_BRENDAN;

    // Reload base palette then hook will apply style
    PatchObjectPalette(palTag, palSlot);
}

// Called every frame from the overworld loop to ensure style persists
// through weather and menu transitions.
//
// ONLY writes to gPlttBufferUnfaded. The fade system reads from unfaded
// as its blend target, so all fades naturally blend toward styled colors.
// We never touch gPlttBufferFaded here — the fade system manages it:
//   - During fades: blends unfaded (styled) toward fade color
//   - When fade completes at coeff 0: faded = unfaded (styled)
void PlayerStyleOverworldUpdate(void)
{
    struct ObjectEvent *obj;
    u8 palSlot;
    bool8 isFemale;
    u16 styleIdx;
    const struct PlayerStyleData *style;

    styleIdx = GetPlayerStyle();
    if (styleIdx == 0 || styleIdx > PLAYER_STYLE_COUNT)
        return;

    if (gPlayerAvatar.objectEventId == 0 && gObjectEvents[0].active == 0)
        return;

    obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!obj->active)
        return;

    palSlot = gSprites[obj->spriteId].oam.paletteNum;
    isFemale = (gSaveBlock2Ptr->playerGender != MALE);
    style = &sPlayerStyles[styleIdx - 1];

    ApplyStyleOverlay(&gPlttBufferUnfaded[OBJ_PLTT_ID(palSlot)], style, isFemale, TRUE);
}

// ============================================================
// Style selection menu
// ============================================================

#define tStyleWindowId data[4]
#define tStyleListId   data[5]

void ShowStyleMenu(u8 taskId)
{
    u32 pixelWidth;
    u8 width;
    u8 maxShowed;
    u8 i;
    struct ListMenuTemplate menuTemplate;

    pixelWidth = 0;
    for (i = 0; i < STYLE_MENU_ITEM_COUNT; i++)
        pixelWidth = DisplayTextAndGetWidth(sStyleMenuItems[i].name, pixelWidth);

    maxShowed = STYLE_MENU_MAX_VISIBLE;
    width = ConvertPixelWidthToTileWidth(pixelWidth);

    gTasks[taskId].tStyleWindowId = CreateWindowFromRect(0, 0, width, maxShowed * 2);
    SetStandardWindowBorderStyle(gTasks[taskId].tStyleWindowId, FALSE);

    menuTemplate.items = sStyleMenuItems;
    menuTemplate.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    menuTemplate.itemPrintFunc = NULL;
    menuTemplate.totalItems = STYLE_MENU_ITEM_COUNT;
    menuTemplate.maxShowed = maxShowed;
    menuTemplate.windowId = gTasks[taskId].tStyleWindowId;
    menuTemplate.header_X = 0;
    menuTemplate.item_X = 8;
    menuTemplate.cursor_X = 0;
    menuTemplate.upText_Y = 1;
    menuTemplate.cursorPal = 2;
    menuTemplate.fillValue = 1;
    menuTemplate.cursorShadowPal = 3;
    menuTemplate.lettersSpacing = 0;
    menuTemplate.itemVerticalPadding = 0;
    menuTemplate.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    menuTemplate.fontId = FONT_NORMAL;
    menuTemplate.cursorKind = CURSOR_BLACK_ARROW;

    gTasks[taskId].tStyleListId = ListMenuInit(&menuTemplate, 0, 0);
    CopyWindowToVram(gTasks[taskId].tStyleWindowId, COPYWIN_FULL);
    gTasks[taskId].func = StyleMenu_ProcessInput;
}

static void CloseStyleMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tStyleListId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].tStyleWindowId, FALSE);
    ClearWindowTilemap(gTasks[taskId].tStyleWindowId);
    RemoveWindow(gTasks[taskId].tStyleWindowId);
    ScheduleBgCopyTilemapToVram(0);
}

static void StyleMenu_ProcessInput(u8 taskId)
{
    s32 input;

    if (gPaletteFade.active)
        return;

    input = ListMenu_ProcessInput(gTasks[taskId].tStyleListId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    PlaySE(SE_SELECT);

    if (input == LIST_CANCEL || input == STYLE_MENU_ID_CANCEL)
    {
        // B pressed or Cancel selected
        CloseStyleMenu(taskId);
        ReshowPlayerPC(taskId);
    }
    else
    {
        // Style selected: 0 = Default, 1-24 = styles
        VarSet(VAR_PLAYER_STYLE, input);
        CloseStyleMenu(taskId);
        RefreshPlayerOWPalette();
        ReshowPlayerPC(taskId);
    }
}

#undef tStyleWindowId
#undef tStyleListId
