/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Entity Manager
 * Written by Wend4r.
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef _INCLUDE_METAMOD_SOURCE_IENTITYMGR_HPP_
#	define _INCLUDE_METAMOD_SOURCE_IENTITYMGR_HPP_

#	pragma once

#	include <stddef.h>

#	include <functional>

#	include <tier0/basetypes.h>
#	include <tier1/utlmap.h>
#	include <tier1/utlvector.h>
#	include <entity2/entityidentity.h>
#	include <gamesystems/spawngroup_manager.h>

#	define ENTITY_MANAGER_INTERFACE_NAME "Entity Manager v1.0"

#	define INVALID_SPAWN_GROUP ((SpawnGroupHandle_t)-1)
#	define ANY_SPAWN_GROUP INVALID_SPAWN_GROUP

/**
 * @brief A Entity Manager interface.
 * Note: gets with "reinterpret_cast<IEntityManager *>(ismm->MetaFactory(ENTITY_MANAGER_INTERFACE_NAME, NULL, NULL));"
 */
class IEntityManager
{
public: // Provider agent ones.
	/**
	 * @brief A provider agent interface.
	 */
	class IProviderAgent
	{
	public: // Spawn group ones.
		/**
		 * @brief A spawn group loader interface.
		 */
		class ISpawnGroupLoader
		{
		public:
			/**
			 * @brief Load the spawn group by a description.
			 * 
			 * @param aDesc             A spawn group description. 
			 *                          Set an unique name to `aDesc.m_sLocalNameFixup` field.
			 * @param vecLandmarkOffset A landmark offset.
			 * 
			 * @return                  Returns true if successed added to the queue, 
			 *                          otherwise false if failed to add.
			 */
			virtual bool Load(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset) = 0;

			/**
			 * @brief Unload the spawn group.
			 * 
			 * @return                  Returns true if successed added to the queue, 
			 *                          otherwise false if failed to add.
			 */
			virtual bool Unload() = 0;
		}; // ISpawnGroupLoader

		/**
		 * @brief A spawn group notifications interface.
		 */
		class ISpawnGroupNotifications
		{
		public:
			/**
			 * @brief Calls when spawn group are allocated.
			 * 
			 * @param hSpawnGroup       A spawn group handle to allocate.
			 * @param pSpawnGroup       A spawn group pointer.
			 */
			virtual void OnSpawnGroupAllocated(SpawnGroupHandle_t hSpawnGroup, ISpawnGroup *pSpawnGroup) {}

			/**
			 * @brief Calls when spawn group are initialize.
			 * 
			 * @param hSpawnGroup       A spawn group handle to initialize.
			 * @param pManifest         A entity resource manifest pointer.
			 * @param pConfig           A entity precache configuration pointer.
			 * @param pRegistry         A spawn group prerequisite registry pointer.
			 */
			virtual void OnSpawnGroupInit(SpawnGroupHandle_t hSpawnGroup, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry) {}

			/**
			 * @brief Calls when spawn group are create loading one.
			 * 
			 * @param hSpawnGroup       A spawn group handle to create loading.
			 * @param pMapSpawnGroup    A map spawn group pointer.
			 * @param bSynchronouslySpawnEntities   Are spawn group entities will create synchronously.
			 * @param bConfirmResourcesLoaded       A connfirm resource are loaded.
			 * @param vecKeyValues      A spawn group entity key values to add/remove.
			 */
			virtual void OnSpawnGroupCreateLoading(SpawnGroupHandle_t hSpawnGroup, CMapSpawnGroup *pMapSpawnGroup, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, CUtlVector<const CEntityKeyValues *> &vecKeyValues) {}

			/**
			 * @brief Calls when spawn group are destroyed.
			 * 
			 * @param hSpawnGroup       A spawn group handle to destroy.
			 */
			virtual void OnSpawnGroupDestroyed(SpawnGroupHandle_t hSpawnGroup) {}
		}; // ISpawnGroupNotifications

		/**
		 * @brief A spawn group callbacks interface.
		 */
		class ISpawnGroupCallbacks
		{
		public:
			/**
			 * @brief Adds notifications listener callback.
			 * 
			 * @param pNotifications    A notifications pointer to listen.
			 */
			virtual void AddNotificationsListener(ISpawnGroupNotifications *pNotifications) = 0;

