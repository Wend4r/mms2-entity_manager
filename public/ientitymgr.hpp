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

#	include <entity2/entityidentity.h>

#	define ENTITY_MANAGER_INTERFACE_NAME "Entity Manager v1.0"

class IEntityResourceManifest;
struct EntitySpawnInfo_t;

class ISpawnGroup;
struct SpawnGroupDesc_t;
class Vector;

/**
 * @brief A Entity Manager interface.
 * Note: gets with "ismm->MetaFactory(ENTITY_MANAGER_INTERFACE_NAME, NULL, NULL);"
**/
class IEntityManager
{
public: // Provider agent ones.
	/**
	 * @brief A provider agent interface.
	**/
	class IProviderAgent
	{
	public: // Spawn group ones.
		/**
		 * @brief A spawn group loader interface.
		**/
		class ISpawnGroupLoader
		{
		public:
			/**
			 * @brief Load the spawn group by a description.
			 * 
			 * @param aDesc             A spawn group description.
			 * @param vecLandmarkOffset A landmark offset.
			 * 
			 * @return                  True if successed added to the queue, overwise false if failed to add.
			 */
			virtual bool Load(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset) = 0;

			/**
			 * @brief Unload the spawn group.
			 * 
			 * @return                  True if successed added to the queue, overwise false if failed to add.
			 */
			virtual bool Unload() = 0;
		}; // ISpawnGroupLoader

		/**
		 * @brief A spawn group notifications interface.
		**/
		class ISpawnGroupNotifications
		{
		public:
			/**
			 * @brief Calls when spawn group are allocated.
			 * 
			 * @param handle            A spawn group handle to destroy.
			 * @param pSpawnGroup       A spawn group pointer.
			 */
			virtual void OnSpawnGroupAllocated(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup) = 0;

			/**
			 * @brief Calls when spawn group are destroyed.
			 * 
			 * @param handle            A spawn group handle to destroy.
			 */
			virtual void OnSpawnGroupDestroyed(SpawnGroupHandle_t handle) = 0;
		}; // ISpawnGroupNotifications

		/**
		 * @brief A spawn group instance interface.
		**/
		class ISpawnGroupInstance : public ISpawnGroupLoader, public ISpawnGroupNotifications
		{
		public:
			/**
			 * @brief Destructor of the instance, which calls the unload of the spawn group. 
			 *        Used internally, call `ReleaseSpawnGroup` instead.
			 */
			virtual ~ISpawnGroupInstance() {};

			/**
			 * @brief Gets the status of the spawn group.
			 * 
			 * @return                  The status value of a spawn group.
			 */
			virtual int GetStatus() const = 0;

			/**
			 * @brief Gets a spawn group.
			 * 
			 * @return                  A spawn group pointer.
			 */
			virtual ISpawnGroup *GetSpawnGroup() const = 0;

			/**
			 * @brief Gets an allocated spawn group handle.
			 * 
			 * @return                  A spawn group handle.
			 */
			virtual SpawnGroupHandle_t GetSpawnGroupHandle() const = 0;

			/**
			 * @brief Gets a level name string.
			 * 
			 * @return                  A string of the name.
			 */
			virtual const char *GetLevelName() const = 0;

			/**
			 * @brief Gets a landmark name string.
			 * 
			 * @return                  A string of the landmark name.
			 */
			virtual const char *GetLandmarkName() const = 0;

			/**
			 * @brief Gets a landmark offset.
			 * 
			 * @return                  A vector of the offset.
			 */
			virtual const Vector &GetLandmarkOffset() const = 0;
		}; // ISpawnGroupInstance

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
		virtual IEntityResourceManifest *GetEntityManifest() = 0;

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
		 * @return                  True if spawn group are released, overwise false if failed to find.
		 */
		virtual bool ReleaseSpawnGroup(ISpawnGroupInstance *pSpawnGroupInstance) = 0;
	};

	/**
	 * @brief Gets a provider agent.
	 * 
	 * @param handle            A provider agent pointer.
	 */
	virtual IProviderAgent *GetProviderAgent() = 0;
}; // ISample

#endif // _INCLUDE_METAMOD_SOURCE_IENTITYMGR_HPP_
