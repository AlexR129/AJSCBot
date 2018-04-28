#include <sc2api/sc2_api.h>

#include <iostream>

using namespace sc2;

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Hello, World!" << std::endl;
	}

	virtual void OnStep() final {
		// std::cout << Observation()->GetGameLoop() << std::endl;
		//std::cout << Observation()->GetMinerals() << std::endl;
		TryBuildSupplyDepot();
		TryBuildRefinery();
		TryBuildBarracks();
	}

	virtual void OnUnitCreated(const Unit* unit) final
	{
		switch (unit->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV:
		{
			const Unit* target = GetUnsaturatedRefinery();
			if (!target)
				target = FindNearestMineralPatch(unit->pos);
			if(!target)
				break;
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, target);
			break;
		}
		default: {
			break;
		}
		}
	}

	virtual void OnUnitIdle(const Unit* unit) final {
		switch (unit->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
			break;
		}
		case UNIT_TYPEID::TERRAN_SCV :
		{
			const Unit* target = FindNearestMineralPatch(unit->pos);
			if (!target)
				break;
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, target);
			break;
		}
		case UNIT_TYPEID::TERRAN_BARRACKS: {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
			break;
		}
		case UNIT_TYPEID::TERRAN_MARINE: {
			const GameInfo& game_info = Observation()->GetGameInfo();
			Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
			break;
		}
		default: {
			break;
		}
		}
	}
private:
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
		const ObservationInterface* observation = Observation();

		// If a unit already is building a supply structure of this type, do nothing.
		// Also get an scv to build the structure.
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			for (const auto& order : unit->orders) {
				if (order.ability_id == ability_type_for_structure) {
					return false;
				}
			}

			if (unit->unit_type == unit_type) {
				unit_to_build = unit;
			}
		}

		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 1.0f, unit_to_build->pos.y + ry * 1.0f));

		return true;
	}

	bool TryBuildStructure(ABILITY_ID ability_type_for_structure,Point2D place, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
		const ObservationInterface* observation = Observation();

		// If a unit already is building a supply structure of this type, do nothing.
		// Also get an scv to build the structure.
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			for (const auto& order : unit->orders) {
				if (order.ability_id == ability_type_for_structure) {
					return false;
				}
			}

			if (unit->unit_type == unit_type) {
				unit_to_build = unit;
			}
		}

		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure,place);

		return true;
	}

	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, const Unit* u, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
		const ObservationInterface* observation = Observation();

		// If a unit already is building a supply structure of this type, do nothing.
		// Also get an scv to build the structure.
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			for (const auto& order : unit->orders) {
				if (order.ability_id == ability_type_for_structure) {
					return false;
				}
			}

			if (unit->unit_type == unit_type) {
				unit_to_build = unit;
			}
		}

		Actions()->UnitCommand(unit_to_build,
			ability_type_for_structure, u);
		return true;
	}

	bool TryBuildSupplyDepot() {
		const ObservationInterface* observation = Observation();

		// If we are not supply capped, don't build a supply depot.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
			return false;

		// Try and build a depot. Find a random SCV and give it the order.
		return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
	}

	bool TryBuildBarracks() {
		const ObservationInterface* observation = Observation();

		if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
			return false;
		}

		if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) > 0) {
			return false;
		}

		return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
	}

	bool TryBuildRefinery() {
		const ObservationInterface* observation = Observation();
		
		const Unit* home = nullptr;
		for(const auto & unit : observation->GetUnits())
			if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER)
			{
				home = unit;
				break;
			}
		if (!home)
			return false;
		const Unit* vg = FindNearestGeyser(home->pos);
		if (!vg)
			return false;
		return TryBuildStructure(ABILITY_ID::BUILD_REFINERY, vg);
	}

	const Unit* FindNearestMineralPatch(const Point2D& start) {
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float distance = std::numeric_limits<float>::max();
		const Unit* target = nullptr;
		for (const auto& u : units) {
			if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
				float d = DistanceSquared2D(u->pos, start);
				if (d < distance) {
					distance = d;
					target = u;
				}
			}
		}
		return target;
	}

	const Unit* FindNearestGeyser(const Point2D& start) {
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float distance = std::numeric_limits<float>::max();
		const Unit* target = nullptr;
		for (const auto& u : units) {
			if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
				float d = DistanceSquared2D(u->pos, start);
				if (d < distance) {
					distance = d;
					target = u;
				}
			}
		}
		return target;
	}

	const Unit* GetUnsaturatedRefinery()
	{
		for (const auto& unit : Observation()->GetUnits())
		{
			if (unit->unit_type == UNIT_TYPEID::TERRAN_REFINERY)
			{
				std::cout << unit->assigned_harvesters << std::endl;
				if (unit->assigned_harvesters < unit->ideal_harvesters)
					return unit;
			}
		}
		return nullptr;
	}

	size_t CountUnitType(UNIT_TYPEID unit_type) {
		return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
	}
};

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Terran, &bot),
		CreateComputer(Race::Zerg)
		});

	coordinator.LaunchStarcraft();
	coordinator.StartGame(sc2::kMapBelShirVestigeLE);
	while (coordinator.Update()) {
	}
	return 0;
}