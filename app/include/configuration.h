#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define CONFIG_ID 1

struct config {
    int random_value;
};

extern struct config cfg;

/**
 * @brief: Initialize the nvs to use it
 * @return: 0 on sucess
 */
int init_nvs(void);

/**
 * @brief: Save the current configuration to the nvs
 * @return: 0 on success
 */
int save_config(void);

/**
 * @brief: Reads the current config stored on the nvs
 * @return: 0 on success
 */
int load_config(void);

/**
 * @brief: Loads the defaults values if nothing is saved on the NVS
 */
void set_dflt_cfg(void);

#endif
