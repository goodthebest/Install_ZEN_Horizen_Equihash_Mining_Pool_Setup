// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"
// ZEN_MOD_START
#include <map>
#include <string>
// ZEN_MOD_END

namespace Consensus {

// ZEN_MOD_START
enum DeploymentPos
{
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    // EXAMPLE: DEPLOYMENT_CBAH,
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;
};
// ZEN_MOD_END

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;

// ZEN_MOD_START
    /* Don't allow to send coinbase coins to transparent addresses. They should be first sent to the shielded addr */
// ZEN_MOD_END
    bool fCoinbaseMustBeProtected;

// ZEN_MOD_START
    /* Disable coinbase protection for founders reward coins, so it would be possible to send FR coins to transparent addr.
     * Activated only if fCoinbaseMustBeProtected is true, otherwise all coins could be sent to transparent addr in spite of this option */
    bool fDisableCoinbaseProtectionForFoundersReward;
// ZEN_MOD_END

    /** Needs to evenly divide MAX_SUBSIDY to avoid rounding errors. */
    int nSubsidySlowStartInterval;
    /**
     * Shift based on a linear ramp for slow start:
     *
     * MAX_SUBSIDY*(t_s/2 + t_r) = MAX_SUBSIDY*t_h  Coin balance
     *              t_s   + t_r  = t_h + t_c        Block balance
     *
     * t_s = nSubsidySlowStartInterval
     * t_r = number of blocks between end of slow start and first halving
     * t_h = nSubsidyHalvingInterval
     * t_c = SubsidySlowStartShift()
     */
    int SubsidySlowStartShift() const { return nSubsidySlowStartInterval / 2; }
    int nSubsidyHalvingInterval;
    int GetLastFoundersRewardBlockHeight() const {
        return nSubsidyHalvingInterval + SubsidySlowStartShift() - 1;
    }
    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;
// ZEN_MOD_START
    /**
    * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargeting period,
    * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
    * Examples: 1916 for 95%, 1512 for testchains.
    */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
// ZEN_MOD_END
    /** Proof of work parameters */
    uint256 powLimit;
    int64_t nPowAveragingWindow;
    int64_t nPowMaxAdjustDown;
    int64_t nPowMaxAdjustUp;
    int64_t nPowTargetSpacing;
    int64_t AveragingWindowTimespan() const { return nPowAveragingWindow * nPowTargetSpacing; }
    int64_t MinActualTimespan() const { return (AveragingWindowTimespan() * (100 - nPowMaxAdjustUp  )) / 100; }
    int64_t MaxActualTimespan() const { return (AveragingWindowTimespan() * (100 + nPowMaxAdjustDown)) / 100; }
// ZEN_MOD_START
    /** Zen chainsplit*/
    int nChainsplitIndex;
    int nChainsplitTime;
// ZEN_MOD_END
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
