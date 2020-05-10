#include "MyPlugin.h"

namespace Undaunted {
	
	// Triggers a new bounty stage to start.
	float hook_StartBounty(StaticFunctionTag* base, bool nearby) {
		BountyManager::getInstance()->StartBounty(nearby);
		return 2;
	}

	// Fill out the WorldList, this checks the loaded world cells and finds the persistant reference cells.
	// This takes a while so we only do this once at the start
	bool hook_InitSystem(StaticFunctionTag* base)
	{
		if (!BountyManager::getInstance()->isReady)
		{
			BuildWorldList();
			BountyManager::getInstance()->isReady = true;
		}
		return BountyManager::getInstance()->isReady;
	}

	// A check to see if the Init call has finished
	bool hook_isSystemReady(StaticFunctionTag* base)
	{
		return BountyManager::getInstance()->isReady;
	}

	// Check if all the bounty objectives have been complete
	bool hook_isBountyComplete(StaticFunctionTag* base) {
		_MESSAGE("Starting Bounty Check");
		return BountyManager::getInstance()->BountyUpdate();
	}

	// Pass the reference to the XMarker that we use as the quest target and the target of the placeatme calls
	bool hook_SetXMarker(StaticFunctionTag* base, TESObjectREFR* marker) {
		BountyManager::getInstance()->xmarkerref = marker;
		return true;
	}

	// Pass the reference to the quest objective message. This allows us to edit it from the code.
	bool hook_SetBountyMessageRef(StaticFunctionTag* base, BGSMessage* ref) {
		BountyManager::getInstance()->bountymessageref = ref;
		return true;
	}

	// For reasons as yet unknown, some of the regions in memory cause crash to desktops. We have to skip processing these. Hoping to fix this.
	bool hook_AddBadRegion(StaticFunctionTag* base, UInt32 region) {
		AddBadRegionToConfig(region);
		return true;
	}

	// Process the Group header line. We return the groups position which we can use to add to later.
	UInt32 hook_AddGroup(StaticFunctionTag* base, BSFixedString questText, BSFixedString modRequirement, UInt32 minLevel, UInt32 maxLevel, UInt32 playerLevel){
		//Player is too low level for this bounty
		if (playerLevel + GetConfigValueInt("BountyLevelCache") < minLevel && minLevel != 0)
		{
			_MESSAGE("%s: Player level too low", questText.Get());
			return -1;
		}
		//Player is too high level for this bounty
		if (playerLevel > maxLevel && maxLevel != 0)
		{
			_MESSAGE("%s: Player level too high", questText.Get());
			return -1;
		}
		return AddGroup(questText.Get());
	}

	// Add a member to a group.
	void hook_AddMembertoGroup(StaticFunctionTag* base, UInt32 groupid, UInt32 member, BSFixedString BountyType, BSFixedString ModelFilepath) {
		GroupMember newMember = GroupMember();
		newMember.FormId = member;
		newMember.BountyType = BountyType;
		newMember.ModelFilepath = ModelFilepath;
		AddMembertoGroup(groupid, newMember);
	}

	// Given a mod name and a FormId - load order, return the actualy form id
	UInt32 hook_GetModForm(StaticFunctionTag* base, BSFixedString ModName, UInt32 FormId){
		DataHandler* dataHandler = DataHandler::GetSingleton();
		_MESSAGE("mod: %08X , %s, %08X", FormId, ModName.Get(), dataHandler->GetLoadedModIndex(ModName.Get()));		
		return (dataHandler->GetLoadedModIndex(ModName.Get()) << 24) + FormId;
	}

	// Return a reward form. We seed the random data with the offset + time so that we can spawn multiple things at once.
	TESForm* hook_SpawnRandomReward(StaticFunctionTag* base, UInt32 rewardOffset, UInt32 playerlevel)
	{
		UInt32 rewardid = GetReward(rewardOffset, playerlevel);
		_MESSAGE("RewardID: %08X", rewardid);
		return LookupFormByID(rewardid);
	}

	// Tell the bounty system that this object should be marked as complete
	void hook_SetGroupMemberComplete(StaticFunctionTag* base, TESObjectREFR* taget)
	{
		BountyManager::getInstance()->bountygrouplist.SetGroupMemberComplete(taget->formID);
	}

	// Pass in a config value
	void hook_SetConfigValue(StaticFunctionTag* base, BSFixedString key, BSFixedString value)
	{
		SetConfigValue(key.Get(), value.Get());
	}

