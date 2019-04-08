#include "GraphNode.h"

//Lifehacks, boiii
void read_callback(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	// std::cout << "bytes_transferred: " << bytes_transferred << ", error: " << error << std::endl;  
}
void write_callback(const boost::system::error_code& error, std::size_t bytes_transferred)
{
}
void run_io_service(boost::shared_ptr<boost::asio::io_service>& io_service)
{
	io_service->run();
}

void OnConnected(const boost::system::error_code & ec, boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
}
void OnConnect(const boost::system::error_code & ec)
{
}

GraphNode::GraphNode(int port)
		: port(port)
{
	ip_address = boost::asio::ip::address::from_string("127.0.0.1");
	ios.reset(new boost::asio::io_service);
	work.reset(new boost::asio::io_service::work(*ios));

	boost::thread io_service_thread = boost::thread(boost::bind(&run_io_service, ios));

	acceptor.reset(new boost::asio::ip::tcp::acceptor(*ios));
	
	this->endpoint = boost::asio::ip::tcp::endpoint(ip_address, port);
	
	sock.reset(new boost::asio::ip::tcp::socket(*ios));

	try
	{
		acceptor->open(endpoint.protocol());
		acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		acceptor->bind(endpoint);
		acceptor->listen(boost::asio::socket_base::max_connections);

		std::cout << "Listnening on: " << endpoint << std::endl;
	}
	catch(std::exception & ex)
	{
		// std::cout << boost::this_thread::get_id() << " connecion exception " << ex.what() << std::endl;
	}
}

/*************************************************/

void GraphNode::make_connection(const boost::asio::ip::tcp::endpoint& ep)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered make_connection with: " 
			<< boost::lexical_cast<std::string>(ep) << std::endl;

	/*Adding endpoint to this->connections*/
	if(this->connections.find(ep) != this->connections.end())
		return;

	if(std::find(this->friend_list.begin(), this->friend_list.end(), ep) != this->friend_list.end())
		return;


	boost::shared_ptr<boost::asio::ip::tcp::socket> client_socket (
			new boost::asio::ip::tcp::socket(*ios, this->endpoint.protocol()));

	this->connections.insert({ep, client_socket});
	
	boost::asio::ip::tcp::endpoint ep_to_bind(ip_address, ++port);
	this->connections.at(ep)->bind(ep_to_bind);

	/*Connecting to endpoint*/
	boost::system::error_code e_c;
	do
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		this->connections.at(ep)->async_connect(ep, OnConnect);
	}
	while(e_c.value() != boost::system::errc::success);
	
	/*Sending a message*/
	std::string message;
	if(this->friend_list.size() < 6 && this->port != first_endpoint.port())
	{
		// std::cout << "Wants to be friends with " << boost::lexical_cast<std::string>(ep) << std::endl;
		message = "Be friends?:";
		message += std::to_string(this->endpoint.port());
		send(ep, message);
	}
	
}

void GraphNode::new_make_connection(const boost::asio::ip::tcp::endpoint& ep, int remote_port)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered new_make_connection with: " 
			<< boost::lexical_cast<std::string>(ep) << std::endl;
	
	/*Checking if already friends*/
	boost::asio::ip::tcp::endpoint remote_ep(ep.address(), remote_port);
	std::string message = (this->friend_list.size() < 6 ? "Yes" : "No");
	    	// std::cout << "Current ep: " << boost::lexical_cast<std::string>(remote_ep) << std::endl;
	for (auto& it: client_endpoints)
	{
    	// std::cout << "it->first: " << boost::lexical_cast<std::string>(it.first) << std::endl;
    	// std::cout << "it->second: " << boost::lexical_cast<std::string>(it.second) << std::endl;
	    if (it.second == remote_ep)
	    {
	    	message = "No";
	    }
	}
	
	send(ep, message);
}

/*************************************************/

void GraphNode::ask_for_friends(const boost::asio::ip::tcp::endpoint& ep)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered ask_for_friends with: " 
			<< boost::lexical_cast<std::string>(ep) << std::endl;
	
	std::string message{"Send friends"};

	send(ep, message);
}
	
void GraphNode::new_ask_for_friends(const boost::asio::ip::tcp::endpoint& ep)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered new_ask_for_friends with: "
			<< boost::lexical_cast<std::string>(ep)  << std::endl;

	boost::system::error_code ec;

	send(ep, this->pack_friends());
}	

