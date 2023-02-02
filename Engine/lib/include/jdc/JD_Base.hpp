#pragma once
#include <xel/Common.hpp>
#include <cinttypes>

#if defined(X_GAME_OPTION_STATIC)
	#if defined(X_GAME_OPTION_EXPORT_API)
		#error X_GAME_OPTION_STATIC is used with X_GAME_OPTION_EXPORT_API
	#endif
	#define X_GAME_API                      X_EXTERN
	#define X_GAME_API_MEMBER               X_MEMBER
	#define X_GAME_API_STATIC_MEMBER        X_STATIC_MEMBER
#else
	#if defined(X_GAME_OPTION_EXPORT_API)
		#define X_GAME_API                  X_EXPORT
		#define X_GAME_API_MEMBER           X_EXPORT_MEMBER
		#define X_GAME_API_STATIC_MEMBER    X_EXPORT_STATIC_MEMBER
	#else
		#define X_GAME_API                  X_IMPORT
		#define X_GAME_API_MEMBER           X_IMPORT_MEMBER
		#define X_GAME_API_STATIC_MEMBER    X_IMPORT_STATIC_MEMBER
	#endif
#endif