	// Currently unused, checks if the object reference is in the current bounty.
	bool hook_IsGroupMemberUsed(StaticFunctionTag* base, TESObjectREFR* target)
	{
		//Is this reference in the current bounty? If it isn't we can get rid of it.
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			if (BountyManager::getInstance()->bountygrouplist.data[i].objectRef->formID == target->formID)
			{
				return true;
			}
		}
		return false;
	}

	// The player has fast travelled. This causes cells which are marked to reset to reset.
	// This means we can take all bounties off the blacklist.
	void hook_PlayerTraveled(StaticFunctionTag* base, float distance)
	{
		BountyManager::getInstance()->ClearBountyData();
		if (distance > 1.5f)
		{
			BountyManager::getInstance()->ResetBountiesRan();
		}
	}

	// Triggered when leaving a microdungeon. Tells all the doors that the microdungeon has been completed.
	void hook_SetScriptedDoorsComplete(StaticFunctionTag* base)
	{
		_MESSAGE("Starting hook_SetBountyComplete");
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			const char* type = BountyManager::getInstance()->bountygrouplist.data[i].BountyType.Get();
			if (strcmp(type, "ScriptedDoor") == 0)
			{
				BountyManager::getInstance()->bountygrouplist.data[i].isComplete = true;
			}
		}
	}

	// Returns the references of all the spawned objects of a certain type
	VMResultArray<TESObjectREFR*> hook_GetBountyObjectRefs(StaticFunctionTag* base, BSFixedString bountyType)
	{
		VMResultArray<TESObjectREFR*> allies = VMResultArray<TESObjectREFR*>();
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			if (strcmp(BountyManager::getInstance()->bountygrouplist.data[i].BountyType.Get(), bountyType.Get()) == 0)
			{
				if (BountyManager::getInstance()->bountygrouplist.data[i].objectRef != NULL)
				{
					allies.push_back(BountyManager::getInstance()->bountygrouplist.data[i].objectRef);
				}
			}
		}
		return allies;
	}

	// Returns an int that is in the config
	UInt32 hook_GetConfigValueInt(StaticFunctionTag* base, BSFixedString key)
	{
		return GetConfigValueInt(key.Get());
	}

	bool RegisterFuncs(VMClassRegistry* registry) {

		BountyManager::getInstance()->_registry = registry;
		//General
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, float,bool>("StartBounty", "Undaunted_SystemScript", Undaunted::hook_StartBounty, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("isBountyComplete", "Undaunted_SystemScript", Undaunted::hook_isBountyComplete, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, TESObjectREFR*>("SetXMarker", "Undaunted_SystemScript", Undaunted::hook_SetXMarker, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, BGSMessage*>("SetBountyMessageRef", "Undaunted_SystemScript", Undaunted::hook_SetBountyMessageRef, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("isSystemReady", "Undaunted_SystemScript", Undaunted::hook_isSystemReady, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("InitSystem", "Undaunted_SystemScript", Undaunted::hook_InitSystem, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void,float>("PlayerTraveled", "Undaunted_SystemScript", Undaunted::hook_PlayerTraveled, registry));

		//Config

		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, void, BSFixedString, BSFixedString>("SetConfigValue", "Undaunted_SystemScript", Undaunted::hook_SetConfigValue, registry));
		
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, UInt32, BSFixedString>("GetConfigValueInt", "Undaunted_SystemScript", Undaunted::hook_GetConfigValueInt, registry));

		//Regions
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, UInt32>("AddBadRegion", "Undaunted_SystemScript", Undaunted::hook_AddBadRegion, registry));

		//Groups
		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, UInt32, BSFixedString, BSFixedString, UInt32, UInt32, UInt32>("AddGroup", "Undaunted_SystemScript", Undaunted::hook_AddGroup, registry));

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void,UInt32,UInt32, BSFixedString, BSFixedString>("AddMembertoGroup", "Undaunted_SystemScript", Undaunted::hook_AddMembertoGroup, registry));

		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, UInt32, BSFixedString, UInt32>("GetModForm", "Undaunted_SystemScript", Undaunted::hook_GetModForm, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, TESObjectREFR*>("SetGroupMemberComplete", "Undaunted_SystemScript", Undaunted::hook_SetGroupMemberComplete, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, TESObjectREFR*>("IsGroupMemberUsed", "Undaunted_SystemScript", Undaunted::hook_IsGroupMemberUsed, registry));

		registry->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, VMResultArray<TESObjectREFR*>, BSFixedString>("GetBountyObjectRefs", "Undaunted_SystemScript", Undaunted::hook_GetBountyObjectRefs, registry));

		registry->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, void>("SetScriptedDoorsComplete", "Undaunted_SystemScript", Undaunted::hook_SetScriptedDoorsComplete, registry));

		//Rewards
		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, TESForm*, UInt32, UInt32>("SpawnRandomReward", "Undaunted_SystemScript", Undaunted::hook_SpawnRandomReward, registry));


		return true;
	}
}
