#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/List.h>
#include <fnd/ISerialisable.h>
#include <nx/aci.h>
#include <nx/FileSystemAccessControlBinary.h>
#include <nx/ServiceAccessControlBinary.h>
#include <nx/KcBinary.h>

namespace nx
{
	class AccessControlInfoDescBinary : public fnd::ISerialisable
	{
	public:
		struct sProgramIdRestrict
		{
			uint64_t min;
			uint64_t max;

			void operator=(const sProgramIdRestrict& other)
			{
				min = other.min;
				max = other.max;
			}

			bool operator==(const sProgramIdRestrict& other) const
			{
				return (min == other.min) \
					&& (max == other.max);
			}

			bool operator!=(const sProgramIdRestrict& other) const
			{
				return !(*this == other);
			}
		};

		AccessControlInfoDescBinary();
		AccessControlInfoDescBinary(const AccessControlInfoDescBinary& other);

		void operator=(const AccessControlInfoDescBinary& other);
		bool operator==(const AccessControlInfoDescBinary& other) const;
		bool operator!=(const AccessControlInfoDescBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		void generateSignature(const crypto::rsa::sRsa2048Key& key);
		void validateSignature(const crypto::rsa::sRsa2048Key& key) const;

		// variables
		void clear();

		const crypto::rsa::sRsa2048Key& getNcaHeaderSignature2Key() const;
		void setNcaHeaderSignature2Key(const crypto::rsa::sRsa2048Key& key);

		const fnd::List<aci::Flag>& getFlagList() const;
		void setFlagList(const fnd::List<aci::Flag>& flags);

		const sProgramIdRestrict& getProgramIdRestrict() const;
		void setProgramIdRestrict(const sProgramIdRestrict& pid_restrict);

		const nx::FileSystemAccessControlBinary& getFileSystemAccessControl() const;
		void setFileSystemAccessControl(const FileSystemAccessControlBinary& fac);

		const nx::ServiceAccessControlBinary& getServiceAccessControl() const;
		void setServiceAccessControl(const ServiceAccessControlBinary& sac);

		const nx::KcBinary& getKernelCapabilities() const;
		void setKernelCapabilities(const KcBinary& kc);
	private:
		const std::string kModuleName = "ACCESS_CONTROL_INFO_DESC_BINARY";

		// raw data
		fnd::Vec<byte_t> mRawBinary;

		// variables
		crypto::rsa::sRsa2048Key mNcaHeaderSignature2Key;
		fnd::List<aci::Flag> mFlags;
		sProgramIdRestrict mProgramIdRestrict;
		nx::FileSystemAccessControlBinary mFileSystemAccessControl;
		nx::ServiceAccessControlBinary mServiceAccessControl;
		nx::KcBinary mKernelCapabilities;
	};
}