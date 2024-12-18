#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_HPP_

#include <stddef.h>

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitykeyvalues.h>
#include <tier1/keyvalues3.h>
#include <tier1/utlmap.h>
#include <tier1/utlsymbollarge.h>
#include <tier1/utlvector.h>

#include "logger.hpp"

#include <ientitymgr.hpp>
#include "provider_agent/resourcemanifest.hpp"
#include "provider_agent/spawngroup.hpp"

#define LOGGER_COLOR_KEYVALUES {241, 160, 50, 255}
#define LOGGER_COLOR_ENTITY_KV3 {0, 191, 255, 255}

class KeyValues;
class CEntityKeyValues;

namespace EntityManager
{
	class ProviderAgent : public IEntityManager::IProviderAgent, public IEntityManager::IProviderAgent::ISpawnGroupNotifications
	{
		using This = ProviderAgent;

	public:
		using ISpawnGroupInstance = IEntityManager::IProviderAgent::ISpawnGroupInstance;

	public:
		ProviderAgent();

		bool Init();
		void Clear();
		void Destroy();

	public:
		CUtlSymbolLarge AllocPooledString(const char *pString) override;
		CUtlSymbolLarge FindPooledString(const char* pString) override;

	public:
		virtual bool NotifyGameResourceUpdated();
		virtual bool NotifyGameSystemUpdated();
		virtual bool NotifyEntitySystemUpdated();
		virtual bool NotifyGameEventsUpdated();
		virtual bool NotifySpawnGroupMgrUpdated(CSpawnGroupMgrGameSystem *pSpawnGroupManager = NULL);

	public:
		bool ErectResourceManifest(ISpawnGroup *pSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset) override;
		IEntityResourceManifest *GetEntityManifest() override;
		void AddResourceToEntityManifest(IEntityResourceManifest *pManifest, const char *pszPath) override;

	public:
		ISpawnGroupInstance *CreateSpawnGroup() override;
		bool ReleaseSpawnGroup(ISpawnGroupInstance *pSpawnGroup) override;

	protected:
		void ReleaseSpawnGroups();

	public: // IEntityManager::IProviderAgent::ISpawnGroupNotifications
		void OnSpawnGroupAllocated(SpawnGroupHandle_t hSpawnGroup, ISpawnGroup *pSpawnGroup) override;
		void OnSpawnGroupInit(SpawnGroupHandle_t hSpawnGroup, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry) override;
		void OnSpawnGroupCreateLoading(SpawnGroupHandle_t hSpawnGroup, CMapSpawnGroup *pMapSpawnGroup, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, CUtlVector<const CEntityKeyValues *> &vecKeyValues) override;
		void OnSpawnGroupDestroyed(SpawnGroupHandle_t handle) override;

	public: // Spawn queue methods.
		void PushSpawnQueueOld(KeyValues *pOldOne, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) override;
		void PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) override;
		int CopySpawnQueueWithEntitySystemOwnership(CUtlVector<const CEntityKeyValues *> &vecTarget, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) override;
		bool HasInSpawnQueue(const CEntityKeyValues *pKeyValues, SpawnGroupHandle_t *pResultHandle = nullptr) override;
		bool HasInSpawnQueue(SpawnGroupHandle_t hSpawnGroup) override;
		int ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) override;
		int ExecuteSpawnQueued(SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP, CUtlVector<CEntityInstance *> *pEntities = nullptr, IEntityListener *pListener = nullptr, CUtlVector<CUtlString> *pDetails = nullptr, CUtlVector<CUtlString> *pWarnings = nullptr) override;

	public: // Destroy queue methods.
		void PushDestroyQueue(CEntityInstance *pEntity) override;
		void PushDestroyQueue(CEntityIdentity *pEntity) override;
		int AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> &vecTarget) override;
		void ReleaseDestroyQueued() override;
		int ExecuteDestroyQueued() override;

	public: // Dumps.
		enum DumpEntityKeyValuesFlags_t : uint8
		{
			DEKVF_NONE = 0,
			DEKVF_TYPE = (1 << 2),
			DEKVF_SUBTYPE = (1 << 3),
		};

		static const DumpEntityKeyValuesFlags_t s_eDefaultDEKVFlags = DEKVF_NONE;

		static bool DumpOldKeyValues(KeyValues *pOldOne, Logger::Scope &aOutput, Logger::Scope *paWarnings = nullptr);
		static bool DumpEntityKeyValues(const CEntityKeyValues *pKeyValues, DumpEntityKeyValuesFlags_t eFlags, Logger::Scope &aOutput, Logger::Scope *paWarnings = nullptr);
		static int DumpEntityKeyValue(KeyValues3 *pMember, char *psBuffer, size_t nMaxLength);
		static bool MakeDumpColorAlpha(Color &rgba);

	private:
		class SpawnData
		{
			typedef SpawnData ThisClass;

		public:
			SpawnData(CEntityKeyValues *pKeyValues);
			SpawnData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup);
			~SpawnData();

			const CEntityKeyValues *GetKeyValues() const;

			SpawnGroupHandle_t GetSpawnGroup() const;
			bool IsAnySpawnGroup() const;

			void Release();

		private:
			CEntityKeyValues *m_pKeyValues;
			SpawnGroupHandle_t m_hSpawnGroup;
		};

		CUtlVector<SpawnData> m_vecEntitySpawnQueue;

		struct DestoryData
		{
			typedef DestoryData ThisClass;

		public:
			DestoryData(CEntityInstance *pInstance);
			DestoryData(CEntityIdentity *pIdentity);
			~DestoryData();

			CEntityIdentity *GetIdnetity() const;

		private:
			CEntityIdentity *m_pIdentity;
		};

		CKeyValues3Context m_aEntityAllocator;

		CUtlVector<DestoryData> m_vecEntityDestroyQueue;

		ResourceManifest m_aResourceManifest;
		CUtlVector<ISpawnGroupInstance *> m_vecSpawnGroups;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_HPP_
