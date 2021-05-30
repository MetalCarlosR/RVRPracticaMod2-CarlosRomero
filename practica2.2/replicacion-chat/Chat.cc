#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

using msgType = ChatMessage::MessageType;

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data

    char *tmp = _data;

    memcpy(tmp, &type, sizeof(uint8_t));

    tmp += sizeof(uint8_t);

    memcpy(tmp, nick.c_str(), 8 * sizeof(char));

    tmp += 8 * sizeof(char);

    memcpy(tmp, message.c_str(), 80 * sizeof(char));
}

int ChatMessage::from_bin(char *bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    char *tmp = _data;

    memcpy(&type, tmp, sizeof(uint8_t));

    tmp += sizeof(uint8_t);

    memcpy(&nick, tmp, 8 * sizeof(char));

    tmp += 8 * sizeof(char);

    memcpy(&message, tmp, 80 * sizeof(char));

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    Socket *client;
    ChatMessage msg;
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)

        socket.recv(msg, client);
        switch (msg.type)
        {
        case msgType::LOGIN:
        {
            std::unique_ptr<Socket> sockPtr(client);
            clients.push_back(std::move(sockPtr));

            std::string out = msg.nick + " has connected.";
            std::cout << out << std::endl;
            ChatMessage em("Server", out);
            em.type = ChatMessage::MESSAGE;

            sendMessage(em);
            break;
        }
        case msgType::MESSAGE:
        {
            sendMessage(msg, client);
            break;
        }
        case msgType::LOGOUT:
        {
            for(auto it = clients.begin(); it != clients.end(); it++){
                if((*it).get() == client){
                    clients.erase(it);
                    break;
                }
            }

            std::string out = msg.nick + " logged out.";
            std::cout << out << std::endl;
            ChatMessage em("Server", out);
            em.type = ChatMessage::MESSAGE;

            sendMessage(em); 
            break;
        }
        default:
            std::cout << "Unknown type of message recieved" << std::endl;
            break;
        }
    }
}

void ChatServer::sendMessage(ChatMessage msg, Socket *sender)
{
    for (auto const &c : clients)
    {
        if (c.get() != sender)
            socket.send(msg, *c.get());
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while (true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}