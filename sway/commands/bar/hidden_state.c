#define _XOPEN_SOURCE 500
#include <string.h>
#include <strings.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/ipc-server.h"
#include "log.h"

static struct cmd_results *bar_set_hidden_state(struct bar_config *bar,
		const char *hidden_state) {
	char *old_state = bar->hidden_state;
	if (strcasecmp("toggle", hidden_state) == 0 && !config->reading) {
		if (strcasecmp("hide", bar->hidden_state) == 0) {
			bar->hidden_state = strdup("show");
		} else if (strcasecmp("show", bar->hidden_state) == 0) {
			bar->hidden_state = strdup("hide");
		}
	} else if (strcasecmp("hide", hidden_state) == 0) {
		bar->hidden_state = strdup("hide");
	} else if (strcasecmp("show", hidden_state) == 0) {
		bar->hidden_state = strdup("show");
	} else {
		return cmd_results_new(CMD_INVALID, "hidden_state",
				"Invalid value %s", hidden_state);
	}
	if (strcmp(old_state, bar->hidden_state) != 0) {
		if (!config->reading) {
			ipc_event_barconfig_update(bar);
		}
		wlr_log(WLR_DEBUG, "Setting hidden_state: '%s' for bar: %s",
				bar->hidden_state, bar->id);
	}
	// free old mode
	free(old_state);
	return NULL;
}

struct cmd_results *bar_cmd_hidden_state(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "hidden_state", EXPECTED_AT_LEAST, 1))) {
		return error;
	}
	if ((error = checkarg(argc, "hidden_state", EXPECTED_AT_MOST, 2))) {
		return error;
	}
	if (config->reading && argc > 1) {
		return cmd_results_new(CMD_INVALID, "hidden_state",
				"Unexpected value %s in config mode", argv[1]);
	}

	const char *state = argv[0];
	if (config->reading) {
		error = bar_set_hidden_state(config->current_bar, state);
	} else {
		const char *id = argc == 2 ? argv[1] : NULL;
		for (int i = 0; i < config->bars->length; ++i) {
			struct bar_config *bar = config->bars->items[i];
			if (id) {
				if (strcmp(id, bar->id) == 0) {
					error = bar_set_hidden_state(bar, state);
					break;
				}
			} else if ((error = bar_set_hidden_state(bar, state))) {
				break;
			}
		}
	}
	return error ? error : cmd_results_new(CMD_SUCCESS, NULL, NULL);
}
