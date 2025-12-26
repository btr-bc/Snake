#pragma once
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include "Core/Component.hpp"

namespace Bocchi {

    struct ShopItemData {
        int id = 0;
        std::string name;
        int price = 0;
    };

    class ShopManager {
    public:
        void setItems(std::vector<ShopItemData> items) { m_items = std::move(items); }
        const std::vector<ShopItemData>& items() const { return m_items; }

        bool purchaseItem(entt::registry& reg, entt::entity buyer, int itemId) {
            auto it = std::find_if(m_items.begin(), m_items.end(), [&](const ShopItemData& i){ return i.id == itemId; });
            if (it == m_items.end()) return false;
            if (!reg.all_of<Wallet>(buyer) || !reg.all_of<Inventory>(buyer)) return false;
            auto& wallet = reg.get<Wallet>(buyer);
            if (wallet.coins < it->price) return false;
            wallet.coins -= it->price;
            reg.get<Inventory>(buyer).itemIds.push_back(it->id);
            return true;
        }

    private:
        std::vector<ShopItemData> m_items;
    };

    class ShopSystem {
    public:
        explicit ShopSystem(ShopManager& manager) : m_manager(manager) {}

        void update(entt::registry& /*reg*/) {
            // placeholder: UI integration via UiBuilder/UiSystem when needed
        }

        bool purchase(entt::registry& reg, entt::entity buyer, int itemId) {
            return m_manager.purchaseItem(reg, buyer, itemId);
        }

    private:
        ShopManager& m_manager;
    };

} // namespace Bocchi
