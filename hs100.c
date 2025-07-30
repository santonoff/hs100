#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "comms.h"

// handlers for more complicated commands
extern char *handler_associate(int argc, char *argv[]);
extern char *handler_set_server(int argc, char *argv[]);
extern char *handler_set_relay_state(int argc, char *argv[]);
extern char *handler_get_realtime(int argc, char *argv[]);
extern char *handler_set_alias(int argc, char *argv[]);
extern char *handler_set_kldim(int argc, char *argv[]);

// global pointer to hold final command executed from handler or direct execution
char *gcpCommandExecuted = NULL;

// define struct for commands
struct cmd_s {
	char *command;
	char *help;
	char *json;
	char *(*handler) (int argc, char *argv[]);
};

// global struct array with command, help and pointer to handler function
struct cmd_s cmds[] = {
	{
		.command = "associate",
		.help = "associate <ssid> <key> <key_type>\n"
			"\t\t\tset wifi AP to connect to",
		.handler = handler_associate,
	},
	{
		.command = "emeter",
		.help = "emeter\t\tget realtime power consumption (only works with HS110)",
		.handler = handler_get_realtime,
		.json = "{\"emeter\":{\"get_realtime\":{}}}",
	},
	{
		.command = "factory-reset",
		.help = "factory-reset\treset the plug to factory settings",
		.json = "{\"system\":{\"reset\":{\"delay\":0}}}",
	},
	{
		.command = "info",
		.help = "info\t\tget device info",
		.json = "{\"system\":{\"get_sysinfo\":{}}}",
	},
	{
		.command = "off",
		.help = "off\t\tturn the plug off",
		.handler = handler_set_relay_state,
		.json = "{\"system\":{\"set_relay_state\":{\"state\":0}}}",
	},
	{
		.command = "on",
		.help = "on\t\tturn the plug on",
		.handler = handler_set_relay_state,
		.json = "{\"system\":{\"set_relay_state\":{\"state\":1}}}",
	},
	{
		.command = "reboot",
		.help = "reboot\t\treboot the plug",
		.json = "{\"system\":{\"reboot\":{\"delay\":0}}}",
	},
	{
		.command = "scan",
		.help = "scan\t\tscan for nearby wifi APs (probably only 2.4 GHz ones)",
		.json = "{\"netif\":{\"get_scaninfo\":{\"refresh\":1}}}",
	},
	{
		.command = "set_server",
		.help = "set_server <url>\n"
			"\t\t\tset cloud server to <url> instead of tplink's",
		.handler = handler_set_server,
	},
	{
		.command = "kl110_on",
		.help = "kl110_on\tTurn on KL110 bulb",
		.json = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":1}}}",
	},
	{
		.command = "kl110_off",
		.help = "kl110_off\tTurn off KL110 bulb",
		.json = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":0}}}",
	},
	{
		.command = "kl110_dim100",
		.help = "kl110_dim100\tSet KL110 bulb to full intensity",
		.json = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":100}}}",
	},
	{
		.command = "kl110_dim",
		.help = "kl110_dim #\tSet KL110 bulb to # intensity",
		.json = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":%s}}}",
		.handler = handler_set_kldim,
	},
	{
		.command = "kl110_dim0",
		.help = "kl110_dim0\tSet KL110 bulb to minimum intensity",
		.json = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":0}}}",
	},
  	{
		.command = "alias",
		.help = "alias <name>\tset device alias to <name>",
		.handler = handler_set_alias,
	},
	{
		.command = NULL,
	},
};

struct cmd_s *get_cmd_from_name(char *needle)
{
	int cmds_index = 0;
	while (cmds[cmds_index].command) {
		if (!strcmp(cmds[cmds_index].command, needle))
			return &cmds[cmds_index];
		cmds_index++;
	}
	return NULL;
}

void print_usage(char *cmdname)
{
	fprintf(stderr, "%s version %s Copyright (C) 2018-2019 Jason Benaim.\n"
			"\tKL110 commands added 2025, Steve Antonoff\n"
			"A tool for using certain wifi smart plugs.\n\n"
			"usage: %s <ip> <command>\n\n"
			"Commands:\n", cmdname,VERSION_STRING,cmdname
	);
	int cmds_index = 0;
	while (cmds[cmds_index].command) {
		fprintf(stderr, "\t%s\n", cmds[cmds_index].help);
		cmds_index++;
	}
	fprintf(stderr, "\nAny json command can be substituted for the pre-defined commands\n");
	fprintf(stderr, "\tjson command must be enclosed in quotes with embedded quotes and \n\tproperly escaped (i.e., \\\"on_off\\\")\n");
	fprintf(stderr, "\nReport bugs to https://github.com/jkbenaim/hs100\n");
}

int main(int argc, char *argv[])
{
	char *plug_addr = argv[1];
	char *cmd_string = argv[2];
	char *response = NULL;
	char *mode = argv[0];
	char cmd_override[16];
	struct cmd_s *cmd;

	if (argc < 3) {
		print_usage(argv[0]);
		return 1;
	}
	
	gcpCommandExecuted = calloc(1,200);	// global pointer to command that will be executed
	
// handle if executed as kl110 rather than hs100 - rename or symlink
	if (strstr(mode,"kl110") ) {
		if (!strcmp(cmd_string,"on") ) {
			strcpy(cmd_override,"kl110_on");
			cmd_string = cmd_override;
		}
		if (!strcmp(cmd_string,"off") ) {
			strcpy(cmd_override,"kl110_off");
			cmd_string = cmd_override;
		}
		if (!strcmp(cmd_string,"dim") ) {
			strcpy(cmd_override,"kl110_dim");
			cmd_string = cmd_override;
		}
	}
	cmd = get_cmd_from_name(cmd_string);
	if (cmd != NULL) {
		if (cmd->handler != NULL) {
			response = cmd->handler(argc, argv);
		}
		if ((response == NULL) && (cmd->json != NULL)) {
			strcpy(gcpCommandExecuted,cmd->json);
			response = hs100_send(plug_addr, cmd->json);
		}
	} else {
		// command not recognized, so send it to the plug raw
		strcpy(gcpCommandExecuted,cmd_string);
		response = hs100_send(plug_addr, cmd_string);
	}

	if (response == NULL) {
		fprintf(stderr, "failed to send command\n");
		return 1;
	}

	printf("Input arguments: ");
	{
		int iArg;
		for (iArg = 1 ; iArg < argc ; ++iArg)
			printf("%s ",argv[iArg]);
		printf("\n");
	}
	if (gcpCommandExecuted) {
		printf("Command: %s\n", gcpCommandExecuted);
	} else {
		printf("Basic command: %s\n" , cmd->json);
	}
	printf("Result:  %s\n", response);

	free(response);
	free(gcpCommandExecuted);
	return 0;
}
