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
float slotSize = 18.f;
float borderSize = (slotSize - 14.f) / 2;

// Uv positions
glm::tvec2<float> itemSlotUvPos(0.0f / 20.0f, 0.0f / 20.0f);
glm::tvec2<float> itemSlotUvSize(20.0f / 20.0f, 20.0f / 20.0f);
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

void ShulkerRenderer::Render(MinecraftUIRenderContext* ctx, HoverRenderer* hoverRenderer, int index, std::string boxcolor) {
	// Only load inventory resources once
	if (!hasLoadedTexture) {

        mBackgroundTexture = ctx->getTexture("textures/ui/whiteBorder", true);
        mItemSlotTexture = ctx->getTexture("textures/gui/itemslot", true);

        hasLoadedTexture = true;
	}

    std::string hovertext = hoverRenderer->mFilteredContent.substr(0, hoverRenderer->mFilteredContent.find("Minecraft") + 9);

    std::map<std::string, mce::Color> tintColor = {{"0", {0.65, 0.45, 0.64, 0} }, {"1", {1, 1, 1, 0}}, {"2", {0.61, 0.6, 0.56, 0}}, {"3", {0.27, 0.28, 0.30, 0}}, {"4", {0.13, 0.13, 0.15, 0}}, {"5", {0.50, 0.31, 0.17, 0}}, {"6", {0.67, 0.16, 0.15, 0}}, {"7", {1, 0.49, 0.066, 0}}, {"8", {1, 0.85, 0.15, 0}}, {"9", {0.49, 0.80, 0.1, 0}}, {"a", {0.36, 0.47, 0.12, 0}}, {"b", {0.09, 0.58, 0.62, 0}}, {"c", {0.25, 0.77, 0.94, 0}}, {"d", {0.22, 0.23, 0.66, 0}}, {"e", {0.51, 0.16, 0.72, 0}}, {"f", {0.81, 0.27, 0.75, 0}}, {"g", {1, 0.59, 0.73, 0}}};
    std::map<int, mce::Color> durabiltyColor = { {1, {0.08, 1, 0, 1}}, {2, {0.23, 1, 0, 1}}, {3, {0.45, 1, 0, 1}}, {4, {0.57, 1, 0, 1}}, {5, {0.75, 1, 0, 1}}, {6, {0.9, 1, 0, 1}}, {7, {1, 0.96, 0, 1}}, {8, {1, 0.82, 0, 1}}, {9, {1, 0.65, 0, 1}}, {10, {1, 0.51, 0, 1}}, {11, {1, 0.33, 0, 1}}, {12, {1, 0.17, 0, 1}}, {13, {0, 0, 0, 0}}, {14, {0, 0, 0, 0}}};

    float textHeight = (countNewlines(hoverRenderer->mFilteredContent) + 2) * 10.0f;

    float panelWidth = slotSize * 9;
    float panelHeight = slotSize * 3 + textHeight;

    float panelX = hoverRenderer->mCursorPosition.x + hoverRenderer->mOffset.x;
    float panelY = hoverRenderer->mCursorPosition.y + hoverRenderer->mOffset.y;

    // Draw the background panel
    RectangleArea Shulkerbackground = {panelX, panelX + panelWidth + 8, panelY + textHeight - 2, panelY + panelHeight + 6};

    backgroundNineslice.Draw(Shulkerbackground, &mBackgroundTexture, ctx);
    ctx->flushImages(tintColor.at(boxcolor), 1.0f, flushString);

    // Draw the item slots
	for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 3; y++) {
            glm::tvec2<float> size(slotSize, slotSize);
            glm::tvec2<float> position(panelX + 4 + slotSize * x, panelY + 2 + textHeight + slotSize * y);

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
            float xPos = (x * slotSize) + borderSize + panelX + 4;
            float yPos = (y * slotSize) + borderSize + textHeight + panelY + 2;

            renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, itemStack, 0, xPos - 1.f, yPos - 1.f, false, 1.f, 1.f, 1.0f);
        }
    }

    ctx->flushImages(mce::Color::WHITE, 1.0f, flushString);

    // Draw durabilty bars
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 3; y++) {
            const ItemStack* itemStack = &shulkerInventory[index][y * 9 + x];
            if (itemStack->mItem == nullptr) continue;
            if (itemStack->mItem->getMaxDamage() == 0) continue;
            if (itemStack->mItem->getDamageValue(itemStack->mUserData) == 0) continue;

            int max = itemStack->mItem->getMaxDamage();
            int damage = itemStack->mItem->getDamageValue(itemStack->mUserData);

            float spare = max / 13.0 - max / 13;

            int durabiltyState = 0;
            int idk = 1;
            int damageForNext = 1;

            for (int i = 1; i < 14; i++) {
                if (damage >= damageForNext) {
                    durabiltyState++;
                    i * spare >= idk ? (idk++, damageForNext+= max / 13 + 1) : damageForNext += max / 13;
                }
            }
            RectangleArea test = {panelX + slotSize * x + 7, panelX + slotSize * x + 20, panelY + textHeight + slotSize * y + 15.5f, panelY + textHeight + slotSize * y + 17.5f};
            RectangleArea test1 = { panelX + slotSize * x + 7, panelX + slotSize * x + 7 + (13 - durabiltyState), panelY + textHeight + slotSize * y + 15.5f, panelY + textHeight + slotSize * y + 16.5f};

            ctx->fillRectangle(test, mce::Color(0, 0, 0, 1), 1.0f);
            
            ctx->fillRectangle(test1, durabiltyColor.at(durabiltyState), 1.0f);
        }
    }

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

            float top = (y * slotSize) + borderSize + textHeight + panelY;
            float bottom = top + 16.f;

            float right = (x * slotSize) + borderSize + panelX + 2;
            float left = right + 6.f;

            if (itemStack->mCount < 10) left = left + 6.f;

            std::string text = std::format("{}", itemStack->mCount);
            RectangleArea rect = { left, right, top + 9.f, bottom};
            RectangleArea rect1 = { left + 1.f , right, top + 10.f, bottom};

            ctx->drawDebugText(&rect1, &text, &mce::Color::FONT_DARK_GRAY_BG, 1.0f, ui::Right, &textData, &caretData);
            ctx->drawDebugText(&rect, &text, &mce::Color::WHITE, 1.0f, ui::Right, &textData, &caretData);
        }
    }
    ctx->flushText(0.0f);
}