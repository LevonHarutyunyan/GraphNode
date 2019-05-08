#ifndef _MEMPOOL_H__
#define _MEMPOOL_H__

#include <set>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "Transactions.h"

static const boost::asio::ip::tcp::endpoint mempool_endpoint(
			boost::asio::ip::address::from_string("127.0.0.1"), 55555);

class Mempool
{

private:

	std::set<Transaction> Transactions;

	boost::shared_ptr <boost::asio::io_service> ios;
	boost::shared_ptr <boost::asio::io_service::work> work;
	boost::shared_ptr <boost::asio::ip::tcp::acceptor> acceptor;

	boost::asio::ip::address ip_address;
	boost::asio::ip::tcp::endpoint endpoint;
	boost::shared_ptr<boost::asio::ip::tcp::socket> sock;

public:

	Mempool();

	void receive_transactions();
	void send_transactions();

};

#endif //_MEMPOOL_H__