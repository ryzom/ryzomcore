// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdpch.h"
#include "chat_link_codec.h"

#include "sbrick_manager.h"
#include "nel/misc/base64.h"
#include "nel/misc/mem_stream.h"

#include <zlib.h>

using namespace NLMISC;

namespace CHAT_LINK
{
	const char ItemPrefix[] = "ryzom://i/";
	const char PhrasePrefix[] = "ryzom://p/";
	const uint32 MaxMessageCharacters = 255;

	namespace
	{
		const uint8 CurrentVersion = 1;
		const uint16 MaxPhraseBricks = 64;
		const uint32 MaxItemRawBytes = 32768;
		const uint32 MaxPhraseRawBytes = 4096;
		const uint32 MaxItemTextBytes = 4096;
		const uint16 MaxItemSkillMods = 64;

		enum TItemSlotField
		{
			ItemSlotQuantity = 1 << 0,
			ItemSlotWeight = 1 << 1,
			ItemSlotUserColor = 1 << 2,
			ItemSlotEnchant = 1 << 3,
			ItemSlotRMClass = 1 << 4,
			ItemSlotRMFaberStat = 1 << 5,
			ItemSlotKnownFields = (1 << 6) - 1
		};

		enum TItemInfoField
		{
			ItemInfoDurability = 1 << 0,
			ItemInfoCreator = 1 << 1,
			ItemInfoDamage = 1 << 2,
			ItemInfoSapLoad = 1 << 3,
			ItemInfoRange = 1 << 4,
			ItemInfoCombatModifiers = 1 << 5,
			ItemInfoPhysicalProtection = 1 << 6,
			ItemInfoMagicProtection = 1 << 7,
			ItemInfoMagicResistance = 1 << 8,
			ItemInfoBuffs = 1 << 9,
			ItemInfoEnchantment = 1 << 10,
			ItemInfoWearMalus = 1 << 11,
			ItemInfoRequiredSkill = 1 << 12,
			ItemInfoRequiredSkill2 = 1 << 13,
			ItemInfoRequiredCharacteristic = 1 << 14,
			ItemInfoTypeSkillMods = 1 << 15,
			ItemInfoMagicFactors = 1 << 16,
			ItemInfoCustomText = 1 << 17,
			ItemInfoR2Description = 1 << 18,
			ItemInfoR2Comment = 1 << 19,
			ItemInfoPetNumber = 1 << 20,
			ItemInfoKnownFields = (1 << 21) - 1
		};

		bool serialVarUint32(NLMISC::IStream &stream, uint32 &value)
		{
			if (!stream.isReading())
			{
				uint32 remaining = value;
				do
				{
					uint8 byte = (uint8)(remaining & 0x7f);
					remaining >>= 7;
					if (remaining != 0) byte |= 0x80;
					stream.serial(byte);
				}
				while (remaining != 0);
				return true;
			}

			value = 0;
			for (uint shift = 0; shift <= 28; shift += 7)
			{
				uint8 byte = 0;
				stream.serial(byte);
				if (shift == 28 && (byte & 0xf0) != 0) return false;
				value |= uint32(byte & 0x7f) << shift;
				if ((byte & 0x80) == 0) return true;
			}
			return false;
		}

		bool serialVarSint32(NLMISC::IStream &stream, sint32 &value)
		{
			uint32 encoded = 0;
			if (!stream.isReading())
				encoded = (uint32(value) << 1) ^ (value < 0 ? 0xffffffffu : 0u);
			if (!serialVarUint32(stream, encoded)) return false;
			if (stream.isReading())
				value = (encoded & 1) ? sint32(-sint64((encoded >> 1) + 1)) : sint32(encoded >> 1);
			return true;
		}

		bool serialCompactString(NLMISC::IStream &stream, std::string &value, uint32 maxBytes)
		{
			uint32 length = stream.isReading() ? 0 : (uint32)value.size();
			if (length > maxBytes || !serialVarUint32(stream, length) || length > maxBytes) return false;
			if (stream.isReading()) value.resize(length);
			if (length != 0) stream.serialBuffer(reinterpret_cast<uint8*>(&value[0]), length);
			return true;
		}

