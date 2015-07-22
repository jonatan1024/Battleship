#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

CSocket::CSocket() : m_socket(-1) {

}

CSocket::~CSocket() {
	Close();
}

void CSocket::Close() {
	if(m_socket != -1)
		close(m_socket);
	m_socket = -1;
}

void CSocket::SetSocket(int socket) {
	m_socket = socket;
}
const CSocket & CSocket::operator=(int socket) {
	m_socket = socket;
	return *this;
}
CSocket::operator bool() const {
	return m_socket != -1;
}

void CSocket::SetBlocking(bool blocking) const {
	if(m_socket == -1)
		return;
	int flags = fcntl(m_socket, F_GETFL, 0);
	if(blocking) {
		flags &= ~O_NONBLOCK;
	}
	else {
		flags |= O_NONBLOCK;
	}
	fcntl(m_socket, F_SETFL, flags);
}

void CSocket::SendMessage(const string & msg) const {
	if(m_socket == -1) {
		//bad socket
		throw CSocketError();
	}
	int len = msg.length();

	int numsend;
	numsend = send(m_socket, &len, sizeof(len), MSG_NOSIGNAL);
	if(numsend == -1 && errno != EWOULDBLOCK)
		throw CSocketError();

	numsend = send(m_socket, msg.c_str(), len, MSG_NOSIGNAL);
	if(numsend == -1 && errno != EWOULDBLOCK)
		throw CSocketError();
}

bool CSocket::ReadMessage(string & msg) const {
	if(m_socket == -1) {
		//bad socket
		throw CSocketError();
	}
	int msglen;
	int numread = recv(m_socket, &msglen, sizeof(msglen), MSG_NOSIGNAL);
	if(numread == -1 && errno == EWOULDBLOCK)
		return false;
	if(numread <= 0) {
		//bad socket
		throw CSocketError();
	}
	char * buffer = new char[msglen+1];
	do {
		numread = recv(m_socket, buffer, msglen, MSG_NOSIGNAL);
	} while(numread == -1 && errno == EWOULDBLOCK);
	if(numread != msglen) {
		//bad packet
		delete[] buffer;
		throw CSocketError();
	}
	buffer[numread] = '\0';
	msg = buffer;
	delete[] buffer;
	return true;
}
