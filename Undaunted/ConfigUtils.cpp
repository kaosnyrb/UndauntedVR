#include "ConfigUtils.h"
#include <time.h>
#include "SafezoneList.h"

namespace Undaunted
{
	std::string s_configPath;
	IntList BadRegionList;
	ListLibary GroupLibary;
	UnDictionary SettingsList;

	//Regions
	void AddBadRegionToConfig(UInt32 region)
	{
		_MESSAGE("Adding %08X to Bad Region List", region);
		BadRegionList.AddItem(region);
	}

	IntList GetBadRegions() {
		return BadRegionList;
	}

	//Groups
	int AddGroup(std::string questText, UInt32 minlevel, UInt32 maxlevel, UnStringlist tags)
	{
		_MESSAGE("Adding bounty to GroupLibary: %s", questText.c_str());
		GroupList newGroup = GroupList();
		newGroup.questText = questText;
		newGroup.minLevel = minlevel;
		newGroup.maxLevel = maxlevel;
		newGroup.Tags = tags;
		GroupLibary.AddItem(newGroup);
		return GroupLibary.length - 1;
	}

	void AddMembertoGroup(int id, GroupMember member)
	{
		//_MESSAGE("Adding %08X to %i of BountyType %s",member.FormId, id,member.BountyType.Get());
		GroupLibary.data[id].AddItem(member);
	}

	GroupList GetGroup(std::string bountyName)
	{
		for (int i = 0; i < GroupLibary.length; i++)
		{
			if (GroupLibary.data[i].questText.compare(bountyName) == 0)
			{
				return GroupLibary.data[i];
			}
		}
		//All else fails return something at least.
		return GetRandomGroup();
	}

	int GetGroupCount()
	{
		return GroupLibary.length;
	}

	void ShuffleGroupLibary()
	{
		srand(time(NULL));
		for (int i = 0; i < GroupLibary.length + 100; i++)
		{
			GroupLibary.SwapItem(rand() % GroupLibary.length, rand() % GroupLibary.length);
		}
	}

	
	int GroupLibaryIndex = 0;
	GroupList GetRandomGroup()
	{
		UInt32 playerLevel = GetPlayerLevel();
		while (true)
		{
			int groupid = GroupLibaryIndex++;
			_MESSAGE("Random Group: %i", groupid);
			if (groupid >= GroupLibary.length)
			{
				ShuffleGroupLibary();
				GroupLibaryIndex = 0;
			}
			_MESSAGE("Random Member Count: %i", GroupLibary.data[groupid].length);
			//Player is too low level for this bounty
			if (playerLevel + GetConfigValueInt("BountyLevelCache") < GroupLibary.data[groupid].minLevel && GroupLibary.data[groupid].minLevel != 0)
			{
				continue;
			}
			//Player is too high level for this bounty
			if (playerLevel > GroupLibary.data[groupid].maxLevel && GroupLibary.data[groupid].maxLevel != 0)
			{
				continue;
			}
			return GroupLibary.data[groupid];
		}
	}

	GroupList GetRandomTaggedGroup(std::string tag)
	{
		UInt32 playerLevel = GetPlayerLevel();
		int startingGroupLibaryIndex = GroupLibaryIndex;
		while(true)
		{
			int groupid = GroupLibaryIndex++;
			if (groupid >= GroupLibary.length)
			{
				ShuffleGroupLibary();
				GroupLibaryIndex = 0;
				startingGroupLibaryIndex = 0;
			}
			_MESSAGE("Random Group: %i", groupid);
			_MESSAGE("Random Member Count: %i", GroupLibary.data[groupid].length);
			//Player is too low level for this bounty
			if (playerLevel + GetConfigValueInt("BountyLevelCache") < GroupLibary.data[groupid].minLevel && GroupLibary.data[groupid].minLevel != 0)
			{
				continue;
			}
			//Player is too high level for this bounty
			if (playerLevel > GroupLibary.data[groupid].maxLevel && GroupLibary.data[groupid].maxLevel != 0)
			{
				continue;
			}
			
			for (int i = 0; i < GroupLibary.data[groupid].Tags.length; i++)
			{
				_MESSAGE("Comparing Tag: %s = %s", GroupLibary.data[groupid].Tags.data[i].c_str(), tag.c_str());
				if (GroupLibary.data[groupid].Tags.data[i].compare(tag) == 0)
				{
					_MESSAGE("Found Tag: %s", GroupLibary.data[groupid].Tags.data[i].c_str());
					if (GroupLibaryIndex > GroupLibary.length)
					{
						ShuffleGroupLibary();
						GroupLibaryIndex = 0;
					}
					//Take the tagged card out of the pack and place it on top, then draw it.
					GroupLibary.SwapItem(groupid, startingGroupLibaryIndex);
					GroupLibaryIndex = startingGroupLibaryIndex + 1;
					return GroupLibary.data[startingGroupLibaryIndex];
				}
			}
		}
	}

	void AddConfigValue(std::string key, std::string value)
	{
		//_MESSAGE("CONFIGLENGTH: %i", SettingsList.length);
		//check if it exists		
		for (int i = 0; i < SettingsList.length; i++)
		{
			if (SettingsList.data[i].key.compare(key) == 0)
			{
				SettingsList.data[i].value = value;
				//_MESSAGE("SET: %s : %s", key, value);
				return;
			}
		}
		//doesn't exist
		UnKeyValue setting = UnKeyValue();
		setting.key = key;
		setting.value = value;
		SettingsList.AddItem(setting);
		//_MESSAGE("ADD: %s : %s", key.c_str(), value.c_str());
	}

	UInt32 GetConfigValueInt(std::string key)
	{
		for (int i = 0; i < SettingsList.length; i++)
		{
			//_MESSAGE("Comparing %s : %s", key.c_str(), SettingsList.data[i].key.c_str());
			if (SettingsList.data[i].key.compare(key) == 0)
			{
				//_MESSAGE("Found Key %s : %s", key.c_str(), SettingsList.data[i].value.c_str());
				return atoi(SettingsList.data[i].value.c_str());
			}
		}
		//Not found.
		return 0;
	}

	UInt32 Playerlevel;
	void SetPlayerLevel(UInt32 level)
	{
		Playerlevel = level;
	}

	UInt32 GetPlayerLevel()
	{
		return Playerlevel;
	}

	UnDictionary RewardBlacklist = UnDictionary();
	void AddRewardBlacklist(std::string key)
	{
		UnKeyValue data = UnKeyValue();
		data.value = key;
		RewardBlacklist.AddItem(data);
	}

	UnDictionary getRewardBlacklist()
	{
		return RewardBlacklist;
	}

	SafezoneList szlist = SafezoneList();

	void AddSafezone(Safezone zone)
	{
		szlist.AddItem(zone);
	}

	SafezoneList GetSafezones()
	{
		return szlist;
	}

}