		bool isValidUtf8(const std::string &value)
		{
			for (size_t i = 0; i < value.size();)
			{
				const uint8 c0 = (uint8)value[i++];
				if (c0 == 0) return false;
				if (c0 < 0x80) continue;
				uint continuationCount = 0;
				uint8 secondMin = 0x80;
				uint8 secondMax = 0xbf;
				if (c0 >= 0xc2 && c0 <= 0xdf) continuationCount = 1;
				else if (c0 >= 0xe0 && c0 <= 0xef)
				{
					continuationCount = 2;
					if (c0 == 0xe0) secondMin = 0xa0;
					if (c0 == 0xed) secondMax = 0x9f;
				}
				else if (c0 >= 0xf0 && c0 <= 0xf4)
				{
					continuationCount = 3;
					if (c0 == 0xf0) secondMin = 0x90;
					if (c0 == 0xf4) secondMax = 0x8f;
				}
				else return false;
				if (i + continuationCount > value.size()) return false;
				const uint8 c1 = (uint8)value[i++];
				if (c1 < secondMin || c1 > secondMax) return false;
				for (uint j = 1; j < continuationCount; ++j)
				{
					const uint8 cx = (uint8)value[i++];
					if (cx < 0x80 || cx > 0xbf) return false;
				}
			}
			return true;
		}

		bool serialCompactUcString(NLMISC::IStream &stream, ucstring &value, uint32 maxBytes)
		{
			std::string utf8;
			if (!stream.isReading()) utf8 = value.toUtf8();
			if (!serialCompactString(stream, utf8, maxBytes)) return false;
			if (stream.isReading())
			{
				if (!isValidUtf8(utf8)) return false;
				value.fromUtf8(utf8);
			}
			return true;
		}

		bool serialCompactBrickId(NLMISC::IStream &stream, CSheetId &brickId)
		{
			static uint32 sbrickType = CSheetId::typeFromFileExtension("sbrick");
			uint16 compactId = 0;
			if (!stream.isReading())
			{
				if (brickId.asInt() == 0 || brickId.getSheetType() != sbrickType || brickId.getShortId() >= 65535) return false;
				compactId = (uint16)(brickId.getShortId() + 1);
			}
			stream.serial(compactId);
			if (stream.isReading())
			{
				if (compactId == 0) return false;
				brickId.buildSheetId(compactId - 1, sbrickType);
			}
			return true;
		}

		bool serialCompactPhrase(NLMISC::IStream &stream, CSPhraseCom &phrase, bool includeName, bool includeIcon)
		{
			uint8 flags = 0;
			if (!stream.isReading())
			{
				if (includeName && !phrase.Name.empty()) flags |= 1;
				if (includeIcon && phrase.IconIndex != std::numeric_limits<uint8>::max()) flags |= 2;
			}
			stream.serial(flags);
			uint8 allowedFlags = uint8((includeName ? 1 : 0) | (includeIcon ? 2 : 0));
			if ((flags & ~allowedFlags) != 0) return false;

			if (flags & 1)
			{
				if (!serialCompactUcString(stream, phrase.Name, 512)) return false;
			}
			else if (stream.isReading()) phrase.Name.clear();

			uint32 brickCount = stream.isReading() ? 0 : (uint32)phrase.Bricks.size();
			if ((!stream.isReading() && (brickCount == 0 || brickCount > MaxPhraseBricks)) ||
				!serialVarUint32(stream, brickCount) || brickCount == 0 || brickCount > MaxPhraseBricks) return false;
			if (stream.isReading()) phrase.Bricks.resize(brickCount);
			for (uint32 i = 0; i < brickCount; ++i)
				if (!serialCompactBrickId(stream, phrase.Bricks[i])) return false;

			if (flags & 2) stream.serial(phrase.IconIndex);
			else if (stream.isReading()) phrase.IconIndex = std::numeric_limits<uint8>::max();
			return true;
		}

