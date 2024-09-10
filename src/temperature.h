/*
 *  temperature.h: The header file for disk drive temperature sensing
 *
 *  This program is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free Software
 *  Foundation, version 2.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <sys/types.h>
#include "context.h"

/**
 * This function is called after each kwipe_context_t has been created.
 * It initialises the temperature variables in each context and then
 * constructs a path that is placed in the context that points to the
 * appropriate /sys/class/hwmon/hwmonX directory that corresponds with
 * the particular drive represented in the context structure.
 * @param pointer to a drive context
 * @return returns 0 on success < 1 on error
 */
int kwipe_init_temperature( kwipe_context_t* );

void kwipe_update_temperature( kwipe_context_t* );

/**
 * Workaround for obtaining temperatures from SCSI/SAS drives
 * @param pointer to a drive context
 * @return returns 0 on success < 1 on error
 */
int kwipe_init_scsi_temperature( kwipe_context_t* );
int kwipe_get_scsi_temperature( kwipe_context_t* );
void kwipe_shut_scsi_temperature( kwipe_context_t* );
void* kwipe_update_temperature_thread( void* ptr );

/**
 * This function is normally called only once. It's called after both the
 * kwipe_init_temperature() function and kwipe_update_temperature()
 * functions have been called. It logs the drives critical, highest, lowest
 * and lowest critical temperatures. Not all drives report four temperatures.
 * @param pointer to a drive context
 */
void kwipe_log_drives_temperature_limits( kwipe_context_t* );

#define NUMBER_OF_FILES 7

#define NO_TEMPERATURE_DATA 1000000

#endif /* TEMPERATURE_H_ */