/*************************************************/

void GraphNode::connect_friend(const boost::asio::ip::tcp::endpoint& ep)
{	
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered connect_friend with: "
			<< boost::lexical_cast<std::string>(ep)  << std::endl;

	std::string message;

	if(this->friend_list.size() != 6)
	{
		this->friend_list.push_back(ep);
		client_endpoints.insert({ep, ep});

		message = "Added to friends:";
		message += boost::lexical_cast<std::string>(this->endpoint.port());
		
		send(ep, message);
	}
}

void GraphNode::new_connect_friend(const boost::asio::ip::tcp::endpoint& ep, int port)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered new_connect_friend with: "
			<< boost::lexical_cast<std::string>(ep) << std::endl;

	boost::asio::ip::tcp::endpoint original(ep.address(), port);
	
	std::vector<boost::asio::ip::tcp::endpoint>::iterator it = 
		std::find(this->friend_list.begin(), this->friend_list.end(), original);
	if(it != this->friend_list.end())
	{
		send(ep, "Already friends");
		std::cout << "Already friends with: " 
				<< boost::lexical_cast<std::string>(original) << std::endl;
		return; 
	}

	std::string message;
	if(this->friend_list.size() < 6)
	{
		this->friend_list.push_back(original);
		client_endpoints.insert({ep, original});
		message = "Confirmed";
		send(ep, message);
	}
	else
	{
		message = "Not confirmed(friends limit)";
		send(ep, message);
		
		boost::system::error_code ec;
		this->connections.at(ep)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		this->connections.at(ep)->close();

		std::cout << "Manual disconnect (friends limit): " 
				<< boost::lexical_cast<std::string>(ep) << std::endl;

		this->connections.erase(ep);
		this->client_endpoints.erase(ep);
	}

}

/*************************************************/

void GraphNode::disconnect_friend(const boost::asio::ip::tcp::endpoint& connection_ep, const boost::asio::ip::tcp::endpoint& original_ep)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered disconnect_friend with: "
			<< boost::lexical_cast<std::string>(connection_ep) << std::endl;
	
	friend_list.erase(std::remove(friend_list.begin(), friend_list.end(), original_ep), friend_list.end());
	
	std::string message{"Removed:"};
				message += std::to_string(this->endpoint.port());
	send(connection_ep, message);

	boost::system::error_code ec;
	this->connections.at(connection_ep)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	this->connections.at(connection_ep)->close();
	std::cout << "Connection->socket->shutdown: " 
			<< boost::lexical_cast<std::string>(connection_ep) << std::endl;

	this->connections.erase(connection_ep);
	this->client_endpoints.erase(connection_ep);
}

void GraphNode::new_disconnect_friend(const boost::asio::ip::tcp::endpoint& ep, int friend_port)
{
	boost::lock_guard<boost::mutex> lock(m);
	std::cout << "Entered new_disconnect_friend with: "
			<< boost::lexical_cast<std::string>(ep) << std::endl;

	boost::asio::ip::tcp::endpoint removed(ep.address(), friend_port);
	std::vector<boost::asio::ip::tcp::endpoint>::iterator it = 
		std::find(this->friend_list.begin(), this->friend_list.end(), removed);

	// std::cout << "Got disconnected from: " << boost::lexical_cast<std::string>(removed) << std::endl;
	if(it != this->friend_list.end())
	{
		std::cout << "Disconnect from friend: " << boost::lexical_cast<std::string>(removed) << std::endl;
		friend_list.erase(std::remove(friend_list.begin(), friend_list.end(), removed), friend_list.end());
	}
	if(this->connections.find(ep) != this->connections.end())
	{
		boost::system::error_code ec;
		this->connections.at(ep)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		this->connections.at(ep)->close();

		std::cout << "Connection->socket->shutdown" 
				<< boost::lexical_cast<std::string>(ep) << std::endl;

		this->connections.erase(ep);
		this->client_endpoints.erase(ep);
	}
}

/*************************************************/