		bool serialCompactItemInfo(NLMISC::IStream &stream, CItemInfos &info)
		{
			uint32 fields = 0;
			if (!stream.isReading())
			{
				if (info.Hp != 0 || info.HpMax != 0) fields |= ItemInfoDurability;
				if (info.CreatorName != 0) fields |= ItemInfoCreator;
				if (info.CurrentDamage != 0 || info.MaxDamage != 0 || info.HitRate != 0.f) fields |= ItemInfoDamage;
				if (info.SapLoadCurrent != 0 || info.SapLoadMax != 0) fields |= ItemInfoSapLoad;
				if (info.Range != 0.f) fields |= ItemInfoRange;
				if (info.ParryModifier != 0 || info.DodgeModifier != 0 || info.AdversaryParryModifier != 0 || info.AdversaryDodgeModifier != 0) fields |= ItemInfoCombatModifiers;
				if (info.ProtectionFactor != 0.f || info.MaxSlashingProtection != 0 || info.MaxBluntProtection != 0 || info.MaxPiercingProtection != 0) fields |= ItemInfoPhysicalProtection;
				for (uint i = 0; i < CItemInfos::MaxMagicProtectionByJewel; ++i)
					if (info.MagicProtection[i] != PROTECTION_TYPE::None || info.MagicProtectionFactor[i] != 0) fields |= ItemInfoMagicProtection;
				if (info.DesertMagicResistance != 0 || info.ForestMagicResistance != 0 || info.LacustreMagicResistance != 0 || info.JungleMagicResistance != 0 || info.PrimaryRootMagicResistance != 0) fields |= ItemInfoMagicResistance;
				if (info.HpBuff != 0 || info.SapBuff != 0 || info.StaBuff != 0 || info.FocusBuff != 0) fields |= ItemInfoBuffs;
				if (!info.Enchantment.empty()) fields |= ItemInfoEnchantment;
				if (info.WearEquipmentMalus != 0.f) fields |= ItemInfoWearMalus;
				if (info.RequiredSkill != SKILLS::unknown || info.RequiredSkillLevel != 0) fields |= ItemInfoRequiredSkill;
				if (info.RequiredSkill2 != SKILLS::unknown || info.RequiredSkillLevel2 != 0) fields |= ItemInfoRequiredSkill2;
				if (info.RequiredCharac != CHARACTERISTICS::Unknown || info.RequiredCharacLevel != 0) fields |= ItemInfoRequiredCharacteristic;
				if (!info.TypeSkillMods.empty()) fields |= ItemInfoTypeSkillMods;
				for (uint i = 0; i < CItemInfos::NumMagicFactorType; ++i)
					if (info.CastingSpeedFactor[i] != 0.f || info.MagicPowerFactor[i] != 0.f) fields |= ItemInfoMagicFactors;
				if (!info.CustomText.empty()) fields |= ItemInfoCustomText;
				if (!info.R2ItemDescription.empty()) fields |= ItemInfoR2Description;
				if (!info.R2ItemComment.empty()) fields |= ItemInfoR2Comment;
				if (info.PetNumber != 0) fields |= ItemInfoPetNumber;
			}
			else info = CItemInfos();

			if (!serialVarUint32(stream, fields) || (fields & ~uint32(ItemInfoKnownFields)) != 0) return false;
			if (fields & ItemInfoDurability)
				if (!serialVarUint32(stream, info.Hp) || !serialVarUint32(stream, info.HpMax)) return false;
			if (fields & ItemInfoCreator)
				if (!serialVarUint32(stream, info.CreatorName)) return false;
			if (fields & ItemInfoDamage)
			{
				if (!serialVarUint32(stream, info.CurrentDamage) || !serialVarUint32(stream, info.MaxDamage)) return false;
				stream.serial(info.HitRate);
			}
			if (fields & ItemInfoSapLoad)
				if (!serialVarUint32(stream, info.SapLoadCurrent) || !serialVarUint32(stream, info.SapLoadMax)) return false;
			if (fields & ItemInfoRange) stream.serial(info.Range);
			if (fields & ItemInfoCombatModifiers)
				if (!serialVarSint32(stream, info.ParryModifier) || !serialVarSint32(stream, info.DodgeModifier) || !serialVarSint32(stream, info.AdversaryParryModifier) || !serialVarSint32(stream, info.AdversaryDodgeModifier)) return false;
			if (fields & ItemInfoPhysicalProtection)
			{
				stream.serial(info.ProtectionFactor);
				if (!serialVarUint32(stream, info.MaxSlashingProtection) || !serialVarUint32(stream, info.MaxBluntProtection) || !serialVarUint32(stream, info.MaxPiercingProtection)) return false;
			}
			if (fields & ItemInfoMagicProtection)
			{
				uint8 mask = 0;
				if (!stream.isReading())
					for (uint i = 0; i < CItemInfos::MaxMagicProtectionByJewel; ++i)
						if (info.MagicProtection[i] != PROTECTION_TYPE::None || info.MagicProtectionFactor[i] != 0) mask |= uint8(1 << i);
				stream.serial(mask);
				if ((mask & ~uint8((1 << CItemInfos::MaxMagicProtectionByJewel) - 1)) != 0) return false;
				for (uint i = 0; i < CItemInfos::MaxMagicProtectionByJewel; ++i)
				{
					if ((mask & (1 << i)) == 0) continue;
					sint32 protection = stream.isReading() ? 0 : (sint32)info.MagicProtection[i];
					if (!serialVarSint32(stream, protection) || !serialVarUint32(stream, info.MagicProtectionFactor[i])) return false;
					if (stream.isReading()) info.MagicProtection[i] = (PROTECTION_TYPE::TProtectionType)protection;
				}
			}
			if (fields & ItemInfoMagicResistance)
			{
				uint8 mask = 0;
				uint32 *values[] = { &info.DesertMagicResistance, &info.ForestMagicResistance, &info.LacustreMagicResistance, &info.JungleMagicResistance, &info.PrimaryRootMagicResistance };
				if (!stream.isReading()) for (uint i = 0; i < 5; ++i) if (*values[i] != 0) mask |= uint8(1 << i);
				stream.serial(mask);
				if ((mask & ~uint8(0x1f)) != 0) return false;
				for (uint i = 0; i < 5; ++i) if ((mask & (1 << i)) != 0 && !serialVarUint32(stream, *values[i])) return false;
			}
			if (fields & ItemInfoBuffs)
			{
				uint8 mask = 0;
				sint32 *values[] = { &info.HpBuff, &info.SapBuff, &info.StaBuff, &info.FocusBuff };
				if (!stream.isReading()) for (uint i = 0; i < 4; ++i) if (*values[i] != 0) mask |= uint8(1 << i);
				stream.serial(mask);
				if ((mask & ~uint8(0x0f)) != 0) return false;
				for (uint i = 0; i < 4; ++i) if ((mask & (1 << i)) != 0 && !serialVarSint32(stream, *values[i])) return false;
			}
			if (fields & ItemInfoEnchantment)
				if (!serialCompactPhrase(stream, info.Enchantment, true, true)) return false;
			if (fields & ItemInfoWearMalus) stream.serial(info.WearEquipmentMalus);
			if (fields & ItemInfoRequiredSkill)
			{
				sint32 skill = stream.isReading() ? 0 : (sint32)info.RequiredSkill;
				uint32 level = info.RequiredSkillLevel;
				if (!serialVarSint32(stream, skill) || !serialVarUint32(stream, level) || level > 65535) return false;
				if (stream.isReading()) { info.RequiredSkill = (SKILLS::ESkills)skill; info.RequiredSkillLevel = (uint16)level; }
			}
			if (fields & ItemInfoRequiredSkill2)
			{
				sint32 skill = stream.isReading() ? 0 : (sint32)info.RequiredSkill2;
				uint32 level = info.RequiredSkillLevel2;
				if (!serialVarSint32(stream, skill) || !serialVarUint32(stream, level) || level > 65535) return false;
				if (stream.isReading()) { info.RequiredSkill2 = (SKILLS::ESkills)skill; info.RequiredSkillLevel2 = (uint16)level; }
			}
			if (fields & ItemInfoRequiredCharacteristic)
			{
				sint32 characteristic = stream.isReading() ? 0 : (sint32)info.RequiredCharac;
				uint32 level = info.RequiredCharacLevel;
				if (!serialVarSint32(stream, characteristic) || !serialVarUint32(stream, level) || level > 65535) return false;
				if (stream.isReading()) { info.RequiredCharac = (CHARACTERISTICS::TCharacteristics)characteristic; info.RequiredCharacLevel = (uint16)level; }
			}
			if (fields & ItemInfoTypeSkillMods)
			{
				uint32 count = stream.isReading() ? 0 : (uint32)info.TypeSkillMods.size();
				if (count > MaxItemSkillMods || !serialVarUint32(stream, count) || count > MaxItemSkillMods) return false;
				if (stream.isReading()) info.TypeSkillMods.resize(count);
				for (uint32 i = 0; i < count; ++i)
				{
					sint32 type = stream.isReading() ? 0 : (sint32)info.TypeSkillMods[i].Type;
					if (!serialVarSint32(stream, type) || !serialVarSint32(stream, info.TypeSkillMods[i].Modifier)) return false;
					if (stream.isReading()) info.TypeSkillMods[i].Type = (EGSPD::CClassificationType::TClassificationType)type;
				}
			}
			if (fields & ItemInfoMagicFactors)
			{
				uint8 mask = 0;
				if (!stream.isReading())
					for (uint i = 0; i < CItemInfos::NumMagicFactorType; ++i)
					{
						if (info.CastingSpeedFactor[i] != 0.f) mask |= uint8(1 << (i * 2));
						if (info.MagicPowerFactor[i] != 0.f) mask |= uint8(1 << (i * 2 + 1));
					}
				stream.serial(mask);
				for (uint i = 0; i < CItemInfos::NumMagicFactorType; ++i)
				{
					if (mask & (1 << (i * 2))) stream.serial(info.CastingSpeedFactor[i]);
					if (mask & (1 << (i * 2 + 1))) stream.serial(info.MagicPowerFactor[i]);
				}
			}
			if (fields & ItemInfoCustomText)
				if (!serialCompactUcString(stream, info.CustomText, MaxItemTextBytes)) return false;
			if (fields & ItemInfoR2Description)
				if (!serialCompactUcString(stream, info.R2ItemDescription, MaxItemTextBytes)) return false;
			if (fields & ItemInfoR2Comment)
				if (!serialCompactUcString(stream, info.R2ItemComment, MaxItemTextBytes)) return false;
			if (fields & ItemInfoPetNumber)
			{
				uint32 petNumber = info.PetNumber;
				if (!serialVarUint32(stream, petNumber) || petNumber > 255) return false;
				if (stream.isReading()) info.PetNumber = (uint8)petNumber;
			}

			info.slotId = 0;
			info.versionInfo = 1;
			return true;
		}

