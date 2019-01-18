#include "../include-legacy/NetworkInput.h"

NetworkInput::NetworkInput() {

}

void NetworkInput::listen_for_clients(int port) {
	//listener.listen(port);
	client_listener_thread = new std::thread(&NetworkInput::threaded_client_listener, this, port);
}

void NetworkInput::stop_listening_for_clients() {
	listening = false;
	listener.close();
	socket_selector.clear();

	client_listener_thread->join();

}

void NetworkInput::recieve_from_clients()
{

}

void NetworkInput::dispatch_events()
{
	while (event_queue.size() != 0) {
		notify_subscribers(std::move(event_queue.front()));
		event_queue.pop_front();
	}
}

void NetworkInput::threaded_client_listener(int port) {

	listener.listen(port);
	socket_selector.add(listener);

	while (listening)
	{
		// Make the selector wait for data on any socket
		if (socket_selector.wait(sf::Time(sf::milliseconds(100))))
		{
			
			// Test the listener
			if (socket_selector.isReady(listener))
			{
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{
					// Add the new client to the clients list
					client_sockets.push_back(client);

					// Add the new client to the selector so that we will
					// be notified when he sends something
					socket_selector.add(*client);
				}
				else
				{
					// Error, we won't get a new connection, delete the socket
					delete client;
				}
			}
			else
			{
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::vector<sf::TcpSocket*>::iterator it = client_sockets.begin(); it != client_sockets.end(); ++it)
				{
					sf::TcpSocket& client = **it;
					if (socket_selector.isReady(client))
					{
						// Receive a message from the client
						char buffer[1024];

						std::vector<CustomPacket> packets;

						sf::TcpSocket::Status status;

						do {

							std::size_t received = 0;
							status = client.receive(buffer, 1024, received);

							while (received < 12) {
								std::size_t tack_on;
								status = client.receive(&buffer[received], 1024 - received, tack_on);
								received += tack_on;
							}


							int position = 0;
							while (position < received) {
								CustomPacket p;
								std::memcpy(p.data, &buffer[position], p.size);
								packets.push_back(p);
								position += p.size;
							}

							std::cout << "packet_count = " << packets.size() << std::endl;

							int left_over = 12 - static_cast<int>(position - received);
							std::memcpy(buffer, &buffer[received - left_over], left_over);

						} while (status != sf::TcpSocket::Status::Done);

						
						for (auto i: packets) {
		
							float x;
							float y;
							float z;

							memcpy(&x, &i.data, sizeof(x));
							memcpy(&y, &i.data[4], sizeof(y));
							memcpy(&z, &i.data[8], sizeof(z));

							event_queue.push_back(std::make_unique<vr::JoystickMoved>(vr::JoystickMoved(sf::Joystick::Axis::X, 0, x)));
							event_queue.push_back(std::make_unique<vr::JoystickMoved>(vr::JoystickMoved(sf::Joystick::Axis::Y, 0, y)));
							event_queue.push_back(std::make_unique<vr::JoystickMoved>(vr::JoystickMoved(sf::Joystick::Axis::Z, 0, z)));

							//std::cout << "X: " << x << " Y: " << y << " Z: " << z << std::endl;
						}
					}
				}
			}
		}
	}
}

void NetworkInput::threaded_client_reciever()
{

}
