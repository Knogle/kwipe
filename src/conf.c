/*
 *  conf.c: functions that handle the kwipe.conf configuration file
 *  and the creation of the kwipe_customers.csv file. kwipe.conf uses
 *  libconfig format, while kwipe_customers.csv uses comma separted
 *  values. CSV is used so that the user can build there own customer
 *  listing using spreadsheets rather than enter all the customer
 *  information via the kwipe GUI interface.
 *
 *  Copyright PartialVolume <https://github.com/PartialVolume>.
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
 *
 *
 */

#include <libconfig.h>
#include <unistd.h>
#include <sys/types.h>
#include "kwipe.h"
#include "context.h"
#include "logging.h"
#include "method.h"
#include "options.h"
#include "conf.h"

config_t kwipe_cfg;
config_setting_t *kwipe_conf_setting, *group_organisation, *root, *group, *previous_group, *setting;
const char* kwipe_conf_str;
char kwipe_config_directory[] = "/etc/kwipe";
char kwipe_config_file[] = "/etc/kwipe/kwipe.conf";
char kwipe_customers_file[] = "/etc/kwipe/kwipe_customers.csv";
char kwipe_customers_file_backup[] = "/etc/kwipe/kwipe_customers.csv.backup";
char kwipe_customers_file_backup_tmp[] = "/etc/kwipe/kwipe_customers.csv.backup.tmp";

/*
 * Checks for the existence of kwipe.conf and kwipe_customers.csv
 * If either one does not exist, they are created and formatted
 * with a basic configuration.
 */

