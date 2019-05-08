#ifndef _TRANSACTION_H__
#define _TRANSACTION_H__

#include <vector>

class Transaction
{

private:

	uint transaction_id;
	unsigned char txn_hash;

	std::vector<TransactionInput> inputs;
	std::vector<TransactionOutput> outputs;

	vector<char> signature;

public:

	Transaction();

	std::string serialize_transaction() const;
	static Transaction deserialize_transaction(const std::string& transaction);
	
 	bool operator < (const Transaction & other) const;

 	uint64 fee() const;
 	std::string compute_txn_hash();

 	void sign(const PrivateKey& key);
};

#endif //_TRANSACTION_H__
