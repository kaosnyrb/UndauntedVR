#include "SpawnUtils.h"
#include "ConfigUtils.h"
#include "LocationUtils.h"

namespace Undaunted
{
	GroupList SpawnGroupAtTarget(VMClassRegistry* registry, GroupList Types, TESObjectREFR* Target, TESObjectCELL* cell, TESWorldSpace* worldspace)
	{
		TESObjectREFR* spawned = NULL;
		srand(time(NULL));
		NiPoint3 startingpoint = Target->pos;
		for (UInt32 i = 0; i < Types.length; i++)
		{
			TESForm* spawnForm = LookupFormByID(Types.data[i].FormId);
			if (spawnForm == NULL)
			{
				_MESSAGE("Failed to Spawn. Form Invalid");
				return Types;
			}
			if (!strcmp(Types.data[i].ModelFilepath.Get(), "") == 0)
			{
				TESModel* pWorldModel = DYNAMIC_CAST(spawnForm, TESForm, TESModel);
				if (pWorldModel)
				{
					_MESSAGE("GetModelName: %s", pWorldModel->GetModelName());
					pWorldModel->SetModelName(Types.data[i].ModelFilepath.Get());
				}
			}
			if (strcmp(Types.data[i].BountyType.Get(), "Enemy") == 0 || strcmp(Types.data[i].BountyType.Get(), "Ally") == 0)
			{
				//Random Offset
				NiPoint3 offset = NiPoint3(rand() & 1000, rand() & 1000, 0);
				MoveRefToWorldCell(Target, cell, worldspace, startingpoint + offset, NiPoint3(0, 0, 0));
				spawned = PlaceAtMe_Native(registry, 1, Target, spawnForm, 1, false, false);
				Types.data[i].objectRef = spawned;
			}
			else if (strcmp(Types.data[i].BountyType.Get(), "BountyDecoration") == 0 || 
				strcmp(Types.data[i].BountyType.Get(), "SpawnEffect") == 0 ||
				strcmp(Types.data[i].BountyType.Get(), "Scripted") == 0)
			{
				if (spawned != NULL)
				{
					//If a model file path is set then change the form model.

					//Actors jump to the navmesh. Objects don't. This tries to used the jump to find the ground.
					TESObjectREFR* decoration = PlaceAtMe_Native(registry, 1, spawned, spawnForm, 1, false, false);
					Types.data[i].objectRef = decoration;
					Types.data[i].PreBounty();
				}
				else
				{
					TESObjectREFR* decoration = PlaceAtMe_Native(registry, 1, Target, spawnForm, 1, false, false);
					Types.data[i].objectRef = decoration;
					Types.data[i].PreBounty();
				}
			}
			else if (strcmp(Types.data[i].BountyType.Get(), "PhysicsScripted") == 0)
			{
				//We don't want these falling through the floor, so we put them in the air.
				TESObjectREFR* PhysicsScripted = PlaceAtMe_Native(registry, 1, spawned, spawnForm, 1, false, false);
				NiPoint3 offset = NiPoint3(0, 0, -1500);
				//MoveRefToWorldCell(PhysicsScripted, cell, worldspace, PhysicsScripted->pos + offset, NiPoint3(0, 0, 0));
				Types.data[i].objectRef = PhysicsScripted;
				Types.data[i].PreBounty();
			}
		}
		return Types;
	}
}