			/**
			 * @brief Removes notifications listener callback.
			 * 
			 * @param pNotifications    A notifications pointer to remove listening.
			 */
			virtual bool RemoveNotificationsListener(ISpawnGroupNotifications *pNotifications) = 0;
		}; // ISpawnGroupCallbacks

		/**
		 * @brief A spawn group instance interface.
		 */
		class ISpawnGroupInstance : public ISpawnGroupLoader, public ISpawnGroupNotifications, public ISpawnGroupCallbacks
		{
		public:
			/**
			 * @brief Destructor of the instance, which calls the unload of the spawn group. 
			 *        Used internally, call `ReleaseSpawnGroup` instead.
			 */
			virtual ~ISpawnGroupInstance() = default;

			/**
			 * @brief Gets the status of the spawn group.
			 * 
			 * @return                  Returns a status value of the spawn group.
			 */
			virtual int GetStatus() const = 0;

			/**
			 * @brief Gets a spawn group.
			 * 
			 * @return                  Returns a spawn group pointer.
			 */
			virtual ISpawnGroup *GetSpawnGroup() const = 0;

			/**
			 * @brief Gets a map spawn group.
			 * 
			 * @return                  Returns a map spawn group pointer.
			 */
			virtual CMapSpawnGroup *GetMapSpawnGroup() const = 0;

			/**
			 * @brief Gets an allocated spawn group handle.
			 * 
			 * @return                  Returns a spawn group handle.
			 */
			virtual SpawnGroupHandle_t GetSpawnGroupHandle() const = 0;

			/**
			 * @brief Gets a level name string.
			 * 
			 * @return                  Returns a string of the name.
			 */
			virtual const char *GetLevelName() const = 0;

			/**
			 * @brief Gets a landmark name string.
			 * 
			 * @return                  Returns a string of the landmark name.
			 */
			virtual const char *GetLandmarkName() const = 0;

			/**
			 * @brief Gets a local name fixup name string.
			 * 
			 * @return                  Returns a local name fixup name.
			 */
			virtual const char *GetLocalFixupName() const = 0;

			/**
			 * @brief Gets a landmark offset.
			 * 
			 * @return                  Returns a vector of the offset.
			 */
			virtual const Vector &GetLandmarkOffset() const = 0;
		}; // ISpawnGroupInstance

		/**
		 * @brief A entity listener interface.
		 */
		class IEntityListener
		{
		public:
			/**
			 * @brief Calls when an entity are created.
			 * 
			 * @param pEntity           An entity who created.
			 * @param pKeyValues        Entity key values.
			 */
			virtual void OnEntityCreated(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues) = 0;
		}; // IEntityListener

	public: // A entity system things.
		/**
		 * @brief Allocates a pooled string of the entity system.
		 * 
		 * @param pString           A string to allocate.
		 */
		virtual CUtlSymbolLarge AllocPooledString(const char *pString) = 0;

		/**
		 * @brief Find a pooled string in the entity system.
		 * 
		 * @param pString           A string to find.
		 */
		virtual CUtlSymbolLarge FindPooledString(const char* pString) = 0;

	public:
		/**
		 * @brief Erect a resourse manifest with a spawn group.
		 * 
		 * @param pSpawnGroup       A spawn group.
		 * @param nCount            A count of entities.
		 * @param pEntities         A spawn infos of entities.
		 * @param vWorldOffset      A world offset.
		 */
		virtual bool ErectResourceManifest(ISpawnGroup *pSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset) = 0;

		/**
		 * @brief Gets an entity manifest.
		 * 
		 * @return                  An entity manifest pointer.
		 */
		virtual IEntityResourceManifest *GetResouceManifest() = 0;

		/**
		 * @brief Adds a resource to the entity resource manifest.
		 * 
		 * @param pManifest         A entity resource manifest pointer.
		 * @param pszPath           A path of the resource (models, matarials and etc.).
		 */
		virtual void AddResourceToEntityManifest(IEntityResourceManifest *pManifest, const char *pszPath) = 0;

