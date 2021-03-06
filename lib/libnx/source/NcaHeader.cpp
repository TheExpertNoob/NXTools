#include <nx/NcaHeader.h>

nx::NcaHeader::NcaHeader()
{
	clear();
}

nx::NcaHeader::NcaHeader(const NcaHeader & other)
{
	*this = other;
}

bool nx::NcaHeader::operator==(const NcaHeader & other) const
{
	return (mDistributionType == other.mDistributionType) \
		&& (mContentType == other.mContentType) \
		&& (mKeyGeneration == other.mKeyGeneration) \
		&& (mKaekIndex == other.mKaekIndex) \
		&& (mContentSize == other.mContentSize) \
		&& (mProgramId == other.mProgramId) \
		&& (mContentIndex == other.mContentIndex) \
		&& (mSdkAddonVersion == other.mSdkAddonVersion) \
		&& (mPartitions == other.mPartitions) \
		&& (mEncAesKeys == other.mEncAesKeys);
}

bool nx::NcaHeader::operator!=(const NcaHeader & other) const
{
	return !(*this == other);
}

void nx::NcaHeader::operator=(const NcaHeader & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		mRawBinary.clear();
		mDistributionType = other.mDistributionType;
		mContentType = other.mContentType;
		mKeyGeneration = other.mKeyGeneration;
		mKaekIndex = other.mKaekIndex;
		mContentSize = other.mContentSize;
		mProgramId = other.mProgramId;
		mContentIndex = other.mContentIndex;
		mSdkAddonVersion = other.mSdkAddonVersion;
		mPartitions = other.mPartitions;
		mEncAesKeys = other.mEncAesKeys;
	}
}

void nx::NcaHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sNcaHeader));
	sNcaHeader* hdr = (sNcaHeader*)mRawBinary.data();

	
	switch(mFormatVersion)
	{
	case (NCA2_FORMAT):
		hdr->st_magic = nca::kNca2StructMagic;
		break;
	case (NCA3_FORMAT):
		hdr->st_magic = nca::kNca3StructMagic;
		break;
	default:
		throw fnd::Exception(kModuleName, "Unsupported format version");
	}
	hdr->distribution_type = mDistributionType;
	hdr->content_type = mContentType;
	if (mKeyGeneration > 2)
	{
		hdr->key_generation = 2;
		hdr->key_generation_2 = mKeyGeneration;
	}
	else
	{
		hdr->key_generation = mKeyGeneration;
		hdr->key_generation_2 = 0;
	}
	
	hdr->key_area_encryption_key_index = mKaekIndex;
	hdr->content_size = mContentSize;
	hdr->program_id = mProgramId;
	hdr->content_index = mContentIndex;
	hdr->sdk_addon_version = mSdkAddonVersion;
	memcpy(hdr->rights_id, mRightsId, nca::kRightsIdLen);

	// TODO: properly reconstruct NCA layout? atm in hands of user
	for (size_t i = 0; i < mPartitions.size(); i++)
	{
		// determine partition index
		byte_t idx = mPartitions[i].index;

		if (mPartitions[i].index >= nca::kPartitionNum || hdr->partition[idx].enabled) continue;

		hdr->partition[idx].start = sizeToBlockNum(mPartitions[i].offset);
		hdr->partition[idx].end = (sizeToBlockNum(mPartitions[i].offset) + sizeToBlockNum(mPartitions[i].size));
		hdr->partition[idx].enabled = true;
		hdr->partition_hash[idx] = mPartitions[i].hash;
	}

	for (size_t i = 0; i < nca::kAesKeyNum; i++)
	{
		hdr->enc_aes_key[i] = mEncAesKeys[i];
	}
}

void nx::NcaHeader::fromBytes(const byte_t * data, size_t len)
{
	if (len < sizeof(sNcaHeader))
	{
		throw fnd::Exception(kModuleName, "NCA header size is too small");
	}

	clear();

	mRawBinary.alloc(sizeof(sNcaHeader));
	memcpy(mRawBinary.data(), data, sizeof(sNcaHeader));

	sNcaHeader* hdr = (sNcaHeader*)mRawBinary.data();

	switch(hdr->st_magic.get())
	{
		case (nca::kNca2StructMagic) :
			mFormatVersion = NCA2_FORMAT;
			break;
		case (nca::kNca3StructMagic) :
			mFormatVersion = NCA3_FORMAT;
			break;
		throw fnd::Exception(kModuleName, "NCA header corrupt");
	}

	mDistributionType = (nca::DistributionType)hdr->distribution_type;
	mContentType = (nca::ContentType)hdr->content_type;
	mKeyGeneration = _MAX(hdr->key_generation, hdr->key_generation_2);
	mKaekIndex = hdr->key_area_encryption_key_index;
	mContentSize = *hdr->content_size;
	mProgramId = *hdr->program_id;
	mContentIndex = *hdr->content_index;
	mSdkAddonVersion = *hdr->sdk_addon_version;
	memcpy(mRightsId, hdr->rights_id, nca::kRightsIdLen);

	for (size_t i = 0; i < nca::kPartitionNum; i++)
	{
		// skip sections that don't exist
		if (hdr->partition[i].enabled == 0) continue;

		// add high level struct
		mPartitions.addElement({(byte_t)i, blockNumToSize(hdr->partition[i].start.get()), blockNumToSize(hdr->partition[i].end.get() - hdr->partition[i].start.get()), hdr->partition_hash[i] });
	}

	for (size_t i = 0; i < nca::kAesKeyNum; i++)
	{
		mEncAesKeys.addElement(hdr->enc_aes_key[i]);
	}
}