void GraphNode::process_new_message()
{
	for(;;)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		boost::system::error_code ec;

		for(auto& ep: this->connections)
		{
			if(ep.second->available())
			{
				std::vector<char> v(1024);
				boost::system::error_code ignored_error;
				int read = ep.second->receive(boost::asio::buffer(v.data(), v.size()));//, read_callback);

				std::string message(v.begin(), v.begin() + read);
				this->message_queue.push({ep.first, message});
				// std::cout << "Added to queue: <" << message << ">" << std::endl;
			}
		}
		// std::cout << "Message queue size " << this->message_queue.size() << std::endl;
		while(this->message_queue.size() != 0)
		{
			std::string msg = message_queue.front().second;
			std::cout << "New message: <" << msg
					<< "> from " << boost::lexical_cast<std::string>(message_queue.front().first) << std::endl;
			//case: msg = "Be friends?:port"
			if(msg[0] == 'B')
			{
				int remote_port = get_friend_port(msg);
				new_make_connection(message_queue.front().first, remote_port);
			}
			else if(msg == "Yes")
			{
				connect_friend(message_queue.front().first);
			}
			else if(msg == "No")
			{
				ask_for_friends(message_queue.front().first);
			}
			else if(msg == "Send friends")
			{
				boost::system::error_code ec;
				new_ask_for_friends(message_queue.front().first);
				if(this->client_endpoints.find(message_queue.front().first) == this->client_endpoints.end())
				{
					this->connections.at(message_queue.front().first)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
					this->connections.at(message_queue.front().first)->close();

					this->connections.erase(message_queue.front().first);
					std::cout << "In <new_ask_for_friends> erased: " 
							<< boost::lexical_cast<std::string>(message_queue.front().first) << std::endl; 
				}
			}
			else if(msg == "Not confirmed(friends limit)")
			{
				disconnect_friend(message_queue.front().first,
								this->client_endpoints.at(message_queue.front().first));
				std::cout << "Manual disconnect from: "
						<< boost::lexical_cast<std::string>(message_queue.front().first) << std::endl;
			}
			//case msg = "Added to friends:port"
			else if(msg[0] == 'A')
			{
				int port = get_friend_port(msg);
				// std::cout << "Port of a friend " << port << std::endl;
				new_connect_friend(message_queue.front().first, port);
			}
			//case msg = "<friend_ep1;friend_ep2;friend_ep3.."
			else if(msg[0] == '<')
			{
				boost::system::error_code ec;
				std::cout << "Unpacking friends of " 
						<< boost::lexical_cast<std::string>(message_queue.front().first) << std::endl;

				std::cout << "In <else if(msg[0] == '<')> erased: " 
						<< boost::lexical_cast<std::string>(message_queue.front().first); 
				

				this->connections.at(message_queue.front().first)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
				this->connections.at(message_queue.front().first)->close();
				
				this->connections.erase(message_queue.front().first);
				
				unpack_friends(msg);
				
			}
			//case msg = "Removed:port"
			else if(msg[0] == 'R')
			{
				int friend_port = get_friend_port(msg);
				new_disconnect_friend(message_queue.front().first, friend_port);
			}
			//for broken messages
			else
			{
				std::cout << "Undefined message: <" << msg << ">" << std::endl;
			}

			message_queue.pop();
		}

		friends();
		
		std::cout << "friend_list.size(): " << this->friend_list.size() << std::endl;
		std::cout << "connections.size(): " << this->connections.size() << std::endl;
		std::cout << "client_endpoints.size(): " << this->client_endpoints.size() << std::endl;
		
		used_connections();
	}
}

void GraphNode::run_connection_process()
{
	boost::thread read_thread(boost::bind(&GraphNode::process_new_message, this));
	boost::thread disconnection_thread(boost::bind(&GraphNode::run_disconnection_process, this));
	boost::thread supporting_thread(boost::bind(&GraphNode::run_supporting_process, this));
	boost::asio::spawn(*ios, boost::bind(&GraphNode::process_new_accept, this, _1));

	if(this->port != first_endpoint.port())
	{
		for(;;)
		{
			if(pending_endpoints.empty() 
				|| std::find(this->friend_list.begin(), this->friend_list.end(), first_endpoint) != this->friend_list.end())
			{
				this->pending_endpoints.push(first_endpoint);
				// std::cout << "Connecting to that piece of shit" << std::endl;
			}
			while (this->friend_list.size() != 6)
			{
				if(pending_endpoints.empty())
				{
					// std::cout << "No pending endpoints" << std::endl;
					break;
				}
				boost::asio::ip::tcp::endpoint ep = this->pending_endpoints.front();
				this->pending_endpoints.pop();

				if(this->endpoint != ep)
				{
					std::cout << "Now connecting to: " 
							<<boost::lexical_cast<std::string>(ep) << std::endl;
					boost::this_thread::sleep(boost::posix_time::seconds(1));
					make_connection(ep);
				}
				// std::cout << this->endpoint << " Done [New make connection] " << ep << std::endl;
			}
		} 
	}

	supporting_thread.join();
	disconnection_thread.join();
	read_thread.join();
}