		bool serialItemSnapshot(NLMISC::IStream &stream, CItemSnapshot &snapshot)
		{
			uint32 slotFields = 0;
			if (!stream.isReading())
			{
				if (snapshot.Quantity != 1) slotFields |= ItemSlotQuantity;
				if (snapshot.Weight != 0) slotFields |= ItemSlotWeight;
				if (snapshot.UserColor != -1) slotFields |= ItemSlotUserColor;
				if (snapshot.Enchant != 0) slotFields |= ItemSlotEnchant;
				if (snapshot.RMClassType != 0) slotFields |= ItemSlotRMClass;
				if (snapshot.RMFaberStatType != 0) slotFields |= ItemSlotRMFaberStat;
			}
			else snapshot = CItemSnapshot();

			stream.serial(snapshot.SheetId);
			if (snapshot.SheetId == 0 || !serialVarUint32(stream, snapshot.Quality) || !serialVarUint32(stream, slotFields) || (slotFields & ~uint32(ItemSlotKnownFields)) != 0) return false;
			if (slotFields & ItemSlotQuantity) if (!serialVarUint32(stream, snapshot.Quantity)) return false;
			if (slotFields & ItemSlotWeight) if (!serialVarUint32(stream, snapshot.Weight)) return false;
			if (slotFields & ItemSlotUserColor) if (!serialVarSint32(stream, snapshot.UserColor)) return false;
			if (slotFields & ItemSlotEnchant) if (!serialVarUint32(stream, snapshot.Enchant)) return false;
			if (slotFields & ItemSlotRMClass) if (!serialVarSint32(stream, snapshot.RMClassType)) return false;
			if (slotFields & ItemSlotRMFaberStat) if (!serialVarSint32(stream, snapshot.RMFaberStatType)) return false;
			return serialCompactItemInfo(stream, snapshot.Info);
		}

