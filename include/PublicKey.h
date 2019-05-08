#ifndef _PUBLIC_KEY_H__
#define _PUBLIC_KEY_H__

#include <cstdint>
#include <vector>

#include <cstdint>
#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tuple/tuple.hpp>

#include "Serialize.h" //for debug
// #include "PrivateKey.h"
#include "utils.h"

#include "../include/secp256k1.h"


class PublicKey 
{
public:
    PublicKey();

    PublicKey(const PublicKey &rhs) = delete;
    PublicKey &operator=(const PublicKey &rhs) = delete;

    PublicKey(PublicKey &&rhs);
    PublicKey &operator=(PublicKey &&rhs);

    explicit PublicKey(const std::vector<unsigned char> &public_key);

    ~PublicKey();

    inline const std::vector<unsigned char> &get_public_key() const 
    {
        return this->public_key;
    }

    bool operator==(const PublicKey& other) const;

    bool VerifySignature(const std::vector<unsigned char> &hash,
                    const std::vector<unsigned char> &sign) const;

    void Print() const;

    friend class boost::serialization::access;


    template <typename Archive>
    friend void boost::serialization::serialize(Archive& ar, const unsigned int version);

    // can't serialize context object, so in deserialization process
    // need to create object with
    // PublicKey(const std::vector<unsigned char> &public_key) 
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & this->public_key;
    }

    // bool Serialize(const std::string& output) const;

private:
    std::vector<unsigned char> public_key;
    secp256k1_context_struct *context = nullptr;

};

#endif //_PUBLIC_KEY_H__