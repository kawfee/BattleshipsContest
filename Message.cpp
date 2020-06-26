/**
 * Message.cpp: a primitive message object.
 * @author Stefan Brandle, February 2004
 */

#ifndef MESSAGE_CPP        // Double inclusion protection
#define MESSAGE_CPP

#include "Message.h"

using namespace std;

Message::Message( char messageType ) {
    this->messageType = messageType;
    this->row = -1;
    this->col = -1;
    this->str = "";
    this->dir = Horizontal;
    this->length = -1;
}

/**
 * Initializes the object message type to the specified values.
 * @param messageType This should be one of the defined 
 *     "final static char" values in the Defines class.
 * @param row The row related to the message.
 * @param col The col related to the message.
 * @param str The string message related to the message.
 */
Message::Message( char messageType, int row, int col, string str ) {
    setMessage( messageType, row, col, str, None, 0 );
}

/**
 * Initializes the object message type to the specified values.
 * @param messageType This should be one of the defined 
 *     "final static char" values in the Defines class.
 * @param row The row related to the message.
 * @param col The col related to the message.
 * @param str The string message related to the message.
 * @param dir The direction related to the message.
 */
Message::Message( char messageType, int row, int col, string str, Direction dir, int length ) {
    setMessage( messageType, row, col, str, dir, length );
}

/**
 * Sets the message data to the specified values.
 * @param messageType This should be one of the defined 
 *     "final static char" values in the Defines class.
 * @param row The row related to the message.
 * @param col The col related to the message.
 * @param str The string message related to the message.
 */
void Message::setMessage( char messageType, int row, int col, string str, Direction dir, int length ) {
    this->messageType = messageType;
    this->row = row;
    this->col = col;
    this->str = str;
    this->dir = dir;
    this->length = length;
}

/**
* Sets the message type.
 * @param messageType The message type.
 */
void Message::setMessageType( char messageType ) {
    this->messageType = messageType;
}

/**
 * Returns the message type.
 * @return messageType 
 */
char Message::getMessageType( ) {
    return this->messageType;
}

/**
 * Sets the message row.
 * @param row The row value for this message.
 */
void Message::setRow( int row ) {
    this->row = row;
}

/**
 * Returns the message row.
 * @return row The row value for this message. This is undefined for
 * non shot related messages.
 */
int Message::getRow( ) {
    return this->row;
}

/**
 * Sets the message col.
 * @param col The col value for this message.
 */
void Message::setCol( int col ) {
    this->col = col;
}

/**
 * Returns the message col.
 * @return col The col value for this message. This is undefined for
 * non shot related messages.
 */
int Message::getCol( ) {
    return this->col;
}

/**
 * Sets the message string.
 * @param str The string value for this message.
 */
void Message::setString( string str ) {
    this->str = str;
}

/**
 * Returns the message string.
 * @return String The string value for this message. This is an optional
 * field that may or may not be used for various messages at the
 * discretion of the participants.
 */
string Message::getString( ) {
    return this->str;
}

/**
 * Sets the Direction value.
 * @param dir The direction value for this message.
 */
void Message::setDirection( Direction dir ) {
    this->dir = dir;
}

/**
 * Returns the direction value.
 * @return Direction
 */
Direction Message::getDirection( ) {
    return this->dir;
}

/**
 * Sets the length value.
 * @param dir The length value for this message.
 */
void Message::setLength( int length ) {
    this->length = length;
}

/**
 * Returns the length value.
 * @return length
 */
int Message::getLength( ) {
    return this->length;
}

#endif