		bool serialPhrase(NLMISC::IStream &stream, CSPhraseCom &phrase)
		{
			return serialCompactPhrase(stream, phrase, false, true);
		}

		bool validatePhrase(const CSPhraseCom &phrase, bool requireExistingBricks)
		{
			if (phrase.Bricks.empty() || phrase.Bricks.size() > MaxPhraseBricks) return false;
			if (phrase.IconIndex != std::numeric_limits<uint8>::max() && phrase.IconIndex >= phrase.Bricks.size()) return false;
			if (phrase.Name.toUtf8().size() > 512) return false;
			if (requireExistingBricks)
			{
				CSBrickManager *brickManager = CSBrickManager::getInstance();
				if (!brickManager) return false;
				for (uint i = 0; i < phrase.Bricks.size(); ++i)
					if (phrase.Bricks[i].asInt() == 0 || !brickManager->getBrick(phrase.Bricks[i])) return false;
			}
			return true;
		}

		bool validateItem(const CItemSnapshot &snapshot)
		{
			const CItemInfos &info = snapshot.Info;
			if (snapshot.SheetId == 0 || snapshot.Quality > 65535 || snapshot.Quantity == 0 || snapshot.Quantity > 65535) return false;
			if (snapshot.UserColor < -1 || snapshot.UserColor > 255) return false;
			if (snapshot.RMClassType < 0 || snapshot.RMClassType > RM_CLASS_TYPE::Unknown) return false;
			if (snapshot.RMFaberStatType < 0 || snapshot.RMFaberStatType > RM_FABER_STAT_TYPE::Unknown) return false;
			if ((info.HpMax != 0 && info.Hp > info.HpMax) || (info.MaxDamage != 0 && info.CurrentDamage > info.MaxDamage) || (info.SapLoadMax != 0 && info.SapLoadCurrent > info.SapLoadMax)) return false;
			if (!isValidDouble(info.HitRate) || !isValidDouble(info.Range) || !isValidDouble(info.ProtectionFactor) || !isValidDouble(info.WearEquipmentMalus)) return false;
			if ((sint32)info.RequiredSkill < 0 || info.RequiredSkill > SKILLS::unknown || (sint32)info.RequiredSkill2 < 0 || info.RequiredSkill2 > SKILLS::unknown) return false;
			if ((sint32)info.RequiredCharac < 0 || info.RequiredCharac > CHARACTERISTICS::Unknown) return false;
			if (info.TypeSkillMods.size() > MaxItemSkillMods) return false;
			for (uint i = 0; i < info.TypeSkillMods.size(); ++i)
				if ((sint32)info.TypeSkillMods[i].Type < 0 || info.TypeSkillMods[i].Type > EGSPD::CClassificationType::Unknown) return false;
			for (uint i = 0; i < CItemInfos::MaxMagicProtectionByJewel; ++i)
				if ((sint32)info.MagicProtection[i] < 0 || info.MagicProtection[i] > PROTECTION_TYPE::None) return false;
			for (uint i = 0; i < CItemInfos::NumMagicFactorType; ++i)
				if (!isValidDouble(info.CastingSpeedFactor[i]) || !isValidDouble(info.MagicPowerFactor[i])) return false;
			if (!info.Enchantment.empty() && !validatePhrase(info.Enchantment, false)) return false;
			return info.CustomText.toUtf8().size() <= MaxItemTextBytes && info.R2ItemDescription.toUtf8().size() <= MaxItemTextBytes && info.R2ItemComment.toUtf8().size() <= MaxItemTextBytes;
		}

