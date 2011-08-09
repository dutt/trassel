Threads
=======

What is it?
-----------
Trassel is a threading library based on sending messages between client instances.

Messages are produced by MessageClients and are then handled by either Channel or DirectedChannel. These can run a number of worker threads, 
the more worker threads the more messages can be handled at the same time but then you may need more synchronisation between your actors.

A message can contain a plain old data, bool, int and so on, or anything in a void*-struct.

Status
------

Currently trassel is fairly stable, I've tried to keep the feature and line count down to minimise the number of bugs that way.

Probably no major rewrites or changes coming up.

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

Default operating mode is to send asynchronously and return directly, not waiting for the message to be handled or replied to.

Groups
------

I've added support for groups. You can attach a number of message clients to a group and send a message to the group. There are two forwarding modes:

* FIFO - The first client that checks for a new message gets it.
* Broadcast - All clients get all messages.

Other
-----
Each MessageClient is automatically assigned an ID and this is how the directed channel keep tracks on which message should be received by
which client. If you want to see which ID your client has you can retrieve it through getID().

The type Message is a typedef for std::shared_ptr<MessageS> so memory is handled automatically but you access members with 
operator "->" instead of the usual operator ".".

You can inherit tasks from either MessageClient or Task. 
Inheritance from Task gives a cleaner interface but each message takes a virtual method call to handle, inheritance from MessageClient on
the other hand means slightly messier code but on the other hand no virtual call.

Examples
--------
For a more complete example see the code in the foldder test.

Send async message, continue working directly:
    
	BoolMsg bmsg;
	bmsg.value = false;
    sendMessage(bmsg, mReceiver);

Send message - wait until handled:
    
	BoolMsg bmsg;
	bmsg.value = true;
	sendMessage(bmsg, mReceiver, false);

Synd message and wait for reply. This assumes the reply is a BoolMsg, in reality you'd check what kind of message you got:

    StringMsg smsg;
	smsg.value = "muffins";
    Message reply = sendMessage(bmsg, mReceiver, false, true);
    if(reply != 0) {
    	cout  <<(int)getID() <<": Got reply, was our message handled successfully? " (<<reply->boolMsg.value?"yes":"no") <<endl;
    }

TOOD
----

* Shutdown - Some way to wait for all current messages to be handled and then shut down.
* Suspend/Resume - will probably require implementation of system(not meant for the tasks)-messages.
* Other - Set timeout for specific send():s