void GraphNode::process_new_accept(boost::asio::yield_context yield)
{
	std::cout << "Entering process_new_accept" << std::endl;
	boost::system::error_code ec;

	for(;;)
	{
    	boost::shared_ptr<boost::asio::ip::tcp::socket> sock(
    		new boost::asio::ip::tcp::socket(*ios));

		this->acceptor->async_accept(*sock, yield);
		
		// std::cout << "Processing accept: " 
				// << boost::lexical_cast<std::string>(sock->remote_endpoint()) << std::endl;

		this->connections.insert({sock->remote_endpoint(), sock});
		std::cout << "Client accepted: " 
				<< boost::lexical_cast<std::string>(sock->remote_endpoint()) << std::endl;
	}
}

void GraphNode::run_disconnection_process()
{
	srand(time(0));

	for(;;)
	{
		if(this->friend_list.size() == 6)
		{
			if(!this->client_endpoints.empty() && this->client_endpoints.size() != 0)
			{
				auto it = this->client_endpoints.begin();
				std::advance(it, rand() % this->client_endpoints.size());
				disconnect_friend(it->first, it->second);
			}

			if(this->endpoint == first_endpoint)
			{
				boost::this_thread::sleep(boost::posix_time::seconds(rand() % 10 + 10));
			}
			else
			{
				boost::this_thread::sleep(boost::posix_time::seconds(rand() % 10 + 30));
			}
		}
		
		boost::this_thread::sleep(boost::posix_time::seconds(rand() % 10 + 5));
	}
}

void GraphNode::run_supporting_process()
{
	if(this->endpoint == first_endpoint)
		return;
	
	srand(time(0));

	std::map<boost::asio::ip::tcp::endpoint, int> unused_endpoints;//stagnant ones
	
	int count = 0;
	int friend_size_count = 0;
	int previous_friend_size = this->friend_list.size();

	for(;;)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(7));
		std::cout << "Making lap in <run_supporting_process> " << count++ << std::endl;

		/*Pushing not friendly endpoints into unused_endpoints*/
		for(auto& ep: this->connections)
		{
			if(this->client_endpoints.find(ep.first) == this->client_endpoints.end())
			{
				unused_endpoints.insert({ep.first, 1});

				std::cout << "New not used: " << boost::lexical_cast<std::string>(ep.first)
						<< "--" << std::to_string(unused_endpoints.at(ep.first)) << std::endl;
			}
		}
		
		/*Checking if still have connections and incrementing(if have)*/
		std::map<boost::asio::ip::tcp::endpoint, int> copy_unused_endpoints = unused_endpoints;;//stagnant ones
		int endpoints_count = 0;
		for(auto& unused_ep: copy_unused_endpoints)
		{
			std::cout << "Current unused_endpoints.size(): " << unused_endpoints.size() << std::endl; 
			std::cout << "Looping through " << ++endpoints_count << " element" << std::endl;

			std::cout << "Not used: " << boost::lexical_cast<std::string>(unused_ep.first)
						<< "--" << unused_ep.second << std::endl;

			++unused_ep.second;
			
			if(this->connections.find(unused_ep.first) == this->connections.end() 
				|| this->client_endpoints.find(unused_ep.first) != this->client_endpoints.end())
			{
				
				unused_endpoints.erase(unused_ep.first);
				std::cout << "Deleting from unused_endpoints: " 
							<< boost::lexical_cast<std::string>(unused_ep.first) << std::endl;
				continue;
			}
			
			if(unused_ep.second >= 5 && this->client_endpoints.find(unused_ep.first) == this->client_endpoints.end())
			{
				boost::system::error_code ec;
				this->connections.at(unused_ep.first)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
				this->connections.at(unused_ep.first)->close();
		
				std::cout << " Manualy deleting unused connection: " 
						<< boost::lexical_cast<std::string>(unused_ep.first) << std::endl;

				this->connections.erase(unused_ep.first);

				unused_endpoints.erase(unused_ep.first);
			}
		}

		/*Asking for friends if friend_size remains same for too long*/
		if(this->friend_list.size() == previous_friend_size && this->friend_list.size() != 0)
		{
			++friend_size_count;
			if(friend_size_count == 5)
			{
				boost::asio::ip::tcp::endpoint random_friend = this->friend_list[rand() % this->friend_list.size()];
				
				std::cout << "Asking randomly chosen friend: " 
							<< boost::lexical_cast<std::string>(this->client_endpoints.at(random_friend)) << std::endl;
				
				// std::cout << "Manual ask_for_friends: " 
				// 			<< boost::lexical_cast<std::string>(this->client_endpoints.at(random_friend)) << std::endl;
				ask_for_friends(random_friend);
				friend_size_count = 0;
			}
		}
		else
		{
			std::cout << "Previous_size: " << previous_friend_size << std::endl;
			std::cout << "Current friend_size: " << this->friend_list.size() << std::endl;

			previous_friend_size = this->friend_list.size();
		}
	}
}

