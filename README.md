    Client-Server App for Message Management

* Functionality

The beginning of the app is given by the start of the server, to which a variable
number of TCP/UDP clients will be able to connect. The server allows new clients
to connect/disconnect at any moment.
Each client will be identified by the client_id with which it was started.
There can not be 2 clients with same ID at a given time during running.
A client ID will be a string of up to 10 ASCII characters.
The server is, of course, aware of the topics to which each client is subscribed.
Upon receiving a message from an UDP client, the serves ensures the message is
sent to all TCP clients who are subscribed to the topic from the message.
The subscription command has a SF (store & forward) parameter.
If it is set to 1, it means that a subscriber wants to make sure they don't miss
any messages sent on that topic. Thus, if a TCP client disconnects,
then returns, he will have to receive from the server all the messages that were
not sent to him already, even if they were sent/published at the time it was
disconnected. This only applies to subscriptions made with SF = 1, active before
the publication of the message. For SF = 0, messages sent when the client is
offline will be lost, but the client will remain a subscriber to that topic. 