		std::string toBase64Url(const std::string &data)
		{
			std::string encoded = base64::encode(data);
			for (std::string::iterator it = encoded.begin(); it != encoded.end(); ++it)
			{
				if (*it == '+') *it = '-';
				else if (*it == '/') *it = '_';
			}
			while (!encoded.empty() && encoded[encoded.size() - 1] == '=') encoded.resize(encoded.size() - 1);
			return encoded;
		}

		bool fromBase64Url(const std::string &data, std::string &decoded)
		{
			if (data.empty() || (data.size() & 3) == 1) return false;
			for (std::string::const_iterator it = data.begin(); it != data.end(); ++it)
			{
				const char c = *it;
				if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
					(c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
			}
			std::string encoded = data;
			for (std::string::iterator it = encoded.begin(); it != encoded.end(); ++it)
			{
				if (*it == '-') *it = '+';
				else if (*it == '_') *it = '/';
			}
			while ((encoded.size() % 4) != 0) encoded += '=';
			decoded = base64::decode(encoded);
			return !decoded.empty();
		}

		bool encodeCompressed(const char *prefix, CMemStream &stream, std::string &url)
		{
			if (stream.length() == 0) return false;
			uLongf compressedSize = compressBound(stream.length());
			std::string compressed(compressedSize, '\0');
			int result = compress2(reinterpret_cast<Bytef*>(&compressed[0]), &compressedSize, reinterpret_cast<const Bytef*>(stream.buffer()), stream.length(), Z_BEST_COMPRESSION);
			if (result != Z_OK) return false;
			compressed.resize(compressedSize);
			url = prefix + toBase64Url(compressed);
			return true;
		}

		bool decodeCompressed(const std::string &url, const char *prefix, uint32 maxRawBytes, CMemStream &stream, uint8 &version)
		{
			const size_t prefixLength = strlen(prefix);
			if (url.compare(0, prefixLength, prefix) != 0 || url.size() > MaxMessageCharacters) return false;
			std::string compressed;
			if (!fromBase64Url(url.substr(prefixLength), compressed)) return false;

			std::string raw(maxRawBytes, '\0');
			z_stream inflater;
			memset(&inflater, 0, sizeof(inflater));
			if (inflateInit(&inflater) != Z_OK) return false;
			inflater.next_in = reinterpret_cast<Bytef*>(&compressed[0]);
			inflater.avail_in = (uInt)compressed.size();
			inflater.next_out = reinterpret_cast<Bytef*>(&raw[0]);
			inflater.avail_out = (uInt)raw.size();
			int result = inflate(&inflater, Z_FINISH);
			bool valid = result == Z_STREAM_END && inflater.avail_in == 0 && inflater.total_out > 0 && inflater.total_out <= maxRawBytes;
			size_t rawSize = (size_t)inflater.total_out;
			inflateEnd(&inflater);
			if (!valid) return false;
			raw.resize(rawSize);

			stream.fill(reinterpret_cast<const uint8*>(raw.data()), raw.size());
			version = 0;
			stream.serial(version);
			return version == CurrentVersion;
		}
	}

