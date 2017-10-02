#include <SFML/Network.hpp>
#include <thread>
#include "Event.hpp"
#include "Pub_Sub.h"
#include <list>

struct CustomPacket {

	char data[1024];
	int position = 0;
	int size = 12;

};

class NetworkInput : public VrEventPublisher {
public:
	NetworkInput();

	void listen_for_clients(int port);
	void stop_listening_for_clients();

	void recieve_from_clients();
	void stop_recieving_from_clients();

	void generate_events();

private:

	std::vector<sf::TcpSocket*> client_sockets;
	sf::SocketSelector socket_selector;

	std::thread *client_listener_thread;
	std::thread *client_reciever_thread;

	void threaded_client_listener(int port);
	void threaded_client_reciever();

	sf::TcpListener listener;
	bool listening = true;

};