int kwipe_conf_init()
{
    FILE* fp_config;
    FILE* fp_customers;

    config_init( &kwipe_cfg );
    char kwipe_customers_initial_content[] =
        "\"Customer Name\";\"Contact Name\";\"Customer Address\";\"Contact Phone\"\n"
        "\"Not Applicable\";\"Not Applicable\";\"Not Applicable\";\"Not Applicable\"\n";

    /* Read /etc/kwipe/kwipe.conf. If there is an error, determine whether
     * it's because it doesn't exist. If it doesn't exist create it and
     * populate it with a basic structure.
     */

    /* Does the /etc/kwipe/kwipe.conf file exist? If not, then create it */
    if( access( kwipe_config_file, F_OK ) == 0 )
    {
        kwipe_log( NWIPE_LOG_INFO, "Nwipes config file %s exists", kwipe_config_file );

        /* Read the kwipe.conf configuration file and report any errors */

        kwipe_log( NWIPE_LOG_INFO, "Reading kwipe's config file %s", kwipe_config_file );
        if( !config_read_file( &kwipe_cfg, kwipe_config_file ) )
        {
            kwipe_log( NWIPE_LOG_ERROR,
                       "Syntax error: %s:%d - %s\n",
                       config_error_file( &kwipe_cfg ),
                       config_error_line( &kwipe_cfg ),
                       config_error_text( &kwipe_cfg ) );
        }
    }
    else
    {
        kwipe_log( NWIPE_LOG_WARNING, "%s does not exist", kwipe_config_file );

        /* We assume the /etc/kwipe directory doesn't exist, so try to create it */
        mkdir( kwipe_config_directory, 0755 );

        /* create the kwipe.conf file */
        if( !( fp_config = fopen( kwipe_config_file, "w" ) ) )
        {
            kwipe_log( NWIPE_LOG_ERROR, "Failed to create %s", kwipe_config_file );
        }
        else
        {
            kwipe_log( NWIPE_LOG_INFO, "Created %s", kwipe_config_file );
            fclose( fp_config );
        }
    }

    root = config_root_setting( &kwipe_cfg );

    /**
     * If they don't already exist, populate kwipe.conf with groups, settings and values.
     * This will also fill in missing group or settings if they have been corrupted or
     * accidently deleted by the user. It will also update an existing kwipe.conf
     * file as new groups and settings are added to kwipe. If new settings are added
     * to kwipes conf file they MUST appear below in this list of groups and settings.
     */

    kwipe_conf_populate( "Organisation_Details.Business_Name", "Not Applicable (BN)" );
    kwipe_conf_populate( "Organisation_Details.Business_Address", "Not Applicable (BA)" );
    kwipe_conf_populate( "Organisation_Details.Contact_Name", "Not Applicable (BCN)" );
    kwipe_conf_populate( "Organisation_Details.Contact_Phone", "Not Applicable (BCP)" );
    kwipe_conf_populate( "Organisation_Details.Op_Tech_Name", "Not Applicable (OTN)" );

    /**
     * Add PDF Certificate/Report settings
     */
    kwipe_conf_populate( "PDF_Certificate.PDF_Enable", "ENABLED" );
    kwipe_conf_populate( "PDF_Certificate.PDF_Preview", "DISABLED" );

    /**
     * The currently selected customer that will be printed on the report
     */
    kwipe_conf_populate( "Selected_Customer.Customer_Name", "Not Applicable (CN)" );
    kwipe_conf_populate( "Selected_Customer.Customer_Address", "Not Applicable (CA)" );
    kwipe_conf_populate( "Selected_Customer.Contact_Name", "Not Applicable (CCN)" );
    kwipe_conf_populate( "Selected_Customer.Contact_Phone", "Not Applicable (CP)" );

    /**
     * Write out the new configuration.
     */
    if( !config_write_file( &kwipe_cfg, kwipe_config_file ) )
    {
        kwipe_log( NWIPE_LOG_ERROR, "Failed to write kwipe config to %s", kwipe_config_file );
    }
    else
    {
        kwipe_log( NWIPE_LOG_INFO, "Sucessfully written kwipe config to %s", kwipe_config_file );
    }

    /* Read the kwipe.conf configuration file and report any errors */
    if( !config_read_file( &kwipe_cfg, kwipe_config_file ) )
    {
        kwipe_log( NWIPE_LOG_ERROR,
                   "Syntax error: %s:%d - %s\n",
                   config_error_file( &kwipe_cfg ),
                   config_error_line( &kwipe_cfg ),
                   config_error_text( &kwipe_cfg ) );
    }

    /* -----------------------------------------------------------------------------
     * Now check kwipe_customers.csv exists, if not create it with a basic structure
     */
    if( access( kwipe_customers_file, F_OK ) == 0 )
    {
        kwipe_log( NWIPE_LOG_INFO, "Nwipes customer file %s exists", kwipe_customers_file );
    }
    else
    {
        kwipe_log( NWIPE_LOG_WARNING, "%s does not exist", kwipe_customers_file );

        /* We assume the /etc/kwipe directory doesn't exist, so try to create it */
        mkdir( kwipe_config_directory, 0755 );

        /* create the kwipe_customers.csv file */
        if( !( fp_customers = fopen( kwipe_customers_file, "w" ) ) )
        {
            kwipe_log( NWIPE_LOG_ERROR, "Failed to create %s", kwipe_customers_file );
        }
        else
        {
            kwipe_log( NWIPE_LOG_INFO, "Created %s", kwipe_customers_file );

            /* Now populate the csv header and first entry, Lines 1 and 2 */
            if( fwrite( kwipe_customers_initial_content, sizeof( kwipe_customers_initial_content ), 1, fp_customers )
                == 1 )
            {
                kwipe_log( NWIPE_LOG_INFO, "Populated %s with basic config", kwipe_customers_file );
            }
            else
            {
                kwipe_log( NWIPE_LOG_ERROR, "Failed to write basic config to %s", kwipe_customers_file );
            }
            fclose( fp_customers );
        }
    }
    return ( 0 );
}

