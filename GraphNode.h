#ifndef _GRAPHNODE_H__
#define _GRAPHNODE_H__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <cctype>
#include <algorithm>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define BOOST_COROUTINES_NO_DEPRECATION_WARNING //ignores coroutine warning(doesn't work at all though)

//first node's endpoint. First node can't conenct or send, only accepts and answers
static const boost::asio::ip::tcp::endpoint first_endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 12345);

/**\brief[GraphNode] Class for quick network establishment and later maintenance 
 *param[port] used for making endpoints for acceptor and clients
 *param[ip_address] used for making this->endpoint
 *param[sock] socket for listening 
 *param[pending_endpoints] collects all incoming requests 
 *param[connections] contains client_endpoint and socket for communication with it
 *param[message_queue] collects all incoming messages
 *param[friend_list] contains all original client endpoints that considered as friends 
 *param[client_endpoints] contains client_endpoint for connecting and original one
 */
class GraphNode
{
private:
	boost::mutex m;

	boost::shared_ptr <boost::asio::io_service> ios;
	boost::shared_ptr <boost::asio::io_service::work> work;
	boost::shared_ptr <boost::asio::ip::tcp::acceptor> acceptor;

	int port;
	boost::asio::ip::address ip_address;
	boost::asio::ip::tcp::endpoint endpoint;
	boost::shared_ptr<boost::asio::ip::tcp::socket> sock;

	std::queue<boost::asio::ip::tcp::endpoint> pending_endpoints;
	std::map<boost::asio::ip::tcp::endpoint, boost::shared_ptr<boost::asio::ip::tcp::socket>> connections;
	std::queue<std::pair<boost::asio::ip::tcp::endpoint, std::string>> message_queue;
	
	std::vector<boost::asio::ip::tcp::endpoint> friend_list;
	std::map<boost::asio::ip::tcp::endpoint, boost::asio::ip::tcp::endpoint> client_endpoints;

public:

	GraphNode(int port);

	/*brief[send] just sends messages to particular ep
	 */
	void send(const boost::asio::ip::tcp::endpoint& ep, std::string message);
	
	/*brief[friends] prints all friends (used for debugging)
	*/
	void friends();
	
	/*brief[used_conenctions] prints all connections (used for debugging)
	*/
	void used_connections();

	/*brief[get_friend_port] gets original port of client from message
	*/
	int get_friend_port(std::string& message);

	/*brief[pack_friends] creates string from friends (<friend1;friend2..friendn;)
	*/
	std::string pack_friends();

	/*brief[unpack_friends] creates endpoints and pushes them to pending_endpoints
	*/
	void unpack_friends(std::string& endpoints);


	/*brief[make_connection] establishes a connection and asks for friendship to ep
	*/
	void make_connection(const boost::asio::ip::tcp::endpoint& ep);

	/*brief[new_make_connection] answers to previous request (Yes or No)
	 *param[remote_port] port of original client endpoint
	*/	
	void new_make_connection(const boost::asio::ip::tcp::endpoint& ep, int remote_port);


	/*brief[ask_for_friends] asks client to give his friend_list
	*/
	void ask_for_friends(const boost::asio::ip::tcp::endpoint& ep);

	/*brief[new_ask_for_friends] sends pack_friends() to client
	*/
	void new_ask_for_friends(const boost::asio::ip::tcp::endpoint& ep);


	/*brief[connect_friend] adds client to friend_list and tells him about it
	*/
	void connect_friend(const boost::asio::ip::tcp::endpoint& ep);
	
	/*brief[new_connect_friend] adds client to friend_list if can and tells him about success or failure
	 *param[port] port of original client endpoint, used for adding client to friend_list
	*/
	void new_connect_friend(const boost::asio::ip::tcp::endpoint& ep, int port);


	/*brief[disconnect_friend] removes client from connections, friends, client_endpoints 
	 *						   tells him about it then closes conenction socket
	*/
	void disconnect_friend(const boost::asio::ip::tcp::endpoint& connection_ep, const boost::asio::ip::tcp::endpoint& original_ep);
	
	/*brief[new_disconnect_friend] removes client from all containers and closes connection socket
	*/
	void new_disconnect_friend(const boost::asio::ip::tcp::endpoint& ep, int friend_port);


	/**
		\brief [run_connection_process] creates threads for other processes 
	 * 								and connects to all pending connections
	*/
	void run_connection_process();

	/*brief[process_new_accept] maintains all incoming connection requests and accepts them
	*/
	void process_new_accept(boost::asio::yield_context yield);
	
	/*brief[process_new_message] maintains all incoming messages and answers to them
	*/
	void process_new_message();
	
	/*brief[run_disconnection_process] from time to time disconnects random friend
	 * 								   used to prevent states when new node can't connect to anybody
	*/
	void run_disconnection_process();

	/*brief[run_supporting_process] from time to time searches for unused conenctions and removes them
	 * 								and asks for someone to send friends if has not enough( 1 or 2 )
	*/
	void run_supporting_process(); 
};

#endif //_GRAPHNODE_H__
