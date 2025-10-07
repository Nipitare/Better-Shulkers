#include "ShulkerRenderer.hpp"
#include <amethyst/Formatting.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-client/common/client/game/MinecraftGame.hpp>
#include <mc/src-deps/core/math/Color.hpp>
#include <mc/src-client/common/client/game/ClientInstance.hpp>

extern ItemStack shulkerInventory[SHULKER_CACHE_SIZE][27];

// Texture loading
static HashedString flushString(0xA99285D21E94FC80, "ui_flush");

bool hasLoadedTexture = false;

// Slot sizing
float slotSize = 20.f;
float borderSize = (slotSize - 16.f) / 2;

// Uv positions
glm::tvec2<float> itemSlotUvPos(188.0f / 256.0f, 184.0f / 256.0f);
glm::tvec2<float> itemSlotUvSize(22.0f / 256.0f, 22.0f / 256.0f);
Amethyst::NinesliceHelper backgroundNineslice(16, 16, 4, 4);

int countNewlines(const std::string& str) {
    int newlineCount = 0;

    for (char c : str) {
        if (c == '\n') {
            newlineCount++;
        }
    }

    return newlineCount;
}

void ShulkerRenderer::Render(MinecraftUIRenderContext* ctx, HoverRenderer* hoverRenderer, int index, std::string boxcolor, IClientInstance& _client) {
    ClientInstance& client = _client.asInstance();
	// Only load inventory resources once
	if (!hasLoadedTexture) {

        mWhiteBackgroundTexture = ctx->getTexture("textures/ui/whiteBorder", true);
        mBackgroundTexture = ctx->getTexture("textures/ui/purpleBorder", true);
        mItemSlotTexture = ctx->getTexture("textures/gui/Whitegui", true);

        hasLoadedTexture = true;
	}

    std::string hovertext = hoverRenderer->mFilteredContent.substr(0, hoverRenderer->mFilteredContent.find("Minecraft") + 9);

    const std::map<std::string, mce::Color> tintColor = {{"0", {0.65, 0.45, 0.64, 0} }, {"1", {1, 1, 1, 0}}, {"2", {0.61, 0.6, 0.56, 0}}, {"3", {0.27, 0.28, 0.30, 0}}, {"4", {0.13, 0.13, 0.15, 0}}, {"5", {0.50, 0.31, 0.17, 0}}, {"6", {0.67, 0.16, 0.15, 0}}, {"7", {1, 0.49, 0.066, 0}}, {"8", {1, 0.85, 0.15, 0}}, {"9", {0.49, 0.80, 0.1, 0}}, {"a", {0.36, 0.47, 0.12, 0}}, {"b", {0.09, 0.58, 0.62, 0}}, {"c", {0.25, 0.77, 0.94, 0}}, {"d", {0.22, 0.23, 0.66, 0}}, {"e", {0.51, 0.16, 0.72, 0}}, {"f", {0.81, 0.27, 0.75, 0}}, {"g", {1, 0.59, 0.73, 0}}};
    // Find other color tints too other than red
    const std::map<std::string, mce::Color> test = { {"0", {0.73, 0.06, 0.07, 0}} };

    float textHeight = (countNewlines(hovertext) + 1) * 10.0f;
    float textWidth = 0.0f;

    try {
        textWidth = ctx->getLineLength(*client.mMinecraftGame->mFontHandle.mDefaultFont, hovertext.substr(hovertext.find("minecraft:"), hovertext.find("_box") - hovertext.find("minecraft:")), 1.0f, false) * 1.3f;
    }
    catch (...) {
        textWidth = ctx->getLineLength(*client.mMinecraftGame->mFontHandle.mDefaultFont, hovertext.substr(0, hovertext.find(std::format("{}r", "\xc2\xa7"))), 1.0f, false) * 1.1f;
    }

    float panelWidth = slotSize * 9;
    float panelHeight = slotSize * 3 + textHeight;

    float panelX = hoverRenderer->mCursorPosition.x + hoverRenderer->mOffset.x;
    float panelY = hoverRenderer->mCursorPosition.y + hoverRenderer->mOffset.y;

    // Draw the background panel
    RectangleArea Shulkerbackground = {panelX - 4, panelX + panelWidth + 4, panelY + 4 + textHeight, panelY + panelHeight + 12};
    RectangleArea Textbackground = {panelX - 4, panelX + textWidth + 4, panelY - 4, panelY + textHeight + 4};

    backgroundNineslice.Draw(Shulkerbackground, &mWhiteBackgroundTexture, ctx);
    ctx->flushImages(tintColor.at(boxcolor), 1.0f, flushString);

    backgroundNineslice.Draw(Textbackground, &mBackgroundTexture, ctx);
    ctx->flushImages(mce::Color::WHITE, 1.0f, flushString);

    // Draw the item slots
	for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 3; y++) {
            glm::tvec2<float> size(slotSize, slotSize);
            glm::tvec2<float> position(panelX + slotSize * x, panelY + 8 + textHeight + slotSize * y);

            ctx->drawImage(mItemSlotTexture, &position, &size, &itemSlotUvPos, &itemSlotUvSize, 0);
        }
    }

    // It's possible to tint the background here
    ctx->flushImages(tintColor.at(boxcolor), 1.0f, flushString);



    // Draw the item icons
    BaseActorRenderContext renderCtxPtr = BaseActorRenderContext(*ctx->mScreenContext, *ctx->mClient, *ctx->mClient->mMinecraftGame);

    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 3; y++) {
            const ItemStack* itemStack = &shulkerInventory[index][y * 9 + x];
            if (itemStack->mItem == nullptr) continue;

            float xPos = (x * slotSize) + borderSize + panelX;
            float yPos = (y * slotSize) + borderSize + textHeight + panelY + 8;

            renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, itemStack, 0, xPos - 1.f, yPos - 1.f, false, 1.f, 1.f, 1.125f);
        }
    }

    ctx->flushImages(mce::Color::WHITE, 1.0f, flushString);

    // Draw Item Counts
    TextMeasureData textData;
    memset(&textData, 0, sizeof(TextMeasureData));
    textData.fontSize = 1.0f;

    CaretMeasureData caretData;
    memset(&caretData, 1, sizeof(CaretMeasureData));

    // Draw the item counts
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 3; y++) {
            ItemStack* itemStack = &shulkerInventory[index][y * 9 + x];
            if (itemStack->mItem == nullptr) continue;
            if (itemStack->mCount == 1) continue;

            float top = (y * slotSize) + borderSize + textHeight + panelY + 8;
            float bottom = top + 16.f;

            float right = (x * slotSize) + borderSize + panelX;
            float left = right + 6.f;

            std::string text = std::format("{}", itemStack->mCount);
            RectangleArea rect = { left, right, top + 9.f, bottom};
            RectangleArea rect1 = { left + 1.f , right, top + 10.f, bottom};

            ctx->drawDebugText(&rect1, &text, &mce::Color::FONT_DARK_GRAY_BG, 1.0f, ui::Left, &textData, &caretData);
            ctx->drawDebugText(&rect, &text, &mce::Color::WHITE, 1.0f, ui::Left, &textData, &caretData);
        }
    }

    ctx->flushText(0.0f);

    RectangleArea textArea = { panelX, panelX + panelWidth, panelY, panelY + textHeight };
    RectangleArea rect = { panelX, panelX + panelWidth, panelY, panelY + panelHeight };
    ctx->drawDebugText(&textArea, &hovertext, &mce::Color::WHITE, 1.0f, ui::Left, &textData, &caretData);
    ctx->flushText(0.0f);
}