#include "provider.h"

EntityManager::Provider::GameDataStorage::EntityKeyValues::EntityKeyValues()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("CEntityKeyValues::CEntityKeyValues", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnEntityKeyValues = pFucntion.RCast<decltype(this->m_pfnEntityKeyValues)>();
		});
		aCallbacks.Insert("CEntityKeyValues::GetAttribute", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnGetAttribute = pFucntion.RCast<decltype(this->m_pfnGetAttribute)>();
		});
		aCallbacks.Insert("CEntityKeyValuesAttribute::GetValueString", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnAttributeGetValueString = pFucntion.RCast<decltype(this->m_pfnAttributeGetValueString)>();
		});
		aCallbacks.Insert("CEntityKeyValues::SetAttributeValue", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnSetAttributeValue = pFucntion.RCast<decltype(this->m_pfnSetAttributeValue)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("sizeof(CEntityKeyValues)", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nSizeof = nOffset;
		});
		aCallbacks.Insert("CEntityKeyValues::m_pRoot", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nRootOffset = nOffset;
		});
		aCallbacks.Insert("CEntityKeyValues::m_nRefCount", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nRefCountOffset = nOffset;
		});
		aCallbacks.Insert("CEntityKeyValues::m_eContextType", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nContainerTypeOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntityKeyValues::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::EntityKeyValues::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_pfnEntityKeyValues = nullptr;
	this->m_pfnGetAttribute = nullptr;
	this->m_pfnAttributeGetValueString = nullptr;
	this->m_pfnSetAttributeValue = nullptr;

	this->m_nSizeof = -1;
	this->m_nRootOffset = -1;
	this->m_nRefCountOffset = -1;
	this->m_nContainerTypeOffset = -1;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnEntityKeyValuesPtr EntityManager::Provider::GameDataStorage::EntityKeyValues::EntityKeyValuesFunction() const
{
	return this->m_pfnEntityKeyValues;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnGetAttributePtr EntityManager::Provider::GameDataStorage::EntityKeyValues::GetAttributeFunction() const
{
	return this->m_pfnGetAttribute;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnAttributeGetValueStringPtr EntityManager::Provider::GameDataStorage::EntityKeyValues::AttributeGetValueStringFunction() const
{
	return this->m_pfnAttributeGetValueString;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnSetAttributeValuePtr EntityManager::Provider::GameDataStorage::EntityKeyValues::SetAttributeValueFunction() const
{
	return this->m_pfnSetAttributeValue;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetSizeof() const
{
	return this->m_nSizeof;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetRootOffset() const
{
	return this->m_nRootOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetRefCountOffset() const
{
	return this->m_nRefCountOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetContainerTypeOffset() const
{
	return this->m_nContainerTypeOffset;
}