void save_selected_customer( char** customer )
{
    /* This function saves the user selected customer
     * to kwipe's config file /etc/kwipe/kwipe.conf
     * for later use by the PDF creation functions.
     */

    int idx;
    int field_count;
    int field_idx;

    char field_1[FIELD_LENGTH];
    char field_2[FIELD_LENGTH];
    char field_3[FIELD_LENGTH];
    char field_4[FIELD_LENGTH];

    /* zero the field strings */
    for( idx = 0; idx < FIELD_LENGTH; idx++ )
        field_1[idx] = 0;
    for( idx = 0; idx < FIELD_LENGTH; idx++ )
        field_2[idx] = 0;
    for( idx = 0; idx < FIELD_LENGTH; idx++ )
        field_3[idx] = 0;
    for( idx = 0; idx < FIELD_LENGTH; idx++ )
        field_4[idx] = 0;

    /* Extract the field contents from the csv string
     */
    idx = 0;
    field_idx = 0;
    field_count = 1;

    while( *( *customer + idx ) != 0 && field_count < NUMBER_OF_FIELDS + 1 )
    {
        /* Start of a field? */
        if( *( *customer + idx ) == '\"' )
        {
            idx++;

            while( *( *customer + idx ) != '\"' && *( *customer + idx ) != 0 )
            {
                if( field_count == 1 && field_idx < ( FIELD_LENGTH - 1 ) )
                {
                    field_1[field_idx++] = *( *customer + idx );
                }
                else
                {
                    if( field_count == 2 && field_idx < ( FIELD_LENGTH - 1 ) )
                    {
                        field_2[field_idx++] = *( *customer + idx );
                    }
                    else
                    {
                        if( field_count == 3 && field_idx < ( FIELD_LENGTH - 1 ) )
                        {
                            field_3[field_idx++] = *( *customer + idx );
                        }
                        else
                        {
                            if( field_count == 4 && field_idx < ( FIELD_LENGTH - 1 ) )
                            {
                                field_4[field_idx++] = *( *customer + idx );
                            }
                        }
                    }
                }
                idx++;
            }
            if( *( *customer + idx ) == '\"' )
            {
                /* Makesure the field string is terminated */
                switch( field_count )
                {
                    case 1:
                        field_1[field_idx] = 0;
                        break;
                    case 2:
                        field_2[field_idx] = 0;
                        break;
                    case 3:
                        field_3[field_idx] = 0;
                        break;
                    case 4:
                        field_4[field_idx] = 0;
                        break;
                }

                field_count++;
                field_idx = 0;
            }
        }
        idx++;
    }

    /* All 4 fields present? */
    if( field_count != NUMBER_OF_FIELDS + 1 )
    {
        kwipe_log( NWIPE_LOG_ERROR,
                   "Insuffient fields in customer entry, expected %i, actual(field_count) %i, idx=%i",
                   NUMBER_OF_FIELDS,
                   field_count,
                   idx );
        return;
    }

    /* -------------------------------------------------------------
     * Write the fields to kwipe's config file /etc/kwipe/kwipe.conf
     */
    if( ( setting = config_lookup( &kwipe_cfg, "Selected_Customer.Customer_Name" ) ) )
    {
        config_setting_set_string( setting, field_1 );
    }
    else
    {
        kwipe_log( NWIPE_LOG_ERROR, "Can't find \"Selected Customers.Customer_Name\" in %s", kwipe_config_file );
    }

    if( ( setting = config_lookup( &kwipe_cfg, "Selected_Customer.Customer_Address" ) ) )
    {
        config_setting_set_string( setting, field_2 );
    }
    else
    {
        kwipe_log( NWIPE_LOG_ERROR, "Can't find \"Selected Customers.Customer_Address\" in %s", kwipe_config_file );
    }

    if( ( setting = config_lookup( &kwipe_cfg, "Selected_Customer.Contact_Name" ) ) )
    {
        config_setting_set_string( setting, field_3 );
    }
    else
    {
        kwipe_log( NWIPE_LOG_ERROR, "Can't find \"Selected Customers.Contact_Name\" in %s", kwipe_config_file );
    }

    if( ( setting = config_lookup( &kwipe_cfg, "Selected_Customer.Contact_Phone" ) ) )
    {
        config_setting_set_string( setting, field_4 );
    }
    else
    {
        kwipe_log( NWIPE_LOG_ERROR, "Can't find \"Selected Customers.Contact_Phone\" in %s", kwipe_config_file );
    }

    /* Write out the new configuration. */
    if( !config_write_file( &kwipe_cfg, kwipe_config_file ) )
    {
        kwipe_log( NWIPE_LOG_ERROR, "Failed to write user selected customer to %s", kwipe_config_file );
    }
    else
    {
        kwipe_log( NWIPE_LOG_INFO, "Populated %s with user selected customer", kwipe_config_file );
    }
}

