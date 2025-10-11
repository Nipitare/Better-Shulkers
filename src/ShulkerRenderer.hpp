#pragma once
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>
#include <mc/src-client/common/client/renderer/TexturePtr.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src-client/common/client/game/IClientInstance.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src-client/common/client/gui/controls/renderers/HoverRenderer.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-client/common/client/renderer/NinesliceInfo.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>

#define SHULKER_CACHE_SIZE 256

class ShulkerRenderer {
private:
	mce::TexturePtr mBackgroundTexture;
	mce::TexturePtr mItemSlotTexture;

public:
	void Render(MinecraftUIRenderContext* ctx, HoverRenderer* hoverRenderer, int index, std::string boxcolor);
};