const fnd::Vec<byte_t>& nx::NcaHeader::getBytes() const
{
	return mRawBinary;
}

void nx::NcaHeader::clear()
{
	mFormatVersion = NCA3_FORMAT;
	mDistributionType = nca::DIST_DOWNLOAD;
	mContentType = nca::TYPE_PROGRAM;
	mKeyGeneration = 0;
	mKaekIndex = 0;
	mContentSize = 0;
	mProgramId = 0;
	mContentIndex = 0;
	mSdkAddonVersion = 0;

	mPartitions.clear();
	mEncAesKeys.clear();
}

nx::NcaHeader::FormatVersion nx::NcaHeader::getFormatVersion() const
{
	return mFormatVersion;
}

void nx::NcaHeader::setFormatVersion(FormatVersion version)
{
	mFormatVersion = version;
}

nx::nca::DistributionType nx::NcaHeader::getDistributionType() const
{
	return mDistributionType;
}

void nx::NcaHeader::setDistributionType(nca::DistributionType type)
{
	mDistributionType = type;
}

nx::nca::ContentType nx::NcaHeader::getContentType() const
{
	return mContentType;
}

void nx::NcaHeader::setContentType(nca::ContentType type)
{
	mContentType = type;
}

byte_t nx::NcaHeader::getKeyGeneration() const
{
	return mKeyGeneration;
}

void nx::NcaHeader::setKeyGeneration(byte_t gen)
{
	mKeyGeneration = gen;
}

byte_t nx::NcaHeader::getKaekIndex() const
{
	return mKaekIndex;
}

void nx::NcaHeader::setKaekIndex(byte_t index)
{
	mKaekIndex = index;
}

uint64_t nx::NcaHeader::getContentSize() const
{
	return mContentSize;
}

void nx::NcaHeader::setContentSize(uint64_t size)
{
	mContentSize = size;
}

uint64_t nx::NcaHeader::getProgramId() const
{
	return mProgramId;
}

void nx::NcaHeader::setProgramId(uint64_t program_id)
{
	mProgramId = program_id;
}

uint32_t nx::NcaHeader::getContentIndex() const
{
	return mContentIndex;
}

void nx::NcaHeader::setContentIndex(uint32_t index)
{
	mContentIndex = index;
}

uint32_t nx::NcaHeader::getSdkAddonVersion() const
{
	return mSdkAddonVersion;
}

void nx::NcaHeader::setSdkAddonVersion(uint32_t version)
{
	mSdkAddonVersion = version;
}

bool nx::NcaHeader::hasRightsId() const
{
	bool rightsIdIsSet = false;

	for (size_t i = 0; i < nca::kRightsIdLen; i++)
	{
		if (mRightsId[i] != 0)
			rightsIdIsSet = true;
	}

	return rightsIdIsSet;
}

const byte_t* nx::NcaHeader::getRightsId() const
{
	return mRightsId;
}

void nx::NcaHeader::setRightsId(const byte_t* rights_id)
{
	memcpy(mRightsId, rights_id, nca::kRightsIdLen);
}

const fnd::List<nx::NcaHeader::sPartition>& nx::NcaHeader::getPartitions() const
{
	return mPartitions;
}

void nx::NcaHeader::setPartitions(const fnd::List<nx::NcaHeader::sPartition>& partitions)
{
	mPartitions = partitions;
	if (mPartitions.size() >= nca::kPartitionNum)
	{
		throw fnd::Exception(kModuleName, "Too many NCA partitions");
	}
}

const fnd::List<crypto::aes::sAes128Key>& nx::NcaHeader::getEncAesKeys() const
{
	return mEncAesKeys;
}

void nx::NcaHeader::setEncAesKeys(const fnd::List<crypto::aes::sAes128Key>& keys)
{
	mEncAesKeys = keys;
}

uint64_t nx::NcaHeader::blockNumToSize(uint32_t block_num) const
{
	return block_num*nca::kSectorSize;
}

uint32_t nx::NcaHeader::sizeToBlockNum(uint64_t real_size) const
{
	return (uint32_t)(align(real_size, nca::kSectorSize) / nca::kSectorSize);
}