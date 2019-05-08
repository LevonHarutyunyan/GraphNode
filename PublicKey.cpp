#include "PublicKey.h"

#include <cstring>
#include <stdio.h>
#include <iostream>

//just copied from secp library, can't imagine how should i wite this on my own
static int ecdsa_signature_parse_der_lax(const secp256k1_context *ctx, 
										secp256k1_ecdsa_signature *sig, 
										const unsigned char *input, 
										size_t inputlen) 
{
  size_t rpos, rlen, spos, slen;
  size_t pos = 0;
  size_t lenbyte;
  unsigned char tmpsig[64] = {0};
  int overflow = 0;

  /* Hack to initialize sig with a correctly-parsed but invalid signature. */
  secp256k1_ecdsa_signature_parse_compact(ctx, sig, tmpsig);

  /* Sequence tag byte */
  if (pos == inputlen || input[pos] != 0x30) {
    return 0;
  }
  pos++;

  /* Sequence length bytes */
  if (pos == inputlen) {
    return 0;
  }
  lenbyte = input[pos++];
  if (lenbyte & 0x80) {
    lenbyte -= 0x80;
    if (pos + lenbyte > inputlen) {
      return 0;
    }
    pos += lenbyte;
  }

  /* Integer tag byte for R */
  if (pos == inputlen || input[pos] != 0x02) {
    return 0;
  }
  pos++;

  /* Integer length for R */
  if (pos == inputlen) {
    return 0;
  }
  lenbyte = input[pos++];
  if (lenbyte & 0x80) {
    lenbyte -= 0x80;
    if (pos + lenbyte > inputlen) {
      return 0;
    }
    while (lenbyte > 0 && input[pos] == 0) {
      pos++;
      lenbyte--;
    }
    if (lenbyte >= sizeof(size_t)) {
      return 0;
    }
    rlen = 0;
    while (lenbyte > 0) {
      rlen = (rlen << 8) + input[pos];
      pos++;
      lenbyte--;
    }
  } else {
    rlen = lenbyte;
  }
  if (rlen > inputlen - pos) {
    return 0;
  }
  rpos = pos;
  pos += rlen;

  /* Integer tag byte for S */
  if (pos == inputlen || input[pos] != 0x02) {
    return 0;
  }
  pos++;

  /* Integer length for S */
  if (pos == inputlen) {
    return 0;
  }
  lenbyte = input[pos++];
  if (lenbyte & 0x80) {
    lenbyte -= 0x80;
    if (pos + lenbyte > inputlen) {
      return 0;
    }
    while (lenbyte > 0 && input[pos] == 0) {
      pos++;
      lenbyte--;
    }
    if (lenbyte >= sizeof(size_t)) {
      return 0;
    }
    slen = 0;
    while (lenbyte > 0) {
      slen = (slen << 8) + input[pos];
      pos++;
      lenbyte--;
    }
  } else {
    slen = lenbyte;
  }
  if (slen > inputlen - pos) {
    return 0;
  }
  spos = pos;

  /* Ignore leading zeroes in R */
  while (rlen > 0 && input[rpos] == 0) {
    rlen--;
    rpos++;
  }
  /* Copy R value */
  if (rlen > 32) {
    overflow = 1;
  } else {
    std::memcpy(tmpsig + 32 - rlen, input + rpos, rlen);
  }

  /* Ignore leading zeroes in S */
  while (slen > 0 && input[spos] == 0) {
    slen--;
    spos++;
  }
  /* Copy S value */
  if (slen > 32) {
    overflow = 1;
  } else {
    std::memcpy(tmpsig + 64 - slen, input + spos, slen);
  }

  if (!overflow) {
    overflow = !secp256k1_ecdsa_signature_parse_compact(ctx, sig, tmpsig);
  }
  if (overflow) {
    /* Overwrite the result again with a correctly-parsed but invalid
       signature if parsing failed. */
    std::memset(tmpsig, 0, 64);
    secp256k1_ecdsa_signature_parse_compact(ctx, sig, tmpsig);
  }
  return 1;
}

PublicKey::PublicKey()
    : public_key(NULL)
    , context(nullptr)
{}

PublicKey::PublicKey(const std::vector<unsigned char> &public_key)
    : public_key(public_key) 
	, context(secp256k1_context_create(SECP256K1_CONTEXT_VERIFY))
{}

PublicKey::PublicKey(PublicKey &&rhs) 
{
    this->public_key.assign(std::begin(rhs.public_key),
                            std::end(rhs.public_key));
    this->context = rhs.context;

    rhs.public_key.clear();
    rhs.context = nullptr;
}

PublicKey::~PublicKey()
{
	if(this->context)
	{
		secp256k1_context_destroy(this->context);
		this->context = nullptr;
	}
}

// bool PublicKey::VerifySignature(const std::vector<unsigned char> &hash,
//               const std::vector<unsigned char> &sign) const
// {
// 	secp256k1_ecdsa_verify( this->context,
// 						              sign,
// 						              hash,
// 						              this->public_key);
// }

PublicKey& PublicKey::operator=(PublicKey &&rhs)
{
    this->public_key.assign(std::begin(rhs.public_key),
                        std::end(rhs.public_key));
    this->context = rhs.context;

    rhs.public_key.clear();
    rhs.context = nullptr; 

    return *this;
}

bool PublicKey::operator==(const PublicKey& other) const
{
	return this->public_key == other.public_key;
}

bool PublicKey::VerifySignature(const std::vector<unsigned char> &hash,
                    const std::vector<unsigned char> &signature_input) const 
{
    // Parse public key.
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_parse(this->context, &pubkey,
    								                this->public_key.data(),
                                  this->public_key.size())) 
    {
    return false;
    }

    // Parse signature.
    secp256k1_ecdsa_signature signature;
    if (!ecdsa_signature_parse_der_lax(this->context, &signature,
    									                 signature_input.data(),
                                     signature_input.size()))
    {
    return false;
    }

    /* libsecp256k1's ECDSA verification requires lower-S signatures, which have
    * not historically been enforced in Bitcoin, so normalize them first. */
    secp256k1_ecdsa_signature_normalize(this->context, &signature, &signature);
    return secp256k1_ecdsa_verify(this->context, &signature, hash.data(), &pubkey);
}

void PublicKey::Print() const
{
    std::cout << "Public Key Size: " << this->public_key.size() << std::endl;
    std::cout << "Public Key: ";
    for (int i = 0; i < this->public_key.size(); ++i)
    {
        std::cout << this->public_key[i];
    }
    std::cout << std::endl;
}

// template<class Archive>
// void PublicKey::serialize(Archive & ar, const unsigned int version)
// {
//     ar & this->public_key;
//     ar & this-> context;
// }
