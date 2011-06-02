Threads
=======

What is it?
-----------
Trassel is a threading library based on sending messages between actors.

Messages are produced by MessageClients and are then handled by either Channel or DirectedChannel. These can run a number of worker threads, 
the more worker threads the more messages can be handled at the same time but then you may need more synchronisation between your actors.

A message can contain a plain old data, bool, int and so on, or anything in a void*-struct.

Usage
-----
You have either nondirected messages, you just send them and they are received on a first-come-first-served basis, or you can have
directed messages. Then you address the messages to the receiver and they will wait in the channel until the receiver checks for a message.

Before you do anything you need call setup() on your channel and when you're done shutdown().
 
Messages can be sent in two ways:

* Asynchronously: Send it and go on doing your thing.
* Synchronously: The send operation doesn't return until the receiver has handled the message.
	If you do send it synchronously you can wait for the reply which is returned from the send-function.
	The receiver in turn may wait for a new reply from you and so on.

Examples
--------
For a more complete example see src/main.cpp.

Send async message, continue working directly:
    
	BoolMsg bmsg;
	bmsg.value = false;
    cout <<(int)getID() <<": Sending false bool" <<endl;
    sendMessage(bmsg, mReceiver);

Send message - wait until handled:
    
	BoolMsg bmsg;
	bmsg.value = true;
	cout <<(int)getID() <<": Sending true bool" <<endl;
	sendMessage(bmsg, mReceiver, false);

Synd message and wait for reply. This assumes the reply is a BoolMsg, in reality you'd check what kind of message you got:

    StringMsg smsg;
	smsg.value = "muffins";
	cout <<(int)getID() <<": Sending string" <<endl;
    Message* reply = sendMessage(bmsg, mReceiver, false, true);
    if(reply != 0) {
    	cout  <<(int)getID() <<": Got reply: " <<reply->boolMsg.value <<endl;
    }