int kwipe_conf_update_setting( char* group_name_setting_name, char* setting_value )
{
    /* You would call this function of you wanted to update an existing setting in kwipe.conf, i.e
     *
     * kwipe_conf_update_setting( "PDF_Certificate.PDF_Enable", "ENABLED" )
     *
     * It is NOT used for creating a new group or setting name.
     */

    /* -------------------------------------------------------------
     * Write the field to kwipe's config file /etc/kwipe/kwipe.conf
     */
    if( ( setting = config_lookup( &kwipe_cfg, group_name_setting_name ) ) )
    {
        config_setting_set_string( setting, setting_value );
    }
    else
    {
        kwipe_log(
            NWIPE_LOG_ERROR, "Can't find group.setting_name %s in %s", group_name_setting_name, kwipe_config_file );
        return 1;
    }

    /* Write the new configuration to kwipe.conf
     */
    if( !config_write_file( &kwipe_cfg, kwipe_config_file ) )
    {
        kwipe_log( NWIPE_LOG_ERROR, "Failed to write %s to %s", group_name_setting_name, kwipe_config_file );
        return 2;
    }
    else
    {
        kwipe_log( NWIPE_LOG_INFO,
                   "Updated %s with value %s in %s",
                   group_name_setting_name,
                   setting_value,
                   kwipe_config_file );
    }

    return 0;

}  // end kwipe_conf_update_setting()

int kwipe_conf_read_setting( char* group_name_setting_name, const char** setting_value )
{
    /* This function returns a setting value from kwipe's configuration file kwipe.conf
     * when provided with a groupname.settingname string.
     *
     * Example:
     * const char ** pReturnString;
     * kwipe_conf_read_setting( "PDF_Certificate", "PDF_Enable", pReturnString );
     */

    /* Separate group_name_setting_name i.e "PDF_Certificate.PDF_Enable" string
     * into two separate strings by replacing the period with a NULL.
     */

    int return_status;
    int length = strlen( group_name_setting_name );

    char* group_name = calloc( length, sizeof( char ) );
    char* setting_name = calloc( length, sizeof( char ) );

    int idx = 0;

    while( group_name_setting_name[idx] != 0 && group_name_setting_name[idx] != '.' )
    {
        idx++;
    }
    // Copy the group name from the combined input string
    memcpy( group_name, group_name_setting_name, idx );
    group_name[idx] = '\0';  // Null-terminate group_name

    // Copy the setting name from the combined input string
    strcpy( setting_name, &group_name_setting_name[idx + 1] );

    if( !( setting = config_lookup( &kwipe_cfg, group_name ) ) )
    {
        kwipe_log( NWIPE_LOG_ERROR, "Can't find group name %s.%s in %s", group_name, setting_name, kwipe_config_file );
        return_status = -1;
    }
    else
    {
        /* Retrieve data from kwipe.conf */
        if( CONFIG_TRUE == config_setting_lookup_string( setting, setting_name, setting_value ) )
        {
            return_status = 0; /* Success */
        }
        else
        {
            kwipe_log(
                NWIPE_LOG_ERROR, "Can't find setting_name %s.%s in %s", group_name, setting_name, kwipe_config_file );
            return_status = -2;
        }
    }

    free( group_name );
    free( setting_name );
    return ( return_status );

}  // end kwipe_conf_read_setting()

