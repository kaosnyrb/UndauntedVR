#include <Undaunted\ConfigUtils.h>
#include "StartupManager.h"
#include "RSJparser.tcc"
#include <filesystem>

namespace Undaunted {
	RSJresource currentfile;
	void LoadJson(const char* filepath)
	{
		_MESSAGE("Loading %s", filepath);
		std::ifstream t(filepath);
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		std::string buffer(size, ' ');
		t.seekg(0);
		t.read(&buffer[0], size);
		RSJresource my_json(buffer.c_str());
		currentfile = my_json;
	}

	void LoadSettings()
	{
		LoadJson("Data/Undaunted/Settings.json");
		RSJresource settings = currentfile; 

		auto data = settings.as_array();
		_MESSAGE("size: %i", data.size());
		for (int i = 0; i < data.size(); i++)
		{
			auto inner = data[i].as_array();
			std::string key = inner[0].as<std::string>("default string");
			std::string value = inner[1].as<std::string>("default string");
			AddConfigValue(key.c_str(), value.c_str());
		}

		LoadJson("Data/Undaunted/RewardModBlacklist.json");
		auto modblacklist = currentfile.as_array();
		for (int i = 0; i < modblacklist.size(); i++)
		{
			std::string modname = modblacklist[i].as<std::string>("default string");
			AddRewardBlacklist(modname);
			_MESSAGE("RewardModBlacklist modname: %s", modname.c_str());
		}

		LoadJson("Data/Undaunted/Safezones.json");
		auto Safezones = currentfile.as_array();
		for (int i = 0; i < Safezones.size(); i++)
		{
			auto obj = Safezones[i].as_object();
			std::string Zonename = obj.at("Zonename").as<std::string>("default string");
			std::string Worldspace = obj.at("Worldspace").as<std::string>("default string");
			int PosX = obj.at("PosX").as<int>();
			int PosY = obj.at("PosY").as<int>();
			int PosZ = obj.at("PosZ").as<int>();
			int Radius = obj.at("Radius").as<int>();
			Safezone zone = Safezone();
			zone.Zonename = Zonename;
			zone.Worldspace = Worldspace;
			zone.PosX = PosX;
			zone.PosY = PosY;
			zone.PosZ = PosZ;
			zone.Radius = Radius;
			AddSafezone(zone);
			_MESSAGE("Safezone %s, %s, %i, %i, %i, %i", Zonename.c_str(), Worldspace.c_str(), PosX, PosY, PosZ, Radius);
		}
	}

	void LoadGroups()
	{
		DataHandler* dataHandler = GetDataHandler();
		_MESSAGE("Loading Groups...");
		std::string path = "Data/Undaunted/Groups";
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			auto filename = entry.path().u8string();
			_MESSAGE("file: %s", filename.c_str());
			if (entry.is_regular_file())
			{
				LoadJson(filename.c_str());
				RSJresource settings = currentfile;
				auto data = settings.as_array();
				_MESSAGE("size: %i", data.size());
				for (int i = 0; i < data.size(); i++)
				{
					auto group = data[i].as_array();
					std::string groupname = group[0][0].as<std::string>("groupname");
					std::string modreq = group[0][1].as<std::string>("modreq");
					int minlevel = group[0][2].as<int>(0);
					int maxlevel = group[0][3].as<int>(0);
					const ModInfo* modInfo = dataHandler->LookupModByName(modreq.c_str());
					if (modInfo != NULL)
					{
						int groupid = AddGroup(groupname, minlevel, maxlevel);
						for (int j = 1; j < group.size(); j++)
						{
							std::string esp = group[j][1].as<std::string>("esp");
							const ModInfo* modInfo = dataHandler->LookupModByName(esp.c_str());
							int form = group[j][2].as<int>(0);
							if (modInfo != NULL)
							{
								form = (modInfo->modIndex << 24) + form;
							}
							std::string type = group[j][3].as<std::string>("type");
							std::string model = std::string("");
							if (group[j].size() > 3)
							{
								model = group[j][4].as<std::string>("");
							}
							GroupMember newmember = GroupMember();
							newmember.BountyType = type.c_str();
							newmember.FormId = form;
							newmember.ModelFilepath = model.c_str();
							AddMembertoGroup(groupid, newmember);
						}
					}
				}
			}
		}
		_MESSAGE("Groups loaded: %i", GetGroupCount());
	}
}
