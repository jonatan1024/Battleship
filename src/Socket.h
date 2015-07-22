#pragma once
#include <string>
using namespace std;

/// Socket error exception
class CSocketError {};

/// Socket, abstraction layer for easier communication.
class CSocket {
public:
	CSocket();
	~CSocket();
	/// @param socket Socket's FD.
	void SetSocket(int socket);
	/// See SetSocket.
	const CSocket & operator=(int socket);
	/// @return True when valid (FD != -1).
	operator bool() const;
	/// Enables/Disables nonblocking mode.
	void SetBlocking(bool blocking) const;
	/// @param msg Message
	/// @throws CSocketError On connection failure.
	void SendMessage(const string & msg) const;
	/// @param msg Read message.
	/// @throws CSocketError On connection failure.
	/// @return True when succesfuly read a message.
	bool ReadMessage(string & msg) const;
	/// Closes the socket so it can be reused.
	void Close();
protected:
	/// Socket's FD.
	int m_socket;
};

