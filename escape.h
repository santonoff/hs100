#pragma once
#ifdef _WIN32
	char *escape_json(const char *orig);
#else
	char *escape_json(const char *orig) __attribute__((nonnull(1)));
#endif

