Trassel is a threading library based on sending messages between actors.

A message can contain a plain old data, bool, int and so on, or anything in a void*-struct.

Messages can be sent in three ways:
Asynchronously, send it and go on doing your thing.
Synchronously, the send operation doesn't return until the receiver has handled the message.
Wait for reply, send waits for the receiver to reply with a new message. 
	The receiver in turn may wait for a new reply and so on.