int kwipe_conf_populate( char* path, char* value )
{
    /* This function will check that a path containing a group or multiple groups that lead to a setting all exist,
     * if they don't exist, the group/s, settings and associated value are created.
     *
     * The path, a string consisting of one or more groups separted by a period symbol
     * '.' with the final element in the path being the setting name. For instance the path might be
     * PDF_Certificate.PDF_Enable. Where PDF_Certificate is the group name and PDF_Enable is the setting name.
     * If one or other don't exist then they will be created.
     *
     * An arbitary depth of four groups are allowed for kwipe's configuration file, although we only currently, as of
     * October 2023 use a depth of one group. The number of groups can be increased in the future if required by
     * changing the definition MAX_GROUP_DEPTH in conf.h
     */

    char* path_copy;
    char* path_build;

    char* group_start[MAX_GROUP_DEPTH + 2];  // A pointer to the start of each group string
    char* setting_start;

    int idx;  // General index
    int group_count;  // Counts the number of groups in the path

    /* Initialise the pointers */
    memset( group_start, 0, MAX_GROUP_DEPTH + 2 );
    memset( &setting_start, 0, 1 );

    /* Allocate enough memory for a copy of the path and initialise to zero */
    path_copy = calloc( strlen( path ) + 1, sizeof( char ) );
    path_build = calloc( strlen( path ) + 1, sizeof( char ) );

    /* Duplicate the path */
    strcpy( path_copy, path );

    /* Create individual strings by replacing the period '.' with NULL, counting the number of groups. */
    idx = 0;
    group_count = 0;

    /* pointer to first group */
    group_start[group_count] = path_copy;

    while( *( path_copy + idx ) != 0 )
    {
        if( group_count > MAX_GROUP_DEPTH )
        {
            kwipe_log( NWIPE_LOG_ERROR,
                       "Too many groups in path, specified = %i, allowed = %i ",
                       group_count,
                       MAX_GROUP_DEPTH );
            return 1;
        }

        if( *( path_copy + idx ) == '.' )
        {
            *( path_copy + idx ) = 0;
            group_count++;
            group_start[group_count] = path_copy + idx + 1;
        }
        idx++;
    }

    /* setting_start points to a string that represents the setting to be created */
    setting_start = group_start[group_count];

    /* Remove the last entry from group_start as that would be the setting and not a group */
    group_start[group_count] = 0;

    if( group_count == 0 )
    {
        kwipe_log( NWIPE_LOG_ERROR, "No groups specified in path, i.e. no period . separator found." );
        return 2;
    }

    /**
     * Now determine whether the group/s in the path exist, if not create them.
     */

    idx = 0;
    while( group_start[idx] != 0 )
    {
        strcat( path_build, group_start[idx] );

        if( !( group = config_lookup( &kwipe_cfg, path_build ) ) )
        {
            if( idx == 0 )
            {
                group = config_setting_add( root, path_build, CONFIG_TYPE_GROUP );
                previous_group = group;
            }
            else
            {
                group = config_setting_add( previous_group, group_start[idx], CONFIG_TYPE_GROUP );
                previous_group = group;
            }
            if( group )
            {
                kwipe_log( NWIPE_LOG_INFO, "Created group [%s] in %s", path_build, kwipe_config_file );
            }
            else
            {
                kwipe_log( NWIPE_LOG_ERROR, "Failed to create group [%s] in %s", path_build, kwipe_config_file );
            }
        }
        else
        {
            previous_group = group;
        }

        idx++;

        if( group_start[idx] != 0 )
        {
            strcat( path_build, "." );
        }
    }

    /**
     * And finally determine whether the setting already exists, if not create it and assign the value to it
     */

    /* Does the full path exist ? i.e AAA.BBB */
    if( ( group = config_lookup( &kwipe_cfg, path_build ) ) )
    {
        /* Does the path and setting exist? AAA.BBB.SETTING_NAME */
        if( !( setting = config_lookup( &kwipe_cfg, path ) ) )
        {
            /* Add the new SETTING_NAME */
            if( ( setting = config_setting_add( group, setting_start, CONFIG_TYPE_STRING ) ) )
            {
                kwipe_log( NWIPE_LOG_INFO, "Created setting name %s in %s", path, kwipe_config_file );
            }
            else
            {
                kwipe_log(
                    NWIPE_LOG_ERROR, "Failed to create setting name %s in %s", setting_start, kwipe_config_file );
            }

            if( config_setting_set_string( setting, value ) == CONFIG_TRUE )
            {
                kwipe_log( NWIPE_LOG_INFO, "Set value for %s in %s to %s", path, kwipe_config_file, value );
            }
            else
            {
                kwipe_log( NWIPE_LOG_ERROR, "Failed to set value for %s in %s to %s", path, kwipe_config_file, value );
            }
        }
        else
        {
            if( kwipe_options.verbose )
            {
                kwipe_log( NWIPE_LOG_INFO, "Setting already exists [%s] in %s", path, kwipe_config_file );
            }
        }
    }
    else
    {
        kwipe_log( NWIPE_LOG_INFO, "Couldn't find constructed path [%s] in %s", path_build, kwipe_config_file );
    }

    free( path_copy );
    free( path_build );

    return 0;
}

void kwipe_conf_close()
{
    config_destroy( &kwipe_cfg );
}