	CItemSnapshot::CItemSnapshot()
	: SheetId(0), Quality(0), Quantity(1), Weight(0), UserColor(-1), Enchant(0), RMClassType(0), RMFaberStatType(0)
	{
	}

	bool encodeItemSnapshot(CItemSnapshot snapshot, std::string &url)
	{
		try
		{
			if (!validateItem(snapshot)) return false;
			CMemStream stream;
			uint8 version = CurrentVersion;
			stream.serial(version);
			if (!serialItemSnapshot(stream, snapshot)) return false;
			return stream.length() <= MaxItemRawBytes && encodeCompressed(ItemPrefix, stream, url);
		}
		catch(...) { return false; }
	}

	bool decodeItemSnapshot(const std::string &url, CItemSnapshot &snapshot)
	{
		try
		{
			CMemStream stream(true);
			uint8 version = 0;
			if (!decodeCompressed(url, ItemPrefix, MaxItemRawBytes, stream, version)) return false;
			return serialItemSnapshot(stream, snapshot) && stream.getPos() == (sint32)stream.length() && validateItem(snapshot);
		}
		catch(...) { return false; }
	}

	bool encodePhrase(CSPhraseCom phrase, std::string &url)
	{
		try
		{
			if (!validatePhrase(phrase, false)) return false;
			CMemStream stream;
			uint8 version = CurrentVersion;
			stream.serial(version);
			if (!serialPhrase(stream, phrase)) return false;
			return stream.length() <= MaxPhraseRawBytes && encodeCompressed(PhrasePrefix, stream, url);
		}
		catch(...) { return false; }
	}

	bool decodePhrase(const std::string &url, CSPhraseCom &phrase)
	{
		try
		{
			CMemStream stream(true);
			uint8 version = 0;
			if (!decodeCompressed(url, PhrasePrefix, MaxPhraseRawBytes, stream, version)) return false;
			return serialPhrase(stream, phrase) && stream.getPos() == (sint32)stream.length() && validatePhrase(phrase, true);
		}
		catch(...) { return false; }
	}

	std::string makeVisibleLink(std::string title, const std::string &url)
	{
		for (std::string::iterator it = title.begin(); it != title.end(); ++it)
			if (*it == ')' || *it == '\r' || *it == '\n' || *it == '\t') *it = ' ';
		return "(" + title + ")[" + url + "]";
	}

	bool isItemUrl(const std::string &url) { return url.compare(0, strlen(ItemPrefix), ItemPrefix) == 0; }
	bool isPhraseUrl(const std::string &url) { return url.compare(0, strlen(PhrasePrefix), PhrasePrefix) == 0; }
}