//-----------------------------------------------------//

std::string GraphNode::pack_friends()
{
	std::string result{'<'};
	
	for(int i = 0; i < this->friend_list.size(); ++i)
	{
		// result += this->friend_list[i].address().to_string() 
				// + std::to_string(this->friend_list[i].port()) + ';';
		result += boost::lexical_cast<std::string>(this->friend_list[i]) + ';';
	}

	// std::cout << "Packed successefuly" << std::endl;

	return result;
}

void GraphNode::unpack_friends(std::string& endpoints)
{
	std::vector<boost::asio::ip::tcp::endpoint> sended_endpoints;

	std::string port;
	std::string address;

	while(endpoints[0] != '<')
	{
		endpoints.erase(endpoints.begin());
	}
	endpoints.erase(endpoints.begin());
	
	while(endpoints.size())
	{
		while(endpoints[0] != ':')
		{
			address += endpoints[0];
			endpoints.erase(endpoints.begin());
		}
		endpoints.erase(endpoints.begin());
		while(endpoints[0] != ';')
		{
			port += endpoints[0];
			endpoints.erase(endpoints.begin());
		}
		endpoints.erase(endpoints.begin());	

		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(address), std::stoi(port));
		sended_endpoints.push_back(ep);
		// std::cout << "Endpoint added to sended_endpoints:" 
					// << boost::lexical_cast<std::string>(ep) << std::endl;

		address.clear();
		port.clear();
	}

	for(int i = 0; i < sended_endpoints.size(); ++i)
	{
		this->pending_endpoints.push(sended_endpoints[i]);
	}
	// std::cout << "Unpacked successefuly" << std::endl;
}

/*************************************************/

void GraphNode::send(const boost::asio::ip::tcp::endpoint& ep, std::string message)
{
	boost::system::error_code e_c;
	std::cout << "Sending message = " << message 
				<< " to " << boost::lexical_cast<std::string>(ep) << std::endl;
	this->connections.at(ep)->write_some(boost::asio::buffer(message.data(), message.size()), e_c);
}

void GraphNode::friends()
{
	std::cout << "********FRIENDS*************" << std::endl;
	std::cout << "Me: " << boost::lexical_cast<std::string>(this->endpoint) << std::endl;
	for(int i = 0; i < this->friend_list.size(); ++i)
	{
		std::cout << "Friend " << i << ": " 
				<< boost::lexical_cast<std::string>(this->friend_list[i]) <<std::endl;
	}
	std::cout << "*****************************" << std::endl;
}

void GraphNode::used_connections()
{
	std::cout << "/-------------Connections-------------------/" << std::endl;
	for(auto& it: this->connections)
	{
		std::cout << boost::lexical_cast<std::string>(it.first) << " ";
		// if(it.second->is_open())
		// {
		// 	std::cout << it.second->remote_endpoint().port() << ";";
		// }
		std::cout << std::endl;
	}
	std::cout << "/--------------------------------------------/" << std::endl;

}
int GraphNode::get_friend_port(std::string& message)
{
	// std::cout << "Entered to get_friend_port()" << std::endl;

	int port = 0;
	while(message[0] != ':')
	{
		message.erase(message.begin());
	}

	message.erase(message.begin());
	
	while(message.size() != 0)
	{
		port = port * 10 + message[0] - '0';
		message.erase(message.begin());
	}	

	return port;
}
