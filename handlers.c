#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comms.h"
#include "escape.h"

extern char *gcpCommandExecuted; // captute command executed for final output

char *handler_associate(int argc, char *argv[])
{
	const char *template =
		"{\"netif\":{\"set_stainfo\":{\"ssid\":\"%s\",\"password\":"
		"\"%s\",\"key_type\":%d}}}";
	char *plug_addr = argv[1];
	char *ssid = argv[3];
	char *password = argv[4];
	char *key_type = argv[5];
	char *endptr, *msg, *response;
	int key_type_num;
	size_t len;

	if (password)
		password = escape_json(password);

	if (argc < 6) {
		fprintf(stderr, "associate: not enough arguments\n");
		free(password);
		return NULL; // exit(1)
	}
	errno = 0;
	key_type_num = (int)strtol(key_type, &endptr, 10);
	if (errno || endptr == key_type) {
		fprintf(stderr, "invalid key type: %s\n", key_type);
		free(password);
		return NULL; // exit(1)
	}

	len = snprintf(NULL, 0, template, ssid, password,
			key_type_num);
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, ssid, password, key_type_num);
	strcpy(gcpCommandExecuted, msg);
	response = hs100_send(plug_addr, msg);
	free(password);
	return response;
}

char *handler_set_server(int argc, char *argv[])
{
	const char *template =
		"{\"cnCloud\":{\"set_server_url\":{\"server\":\"%s\"}}}";
	char *plug_addr = argv[1];
	char *server = argv[3];
	size_t len;
	char *msg, *response;

	if (argc < 4) {
		fprintf(stderr, "set_server: not enough arguments\n");
		return NULL; // exit(1)
	}
	len = snprintf(NULL, 0, template, server);
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, server);
	strcpy(gcpCommandExecuted, msg);

	response = hs100_send(plug_addr, msg);

	return response;
}

char *handler_set_alias(int argc, char *argv[])
{
	const char *template =
		"{\"system\":{\"set_dev_alias\":{\"alias\":\"%s\"}}}";
	char *plug_addr = argv[1];
	char *name = argv[3];
	size_t len;
	char *msg, *response;

	if (argc < 4) {
		fprintf(stderr, "set_alias: not enough arguments\n");
		return NULL; // exit(1)
	}
	len = snprintf(NULL, 0, template, name);
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, name);
	strcpy(gcpCommandExecuted, msg);

	response = hs100_send(plug_addr, msg);

	return response;
}

char *handler_set_relay_state(int argc, char *argv[])
{
	const char *template =
		"{\"context\":{\"child_ids\":[\"%s\"]},"
		"\"system\":{\"set_relay_state\":{\"state\":%c}}}";
	char *plug_addr = argv[1];
	char *onoff = argv[2];
	char *plug = argv[3];
	size_t len;
	char *msg, *response;

	if (argc < 4) {
		fprintf(stderr, "set_relay_state (on or off): not enough arguments");
		return NULL;
	}

	len = snprintf(NULL, 0, template, plug, (onoff[1] == 'n' ? '1' : '0'));
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, plug, (onoff[1] == 'n' ? '1' : '0'));
	strcpy(gcpCommandExecuted, msg);

	response = hs100_send(plug_addr, msg);

	return response;
}

char *handler_get_realtime(int argc, char *argv[])
{
	const char *template =
		"{\"context\":{\"child_ids\":[\"%s\"]},"
		"\"emeter\":{\"get_realtime\":{}}}";
	char *plug_addr = argv[1];
	char *plug = argv[3];
	size_t len;
	char *msg, *response;

	if (argc < 4) {
		fprintf(stderr, "get_realtime: not enough arguments");
		return NULL;
	}

	len = snprintf(NULL, 0, template, plug);
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, plug);
	strcpy(gcpCommandExecuted, msg);

	response = hs100_send(plug_addr, msg);

	return response;
}

char *handler_set_kldim(int argc, char *argv[])
{
	const char *template =
//		"{\"system\":{\"set_dev_alias\":{\"alias\":\"%s\"}}}";
		"{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":%s}}}";
	char *plug_addr = argv[1];
	char *dim = argv[3];
	size_t len;
	char *msg, *response;

	if (argc < 4) {
		fprintf(stderr, "set klmdim: not enough arguments\n");
		return NULL; // exit(1)
	}
	len = snprintf(NULL, 0, template, dim);
	len++;	/* snprintf does not count the null terminator */

	msg = calloc(1, len);
	snprintf(msg, len, template, dim);
	strcpy(gcpCommandExecuted, msg);

	response = hs100_send(plug_addr, msg);

	return response;
}