	public: // Spawn group ones.
		/**
		 * @brief Creates a spawn group instance. 
		 *        To load, use the `Load` method
		 * 
		 * @return                  An entity manifest pointer.
		 */
		virtual ISpawnGroupInstance *CreateSpawnGroup() = 0;

		/**
		 * @brief Releases a spawn group instance. 
		 *        To load, use the `Load` method
		 * 
		 * @return                  Returns true if spawn group are released, 
		 *                          otherwise false if failed to find.
		 */
		virtual bool ReleaseSpawnGroup(ISpawnGroupInstance *pSpawnGroupInstance) = 0;

	public: // Create queued entities.
		/**
		 * @brief Push an old entity structure to the spawn queue.
		 * 
		 * @param pOldOne           An old entity structure.
		 * @param hSpawnGroup       A spawn group on which the entity should spawned.
		 *                          If ANY_SPAWN_GROUP, entity will spawned on the active spawn group.
		 */
		virtual void PushSpawnQueueOld(KeyValues *pOldOne, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) = 0;

		/**
		 * @brief Push an entity structure to the spawn queue.
		 * 
		 * @param pKeyValues        An entity structure.
		 * @param hSpawnGroup       A spawn group on which the entity should spawned.
		 *                          If ANY_SPAWN_GROUP, entity will spawned on the active spawn group.
		 */
		virtual void PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) = 0;

		/**
		 * @brief Copies the spawn queue to a vector with an entity system ownership.
		 * 
		 * @param vecTarget         A vector target to store.
		 * @param hSpawnGroup       An optional filter spawn group.
		 *                          If ANY_SPAWN_GROUP, will be added all entities.
		 * 
		 * @return                  Returns the number of added elements.
		 */
		virtual int CopySpawnQueueWithEntitySystemOwnership(CUtlVector<const CEntityKeyValues *> &vecTarget, SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) = 0;

		/**
		 * @brief Checks to has entity structure in the spawn queue.
		 * 
		 * @param pKeyValues        An entity structure to find.
		 * @param pResultHandle     An optional spawn group handle pointer of the found entity.
		 * 
		 * @return                  Returns true if found the the first entity by the spawn group handle, 
		 *                          otherwise false if not found.
		 */
		virtual bool HasInSpawnQueue(const CEntityKeyValues *pKeyValues, SpawnGroupHandle_t *pResultHandle = nullptr) = 0;

		/**
		 * @brief Checks to has spawn group in the spawn queue.
		 * 
		 * @param hSpawnGroup       A spawn group handle to find.
		 * 
		 * @return                  Returns true if found the the first entity by the spawn group handle, 
		 *                          otherwise false if not found.
		 */
		virtual bool HasInSpawnQueue(SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) = 0;

