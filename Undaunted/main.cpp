#include <Undaunted\MyPlugin.h>
#include <Undaunted\BountyManager.h>
#include <Undaunted\ConfigUtils.h>
#include <Undaunted\SKSELink.h>

static PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
static SKSEPapyrusInterface         * g_papyrus = NULL;
SKSESerializationInterface* g_serialization = NULL;
SKSEMessagingInterface* g_messageInterface = NULL;

extern "C"	{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)	{	// Called by SKSE to learn about this plugin and check that it's safe to load it
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim VR\\SKSE\\Undaunted.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
	
		_MESSAGE("Undaunted");

		// populate info structure
		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Undaunted";
		info->version =		1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if(skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}
		/*
		else if(skse->runtimeVersion != RUNTIME_VERSION_1_4_15)
		{
			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}*/

		g_serialization = (SKSESerializationInterface*)skse->QueryInterface(kInterface_Serialization);
		g_messageInterface = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		// supported runtime version
		return true;
	}

	void SKSEMessageReceptor(SKSEMessagingInterface::Message* msg)
	{
		static bool active = true;
		if (!active)
			return;

		if (msg->type == SKSEMessagingInterface::kMessage_PreLoadGame)
		{
			//We're loading the game. Clear up any bounty data.
			_MESSAGE("kMessage_PreLoadGame rechieved, clearing bounty data.");
			Undaunted::BountyManager::getInstance()->ClearBountyData();
		}

		//Register to recieve interface from Enchantment Framework
		//if (msg->type == SKSEMessagingInterface::kMessage_PostLoad)


		//kMessage_InputLoaded only sent once, on initial Main Menu load
		//else if (msg->type == SKSEMessagingInterface::kMessage_InputLoaded)

	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)	{	// Called by SKSE to load this plugin
		_MESSAGE("Loading Undaunted..");

		g_papyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
		g_messageInterface->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageReceptor);

		//Check if the function registration was a success...
		bool btest = g_papyrus->Register(Undaunted::RegisterFuncs);

		if (btest) {
			_MESSAGE("Register Succeeded");
		}

		return true;
	}
};