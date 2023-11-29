//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IGAMESYSTEM_H
#define IGAMESYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include <string.h>
#include <tier0/platform.h>

struct EventGameInit_t;
struct EventGameShutdown_t;
struct EventGamePostInit_t;
struct EventGamePreShutdown_t;
struct EventBuildGameSessionManifest_t;
struct EventGameActivate_t;
struct EventClientFullySignedOn_t;
struct EventDisconnect_t;
struct EventGameDeactivate_t;
struct EventSpawnGroupPrecache_t;
struct EventSpawnGroupUncache_t;
struct EventPreSpawnGroupLoad_t;
struct EventPostSpawnGroupLoad_t;
struct EventPreSpawnGroupUnload_t;
struct EventPostSpawnGroupUnload_t;
struct EventActiveSpawnGroupChanged_t;
struct EventClientPostDataUpdate_t;
struct EventClientPreRender_t;
struct EventClientPreEntityThink_t;
struct EventClientUpdate_t;
struct EventClientPostRender_t;
struct EventServerPreEntityThink_t;
struct EventServerPostEntityThink_t;
struct EventServerPreClientUpdate_t;
struct EventServerGamePostSimulate_t;
struct EventClientGamePostSimulate_t;
struct EventGameFrameBoundary_t;
struct EventOutOfGameFrameBoundary_t;
struct EventSaveGame_t;
struct EventRestoreGame_t;

//-----------------------------------------------------------------------------
// Game systems are singleton objects in the client + server codebase responsible for
// various tasks
// The order in which the server systems appear in this list are the
// order in which they are initialized and updated. They are shut down in
// reverse order from which they are initialized.
//-----------------------------------------------------------------------------
abstract_class IGameSystem
{
public:
	// Init, shutdown
	// return true on success. false to abort DLL init!
	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;

	// Game init, shutdown
	virtual void GameInit(const EventGameInit_t* const msg) = 0;
	virtual void GameShutdown(const EventGameShutdown_t* const msg) = 0;
	virtual void GamePostInit(const EventGamePostInit_t* const msg) = 0;
	virtual void GamePreShutdown(const EventGamePreShutdown_t* const msg) = 0;

	virtual void BuildGameSessionManifest(const EventBuildGameSessionManifest_t* const msg) = 0;

	virtual void GameActivate(const EventGameActivate_t* const msg) = 0;

	virtual void ClientFullySignedOn(const EventClientFullySignedOn_t* const msg) = 0;
	virtual void ClientDisconnect(const EventDisconnect_t* const msg) = 0;

	virtual void GameDeactivate(const EventGameDeactivate_t* const msg) = 0;

	virtual void SpawnGroupPrecache(const EventSpawnGroupPrecache_t* const msg) = 0;
	virtual void SpawnGroupUncache(const EventSpawnGroupUncache_t* const msg) = 0;
	virtual void PreSpawnGroupLoad(const EventPreSpawnGroupLoad_t* const msg) = 0;
	virtual void PostSpawnGroupLoad(const EventPostSpawnGroupLoad_t* const msg) = 0;
	virtual void PreSpawnGroupUnload(const EventPreSpawnGroupUnload_t* const msg) = 0;
	virtual void PostSpawnGroupUnload(const EventPostSpawnGroupUnload_t* const msg) = 0;
	virtual void ActiveSpawnGroupChanged(const EventActiveSpawnGroupChanged_t* const msg) = 0;

	virtual void PostDataUpdate(EventClientPostDataUpdate_t* const msg) = 0;

	// Called before rendering
	virtual void PreRender(const EventClientPreRender_t* const msg) = 0;

	virtual void ClientPreEntityThink(const EventClientPreEntityThink_t* const msg) = 0;

	virtual void nullsub_1271(const void* const msg) = 0;

	// Gets called each frame
	virtual void Update(const EventClientUpdate_t* const msg) = 0;

	// Called after rendering
	virtual void PostRender(const EventClientPostRender_t* const msg) = 0;

	// Called each frame before entities think
	virtual void FrameUpdatePreEntityThink(const EventServerPreEntityThink_t* const msg) = 0;
	// called after entities think
	virtual void FrameUpdatePostEntityThink(const EventServerPostEntityThink_t* const msg) = 0;
	virtual void PreClientUpdate(const EventServerPreClientUpdate_t* const msg) = 0;

	virtual void nullsub_1277(const void* const msg) = 0;
	virtual void nullsub_1278(const void* const msg) = 0;

	virtual void OnServerGamePostSimulate(const EventServerGamePostSimulate_t* const msg) = 0;
	virtual void OnClientGamePostSimulate(const EventClientGamePostSimulate_t* const msg) = 0;

	virtual void nullsub_1281(const void* const msg) = 0;

	virtual void FrameBoundary(const EventGameFrameBoundary_t* const msg) = 0;
	virtual void OutOfGameFrameBoundary(const EventOutOfGameFrameBoundary_t* const msg) = 0;

	virtual void SaveGame(const EventSaveGame_t* const msg) = 0;
	virtual void RestoreGame(const EventRestoreGame_t* const msg) = 0;

	virtual void nullsub_1285(const void* const msg) = 0;
	virtual void nullsub_1286(const void* const msg) = 0;
	virtual void nullsub_1287(const void* const msg) = 0;
	virtual void nullsub_1288(const void* const msg) = 0;
	virtual void nullsub_1289(const void* const msg) = 0;

	virtual const char* GetName() = 0;
	virtual void SetGameSystemGlobalPtrs(void* pValue) = 0;
	virtual void SetName(const char* pName) = 0;
	virtual bool DoesGameSystemReallocate() = 0;
	virtual ~IGameSystem() = 0;
	virtual void YouForgot_DECLARE_GAME_SYSTEM_InYourClassDefinition() = 0;
};

