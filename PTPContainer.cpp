/**
 * @file PTPContainer.cpp
 * 
 * @brief A class for handling PTP data structures
 * 
 * Used extensively by other classes as a base for holding PTP data.  This class
 * handles all of the data included in a PTP structure, as well as convenience
 * functions for extacting this data in a few different ways.
 */
 
#include <cstdlib>
#include <cstring>
#include <stdint.h>
 
#include "PTPContainer.hpp"
#include "libptp++.hpp"

namespace PTP {

/**
 * @brief Create a new, empty \c PTPContainer
 *
 * @see PTPContainer::PTPContainer(uint16_t type, uint16_t op_code)
 */
PTPContainer::PTPContainer() {
    // Not sure what I want to do here
    this->init();
}

/**
 * @brief Create a new \c PTPContainer with \a type and \a op_code
 *
 * @param[in] type A \c PTP_CONTAINER_TYPE for this \c PTPContainer
 * @param[in] op_code The operation for this \c PTPContainer
 */
PTPContainer::PTPContainer(uint16_t type, uint16_t op_code) {
    this->init();
    this->type = type;
    this->code = op_code;
}

/**
 * @brief Create a new \c PTPContainer of the message contained in \c data
 *
 * @param[in] data A received PTP message
 * @see PTPContainer::unpack
 */
PTPContainer::PTPContainer(unsigned char * data) {
    // This is essentially lv_framebuffer_desc .unpack() function, in the form of a constructor
    this->unpack(data);
}

/**
 * @brief Frees up memory malloc()ed by \c PTPContainer
 */
PTPContainer::~PTPContainer() {
    if(this->payload != NULL) {
        delete[] this->payload;    // Be sure to free up this memory
        this->payload = NULL;
    }
}

/**
 * @brief Initialize the variables in a \c PTPContainer
 */
void PTPContainer::init() {
    this->length = this->default_length; // Length is at least the sum of the header parts
    this->payload = NULL;
}

/**
 * @brief Add a parameter to a \c PTPContainer
 *
 * This function can add any \c uint32_t as a parameter, but is most useful for
 * adding a member of \c CHDK_OPERATIONS or a parameter to that operation.  But,
 * the function is generic enough that any type of data could be added, so this
 * can help create any generic PTP command.
 *
 * @param[in] param The parameter to be added
 */
void PTPContainer::add_param(uint32_t param) {
    // Allocate new memory for the payload
    uint32_t old_length = (this->length)-(this->default_length);
    uint32_t new_length = this->length + sizeof(uint32_t);
    unsigned char * new_payload = new unsigned char[new_length];
    
    // Copy old payload into new payload
	std::copy(this->payload, this->payload+old_length, new_payload);
    // Copy new data into new payload
	std::copy(&param, &param + sizeof(uint32_t), &(new_payload[old_length]));
    // Free up old payload memory
	delete[] this->payload;
    // Change payload pointer to new payload
    this->payload = new_payload;
    // Update length
    this->length = new_length;
}

/**
 * @brief Store a payload in a \c PTPContainer
 *
 * Useful for dumping large amounts of data into a \c PTPContainer for a 
 * data operation.  However, this could be used to set up a \c PTPContainer
 * for any operation.  Usually, \c PTPContainer::add_param is more useful
 * for adding individual parameters, though.
 *
 * @param[in] payload The data to dump into the \c PTPContainer
 * @param[in] payload_length The amount of data to read from \a payload
 */
void PTPContainer::set_payload(unsigned char * payload, int payload_length) {
    // Allocate new memory to copy the payload into
    // This way, we can ensure that we always want to free() the memory
    uint32_t new_length = this->default_length + payload_length;
	unsigned char * new_payload = new unsigned char[payload_length];
    
    // Copy the payload over
	std::copy(payload, payload + payload_length, new_payload);
    // Free up the old payload
	delete[] this->payload;
    // Change payload pointer to new payload
    this->payload = new_payload;
    // Update length
    this->length = new_length;
}

/**
 * @brief Pack \c PTPContainer data into a byte stream for sending
 *
 * Packs the data currently stored in the \c PTPContainer into a array of
 * unsigned characters which can be sent through USB, etc. to the device.
 * The length of this data can be found with \c PTPContainer::get_length
 *
 * @warning Since this method malloc()s space for the packed data, the caller
 *          must make sure to avoid memory leaks by free()ing this data.
 * @return A pointer to the first unsigned character which makes up the data
 *         in the container.
 * @see PTPContainer::get_length
 */
unsigned char * PTPContainer::pack() {
	unsigned char * packed = new unsigned char[this->length];
    
    uint32_t header_size = (sizeof this->length)+(sizeof this->type)+(sizeof this->code)+(sizeof this->transaction_id);
    
	std::copy(&(this->length), &(this->length) + sizeof this->length, &packed[0]);	// Copy length
	std::copy(&(this->type), &(this->type) + sizeof this->type, &packed[4]);		// Type
	std::copy(&(this->code), &(this->code) + sizeof this->code, &packed[6]);		// Two bytes of code
	std::copy(&(this->transaction_id), &(this->transaction_id) + sizeof this->transaction_id, &packed[8]);	// Four bytes of transaction ID
	std::copy(this->payload, this->payload + this->length - header_size, &packed[12]);	// The rest of payload
    
    return packed;
}

/**
 * @brief Retrieve the payload stored in this \c PTPContainer
 *
 * @param[out] size_out The size of the payload returned
 * @return A new copy of the payload contained in this \c PTPContainer
 */
unsigned char * PTPContainer::get_payload(int * size_out) {
    unsigned char * out;
    
    *size_out = this->length - this->default_length;
    
	out = new unsigned char[*size_out];
	std::copy(this->payload, this->payload + *size_out, out);
    
    return out;
}

/**
 * @brief Retrieve the size of all data stored in the payload
 *
 * @return The total length of data contained in this \c PTPContainer
 */
uint32_t PTPContainer::get_length() {
    return length;
}

/**
 * @brief Unpack data from a byte stream into a \c PTPContainer
 *
 * This function will overwrite any data currently stored in this
 * \c PTPContainer with the new data from \a data.  \a data is parsed
 * for each part of the PTP message, and individual items are stored
 * appropriately.
 *
 * @warning \a data must be at least 12 bytes in length, or this could
 *           segfault.
 *
 * @param[in] data The address of the first unsigned character of
 *                 the new container data.  Must be at least 12 bytes
 *                 in length.
 */
void PTPContainer::unpack(unsigned char * data) {
    // Free up our current payload
	delete[] this->payload;
    this->payload = NULL;
    
    // First four bytes are the length
	std::copy(data, data + 4, &this->length);
    // Next, container type
	std::copy(data + 4, data + 6, &this->type);
    // Copy over code
	std::copy(data + 6, data + 8, &this->code);
    // And transaction ID...
	std::copy(data + 8, data + 12, &this->transaction_id);
    
    // Finally, copy over the payload
	this->payload = new unsigned char[this->length - 12];
	std::copy(data + 12, data + this->length, this->payload);
    
    // Since we copied all of this data, the data passed in can be free()d
}

/**
 * @brief Convenience function to retrieve parameter #\a n from \c PTPContainer.
 *
 * @param[in] n Parameter number to extract.
 * @return Value stored in parameter \n.
 * @exception PTP::ERR_PTPCONTAINER_NO_PAYLOAD If this \c PTPContainer has no payload.
 * @exception PTP::ERR_PTPCONTAINER_INVALID_PARAM If this \c PTPContainer is too short to have a parameter \a n.
 */
uint32_t PTPContainer::get_param_n(uint32_t n) {
    uint32_t out;
    uint32_t first_byte;
    
    if(this->payload == NULL) {
        throw PTP::ERR_PTPCONTAINER_NO_PAYLOAD;
        return 0;
    }
    
    first_byte = 4*n;   // First byte of parameter n is 4*n bytes into container
    
    // 4*n = first bit of parameter n
    //  Add an extra four to ensure we have a parameter n
    // Subtract 12 bytes (header) from length
    if((this->length-12) < 4+4*n) {
        throw PTP::ERR_PTPCONTAINER_INVALID_PARAM;
        return 0;
    }
    
	std::copy(payload + first_byte, payload + first_byte + 4, &out); // Copy parameter into out
    
    return out; // Return parameter
}

} /* namespace PTP */
