#include "../Entities/EntityManager.h"
#include "../GameCommands/GameCommands.h"
#include "../Interop/Interop.hpp"
#include "../Ui/WindowManager.h"
#include "Orders.h"
#include "Vehicle.h"

using namespace OpenLoco::Interop;

namespace OpenLoco::Vehicles
{
    static uint32_t overhaulVehicle(uint16_t head, uint8_t flags)
    {
        static loco_global<uint16_t, 0x0113642A> _113642A;
        Vehicles::Vehicle existingTrain(head);

        // Calculate Cost to Replace all powered components
        uint32_t totalBuyCost = 0;
        uint32_t totalSellCost = 0;
        for (auto& car : existingTrain.cars)
        {
            auto* vehObject = car.body->object();
            // Unpowered vehicles can not breakdown - so don't repair them
            if (vehObject->power == 0)
            {
                continue;
            }

            auto buyCost = GameCommands::queryDo_5(car.front->object_id, -1);
            if (buyCost == GameCommands::FAILURE)
            {
                totalBuyCost = GameCommands::FAILURE;
                break;
            }
            else
            {
                totalBuyCost += buyCost;
            }
            auto sellCost = -car.front->refund_cost; // TODO is this right with inflation?
            //auto sellCost = GameCommands::queryDo_6(car.front->id);
            if (sellCost == GameCommands::FAILURE)
            {
                totalSellCost = GameCommands::FAILURE;
                break;
            }
            else
            {
                totalSellCost += sellCost;
            }
        }

        if (totalBuyCost == GameCommands::FAILURE || totalSellCost == GameCommands::FAILURE)
        {
            return GameCommands::FAILURE;
        }

        // Charge difference between bought and sold components
        auto overhaulCost = totalBuyCost + totalSellCost;
        // If not asked to apply, return early
        if (!(flags & GameCommands::Flags::apply))
        {
            return overhaulCost;
        }

        // Repair components
        auto newReliability = 0;
        // TODO encapsulate and understand logic stolen from Cheat.cpp
        existingTrain.veh2->reliability = newReliability;
        for (auto& car : existingTrain.cars)
        {
            auto* vehObject = car.body->object();
            // Unpowered vehicles can not breakdown - so don't repair them
            if (vehObject->power == 0)
            {
                continue;
            }

            auto& component = *(car.begin());
            component.front->reliability = newReliability * 256;
        }
        // TODO Technically it should be part purchases, part sale, but that's a future improvement
        GameCommands::setExpenditureType(ExpenditureType::VehiclePurchases);
        return overhaulCost;
    }

    void overhaulVehicle(registers& regs)
    {
        regs.ebx = overhaulVehicle(regs.ax, regs.bl);
    }
}