		/**
		 * @brief Releases/Destroy spawn queued entities.
		 * 
		 * @param hSpawnGroup       A spawn group filter.
		 * 
		 * @return                  Returns released count of the queued entities.
		 */
		virtual int ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP) = 0;

		/**
		 * @brief Executes the spawn queued entities.
		 * 
		 * @param hSpawnGroup       A spawn group to spawn.
		 * @param pEntities         An optional vector pointer of created entities.
		 * @param pListener         A listener to call the entity events while.
		 * @param pDetails          An optional vector pointer of detailed messages.
		 * @param pWarnings         An opitonal vector pointer of warning messages.
		 * 
		 * @return                  Returns the executed spawn queue length.
		 */
		virtual int ExecuteSpawnQueued(SpawnGroupHandle_t hSpawnGroup = ANY_SPAWN_GROUP, CUtlVector<CEntityInstance *> *pEntities = nullptr, IEntityListener *pListener = nullptr, CUtlVector<CUtlString> *pDetails = nullptr, CUtlVector<CUtlString> *pWarnings = nullptr) = 0;

	public: // Destory queued entities.
		/**
		 * @brief Push a entity instance to destroy queue.
		 * 
		 * @param pEntity           An entity instance to destoy.
		 */
		virtual void PushDestroyQueue(CEntityInstance *pEntity) = 0;

		/**
		 * @brief Push a entity identity to destroy queue.
		 * 
		 * @param pEntity           An entity identity to destoy.
		 */
		virtual void PushDestroyQueue(CEntityIdentity *pEntity) = 0;

		/**
		 * @brief Adds the destroy queue to a vector.
		 * 
		 * @param vecTarget         A vector target to store.
		 * 
		 * @return                  Returns the number of added elements.
		 */
		virtual int AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> &vecTarget) = 0;

		/**
		 * @brief Release destroy queued entities.
		 */
		virtual void ReleaseDestroyQueued() = 0;

		/**
		 * @brief Execute destory queued entities.
		 * 
		 * @return                  Returns the executed spawn queue length.
		 */
		virtual int ExecuteDestroyQueued() = 0;
	}; // ISpawnGroupLoader

	/**
	 * @brief A spawn group mamager access interface.
	 */
	class ISpawnGroupAccessor
	{
	public:
		/**
		 * @brief Gets a spawn group manager.
		 * 
		 * @return                  Returns the spawn group manager pointer.
		 */
		virtual CSpawnGroupMgrGameSystem *GetManager() = 0;

		/**
		 * @brief Gets a map spawn group from a handle.
		 * 
		 * @return                  Returns the map spawn group pointer.
		 */
		virtual CMapSpawnGroup *Get(SpawnGroupHandle_t hSpawnGroup) = 0;

		/**
		 * @brief Gets a spawn group map.
		 * 
		 * @return                  Returns the spawn group map pointer.
		 */
		virtual CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *GetSpawnGroups() = 0;
	}; // ISpawnGroupAccessor

	/**
	 * @brief A spawn group mamager provider.
	 */
	class CSpawnGroupProvider : public ISpawnGroupAccessor
	{
	public:
		/**
		 * @brief A spawn group found function declaration.
		 * 
		 * @param hSpawnGroup       A spawn group handle.
		 * @param pMapSpawnGroup    A map spawn group pointer.
		 */
		using OnSpawnGroupFound_t = void (SpawnGroupHandle_t hSpawnGroup, CMapSpawnGroup *pMapSpawnGroup);

		/**
		 * @brief Loops by a spawn group map.
		 * 
		 * @param funcCallback      A callback to call by each spawn group.
		 * 
		 * @return                  A count of spawn groups.
		 */
		uint LoopBySpawnGroups(const std::function<OnSpawnGroupFound_t> &funcCallback)
		{
			auto *pSpawnGroups = GetSpawnGroups();

			uint nSpawnGroupLength = 0;

			{
				const auto iInvalidIndex = pSpawnGroups->InvalidIndex();

				for(auto i = pSpawnGroups->FirstInorder(); i != iInvalidIndex; i = pSpawnGroups->NextInorder(i))
				{
					funcCallback(pSpawnGroups->Key(i), pSpawnGroups->Element(i));
					nSpawnGroupLength++;
				}
			}

			return nSpawnGroupLength;
		}

		/**
		 * @brief Fast loops by a spawn group map.
		 * 
		 * @param funcCallback      A callback to call by each spawn group.
		 * 
		 * @return                  A count of spawn groups.
		 */
		uint FastLoopBySpawnGroups(const std::function<OnSpawnGroupFound_t> &funcCallback)
		{
			auto *pSpawnGroups = GetSpawnGroups();

			uint nSpawnGroupLength = 0;

			{
				auto iMaxElement = pSpawnGroups->MaxElement();

				for(decltype(iMaxElement) i = 0; i < iMaxElement; ++i)
				{
					if(pSpawnGroups->IsValidIndex(i))
					{
						funcCallback(pSpawnGroups->Key(i), pSpawnGroups->Element(i));
						nSpawnGroupLength++;
					}
				}
			}

			return nSpawnGroupLength;
		}
	}; // CSpawnGroupProvider

	/**
	 * @brief Gets a provider agent.
	 * 
	 * @return                  Returns a provider agent pointer.
	 */
	virtual IProviderAgent *GetProviderAgent() = 0;

	/**
	 * @brief Gets a spawn group manager.
	 * 
	 * @return                  Returns a spawn group manager pointer.
	 */
	virtual CSpawnGroupProvider *GetSpawnGroupManager() = 0;
}; // IEntityManager

#endif // _INCLUDE_METAMOD_SOURCE_IENTITYMGR_HPP_
