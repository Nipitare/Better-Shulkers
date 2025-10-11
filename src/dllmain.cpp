#include "dllmain.hpp"

ClientInstance* client;

ShulkerRenderer shulkerRenderer;
ItemStack shulkerInventory[SHULKER_CACHE_SIZE][27]; 

SafetyHookInline _ShulkerBoxBlockItem_appendFormattedHovertext;
SafetyHookInline _HoverRenderer__renderHoverBox;

int index = 0;
bool ShowShulkerPreview = false;

void ShulkerBoxBlockItem_appendFormattedHovertext(ShulkerBoxBlockItem* self, const ItemStackBase& itemStack, Level& level, std::string& hovertext, bool showCategory) {

        _ShulkerBoxBlockItem_appendFormattedHovertext.thiscall<
        void,
        ShulkerBoxBlockItem*,
        const ItemStackBase&,
        Level&,
        std::string&,
        bool>(self, itemStack, level, hovertext, showCategory);

    index++;
    if (index >= SHULKER_CACHE_SIZE) index = 0;
    // Hide a secret index in the shulker name
    // We do this because appendFormattedHovertext gets called for the neightboring items so if there is a shulker
    // to the right of this one then its preview will get overriden, so we keep track of multiple at once using a rolling identifier
    const std::map<std::string, std::string> colorcodes = {{"undyed_shulker_box", "0"}, {"white_shulker_box", "1"}, {"light_gray_shulker_box", "2"}, {"gray_shulker_box", "3"}, {"black_shulker_box", "4"}, {"brown_shulker_box", "5"}, {"red_shulker_box", "6"}, {"orange_shulker_box", "7"}, {"yellow_shulker_box", "8"}, {"lime_shulker_box", "9"}, {"green_shulker_box", "a"}, {"cyan_shulker_box", "b"}, {"light_blue_shulker_box", "c"}, {"blue_shulker_box", "d"}, {"purple_shulker_box", "e"}, {"magenta_shulker_box", "f"}, {"pink_shulker_box", "g"}};
    
    std::string indexhex = std::format("{:02x}", index);

    hovertext.insert(0, std::format("§{}§{}", indexhex[0], indexhex[1]));
    hovertext.insert(6, std::format("§{}", colorcodes.at(itemStack.mItem->mRawNameId.c_str())));

    hovertext = hovertext.substr(0, hovertext.find("Minecraft") + 9);
    
    if (!ShowShulkerPreview) {
    auto& options = *Amethyst::GetClientCtx().mOptions;
    auto& mapping = *options.getCurrentKeyboardRemapping();
    auto* keymapping = mapping.getKeymappingByAction("key.better_shulkers.show_shulker_preview");
    std::string keyName = keymapping ? mapping.getMappedKeyName(*keymapping) : "Unknown";
    if (keymapping && keymapping->isAssigned()) 
        hovertext.insert(hovertext.find("\n§o§9Minecraft"), "\n" + std::vformat("action.better_shulkers.show_shulker_preview"_i18n, std::make_format_args(keyName)));
    }
    hovertext += std::format("§v");
    int thisIndex = index;
    // Reset all the currrent item stacks
    for (auto& itemStack : shulkerInventory[index]) {
        itemStack = ItemStack();
    }
    if (!itemStack.mUserData) return;
    if (!itemStack.mUserData->contains("Items")) return;

    const ListTag* items = itemStack.mUserData->getList("Items");
    
    for (int i = 0; i < items->size(); i++) {
        const CompoundTag* itemCompound = items->getCompound(i);
        byte slot = itemCompound->getByte("Slot");
        shulkerInventory[thisIndex][slot]._loadItem(itemCompound);
    }
}

 void HoverRenderer__renderHoverBox(HoverRenderer* self, MinecraftUIRenderContext* ctx, IClientInstance& client, RectangleArea* aabb, float someFloat) {
    // This is really bad code, it is relying on the fact that I have also hooked appendFormattedHovertext for items to append the item identifier
    // I have no idea where the currently hovered item is stored in the game! I can't find any references to it, so it might be set in some weird place?
    
    _HoverRenderer__renderHoverBox.thiscall<
        void,
        HoverRenderer*,
        MinecraftUIRenderContext*,
        IClientInstance&,
        RectangleArea*,
        float>(self, ctx, client, aabb, someFloat);

    if (self->mFilteredContent.find("§v") == self->mFilteredContent.size() - 9 && ShowShulkerPreview) {
        std::string color = self->mFilteredContent.substr(8, 1);
        std::string cachedIndex = self->mFilteredContent.substr(2, 1) + self->mFilteredContent.substr(5, 1);
        try {
            int index = std::stoi(cachedIndex, nullptr, 16);
            shulkerRenderer.Render(ctx, self, index, color);
        }
        catch (...) {
            Log::Warning("There was an issue reading the shulker box! No id found, instead got: {}", cachedIndex);
            return;
        }

        return;
    }
}

ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod) {
    Amethyst::InitializeAmethystMod(ctx, mod);

    Amethyst::HookManager& hooks = Amethyst::GetHookManager();

    Amethyst::GetEventBus().AddListener<RegisterInputsEvent>([&](const RegisterInputsEvent& e) {
    Amethyst::InputManager& input = e.inputManager;
    auto& action = input.RegisterNewInput("better_shulkers.show_shulker_preview", {'H'}, true, Amethyst::KeybindContext::Screen);

    action.addButtonDownHandler([](FocusImpact, ClientInstance&) {
        ShowShulkerPreview = true;
        return Amethyst::InputPassthrough::Passthrough;
    });

    action.addButtonUpHandler([](FocusImpact, ClientInstance&) {
        ShowShulkerPreview = false;
        return Amethyst::InputPassthrough::Passthrough;
    });
    });

    VHOOK(ShulkerBoxBlockItem, appendFormattedHovertext, this);

    HOOK(HoverRenderer, _renderHoverBox);
}