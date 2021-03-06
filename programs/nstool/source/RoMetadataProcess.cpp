#include <sstream>
#include <fnd/types.h>

#include "RoMetadataProcess.h"

RoMetadataProcess::RoMetadataProcess() :
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mInstructionType(nx::npdm::INSTR_64BIT),
	mListApi(false),
	mListSymbols(false),
	mApiInfo(),
	mDynSym(),
	mDynStr(),
	mRoBlob(),
	mSdkVerApiList(),
	mPublicApiList(),
	mDebugApiList(),
	mPrivateApiList(),
	mSymbolList()
{

}

void RoMetadataProcess::process()
{
	if (mRoBlob.size() == 0)
	{
		throw fnd::Exception(kModuleName, "No ro binary set.");
	}

	importApiList();
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayRoMetaData();
}

void RoMetadataProcess::setRoBinary(const fnd::Vec<byte_t>& bin)
{
	mRoBlob = bin;
}

void RoMetadataProcess::setApiInfo(size_t offset, size_t size)
{
	mApiInfo.offset = offset;
	mApiInfo.size = size;
}
void RoMetadataProcess::setDynSym(size_t offset, size_t size)
{
	mDynSym.offset = offset;
	mDynSym.size = size;
}
void RoMetadataProcess::setDynStr(size_t offset, size_t size)
{
	mDynStr.offset = offset;
	mDynStr.size = size;
}

void RoMetadataProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void RoMetadataProcess::setInstructionType(nx::npdm::InstructionType type)
{
	mInstructionType = type;
}

void RoMetadataProcess::setListApi(bool listApi)
{
	mListApi = listApi;
}

void RoMetadataProcess::setListSymbols(bool listSymbols)
{
	mListSymbols = listSymbols;
}

void RoMetadataProcess::importApiList()
{
	if (mApiInfo.size > 0)
	{
		std::stringstream list_stream(std::string((char*)mRoBlob.data() + mApiInfo.offset, mApiInfo.size));
		std::string api_str;

		while(std::getline(list_stream, api_str, (char)0x00))
		{
			SdkApiString api(api_str);

			if (api.getApiType() == SdkApiString::API_SDK_VERSION)
				mSdkVerApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_MIDDLEWARE)
				mPublicApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_DEBUG)
				mDebugApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_PRIVATE)
				mPrivateApiList.push_back(api);
		}
	}

	if (mDynSym.size > 0)
	{
		mSymbolList.parseData(mRoBlob.data() + mDynSym.offset, mDynSym.size, mRoBlob.data() + mDynStr.offset, mDynStr.size, mInstructionType == nx::npdm::INSTR_64BIT);
	}
}

void RoMetadataProcess::displayRoMetaData()
{
	size_t api_num = mSdkVerApiList.size() + mPublicApiList.size() + mDebugApiList.size() + mPrivateApiList.size();
	
	if (api_num > 0 && (mListApi || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED)))
	{
		printf("[SDK API List]\n");
		if (mSdkVerApiList.size() > 0)
		{
			printf("  Sdk Revision: %s\n", mSdkVerApiList[0].getModuleName().c_str());
		}
		if (mPublicApiList.size() > 0)
		{
			printf("  Public APIs:\n");
			for (size_t i = 0; i < mPublicApiList.size(); i++)
			{
				printf("    %s (vender: %s)\n", mPublicApiList[i].getModuleName().c_str(), mPublicApiList[i].getVenderName().c_str());
			}
		}
		if (mDebugApiList.size() > 0)
		{
			printf("  Debug APIs:\n");
			for (size_t i = 0; i < mDebugApiList.size(); i++)
			{
				printf("    %s (vender: %s)\n", mDebugApiList[i].getModuleName().c_str(), mDebugApiList[i].getVenderName().c_str());
			}
		}
		if (mPrivateApiList.size() > 0)
		{
			printf("  Private APIs:\n");
			for (size_t i = 0; i < mPrivateApiList.size(); i++)
			{
				printf("    %s (vender: %s)\n", mPrivateApiList[i].getModuleName().c_str(), mPrivateApiList[i].getVenderName().c_str());
			}
		}
	}
	if (mSymbolList.getSymbolList().size() > 0 && (mListSymbols || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED)))
	{
		printf("[Symbol List]\n");
		for (size_t i = 0; i < mSymbolList.getSymbolList().size(); i++)
		{
			const ElfSymbolParser::sElfSymbol& symbol = mSymbolList.getSymbolList()[i];
			printf("  %s [SHN=%s (%04x)][STT=%s][STB=%s]\n", symbol.name.c_str(), getSectionIndexStr(symbol.shn_index), symbol.shn_index, getSymbolTypeStr(symbol.symbol_type), getSymbolBindingStr(symbol.symbol_binding));
		}
	}
}

const char* RoMetadataProcess::getSectionIndexStr(nx::elf::SpecialSectionIndex shn_index) const
{
	const char* str;
	switch (shn_index)
	{
		case (nx::elf::SHN_UNDEF):
			str = "UNDEF";
			break;
		case (nx::elf::SHN_LOPROC):
			str = "LOPROC";
			break;
		case (nx::elf::SHN_HIPROC):
			str = "HIPROC";
			break;
		case (nx::elf::SHN_LOOS):
			str = "LOOS";
			break;
		case (nx::elf::SHN_HIOS):
			str = "HIOS";
			break;
		case (nx::elf::SHN_ABS):
			str = "ABS";
			break;
		case (nx::elf::SHN_COMMON):
			str = "COMMON";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* RoMetadataProcess::getSymbolTypeStr(nx::elf::SymbolType symbol_type) const
{
	const char* str;
	switch (symbol_type)
	{
		case (nx::elf::STT_NOTYPE):
			str = "NOTYPE";
			break;
		case (nx::elf::STT_OBJECT):
			str = "OBJECT";
			break;
		case (nx::elf::STT_FUNC):
			str = "FUNC";
			break;
		case (nx::elf::STT_SECTION):
			str = "SECTION";
			break;
		case (nx::elf::STT_FILE):
			str = "FILE";
			break;
		case (nx::elf::STT_LOOS):
			str = "LOOS";
			break;
		case (nx::elf::STT_HIOS):
			str = "HIOS";
			break;
		case (nx::elf::STT_LOPROC):
			str = "LOPROC";
			break;
		case (nx::elf::STT_HIPROC):
			str = "HIPROC";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* RoMetadataProcess::getSymbolBindingStr(nx::elf::SymbolBinding symbol_binding) const
{
	const char* str;
	switch (symbol_binding)
	{
		case (nx::elf::STB_LOCAL):
			str = "LOCAL";
			break;
		case (nx::elf::STB_GLOBAL):
			str = "GLOBAL";
			break;
		case (nx::elf::STB_WEAK):
			str = "WEAK";
			break;
		case (nx::elf::STB_LOOS):
			str = "LOOS";
			break;
		case (nx::elf::STB_HIOS):
			str = "HIOS";
			break;
		case (nx::elf::STB_LOPROC):
			str = "LOPROC";
			break;
		case (nx::elf::STB_HIPROC):
			str = "HIPROC";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}