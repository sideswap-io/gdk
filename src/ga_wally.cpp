#include <boost/algorithm/string/predicate.hpp>

#include "exception.hpp"
#include "ga_strings.hpp"
#include "ga_wally.hpp"
#include "memory.hpp"
#include "utils.hpp"

#define VERIFY_MNEMONIC(x)                                                                                             \
    do {                                                                                                               \
        if ((x) != WALLY_OK) {                                                                                         \
            throw user_error(res::id_invalid_mnemonic);                                                                \
        }                                                                                                              \
    } while (false)

namespace ga {
namespace sdk {

    std::array<unsigned char, HASH160_LEN> hash160(byte_span_t data)
    {
        std::array<unsigned char, HASH160_LEN> ret;
        GDK_VERIFY(wally_hash160(data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, SHA256_LEN> sha256(byte_span_t data)
    {
        std::array<unsigned char, SHA256_LEN> ret;
        GDK_VERIFY(wally_sha256(data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, SHA256_LEN> sha256d(byte_span_t data)
    {
        std::array<unsigned char, SHA256_LEN> ret;
        GDK_VERIFY(wally_sha256d(data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, SHA512_LEN> sha512(byte_span_t data)
    {
        std::array<unsigned char, SHA512_LEN> ret;
        GDK_VERIFY(wally_sha512(data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, HMAC_SHA256_LEN> hmac_sha256(byte_span_t key, byte_span_t data)
    {
        std::array<unsigned char, HMAC_SHA256_LEN> ret;
        GDK_VERIFY(wally_hmac_sha256(key.data(), key.size(), data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, HMAC_SHA512_LEN> hmac_sha512(byte_span_t key, byte_span_t data)
    {
        std::array<unsigned char, HMAC_SHA512_LEN> ret;
        GDK_VERIFY(wally_hmac_sha512(key.data(), key.size(), data.data(), data.size(), ret.data(), ret.size()));
        return ret;
    }

    pbkdf2_hmac512_t pbkdf2_hmac_sha512(byte_span_t password, byte_span_t salt, uint32_t cost)
    {
        const int32_t flags = 0;
        pbkdf2_hmac512_t ret;
        GDK_VERIFY(wally_pbkdf2_hmac_sha512(
            password.data(), password.size(), salt.data(), salt.size(), flags, cost, ret.data(), ret.size()));
        return ret;
    }

    pbkdf2_hmac256_t pbkdf2_hmac_sha512_256(byte_span_t password, byte_span_t salt, uint32_t cost)
    {
        auto tmp = pbkdf2_hmac_sha512(password, salt, cost);
        pbkdf2_hmac256_t out;
        std::copy(std::begin(tmp), std::begin(tmp) + out.size(), std::begin(out));
        wally_bzero(tmp.data(), tmp.size());
        return out;
    }

    //
    // BIP 32
    //
    std::array<unsigned char, BIP32_SERIALIZED_LEN> bip32_key_serialize(const ext_key& hdkey, uint32_t flags)
    {
        std::array<unsigned char, BIP32_SERIALIZED_LEN> ret;
        GDK_VERIFY(::bip32_key_serialize(&hdkey, flags, ret.data(), ret.size()));
        return ret;
    }

    wally_ext_key_ptr bip32_key_unserialize_alloc(byte_span_t data)
    {
        ext_key* p;
        GDK_VERIFY(::bip32_key_unserialize_alloc(data.data(), data.size(), &p));
        return wally_ext_key_ptr{ p };
    }

    ext_key bip32_public_key_from_parent_path(const ext_key& parent, uint32_span_t path)
    {
        const uint32_t flags = BIP32_FLAG_KEY_PUBLIC | BIP32_FLAG_SKIP_HASH;
        ext_key key;
        GDK_VERIFY(::bip32_key_from_parent_path(&parent, path.data(), path.size(), flags, &key));
        return key;
    }

    ext_key bip32_public_key_from_parent(const ext_key& parent, uint32_t pointer)
    {
        return bip32_public_key_from_parent_path(parent, { &pointer, 1u });
    }

    wally_ext_key_ptr bip32_public_key_from_bip32_xpub(const std::string& bip32_xpub)
    {
        return bip32_key_unserialize_alloc(base58check_to_bytes(bip32_xpub));
    }

    wally_ext_key_ptr bip32_key_from_parent_path_alloc(
        const wally_ext_key_ptr& parent, uint32_span_t path, uint32_t flags)
    {
        ext_key* p;
        GDK_VERIFY(::bip32_key_from_parent_path_alloc(parent.get(), path.data(), path.size(), flags, &p));
        return wally_ext_key_ptr{ p };
    }

    wally_ext_key_ptr bip32_key_init_alloc(uint32_t version, uint32_t depth, uint32_t child_num, byte_span_t chain_code,
        byte_span_t public_key, byte_span_t private_key, byte_span_t hash, byte_span_t parent)
    {
        ext_key* p;
        GDK_VERIFY(::bip32_key_init_alloc(version, depth, child_num, chain_code.data(), chain_code.size(),
            public_key.data(), public_key.size(), private_key.data(), private_key.size(), hash.data(), hash.size(),
            parent.data(), parent.size(), &p));
        return wally_ext_key_ptr{ p };
    }

    wally_ext_key_ptr bip32_key_from_seed_alloc(byte_span_t seed, uint32_t version, uint32_t flags)
    {
        ext_key* p;
        GDK_VERIFY(::bip32_key_from_seed_alloc(seed.data(), seed.size(), version, flags, &p));
        return wally_ext_key_ptr{ p };
    }

    //
    // Scripts
    //
    std::vector<unsigned char> scriptsig_p2pkh_from_der(byte_span_t public_key, byte_span_t sig)
    {
        std::vector<unsigned char> out(2 + public_key.size() + 2 + sig.size());
        size_t written;
        GDK_VERIFY(wally_scriptsig_p2pkh_from_der(
            public_key.data(), public_key.size(), sig.data(), sig.size(), out.data(), out.size(), &written));
        GDK_RUNTIME_ASSERT(written <= out.size());
        out.resize(written);
        return out;
    }

    std::vector<unsigned char> scriptsig_p2sh_p2wpkh_from_bytes(byte_span_t public_key)
    {
        return witness_script(public_key, WALLY_SCRIPT_HASH160 | WALLY_SCRIPT_AS_PUSH);
    }

    void scriptpubkey_csv_2of2_then_1_from_bytes(
        byte_span_t keys, uint32_t csv_blocks, bool optimize, std::vector<unsigned char>& out)
    {
        GDK_RUNTIME_ASSERT(!out.empty());
        const uint32_t flags = 0;
        size_t written;
        auto fn = optimize ? wally_scriptpubkey_csv_2of2_then_1_from_bytes_opt
                           : wally_scriptpubkey_csv_2of2_then_1_from_bytes;
        GDK_VERIFY(fn(keys.data(), keys.size(), csv_blocks, flags, &out[0], out.size(), &written));
        GDK_RUNTIME_ASSERT(written <= out.size());
        out.resize(written);
    }

    uint32_t get_csv_blocks_from_csv_redeem_script(byte_span_t redeem_script)
    {
        size_t csv_blocks_offset;

        if (redeem_script[0] == OP_DEPTH && redeem_script[1] == OP_1SUB && redeem_script[2] == OP_IF) {
            // 2of2 redeem script, with csv_blocks at:
            // OP_DEPTH OP_1SUB OP_IF <main_pubkey> OP_CHECKSIGVERIFY OP_ELSE <csv_blocks>
            csv_blocks_offset = 1 + 1 + 1 + (EC_PUBLIC_KEY_LEN + 1) + 1 + 1;
        } else if (redeem_script[0] == EC_PUBLIC_KEY_LEN && redeem_script[EC_PUBLIC_KEY_LEN + 1] == OP_CHECKSIGVERIFY
            && redeem_script[EC_PUBLIC_KEY_LEN + 2] == EC_PUBLIC_KEY_LEN
            && redeem_script[EC_PUBLIC_KEY_LEN * 2 + 3] == OP_CHECKSIG
            && redeem_script[EC_PUBLIC_KEY_LEN * 2 + 4] == OP_IFDUP) {
            // 2of2 optimized redeem script, with csv_blocks at:
            // <recovery_pubkey> OP_CHECKSIGVERIFY <main_pubkey> OP_CHECKSIG OP_IFDUP OP_NOTIF <csv_blocks>
            csv_blocks_offset = (EC_PUBLIC_KEY_LEN + 1) + 1 + (EC_PUBLIC_KEY_LEN + 1) + 1 + 1 + 1;
        } else {
            GDK_RUNTIME_ASSERT_MSG(false, "Invalid CSV redeem script");
            __builtin_unreachable();
        }
        // TODO: Move script integer parsing to wally and generalize
        size_t len = redeem_script[csv_blocks_offset];
        GDK_RUNTIME_ASSERT(len <= 4);
        // Negative CSV blocks are not allowed
        GDK_RUNTIME_ASSERT((redeem_script[csv_blocks_offset + len] & 0x80) == 0);

        uint32_t csv_blocks = 0;
        for (size_t i = 0; i < len; ++i) {
            uint32_t b = redeem_script[csv_blocks_offset + 1 + i];
            csv_blocks |= (b << (8 * i));
        }
        return csv_blocks;
    }

    void scriptpubkey_multisig_from_bytes(byte_span_t keys, uint32_t threshold, std::vector<unsigned char>& out)
    {
        GDK_RUNTIME_ASSERT(!out.empty());
        const uint32_t flags = 0;
        size_t written;
        GDK_VERIFY(wally_scriptpubkey_multisig_from_bytes(
            keys.data(), keys.size(), threshold, flags, &out[0], out.size(), &written));
        GDK_RUNTIME_ASSERT(written <= out.size());
        out.resize(written);
    }

    size_t varbuff_get_length(size_t script_len)
    {
        unsigned char dummy[1] = { 0 };
        size_t written;
        GDK_VERIFY(wally_varbuff_get_length(dummy, script_len, &written));
        return written;
    }

    std::vector<unsigned char> script_push_from_bytes(byte_span_t data)
    {
        std::vector<unsigned char> ret(data.size() + 5); // 5 = OP_PUSHDATA4 + 4 byte size
        const uint32_t flags = 0;
        size_t written;
        GDK_VERIFY(wally_script_push_from_bytes(data.data(), data.size(), flags, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written <= ret.size());
        ret.resize(written);
        return ret;
    }

    std::vector<unsigned char> scriptpubkey_p2pkh_from_hash160(byte_span_t hash)
    {
        GDK_RUNTIME_ASSERT(hash.size() == HASH160_LEN);
        size_t written;
        std::vector<unsigned char> ret(WALLY_SCRIPTPUBKEY_P2PKH_LEN);
        GDK_VERIFY(wally_scriptpubkey_p2pkh_from_bytes(hash.data(), hash.size(), 0, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written == WALLY_SCRIPTPUBKEY_P2PKH_LEN);
        return ret;
    }

    std::vector<unsigned char> scriptpubkey_p2pkh_from_public_key(byte_span_t public_key)
    {
        return scriptpubkey_p2pkh_from_hash160(hash160(public_key));
    }

    std::vector<unsigned char> scriptpubkey_p2sh_from_hash160(byte_span_t hash)
    {
        GDK_RUNTIME_ASSERT(hash.size() == HASH160_LEN);
        size_t written;
        std::vector<unsigned char> ret(WALLY_SCRIPTPUBKEY_P2SH_LEN);
        GDK_VERIFY(wally_scriptpubkey_p2sh_from_bytes(hash.data(), hash.size(), 0, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written == WALLY_SCRIPTPUBKEY_P2SH_LEN);
        return ret;
    }

    std::vector<unsigned char> scriptpubkey_p2sh_p2wsh_from_bytes(byte_span_t script)
    {
        const auto witness_program = witness_script(script, WALLY_SCRIPT_SHA256);
        return scriptpubkey_p2sh_from_hash160(hash160(witness_program));
    }

    uint32_t scriptpubkey_get_type(byte_span_t scriptpubkey)
    {
        size_t typ;
        GDK_VERIFY(wally_scriptpubkey_get_type(scriptpubkey.data(), scriptpubkey.size(), &typ));
        return static_cast<uint32_t>(typ);
    }

    std::vector<unsigned char> witness_script(byte_span_t script, uint32_t flags)
    {
        constexpr uint32_t segwit_v0 = 0;
        size_t written;
        std::vector<unsigned char> ret(WALLY_WITNESSSCRIPT_MAX_LEN);
        GDK_VERIFY(wally_witness_program_from_bytes_and_version(
            script.data(), script.size(), segwit_v0, flags, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written <= ret.size());
        ret.resize(written);
        return ret;
    }

    std::array<unsigned char, SHA256_LEN> format_bitcoin_message_hash(byte_span_t message)
    {
        const uint32_t flags = BITCOIN_MESSAGE_FLAG_HASH;
        size_t written;
        std::array<unsigned char, SHA256_LEN> ret;
        GDK_VERIFY(
            wally_format_bitcoin_message(message.data(), message.size(), flags, ret.data(), ret.size(), &written));
        GDK_RUNTIME_ASSERT(written == ret.size());
        return ret;
    }

    std::string electrum_script_hash_hex(byte_span_t script_bytes) { return b2h_rev(sha256(script_bytes)); }

    std::vector<unsigned char> scrypt(
        byte_span_t password, byte_span_t salt, uint32_t cost, uint32_t block_size, uint32_t parallelism)
    {
        std::vector<unsigned char> ret(64);
        GDK_VERIFY(wally_scrypt(password.data(), password.size(), salt.data(), salt.size(), cost, block_size,
            parallelism, &ret[0], ret.size()));
        return ret;
    }

    std::string bip39_mnemonic_from_bytes(byte_span_t data)
    {
        char* s;
        VERIFY_MNEMONIC(::bip39_mnemonic_from_bytes(nullptr, data.data(), data.size(), &s));
        if (::bip39_mnemonic_validate(nullptr, s) != GA_OK) {
            wally_free_string(s);
            // This should only be possible with bad hardware/cosmic rays
            GDK_RUNTIME_ASSERT_MSG(false, "Mnemonic creation failed!");
        }
        return make_string(s);
    }

    void bip39_mnemonic_validate(const std::string& mnemonic)
    {
        GDK_VERIFY(::bip39_mnemonic_validate(nullptr, mnemonic.c_str()));
    }

    std::vector<unsigned char> bip39_mnemonic_to_seed(const std::string& mnemonic, const std::string& passphrase)
    {
        VERIFY_MNEMONIC(::bip39_mnemonic_validate(nullptr, mnemonic.c_str()));
        size_t written;
        std::vector<unsigned char> ret(BIP39_SEED_LEN_512); // FIXME: secure_array
        VERIFY_MNEMONIC(::bip39_mnemonic_to_seed(
            mnemonic.c_str(), passphrase.empty() ? nullptr : passphrase.c_str(), &ret[0], ret.size(), &written));
        return ret;
    }

    std::vector<unsigned char> bip39_mnemonic_to_bytes(const std::string& mnemonic)
    {
        size_t written;
        std::vector<unsigned char> entropy(BIP39_ENTROPY_LEN_288); // FIXME: secure_array
        VERIFY_MNEMONIC(::bip39_mnemonic_to_bytes(nullptr, mnemonic.data(), entropy.data(), entropy.size(), &written));
        if (written != BIP39_ENTROPY_LEN_128 && written != BIP39_ENTROPY_LEN_256 && written != BIP39_ENTROPY_LEN_288) {
            throw user_error(res::id_invalid_mnemonic);
        }
        entropy.resize(written);
        return entropy;
    }

    //
    // Strings/Addresses
    //
    std::string b2h(byte_span_t data)
    {
        char* ret;
        GDK_VERIFY(wally_hex_from_bytes(data.data(), data.size(), &ret));
        return make_string(ret);
    }

    std::string b2h_rev(byte_span_t data)
    {
        char* ret;
        std::vector<unsigned char> buff(data.rbegin(), data.rend());
        GDK_VERIFY(wally_hex_from_bytes(buff.data(), buff.size(), &ret));
        return make_string(ret);
    }

    static auto h2b(const char* hex, size_t siz, bool rev, uint8_t prefix = 0)
    {
        GDK_RUNTIME_ASSERT(hex != nullptr && siz != 0);
        size_t written;
        const size_t bytes_siz = siz / 2;
        std::vector<unsigned char> buff(bytes_siz + (prefix != 0 ? 1 : 0));
        auto buff_data = buff.data() + (prefix != 0 ? 1 : 0);
        GDK_VERIFY(wally_hex_to_bytes(hex, buff_data, bytes_siz, &written));
        GDK_RUNTIME_ASSERT(written == bytes_siz);
        if (rev) {
            std::reverse(buff_data, buff_data + bytes_siz);
        }
        if (prefix != 0) {
            buff[0] = prefix;
        }
        return buff;
    }

    std::vector<unsigned char> h2b(const char* hex) { return h2b(hex, strlen(hex), false); }
    std::vector<unsigned char> h2b(const std::string& hex) { return h2b(hex.data(), hex.size(), false); }
    std::vector<unsigned char> h2b(const std::string& hex, uint8_t prefix)
    {
        return h2b(hex.data(), hex.size(), false, prefix);
    }

    std::vector<unsigned char> h2b_rev(const char* hex) { return h2b(hex, strlen(hex), true); }
    std::vector<unsigned char> h2b_rev(const std::string& hex) { return h2b(hex.data(), hex.size(), true); }
    std::vector<unsigned char> h2b_rev(const std::string& hex, uint8_t prefix)
    {
        return h2b(hex.data(), hex.size(), true, prefix);
    }

    bool validate_hex(const std::string& hex, size_t len)
    {
        return hex.size() == len * 2 && wally_hex_verify(hex.c_str()) == WALLY_OK;
    }

    std::string base58check_from_bytes(byte_span_t data)
    {
        char* ret;
        GDK_VERIFY(wally_base58_from_bytes(data.data(), data.size(), BASE58_FLAG_CHECKSUM, &ret));
        return make_string(ret);
    }

    bool validate_base58check(const std::string& base58)
    {
        std::vector<unsigned char> ret(BASE58_CHECKSUM_LEN + 1);
        size_t written;
        return wally_base58_to_bytes(base58.data(), BASE58_FLAG_CHECKSUM, &ret[0], ret.size(), &written) == WALLY_OK;
    }

    std::vector<unsigned char> base58check_to_bytes(const std::string& base58)
    {
        size_t written;
        GDK_VERIFY(wally_base58_get_length(base58.data(), &written));
        std::vector<unsigned char> ret(written);
        GDK_VERIFY(wally_base58_to_bytes(base58.data(), BASE58_FLAG_CHECKSUM, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written <= ret.size());
        ret.resize(written);
        return ret;
    }

    wally_string_ptr base64_string_from_bytes(byte_span_t bytes)
    {
        char* output = nullptr;
        GDK_VERIFY(wally_base64_from_bytes(bytes.data(), bytes.size(), 0, &output));
        return wally_string_ptr(output);
    }

    std::string base64_from_bytes(byte_span_t bytes)
    {
        auto ret = base64_string_from_bytes(bytes);
        return std::string(ret.get());
    }

    std::vector<unsigned char> base64_to_bytes(const std::string& base64)
    {
        size_t written;
        GDK_VERIFY(wally_base64_get_maximum_length(base64.data(), 0, &written));
        std::vector<unsigned char> ret(written);
        GDK_VERIFY(wally_base64_to_bytes(base64.data(), 0, &ret[0], ret.size(), &written));
        GDK_RUNTIME_ASSERT(written <= ret.size());
        ret.resize(written);
        return ret;
    }

    //
    // Signing/Encryption
    //
    void aes(byte_span_t key, byte_span_t data, uint32_t flags, std::vector<unsigned char>& out)
    {
        GDK_RUNTIME_ASSERT(!out.empty());
        GDK_VERIFY(wally_aes(key.data(), key.size(), data.data(), data.size(), flags, &out[0], out.size()));
    }

    void aes_cbc(byte_span_t key, byte_span_t iv, byte_span_t data, uint32_t flags, std::vector<unsigned char>& out)
    {
        size_t written;
        GDK_RUNTIME_ASSERT(!out.empty());
        GDK_VERIFY(wally_aes_cbc(key.data(), key.size(), iv.data(), iv.size(), data.data(), data.size(), flags, &out[0],
            out.size(), &written));
        GDK_RUNTIME_ASSERT(written <= out.size());
        out.resize(written);
    }

    ecdsa_sig_t ec_sig_from_bytes(byte_span_t private_key, byte_span_t hash, uint32_t flags)
    {
        ecdsa_sig_t ret;
        GDK_VERIFY(wally_ec_sig_from_bytes(
            private_key.data(), private_key.size(), hash.data(), hash.size(), flags, ret.data(), ret.size()));
        return ret;
    }

    ecdsa_sig_rec_t ec_sig_rec_from_bytes(byte_span_t private_key, byte_span_t hash, uint32_t flags)
    {
        ecdsa_sig_rec_t ret;
        flags |= EC_FLAG_RECOVERABLE;
        GDK_VERIFY(wally_ec_sig_from_bytes(
            private_key.data(), private_key.size(), hash.data(), hash.size(), flags, ret.data(), ret.size()));
        return ret;
    }

    ecdsa_sig_rec_t ec_sig_rec_from_compact(byte_span_t compact_sig, byte_span_t hash, byte_span_t public_key)
    {
        ecdsa_sig_rec_t rec_sig;
        std::copy(compact_sig.begin(), compact_sig.end(), rec_sig.data() + 1);
        // Grind the recid over its possible values (0, 1, 2, 3)
        for (uint32_t recid = 0; recid < 4; ++recid) {
            rec_sig[0] = static_cast<unsigned char>(27 + recid + 4);
            std::vector<unsigned char> rec_public_key(EC_PUBLIC_KEY_LEN);
            if (wally_ec_sig_to_public_key(hash.data(), hash.size(), rec_sig.data(), rec_sig.size(),
                    rec_public_key.data(), rec_public_key.size())
                != WALLY_OK) {
                continue;
            }
            if (!memcmp(rec_public_key.data(), public_key.data(), rec_public_key.size())) {
                return rec_sig;
            }
        }
        GDK_RUNTIME_ASSERT_MSG(false, "Invalid public key for signature and hash");
        __builtin_unreachable();
    }

    std::vector<unsigned char> ec_sig_to_der(byte_span_t sig, uint32_t sighash_flags)
    {
        std::vector<unsigned char> der(EC_SIGNATURE_DER_MAX_LEN + 1);
        size_t written;
        GDK_VERIFY(wally_ec_sig_to_der(sig.data(), sig.size(), der.data(), der.size(), &written));
        GDK_RUNTIME_ASSERT(written <= der.size());
        der.resize(written);
        der.push_back(sighash_flags);
        return der;
    }

    std::string sig_only_to_der_hex(const ecdsa_sig_t& signature)
    {
        std::vector<unsigned char> der = ec_sig_to_der(signature);
        // Remove sighash byte
        der.pop_back();
        return b2h(der);
    }

    ecdsa_sig_t ec_sig_from_der(byte_span_t der, bool has_sighash_byte)
    {
        ecdsa_sig_t sig;
        int ret = WALLY_EINVAL;
        if (der.size()) {
            const auto non_sighash_len = der.size() - (has_sighash_byte ? 1 : 0);
            ret = wally_ec_sig_from_der(der.data(), non_sighash_len, sig.data(), sig.size());
        }
        if (ret != WALLY_OK) {
            throw user_error("Invalid signature");
        }
        return sig;
    }

    bool ec_sig_verify(byte_span_t public_key, byte_span_t message_hash, byte_span_t sig, uint32_t flags)
    {
        return wally_ec_sig_verify(public_key.data(), public_key.size(), message_hash.data(), message_hash.size(),
                   flags, sig.data(), sig.size())
            == WALLY_OK;
    }

    std::vector<unsigned char> ec_public_key_from_private_key(byte_span_t private_key, bool do_decompress)
    {
        std::vector<unsigned char> ret(EC_PUBLIC_KEY_LEN);
        GDK_VERIFY(
            wally_ec_public_key_from_private_key(private_key.data(), private_key.size(), ret.data(), ret.size()));

        if (do_decompress) {
            std::vector<unsigned char> ret_uncompressed(EC_PUBLIC_KEY_UNCOMPRESSED_LEN);
            GDK_VERIFY(wally_ec_public_key_decompress(
                ret.data(), ret.size(), ret_uncompressed.data(), ret_uncompressed.size()));
            return ret_uncompressed;
        }
        return ret;
    }

    std::pair<std::vector<unsigned char>, bool> to_private_key_bytes(
        const std::string& encoded, const std::string& passphrase, bool is_mainnet)
    {
        std::vector<unsigned char> private_key(EC_PRIVATE_KEY_LEN);
        bool is_compressed = false;

        if (boost::algorithm::starts_with(encoded, "xprv") || boost::algorithm::starts_with(encoded, "tprv")) {
            // BIP 32 Serialized private key
            // TODO: Support scanning for children under BIP44 paths
            ext_key master;
            GDK_VERIFY(bip32_key_from_base58(encoded.c_str(), &master));
            memcpy(private_key.data(), master.priv_key + 1, sizeof(master.priv_key) - 1);
            wally_bzero(&master, sizeof(master));
        } else if (encoded.size() == 51u || encoded.size() == 52u) {
            // WIF
            // FIXME: Add wally constants for the WIF base58 lengths
            is_compressed = encoded.size() == 52u;
            uint32_t prefix = is_mainnet ? WALLY_ADDRESS_VERSION_WIF_MAINNET : WALLY_ADDRESS_VERSION_WIF_TESTNET;
            uint32_t flags = is_compressed ? WALLY_WIF_FLAG_COMPRESSED : WALLY_WIF_FLAG_UNCOMPRESSED;
            GDK_VERIFY(wally_wif_to_bytes(encoded.c_str(), prefix, flags, private_key.data(), private_key.size()));
        } else if (encoded.size() == 58) {
            // BIP38
            auto raw = base58check_to_bytes(encoded);
            size_t flags;
            GDK_VERIFY(::bip38_raw_get_flags(raw.data(), raw.size(), &flags));
            flags |= (is_mainnet ? BIP38_KEY_MAINNET : BIP38_KEY_TESTNET);
            is_compressed = (flags & BIP38_KEY_COMPRESSED) != 0;
            GDK_VERIFY(::bip38_raw_to_private_key(raw.data(), raw.size(), ustring_span(passphrase).data(),
                passphrase.size(), flags, private_key.data(), private_key.size()));
        } else {
            throw user_error(res::id_invalid_private_key);
        }
        return { std::move(private_key), is_compressed };
    }

    bool ec_private_key_verify(byte_span_t bytes)
    {
        return wally_ec_private_key_verify(bytes.data(), bytes.size()) == WALLY_OK;
    }

    std::pair<priv_key_t, std::vector<unsigned char>> get_ephemeral_keypair()
    {
        priv_key_t private_key;
        do {
            private_key = get_random_bytes<32>();
        } while (!ec_private_key_verify(private_key));
        return { private_key, ec_public_key_from_private_key(private_key) };
    }

    std::array<unsigned char, SHA256_LEN> ecdh(byte_span_t public_key, byte_span_t private_key)
    {
        std::array<unsigned char, SHA256_LEN> ret;
        GDK_VERIFY(wally_ecdh(
            public_key.data(), public_key.size(), private_key.data(), private_key.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, WALLY_HOST_COMMITMENT_LEN> ae_host_commit_from_bytes(
        byte_span_t host_entropy, uint32_t flags)
    {
        std::array<unsigned char, WALLY_HOST_COMMITMENT_LEN> ret;
        GDK_VERIFY(
            wally_ae_host_commit_from_bytes(host_entropy.data(), host_entropy.size(), flags, ret.data(), ret.size()));
        return ret;
    }

    bool ec_scalar_verify(byte_span_t scalar)
    {
        return wally_ec_scalar_verify(scalar.data(), scalar.size()) == WALLY_OK;
    }

    std::array<unsigned char, EC_SCALAR_LEN> ec_scalar_add(byte_span_t a, byte_span_t b)
    {
        std::array<unsigned char, EC_SCALAR_LEN> ret;
        GDK_VERIFY(wally_ec_scalar_add(a.data(), a.size(), b.data(), b.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, EC_SCALAR_LEN> ec_scalar_subtract(byte_span_t a, byte_span_t b)
    {
        std::array<unsigned char, EC_SCALAR_LEN> ret;
        GDK_VERIFY(wally_ec_scalar_subtract(a.data(), a.size(), b.data(), b.size(), ret.data(), ret.size()));
        return ret;
    }

    //
    // Elements
    //
    std::array<unsigned char, ASSET_GENERATOR_LEN> asset_generator_from_bytes(byte_span_t asset, byte_span_t abf)
    {
        std::array<unsigned char, ASSET_GENERATOR_LEN> generator;
        GDK_VERIFY(wally_asset_generator_from_bytes(
            asset.data(), asset.size(), abf.data(), abf.size(), generator.data(), generator.size()));
        return generator;
    }

    std::array<unsigned char, ASSET_TAG_LEN> asset_final_vbf(
        uint64_span_t values, size_t num_inputs, byte_span_t abf, byte_span_t vbf)
    {
        std::array<unsigned char, ASSET_TAG_LEN> v;
        GDK_VERIFY(wally_asset_final_vbf(values.data(), values.size(), num_inputs, abf.data(), abf.size(), vbf.data(),
            vbf.size(), v.data(), v.size()));
        return v;
    }

    std::array<unsigned char, EC_SCALAR_LEN> asset_scalar_offset(uint64_t value, byte_span_t abf, byte_span_t vbf)
    {
        std::array<unsigned char, EC_SCALAR_LEN> ret;
        GDK_VERIFY(
            wally_asset_scalar_offset(value, abf.data(), abf.size(), vbf.data(), vbf.size(), ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, ASSET_COMMITMENT_LEN> asset_value_commitment(
        uint64_t value, byte_span_t vbf, byte_span_t generator)
    {
        std::array<unsigned char, ASSET_COMMITMENT_LEN> commitment;
        GDK_VERIFY(wally_asset_value_commitment(
            value, vbf.data(), vbf.size(), generator.data(), generator.size(), commitment.data(), commitment.size()));
        return commitment;
    }

    size_t asset_rangeproof_max_size(uint64_t value, int min_bits)
    {
        size_t written;
        GDK_VERIFY(wally_asset_rangeproof_get_maximum_len(value, min_bits, &written));
        return written;
    }

    std::vector<unsigned char> asset_rangeproof(uint64_t value, byte_span_t public_key, byte_span_t private_key,
        byte_span_t asset, byte_span_t abf, byte_span_t vbf, byte_span_t commitment, byte_span_t extra,
        byte_span_t generator, uint64_t min_value, int exp, int min_bits)
    {
        std::vector<unsigned char> rangeproof(ASSET_RANGEPROOF_MAX_LEN);
        size_t written;
        GDK_VERIFY(wally_asset_rangeproof(value, public_key.data(), public_key.size(), private_key.data(),
            private_key.size(), asset.data(), asset.size(), abf.data(), abf.size(), vbf.data(), vbf.size(),
            commitment.data(), commitment.size(), extra.data(), extra.size(), generator.data(), generator.size(),
            min_value, exp, min_bits, rangeproof.data(), rangeproof.size(), &written));
        GDK_RUNTIME_ASSERT(written <= rangeproof.size());
        rangeproof.resize(written);
        return rangeproof;
    }

    std::vector<unsigned char> explicit_rangeproof(
        uint64_t value, byte_span_t nonce_hash, byte_span_t vbf, byte_span_t commitment, byte_span_t generator)
    {
        std::vector<unsigned char> rangeproof(ASSET_EXPLICIT_RANGEPROOF_MAX_LEN);
        size_t written;
        GDK_VERIFY(wally_explicit_rangeproof(value, nonce_hash.data(), nonce_hash.size(), vbf.data(), vbf.size(),
            commitment.data(), commitment.size(), generator.data(), generator.size(), rangeproof.data(),
            rangeproof.size(), &written));
        GDK_RUNTIME_ASSERT(written <= rangeproof.size());
        rangeproof.resize(written);
        return rangeproof;
    }

    bool explicit_rangeproof_verify(
        byte_span_t rangeproof, uint64_t value, byte_span_t commitment, byte_span_t generator)
    {
        return wally_explicit_rangeproof_verify(rangeproof.data(), rangeproof.size(), value, commitment.data(),
                   commitment.size(), generator.data(), generator.size())
            == WALLY_OK;
    }

    size_t asset_surjectionproof_size(size_t num_inputs)
    {
        size_t written;
        GDK_VERIFY(wally_asset_surjectionproof_size(num_inputs, &written));
        return written;
    }

    std::vector<unsigned char> asset_surjectionproof(byte_span_t output_asset, byte_span_t output_abf,
        byte_span_t output_generator, byte_span_t bytes, byte_span_t asset, byte_span_t abf, byte_span_t generator)
    {
        size_t written;
        std::vector<unsigned char> surjproof(asset_surjectionproof_size(asset.size() / ASSET_TAG_LEN));
        GDK_VERIFY(wally_asset_surjectionproof(output_asset.data(), output_asset.size(), output_abf.data(),
            output_abf.size(), output_generator.data(), output_generator.size(), bytes.data(), bytes.size(),
            asset.data(), asset.size(), abf.data(), abf.size(), generator.data(), generator.size(), surjproof.data(),
            surjproof.size(), &written));
        GDK_RUNTIME_ASSERT(written <= surjproof.size());
        surjproof.resize(written);
        return surjproof;
    }

    unblind_t asset_unblind(byte_span_t private_key, byte_span_t rangeproof, byte_span_t commitment,
        byte_span_t nonce_commitment, byte_span_t extra_commitment, byte_span_t generator)
    {
        asset_id_t asset_id;
        vbf_t vbf;
        abf_t abf;
        uint64_t value;

        GDK_VERIFY(wally_asset_unblind(nonce_commitment.data(), nonce_commitment.size(), private_key.data(),
            private_key.size(), rangeproof.data(), rangeproof.size(), commitment.data(), commitment.size(),
            extra_commitment.data(), extra_commitment.size(), generator.data(), generator.size(), asset_id.data(),
            asset_id.size(), abf.data(), abf.size(), vbf.data(), vbf.size(), &value));

        return std::make_tuple(asset_id, vbf, abf, value);
    }

    unblind_t asset_unblind_with_nonce(byte_span_t blinding_nonce, byte_span_t rangeproof, byte_span_t commitment,
        byte_span_t extra_commitment, byte_span_t generator)
    {
        asset_id_t asset_id;
        vbf_t vbf;
        abf_t abf;
        uint64_t value;

        GDK_VERIFY(wally_asset_unblind_with_nonce(blinding_nonce.data(), blinding_nonce.size(), rangeproof.data(),
            rangeproof.size(), commitment.data(), commitment.size(), extra_commitment.data(), extra_commitment.size(),
            generator.data(), generator.size(), asset_id.data(), asset_id.size(), abf.data(), abf.size(), vbf.data(),
            vbf.size(), &value));

        return std::make_tuple(asset_id, vbf, abf, value);
    }

    bool is_possible_confidential_addr(const std::string& address)
    {
        const size_t expected_len = 2 + EC_PUBLIC_KEY_LEN + HASH160_LEN + BASE58_CHECKSUM_LEN;
        size_t len;
        return wally_base58_n_get_length(address.data(), address.size(), &len) == WALLY_OK && len == expected_len;
    }

    std::string confidential_addr_to_addr(const std::string& address, uint32_t prefix)
    {
        char* addr;
        int ret = wally_confidential_addr_to_addr(address.c_str(), prefix, &addr);
        if (ret != WALLY_OK) {
            // Don't log using GDK_VERIFY as this occurs during non-error conditions
            throw assertion_error(address + " is not confidential");
        }
        return make_string(addr);
    }

    std::string confidential_addr_to_addr_segwit(
        const std::string& address, const std::string& confidential_prefix, const std::string& family)
    {
        char* ret;
        GDK_VERIFY(
            wally_confidential_addr_to_addr_segwit(address.c_str(), confidential_prefix.c_str(), family.c_str(), &ret));
        return make_string(ret);
    }

    pub_key_t confidential_addr_to_ec_public_key(const std::string& address, uint32_t prefix)
    {
        pub_key_t pub_key;
        GDK_VERIFY(wally_confidential_addr_to_ec_public_key(address.c_str(), prefix, pub_key.data(), pub_key.size()));
        return pub_key;
    }

    pub_key_t confidential_addr_segwit_to_ec_public_key(
        const std::string& address, const std::string& confidential_prefix)
    {
        pub_key_t pub_key;
        GDK_VERIFY(wally_confidential_addr_segwit_to_ec_public_key(
            address.c_str(), confidential_prefix.c_str(), pub_key.data(), pub_key.size()));
        return pub_key;
    }

    std::string confidential_addr_from_addr(
        const std::string& address, uint32_t prefix, const std::string blinding_pubkey_hex)
    {
        const auto pubkey = h2b(blinding_pubkey_hex);
        char* ret;
        GDK_VERIFY(wally_confidential_addr_from_addr(address.c_str(), prefix, pubkey.data(), pubkey.size(), &ret));
        return make_string(ret);
    }

    std::string confidential_addr_from_addr_segwit(const std::string& address, const std::string& family,
        const std::string& confidential_prefix, const std::string blinding_pubkey_hex)
    {
        const auto pubkey = h2b(blinding_pubkey_hex);
        char* ret;
        GDK_VERIFY(wally_confidential_addr_from_addr_segwit(
            address.c_str(), family.c_str(), confidential_prefix.c_str(), pubkey.data(), pubkey.size(), &ret));
        return make_string(ret);
    }

    blinding_key_t asset_blinding_key_from_seed(byte_span_t seed)
    {
        blinding_key_t blinding_key;
        GDK_VERIFY(
            wally_asset_blinding_key_from_seed(seed.data(), seed.size(), blinding_key.data(), blinding_key.size()));
        return blinding_key;
    }

    priv_key_t asset_blinding_key_to_ec_private_key(byte_span_t blinding_key, byte_span_t script)
    {
        priv_key_t priv_key;
        GDK_VERIFY(wally_asset_blinding_key_to_ec_private_key(
            blinding_key.data(), blinding_key.size(), script.data(), script.size(), priv_key.data(), priv_key.size()));
        return priv_key;
    }

    abf_vbf_t asset_blinding_key_to_abf_vbf(byte_span_t blinding_key, byte_span_t hash_prevouts, uint32_t output_index)
    {
        abf_vbf_t ret;
        GDK_VERIFY(wally_asset_blinding_key_to_abf_vbf(blinding_key.data(), blinding_key.size(), hash_prevouts.data(),
            hash_prevouts.size(), output_index, ret.data(), ret.size()));
        return ret;
    }

    std::array<unsigned char, SHA256_LEN> get_hash_prevouts(byte_span_t txids, uint32_span_t output_indices)
    {
        std::array<unsigned char, SHA256_LEN> ret;
        GDK_VERIFY(wally_get_hash_prevouts(
            txids.data(), txids.size(), output_indices.data(), output_indices.size(), ret.data(), ret.size()));
        return ret;
    }

    cvalue_t tx_confidential_value_from_satoshi(uint64_t satoshi)
    {
        cvalue_t ct_value;
        GDK_VERIFY(wally_tx_confidential_value_from_satoshi(satoshi, ct_value.data(), ct_value.size()));
        return ct_value;
    }

    uint64_t tx_confidential_value_to_satoshi(byte_span_t ct_value)
    {
        uint64_t satoshi;
        GDK_VERIFY(wally_tx_confidential_value_to_satoshi(ct_value.data(), ct_value.size(), &satoshi));
        return satoshi;
    }

    xpub_t make_xpub(const std::string& chain_code_hex, const std::string& public_key_hex)
    {
        size_t written;
        xpub_t xpub;
        GDK_VERIFY(wally_hex_to_bytes(chain_code_hex.c_str(), xpub.first.data(), xpub.first.size(), &written));
        GDK_RUNTIME_ASSERT(written == xpub.first.size());
        GDK_VERIFY(wally_hex_to_bytes(public_key_hex.c_str(), xpub.second.data(), xpub.second.size(), &written));
        GDK_RUNTIME_ASSERT(written == xpub.second.size());
        return xpub;
    }

    xpub_t make_xpub(const ext_key* hdkey)
    {
        xpub_t xpub;
        std::copy(std::begin(hdkey->chain_code), std::end(hdkey->chain_code), std::begin(xpub.first));
        std::copy(std::begin(hdkey->pub_key), std::end(hdkey->pub_key), std::begin(xpub.second));
        return xpub;
    }

    xpub_t make_xpub(const std::string& bip32_xpub)
    {
        return make_xpub(bip32_public_key_from_bip32_xpub(bip32_xpub).get());
    }

    std::string bip32_key_to_base58(const struct ext_key* hdkey, uint32_t flags)
    {
        char* s;
        GDK_VERIFY(::bip32_key_to_base58(hdkey, flags, &s));
        return make_string(s);
    }

} /* namespace sdk */
} /* namespace ga */
