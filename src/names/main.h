// Copyright (c) 2014-2019 Daniel Kraft
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef H_BITCOIN_NAMES_MAIN
#define H_BITCOIN_NAMES_MAIN

#include <amount.h>
#include <names/common.h>
#include <primitives/transaction.h>
#include <serialize.h>

#include <set>

class CBlockUndo;
class CCoinsView;
class CCoinsViewCache;
class CTxMemPool;
class CValidationState;

/* Some constants defining name limits.  */
constexpr unsigned MAX_VALUE_LENGTH = 1023;
constexpr unsigned MAX_NAME_LENGTH = 255;
constexpr unsigned MIN_FIRSTUPDATE_DEPTH = 12;
constexpr unsigned MAX_VALUE_LENGTH_UI = 520;

/** The amount of coins to lock in created transactions.  */
constexpr CAmount NAME_LOCKED_AMOUNT = COIN / 100;

/* ************************************************************************** */
/* CNameTxUndo.  */

/**
 * Undo information for one name operation.  This contains either the
 * information that the name was newly created (and should thus be
 * deleted entirely) or that it was updated including the old value.
 */
class CNameTxUndo
{

private:

  /** The name this concerns.  */
  valtype name;

  /** Whether this was an entirely new name (no update).  */
  bool isNew;

  /** The old name value that was overwritten by the operation.  */
  CNameData oldData;

public:

  ADD_SERIALIZE_METHODS;

  template<typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action)
  {
    READWRITE (name);
    READWRITE (isNew);
    if (!isNew)
      READWRITE (oldData);
  }

  /**
   * Set the data for an update/registration of the given name.  The CCoinsView
   * is used to find out all the necessary information.
   * @param nm The name that is being updated.
   * @param view The (old!) chain state.
   */
  void fromOldState (const valtype& nm, const CCoinsView& view);

  /**
   * Apply the undo to the chain state given.
   * @param view The chain state to update ("undo").
   */
  void apply (CCoinsViewCache& view) const;

};

/* ************************************************************************** */

/**
 * Check a transaction according to the additional Namecoin rules.  This
 * ensures that all name operations (if any) are valid and that it has
 * name operations iff it is marked as Namecoin tx by its version.
 * @param tx The transaction to check.
 * @param nHeight Height at which the tx will be.
 * @param view The current chain state.
 * @param state Resulting validation state.
 * @param flags Verification flags.
 * @return True in case of success.
 */
bool CheckNameTransaction (const CTransaction& tx, unsigned nHeight,
                           const CCoinsView& view,
                           CValidationState& state, unsigned flags);

/**
 * Apply the changes of a name transaction to the name database.
 * @param tx The transaction to apply.
 * @param nHeight Height at which the tx is.  Used for CNameData.
 * @param view The chain state to update.
 * @param undo Record undo information here.
 */
void ApplyNameTransaction (const CTransaction& tx, unsigned nHeight,
                           CCoinsViewCache& view, CBlockUndo& undo);

/**
 * Expire all names at the given height.  This removes their coins
 * from the UTXO set.
 * @param height The new block height.
 * @param view The coins view to update.
 * @param undo The block undo object to record undo information.
 * @param names List all expired names here.
 * @return True if successful.
 */
bool ExpireNames (unsigned nHeight, CCoinsViewCache& view, CBlockUndo& undo,
                  std::set<valtype>& names);

/**
 * Undo name coin expirations.  This also does some checks verifying
 * that all is fine.
 * @param nHeight The height at which the names were expired.
 * @param undo The block undo object to use.
 * @param view The coins view to update.
 * @param names List all unexpired names here.
 * @return True if successful.
 */
bool UnexpireNames (unsigned nHeight, CBlockUndo& undo,
                    CCoinsViewCache& view, std::set<valtype>& names);

/**
 * Check the name database consistency.  This calls CCoinsView::ValidateNameDB,
 * but only if applicable depending on the -checknamedb setting.  If it fails,
 * this throws an assertion failure.
 * @param disconnect Whether we are disconnecting blocks.
 */
void CheckNameDB (bool disconnect);

#endif // H_BITCOIN_NAMES_MAIN