// Quick and dirty server system for users who don't care about precise ordering
// and usually only want to implement a few of the callbacks
class CBaseGameSystem : public IGameSystem
{
public:
	CBaseGameSystem(const char* pszInitName = "unnamed")
	 :  m_pName(pszInitName)
	{
	}

	// Init, shutdown
	// return true on success. false to abort DLL init!
	virtual bool Init() { return true; }
	virtual void PostInit() {}
	virtual void Shutdown() {}

	// Game init, shutdown
	virtual void GameInit(const EventGameInit_t* const msg) {}
	virtual void GameShutdown(const EventGameShutdown_t* const msg) {}
	virtual void GamePostInit(const EventGamePostInit_t* const msg) {}
	virtual void GamePreShutdown(const EventGamePreShutdown_t* const msg) {}

	virtual void BuildGameSessionManifest(const EventBuildGameSessionManifest_t* const msg) {}

	virtual void GameActivate(const EventGameActivate_t* const msg) {}

	virtual void ClientFullySignedOn(const EventClientFullySignedOn_t* const msg) {}
	virtual void ClientDisconnect(const EventDisconnect_t* const msg) {}

	virtual void GameDeactivate(const EventGameDeactivate_t* const msg) {}

	virtual void SpawnGroupPrecache(const EventSpawnGroupPrecache_t* const msg) {}
	virtual void SpawnGroupUncache(const EventSpawnGroupUncache_t* const msg) {}
	virtual void PreSpawnGroupLoad(const EventPreSpawnGroupLoad_t* const msg) {}
	virtual void PostSpawnGroupLoad(const EventPostSpawnGroupLoad_t* const msg) {}
	virtual void PreSpawnGroupUnload(const EventPreSpawnGroupUnload_t* const msg) {}
	virtual void PostSpawnGroupUnload(const EventPostSpawnGroupUnload_t* const msg) {}
	virtual void ActiveSpawnGroupChanged(const EventActiveSpawnGroupChanged_t* const msg) {}

	virtual void PostDataUpdate(EventClientPostDataUpdate_t* const msg) {}

	// Called before rendering
	virtual void PreRender(const EventClientPreRender_t* const msg) {}

	virtual void ClientPreEntityThink(const EventClientPreEntityThink_t* const msg) {}

	virtual void nullsub_1271(const void* const msg) {}

	// Gets called each frame
	virtual void Update(const EventClientUpdate_t* const msg) {}

	// Called after rendering
	virtual void PostRender(const EventClientPostRender_t* const msg) {}

	// Called each frame before entities think
	virtual void FrameUpdatePreEntityThink(const EventServerPreEntityThink_t* const msg) {}
	// called after entities think
	virtual void FrameUpdatePostEntityThink(const EventServerPostEntityThink_t* const msg) {}
	virtual void PreClientUpdate(const EventServerPreClientUpdate_t* const msg) {}

	virtual void nullsub_1277(const void* const msg) {}
	virtual void nullsub_1278(const void* const msg) {}

	virtual void OnServerGamePostSimulate(const EventServerGamePostSimulate_t* const msg) {}
	virtual void OnClientGamePostSimulate(const EventClientGamePostSimulate_t* const msg) {}

	virtual void nullsub_1281(const void* const msg) {}

	virtual void FrameBoundary(const EventGameFrameBoundary_t* const msg) {}
	virtual void OutOfGameFrameBoundary(const EventOutOfGameFrameBoundary_t* const msg) {}

	virtual void SaveGame(const EventSaveGame_t* const msg) {}
	virtual void RestoreGame(const EventRestoreGame_t* const msg) {}

	virtual void nullsub_1285(const void* const msg) {}
	virtual void nullsub_1286(const void* const msg) {}
	virtual void nullsub_1287(const void* const msg) {}
	virtual void nullsub_1288(const void* const msg) {}
	virtual void nullsub_1289(const void* const msg) {}

