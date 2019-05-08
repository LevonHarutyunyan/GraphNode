#ifndef _PRIVATE_KEY_H__
#define _PRIVATE_KEY_H__

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "utils.h"
#include "../include/secp256k1.h"

#include "PublicKey.h"


class PrivateKey 
{

public:

  //main constructor, randomly generates private key
  PrivateKey();

  //Additional constructors from vector and string. Added for no reason I guess
  PrivateKey(const std::vector<unsigned char> &priv_key_data);
  PrivateKey(const std::string &priv_key_data);

  PrivateKey(const PrivateKey &rhs) = delete;
  PrivateKey &operator=(const PrivateKey &rhs) = delete;

  //Needed in HDWallet::child constructor
  PrivateKey(PrivateKey &&rhs);
  PrivateKey &operator=(PrivateKey &&rhs);

  ~PrivateKey();

  //Gor Google Test
  bool operator==(const PrivateKey& other) const;

  //For debug
  inline const std::vector<unsigned char>& get_private_key() const 
  {
    return this->private_key;
  }

  inline const std::vector<unsigned char>& get_public_key() const 
  { 
    return this->public_key;
  }

  //Returns true if PrivateKey is correct, used in constructor
  bool VerifyKey() const;


  PublicKey SetPublicKey() const;

  std::tuple<std::vector<unsigned char>, bool> Sign(
      const std::vector<unsigned char> &hash) const;

  static std::vector<unsigned char> Hash(const std::vector<unsigned char>& data);
  static std::string Hash(const std::string& data);

  void Print() const;

  // This class clearly doesn't need to have serialize functional, 
  // but still don't wan't to delete

  // friend class boost::serialization::access;
  // friend std::ostream & operator << (std::ostream &os, const PrivateKey& privatekey);
  
  // template <typename Archive>
  // void serialize(Archive& ar, const unsigned int version) const;

  // void Serialize(const PrivateKey &privatekey, const std::string& buffer);
  // void Deserialize(const PrivateKey &privatekey, const std::string& buffer);

private:
  void PrintKey(const std::vector<unsigned char>& key) const;
  bool CalculatePublicKey(bool compressed);

private:
  secp256k1_context *context = nullptr;
  std::vector<unsigned char> private_key;
  std::vector<unsigned char> public_key;
};


#endif // _PRIVATE_KEY_H__
