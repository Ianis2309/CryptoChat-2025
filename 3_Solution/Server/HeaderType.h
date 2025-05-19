#pragma once

enum class Header {
	BUFFER,
	__OK__,
	LOGIN,
	SIGNUP,
	TEXT,
	ERROR__,
	ADDCONT,
	AGENDA,
	TALKS,
	DELCONT,
	LOGOUT,
	PADDING
};

//BINARY,

#define ON		true
#define OFF		false
#define DISPEN  0xFF
#define _SIZE_	30000