	virtual const char* GetName() { return m_pName; }
	virtual void SetGameSystemGlobalPtrs(void* pValue) {}
	virtual void SetName(const char* pName) { m_pName = pName; }
	virtual bool DoesGameSystemReallocate() { return false; }
	virtual ~CBaseGameSystem() {}

private:
	const char* m_pName;
};

class CAutoGameSystem : public CBaseGameSystem
{
protected:
	virtual ~CAutoGameSystem() {};
};

class IGameSystemFactory
{
public:
	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;
	virtual IGameSystem* CreateGameSystem() = 0;
	virtual void DestroyGameSystem(IGameSystem* pGameSystem) = 0;
	virtual bool ShouldAutoAdd() = 0;
	virtual int GetPriority() = 0;
	virtual void SetGlobalPtr(void* pValue) = 0;
	virtual bool IsReallocating() = 0;
	virtual IGameSystem* GetStaticGameSystem() = 0;
};

class CBaseGameSystemFactory : public IGameSystemFactory
{
protected:
	CBaseGameSystemFactory(const char* pName)
	{
		m_pName = pName;
		m_pNext = *CBaseGameSystemFactory::sm_pFirst;
		*CBaseGameSystemFactory::sm_pFirst = this;
	}
private:
	CBaseGameSystemFactory* m_pNext;
	const char* m_pName;
public:
	static IGameSystem* GetGlobalPtrByName(const char* pName)
	{
		CBaseGameSystemFactory* pFactoryList = *CBaseGameSystemFactory::sm_pFirst;
		while (pFactoryList)
		{
			if (strcmp(pFactoryList->m_pName, pName) == 0)
			{
				return pFactoryList->GetStaticGameSystem();
			}
			pFactoryList = pFactoryList->m_pNext;
		}
		return nullptr;
	}
	static CBaseGameSystemFactory** sm_pFirst;
};
template <class T, class U = T>
class CGameSystemStaticFactory : public CBaseGameSystemFactory
{
public:
	CGameSystemStaticFactory(const char* pName, T* pActualGlobal, U** ppGlobalPointer = nullptr) : CBaseGameSystemFactory(pName)
	{
		m_pActualGlobal = pActualGlobal;
		m_ppGlobalPointer = ppGlobalPointer;
		pActualGlobal->SetName(pName);
	}
	bool Init() override { return m_pActualGlobal->Init(); }
	void PostInit() override { m_pActualGlobal->PostInit(); }
	void Shutdown() override { m_pActualGlobal->Shutdown(); }
	IGameSystem* CreateGameSystem() override
	{
		m_pActualGlobal->SetGameSystemGlobalPtrs(m_pActualGlobal);
		return m_pActualGlobal;
	}
	void DestroyGameSystem(IGameSystem* pGameSystem) override
	{
		m_pActualGlobal->SetGameSystemGlobalPtrs(nullptr);
	}
	bool ShouldAutoAdd() override { return true; }
	int GetPriority() override { return 0; }
	void SetGlobalPtr(void* pValue) override
	{
		if (m_ppGlobalPointer)
		{
			*m_ppGlobalPointer = reinterpret_cast<T*>(pValue);
		}
	}
	bool IsReallocating() override { return false; }
	IGameSystem* GetStaticGameSystem() override { return m_pActualGlobal; }
private:
	T* m_pActualGlobal;
	U** m_ppGlobalPointer;
};
template <class T, class U = T>
class CGameSystemReallocatingFactory : public CBaseGameSystemFactory
{
public:
	CGameSystemReallocatingFactory(const char* pName, U** ppGlobalPointer = nullptr) : CBaseGameSystemFactory(pName)
	{
		m_ppGlobalPointer = ppGlobalPointer;
	}
	bool Init() override { return true; }
	void PostInit() override { }
	void Shutdown() override { }
	IGameSystem* CreateGameSystem() override
	{
		T* pObject = new T();
		pObject->SetGameSystemGlobalPtrs(pObject);
		return pObject;
	}
	void DestroyGameSystem(IGameSystem* pGameSystem) override
	{
		pGameSystem->SetGameSystemGlobalPtrs(nullptr);
		delete pGameSystem;
	}
	bool ShouldAutoAdd() override { return true; }
	int GetPriority() override { return 0; }
	void SetGlobalPtr(void* pValue) override
	{
		if (m_ppGlobalPointer)
		{
			*m_ppGlobalPointer = reinterpret_cast<T*>(pValue);
		}
	}
	bool IsReallocating() override { return true; }
	IGameSystem* GetStaticGameSystem() override { return nullptr; }
private:
	U** m_ppGlobalPointer;
};


#endif // IGAMESYSTEM_H