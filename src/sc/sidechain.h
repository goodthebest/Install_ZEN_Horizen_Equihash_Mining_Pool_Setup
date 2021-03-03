#ifndef _SIDECHAIN_CORE_H
#define _SIDECHAIN_CORE_H

#include "amount.h"
#include "hash.h"
#include "sc/sidechaintypes.h"
#include <primitives/certificate.h>

class CValidationState;
class CTransaction;

class CSidechainEvents {
public:
    CSidechainEvents(): sidechainEventsVersion(0), ceasingScs(), maturingScs() {};
    ~CSidechainEvents() = default;

    int32_t sidechainEventsVersion;
    std::set<uint256> ceasingScs;
    std::set<uint256> maturingScs;

    bool IsNull() const {return ceasingScs.empty() && maturingScs.empty();}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(sidechainEventsVersion);
        READWRITE(ceasingScs);
        READWRITE(maturingScs);
    }

    inline bool operator==(const CSidechainEvents& rhs) const {
        return (this->sidechainEventsVersion  == rhs.sidechainEventsVersion) &&
               (this->ceasingScs              == rhs.ceasingScs)             &&
               (this->maturingScs             == rhs.maturingScs);
    }

    inline bool operator!=(const CSidechainEvents& rhs) const { return !(*this == rhs); }

    // Calculate the size of the cache (in bytes)
    size_t DynamicMemoryUsage() const;
};

class CSidechain {
public:
    CSidechain():
        sidechainVersion(0), creationBlockHash(), creationBlockHeight(-1), creationTxHash(),
        pastEpochTopQualityCertDataHash(), lastTopQualityCertDataHash(), lastTopQualityCertHash(),
        lastTopQualityCertReferencedEpoch(CScCertificate::EPOCH_NULL),
        lastTopQualityCertQuality(CScCertificate::QUALITY_NULL), lastTopQualityCertBwtAmount(0),
        balance(0) {}

    bool IsNull() const
    {
        return (
             creationBlockHash.IsNull()                                       &&
             creationBlockHeight == -1                                        &&
             creationTxHash.IsNull()                                          &&
             pastEpochTopQualityCertDataHash.IsNull()                         &&
             lastTopQualityCertDataHash.IsNull()                              &&
             lastTopQualityCertHash.IsNull()                                  &&
             lastTopQualityCertReferencedEpoch == CScCertificate::EPOCH_NULL  &&
             lastTopQualityCertQuality == CScCertificate::QUALITY_NULL        &&
             lastTopQualityCertBwtAmount == 0                                 &&
             balance == 0                                                     &&
             creationData.IsNull()                                            &&
             mImmatureAmounts.empty());
    }

    int32_t sidechainVersion;

    // reference to the block containing the tx that created the side chain
    uint256 creationBlockHash;

    // We can not serialize a pointer value to block index, but can retrieve it from chainActive if we have height
    int creationBlockHeight;

    // hash of the tx who created it
    uint256 creationTxHash;

    // Cert data hash section
    libzendoomc::ScFieldElement pastEpochTopQualityCertDataHash;
    libzendoomc::ScFieldElement lastTopQualityCertDataHash;

    // Data for latest top quality cert confirmed in blockchain
    uint256 lastTopQualityCertHash;
    int32_t lastTopQualityCertReferencedEpoch;
    int64_t lastTopQualityCertQuality;
    CAmount lastTopQualityCertBwtAmount;

    // total amount given by sum(fw transfer)-sum(bkw transfer)
    CAmount balance;

    // creation data
    Sidechain::ScCreationParameters creationData;

    // immature amounts
    // key   = height at which amount will be considered as mature and will be part of the sc balance
    // value = the immature amount
    std::map<int, CAmount> mImmatureAmounts;

    enum class State : uint8_t {
        NOT_APPLICABLE = 0,
        UNCONFIRMED,
        ALIVE,
        CEASED
    };

    static std::string stateToString(State s);

    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(sidechainVersion);
        READWRITE(creationBlockHash);
        READWRITE(VARINT(creationBlockHeight));
        READWRITE(creationTxHash);
        READWRITE(pastEpochTopQualityCertDataHash);
        READWRITE(lastTopQualityCertDataHash);
        READWRITE(lastTopQualityCertHash);
        READWRITE(lastTopQualityCertReferencedEpoch);
        READWRITE(lastTopQualityCertQuality);
        READWRITE(lastTopQualityCertBwtAmount);
        READWRITE(balance);
        READWRITE(creationData);
        READWRITE(mImmatureAmounts);
    }

    inline bool operator==(const CSidechain& rhs) const
    {
        return (this->sidechainVersion                   == rhs.sidechainVersion)                   &&
               (this->creationBlockHash                  == rhs.creationBlockHash)                  &&
               (this->creationBlockHeight                == rhs.creationBlockHeight)                &&
               (this->creationTxHash                     == rhs.creationTxHash)                     &&
               (this->pastEpochTopQualityCertDataHash    == rhs.pastEpochTopQualityCertDataHash)    &&
               (this->lastTopQualityCertDataHash         == rhs.lastTopQualityCertDataHash)         &&
               (this->lastTopQualityCertHash             == rhs.lastTopQualityCertHash)             &&
               (this->lastTopQualityCertReferencedEpoch  == rhs.lastTopQualityCertReferencedEpoch)  &&
               (this->lastTopQualityCertQuality          == rhs.lastTopQualityCertQuality)          &&
               (this->lastTopQualityCertBwtAmount        == rhs.lastTopQualityCertBwtAmount)        &&
               (this->balance                            == rhs.balance)                            &&
               (this->creationData                       == rhs.creationData)                       &&
               (this->mImmatureAmounts                   == rhs.mImmatureAmounts);
    }
    inline bool operator!=(const CSidechain& rhs) const { return !(*this == rhs); }

    int EpochFor(int targetHeight) const;
    int GetStartHeightForEpoch(int targetEpoch) const;
    int GetEndHeightForEpoch(int targetEpoch) const;
    int GetCertSubmissionWindowStart(int certEpoch) const;
    int GetCertSubmissionWindowEnd(int certEpoch) const;
    int GetCertSubmissionWindowLength() const;
    int GetCertMaturityHeight(int certEpoch) const;
    int GetScheduledCeasingHeight() const;

    bool isCreationConfirmed() const {
        return this->creationBlockHeight != -1;
    }

    // Calculate the size of the cache (in bytes)
    size_t DynamicMemoryUsage() const;
};

namespace Sidechain {
    bool checkCertSemanticValidity(const CScCertificate& cert, CValidationState& state);
    bool checkTxSemanticValidity(const CTransaction& tx, CValidationState& state);
    bool hasScCreationOutput(const CTransaction& tx, const uint256& scId);
}; // end of namespace

#endif // _SIDECHAIN_CORE_H

