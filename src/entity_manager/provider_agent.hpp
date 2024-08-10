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
#include "provider_agent/resourcemanifest.hpp"
#include "provider_agent/spawngroup.hpp"

#define LOGGER_COLOR_KEYVALUES {241, 160, 50, 255}
#define LOGGER_COLOR_ENTITY_KV3 {0, 191, 255, 255}

class KeyValues;
class CEntityKeyValues;

namespace EntityManager
{
	class ProviderAgent : public ISpawnGroupNotifications
	{
		using This = ProviderAgent;

	public:
		ProviderAgent();

		bool Init();
		void Clear();
		void Destroy();

	public:
		CUtlSymbolLarge AllocPooledString(const char *pString);
		CUtlSymbolLarge FindPooledString(const char* pString);

	public:
		virtual bool NotifyGameResourceUpdated();
		virtual bool NotifyGameSystemUpdated();
		virtual bool NotifyEntitySystemUpdated();
		virtual bool NotifyGameEventsUpdated();
		virtual bool NotifySpawnGroupMgrUpdated();

	public:
		bool ErectResourceManifest(ISpawnGroup *pSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset);
		IEntityResourceManifest *GetMyEntityManifest();

	public:
		bool CreateSpawnGroup(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset);
		void ReleaseSpawnGroups();

	public: // ISpawnGroupNotifications
		void NotifyAllocateSpawnGroup(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup);
		void NotifyDestroySpawnGroup(SpawnGroupHandle_t handle);

	public: // Spawn queue methods.
		void PushSpawnQueueOld(KeyValues *pOldOne, SpawnGroupHandle_t hSpawnGroup = INVALID_SPAWN_GROUP, Logger::Scope *pWarnings = nullptr);
		void PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup = INVALID_SPAWN_GROUP);
		int AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> &vecTarget, SpawnGroupHandle_t hSpawnGroup = INVALID_SPAWN_GROUP);
		bool HasInSpawnQueue(const CEntityKeyValues *pKeyValues);
		bool HasInSpawnQueue(SpawnGroupHandle_t hSpawnGroup);
		int ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup = INVALID_SPAWN_GROUP);
		int SpawnQueued(SpawnGroupHandle_t hSpawnGroup = INVALID_SPAWN_GROUP, Logger::Scope *pDetails = nullptr, Logger::Scope *pWarnings = nullptr);

	public: // Destroy queue methods.
		void PushDestroyQueue(CEntityInstance *pEntity);
		void PushDestroyQueue(CEntityIdentity *pEntity);
		int AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> &vecTarget);
		void ReleaseDestroyQueued();
		int DestroyQueued();

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

			CEntityKeyValues *GetKeyValues() const;

			SpawnGroupHandle_t GetSpawnGroup() const;
			static SpawnGroupHandle_t GetAnySpawnGroup();
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
		CUtlVector<SpawnGroup *> m_vecSpawnGroups;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_HPP_
